#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define MAX_CHAIRS 10
#define MAX_ITERATIONS 1000000000

pthread_mutex_t mutex;
sem_t customers;  // Liczba oczekujących klientów
sem_t barber;     // Liczba wolnych fryzjerów

int waiting = 0;
int total_chairs;
int rejections = 0;
int current_customer = -1;
int waiting_customers[MAX_CHAIRS];
int rejected_customers[100];
int total_rejections = 0;
bool info_mode = false;

void busy_wait(int iterations) {
    for (volatile int i = 0; i < iterations; i++);
}

void print_info() {
    if (info_mode) {
        printf("Kolejka oczekujących: ");
        for (int i = 0; i < waiting; i++) {
            printf("%d ", waiting_customers[i]);
        }
        printf("\n");
        printf("Klienci, którzy nie dostali się do gabinetu: ");
        for (int i = 0; i < total_rejections; i++) {
            printf("%d ", rejected_customers[i]);
        }
        printf("\n");
    }
}

void* barber_thread(void* arg) {
    while (1) {
        printf("Fryzjer ucina sobie drzemkę i czeka na klienta\n");
        sem_wait(&customers); // Czekaj na klienta
        pthread_mutex_lock(&mutex);

        // Zajęcie klienta z poczekalni
        current_customer = waiting_customers[0];
        for (int i = 0; i < waiting - 1; i++) {
            waiting_customers[i] = waiting_customers[i + 1];
        }
        waiting--;

        pthread_mutex_unlock(&mutex);
        sem_post(&barber); // Powiadomienie fryzjera

        printf("Strzyżenie klienta: %d\n", current_customer);
        print_info();

        busy_wait(rand() % MAX_ITERATIONS); // Strzyżenie klienta

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
        waiting_customers[waiting] = id;
        waiting++;
        printf("Klient %d w poczekalni. Poczekalnia: %d/%d\n", id, waiting, total_chairs);
        print_info();
        sem_post(&customers); // Nowy klient
        pthread_mutex_unlock(&mutex);
        sem_wait(&barber); // Czekaj na fryzjera
    } else {
        rejections++;
        rejected_customers[total_rejections] = id;
        total_rejections++;
        printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n", rejections, waiting, total_chairs, current_customer);
        print_info();
        pthread_mutex_unlock(&mutex);
    }
    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <number_of_chairs> [-info]\n", argv[0]);
        return 1;
    }

    total_chairs = atoi(argv[1]);
    if (total_chairs > MAX_CHAIRS) {
        fprintf(stderr, "Maksymalna liczba krzeseł to %d\n", MAX_CHAIRS);
        return 1;
    }

    if (argc == 3 && strcmp(argv[2], "-info") == 0) {
        info_mode = true;
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
        busy_wait(rand() % MAX_ITERATIONS); // Klienci przychodzą w losowym czasie
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
