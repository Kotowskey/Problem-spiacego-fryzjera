#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_CHAIRS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_barber = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_customer = PTHREAD_COND_INITIALIZER;

int waiting_room[MAX_CHAIRS];
int num_chairs;
int num_waiting = 0;
int current_customer = -1;
int rejected_customers = 0;

void* customer(void* num) {
    int id = *(int*)num;

    usleep(rand() % 10000); // losowy czas przyjścia klienta

    pthread_mutex_lock(&mutex);
    if (num_waiting < num_chairs) {
        waiting_room[num_waiting] = id;
        num_waiting++;
        printf("Klient %d siada w poczekalni. Poczekalnia: %d/%d [Fotel: %d]\n", id, num_waiting, num_chairs, current_customer);

        pthread_cond_signal(&cond_barber); // obudź fryzjera, jeśli śpi
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while (current_customer != id) {
            pthread_cond_wait(&cond_customer, &mutex); // czekaj, aż fryzjer będzie gotowy
        }
        pthread_mutex_unlock(&mutex);

        printf("Klient %d jest strzyżony.\n", id);
        usleep(5000); // strzyżenie trwa stały czas
        printf("Klient %d kończy strzyżenie.\n", id);

        pthread_mutex_lock(&mutex);
        current_customer = -1;
        pthread_cond_signal(&cond_barber); // poinformuj fryzjera, że skończono strzyżenie
        pthread_mutex_unlock(&mutex);
    } else {
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
        while (num_waiting == 0) {
            printf("Fryzjer śpi.\n");
            pthread_cond_wait(&cond_barber, &mutex); // czekaj, aż pojawi się klient
        }

        current_customer = waiting_room[0];
        num_waiting--;
        for (int i = 0; i < num_waiting; i++) {
            waiting_room[i] = waiting_room[i + 1];
        }
        printf("Fryzjer budzi się i zaczyna strzyżenie klienta %d. Poczekalnia: %d/%d [Fotel: %d]\n", current_customer, num_waiting, num_chairs, current_customer);
        pthread_cond_signal(&cond_customer); // poinformuj klienta, że może być strzyżony
        pthread_mutex_unlock(&mutex);

        usleep(5000); // strzyżenie trwa stały czas

        pthread_mutex_lock(&mutex);
        current_customer = -1;
        pthread_cond_signal(&cond_barber); // poinformuj, że fryzjer skończył strzyżenie
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
    pthread_create(&barber_thread, NULL, barber, NULL);

    for (int i = 0; i < 20; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_t customer_thread;
        pthread_create(&customer_thread, NULL, customer, id);
        usleep(rand() % 1000); // losowy czas pojawienia się nowego klienta
    }

    pthread_join(barber_thread, NULL);
    return 0;
}
