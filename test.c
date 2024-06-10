#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h> //Pseudolosowosc przychodzenia klientow

#define NUM_CHAIRS 5

pthread_mutex_t mutex;
pthread_cond_t cond_barber;
pthread_cond_t cond_customer;

int waiting_customers = 0;
bool barber_chair = false;
srand(time(NULL));
int czas_przychodzenia = rand();

void time_wasting(int n){

    for(int i=0; i<n; i++){}

}

void* barber(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (waiting_customers == 0) {
            printf("Golibroda śpi...\n");
            pthread_cond_wait(&cond_barber, &mutex);
        }

        printf("Golibroda zaczyna strzyc...\n");
        waiting_customers--;
        barber_chair = true;

        pthread_cond_signal(&cond_customer);
        pthread_mutex_unlock(&mutex);

        // Symulacja strzyżenia
        //sleep(3);
        time_wasting(10000000);

        pthread_mutex_lock(&mutex);
        printf("Golibroda zakończył strzyżenie.\n");
        barber_chair = false;
        pthread_cond_broadcast(&cond_customer);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* customer(void* arg) {
    int id = *(int*)arg;
    pthread_mutex_lock(&mutex);

    if (waiting_customers < NUM_CHAIRS) {
        printf("Klient %d w poczekalni. Poczekalnia: %d/%d\n", id, waiting_customers, NUM_CHAIRS);
        waiting_customers++;

        pthread_cond_signal(&cond_barber);

        while (barber_chair) {
            pthread_cond_wait(&cond_customer, &mutex);
        }

        printf("Klient %d jest strzyżony.\n", id);
        barber_chair = true;
    } else {
        printf("Klient %d odchodzi, brak miejsca.\n", id);
    }

    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Podaj prawidłową liczbę argumentów!\n");
        return EXIT_FAILURE;
    }

    int customers_num = atoi(argv[1]);
    if (customers_num <= 0) {
        printf("Liczba klientów musi być większa niż 0!\n");
        return EXIT_FAILURE;
    }

    pthread_t barber_thread;
    pthread_t customer_threads[customers_num];
    int customer_ids[customers_num];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_barber, NULL);
    pthread_cond_init(&cond_customer, NULL);

    pthread_create(&barber_thread, NULL, barber, NULL);

    for (int i = 0; i < 100; i++) {
        customer_ids[i] = i + 1;
        pthread_create(&customer_threads[i], NULL, customer, (void*)&customer_ids[i]);
        //sleep(1); // symulacja pojawiania się klientów
        time_wasting(czas_przychodzenia * 10000);
    }

    for (int i = 0; i < 100; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Anulowanie wątku golibrody
    pthread_cancel(barber_thread);
    pthread_join(barber_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_barber);
    pthread_cond_destroy(&cond_customer);

    return EXIT_SUCCESS;
}
