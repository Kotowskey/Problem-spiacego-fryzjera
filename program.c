#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_CHAIRS 10 // Maksymalna liczba miejsc w poczekalni

// Inicjalizacja mutexów i zmiennych warunkowych
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_barber = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_customer = PTHREAD_COND_INITIALIZER;

int waiting_room[MAX_CHAIRS]; // Tablica reprezentująca poczekalnię
int num_chairs; // Liczba miejsc w poczekalni
int num_waiting = 0; // Aktualna liczba klientów w poczekalni
int current_customer = -1; // Numer aktualnie obsługiwanego klienta
int rejected_customers = 0; // Liczba klientów, którzy zrezygnowali z powodu braku miejsc

void* customer(void* num) {
    int id = *(int*)num; // Pobranie identyfikatora klienta

    usleep(rand() % 10000); // Losowy czas przyjścia klienta

    pthread_mutex_lock(&mutex);
    if (num_waiting < num_chairs) { // Sprawdzenie czy są wolne miejsca w poczekalni
        waiting_room[num_waiting] = id;
        num_waiting++;
        printf("Klient %d siada w poczekalni. Poczekalnia: %d/%d [Fotel: %d]\n", id, num_waiting, num_chairs, current_customer);

        pthread_cond_signal(&cond_barber); // Obudzenie fryzjera, jeśli śpi
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while (current_customer != id) {
            pthread_cond_wait(&cond_customer, &mutex); // Czekanie, aż fryzjer będzie gotowy
        }
        pthread_mutex_unlock(&mutex);

        printf("Klient %d jest strzyżony.\n", id);
        usleep(5000); // Strzyżenie trwa stały czas
        printf("Klient %d kończy strzyżenie.\n", id);

        pthread_mutex_lock(&mutex);
        current_customer = -1;
        pthread_cond_signal(&cond_barber); // Poinformowanie fryzjera, że skończono strzyżenie
        pthread_mutex_unlock(&mutex);
    } else { // Brak wolnych miejsc, klient rezygnuje
        rejected_customers++;
        printf("Klient %d rezygnuje. Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n", id, rejected_customers, num_waiting, num_chairs, current_customer);
        pthread_mutex_unlock(&mutex);
    }

    free(num);
    pthread_exit(NULL);
}

void* barber(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (num_waiting == 0) { // Czekanie na klientów
            printf("Fryzjer śpi.\n");
            pthread_cond_wait(&cond_barber, &mutex); // Czekanie, aż pojawi się klient
        }

        current_customer = waiting_room[0]; // Pobranie pierwszego klienta z poczekalni
        num_waiting--;
        for (int i = 0; i < num_waiting; i++) {
            waiting_room[i] = waiting_room[i + 1]; // Przesunięcie kolejki w poczekalni
        }
        printf("Fryzjer budzi się i zaczyna strzyżenie klienta %d. Poczekalnia: %d/%d [Fotel: %d]\n", current_customer, num_waiting, num_chairs, current_customer);
        pthread_cond_signal(&cond_customer); // Poinformowanie klienta, że może być strzyżony
        pthread_mutex_unlock(&mutex);

        usleep(5000); // Strzyżenie trwa stały czas

        pthread_mutex_lock(&mutex);
        current_customer = -1;
        pthread_cond_signal(&cond_barber); // Poinformowanie, że fryzjer skończył strzyżenie
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_chairs>\n", argv[0]);
        return 1;
    }

    num_chairs = atoi(argv[1]);
    if (num_chairs > MAX_CHAIRS) {
        fprintf(stderr, "Max number of chairs is %d.\n", MAX_CHAIRS);
        return 1;
    }

    pthread_t barber_thread;
    pthread_create(&barber_thread, NULL, barber, NULL); // Tworzenie wątku fryzjera

    for (int i = 0; i < 20; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_t customer_thread;
        pthread_create(&customer_thread, NULL, customer, id); // Tworzenie wątku klienta
        usleep(rand() % 1000); // Losowy czas pojawienia się nowego klienta
    }

    pthread_join(barber_thread, NULL); // Oczekiwanie na zakończenie wątku fryzjera
    return 0;
}
