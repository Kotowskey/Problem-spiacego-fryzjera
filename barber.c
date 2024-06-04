#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define MAX_CHAIRS 10

pthread_mutex_t mutex;
sem_t customers;  // Liczba oczekujących klientów
sem_t barber;     // Liczba wolnych fryzjerów

int waiting = 0;
int total_chairs;
int rejections = 0;
int current_customer = -1;

void* barber_thread(void* arg) {
    while (1) {
        sem_wait(&customers); // Czekaj na klienta
        pthread_mutex_lock(&mutex);

        // Zajęcie klienta z poczekalni
        waiting--;
        current_customer = rand() % 1000; // Losowy numer klienta dla demonstracji

        pthread_mutex_unlock(&mutex);
        sem_post(&barber); // Powiadomienie fryzjera

        printf("Strzyżenie klienta: %d Poczekalnia: %d/%d [Fotel: %d]\n", rejections, waiting, total_chairs, current_customer);

        sleep(3); // Strzyżenie klienta

        pthread_mutex_lock(&mutex);
        current_customer = -1; // Fotel wolny
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* customer_thread(void* arg) {
    int id = *((int*)arg);

    pthread_mutex_lock(&mutex);
    if (waiting < total_chairs) {
        waiting++;
        printf("Klient %d w poczekalni. Poczekalnia: %d/%d\n", id, waiting, total_chairs);
        sem_post(&customers); // Nowy klient
        pthread_mutex_unlock(&mutex);
        sem_wait(&barber); // Czekaj na fryzjera
    } else {
        rejections++;
        printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n", rejections, waiting, total_chairs, current_customer);
        pthread_mutex_unlock(&mutex);
    }
    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_chairs>\n", argv[0]);
        return 1;
    }

    total_chairs = atoi(argv[1]);
    if (total_chairs > MAX_CHAIRS) {
        fprintf(stderr, "Maksymalna liczba krzeseł to %d\n", MAX_CHAIRS);
        return 1;
    }

    pthread_t barber_tid;
    pthread_t customer_tid[100];

    pthread_mutex_init(&mutex, NULL);
    sem_init(&customers, 0, 0);
    sem_init(&barber, 0, 0);

    srand(time(NULL));

    pthread_create(&barber_tid, NULL, barber_thread, NULL);

    for (int i = 0; i < 100; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        usleep(rand() % 1000000); // Klienci przychodzą w losowym czasie
        pthread_create(&customer_tid[i], NULL, customer_thread, id);
    }

    for (int i = 0; i < 100; i++) {
        pthread_join(customer_tid[i], NULL);
    }

    pthread_cancel(barber_tid);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&customers);
    sem_destroy(&barber);

    return 0;
}
