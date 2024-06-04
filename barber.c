#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_CHAIRS 10
#define HAIRCUT_TIME 3

sem_t waitingRoom; // Semaphore for waiting room chairs
sem_t barberChair; // Semaphore for the barber chair
sem_t barberPillow; // Semaphore for the barber's sleep
sem_t seatBelt; // Semaphore for the barber to signal haircut completion

int waitingCustomers = 0;
int totalCustomers = 0;
int rejections = 0;
pthread_mutex_t mutex;

void *barber(void *arg) {
    while (1) {
        // Wait for a customer to wake up the barber
        sem_wait(&barberPillow);

        // Lock the barber chair
        sem_wait(&barberChair);

        // Critical section - cut hair
        pthread_mutex_lock(&mutex);
        waitingCustomers--;
        printf("Poczekalnia: %d/%d [Fotel: %d]\n", waitingCustomers, MAX_CHAIRS, totalCustomers - rejections - waitingCustomers);
        pthread_mutex_unlock(&mutex);
        
        printf("Strzyżenie klienta %d\n", totalCustomers - rejections - waitingCustomers);

        // Simulate hair cutting
        sleep(HAIRCUT_TIME);

        // Release the barber chair
        sem_post(&barberChair);

        // Signal the customer that the haircut is done
        sem_post(&seatBelt);
    }
    return NULL;
}

void *customer(void *num) {
    int id = *(int *)num;

    // Lock the mutex for critical section
    pthread_mutex_lock(&mutex);

    if (waitingCustomers < MAX_CHAIRS) {
        // Increment the count of waiting customers
        waitingCustomers++;

        printf("Klient %d wchodzi do poczekalni. Poczekalnia: %d/%d [Rezygnacja: %d]\n", id, waitingCustomers, MAX_CHAIRS, rejections);
        
        // Wake up the barber if sleeping
        sem_post(&barberPillow);

        // Release the mutex
        pthread_mutex_unlock(&mutex);

        // Wait for the barber chair to be free
        sem_wait(&barberChair);

        // Signal the barber that we are ready for a haircut
        sem_post(&barberPillow);

        // Wait for the barber to finish the haircut
        sem_wait(&seatBelt);
    } else {
        // Increment the rejection count
        rejections++;
        printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: -]\n", rejections, waitingCustomers, MAX_CHAIRS);

        // Release the mutex
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int infoMode = 0;
    if (argc > 1 && strcmp(argv[1], "-info") == 0) {
        infoMode = 1;
    }

    pthread_t barberThread;
    pthread_t customers[50];

    // Initialize semaphores
    sem_init(&waitingRoom, 0, MAX_CHAIRS);
    sem_init(&barberChair, 0, 1);
    sem_init(&barberPillow, 0, 0);
    sem_init(&seatBelt, 0, 0);

    pthread_mutex_init(&mutex, NULL);

    // Create the barber thread
    pthread_create(&barberThread, NULL, barber, NULL);

    // Create customer threads
    for (int i = 0; i < 50; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        sleep(rand() % HAIRCUT_TIME);
        pthread_create(&customers[i], NULL, customer, (void *)id);
    }

    // Wait for all customer threads to finish
    for (int i = 0; i < 50; i++) {
        pthread_join(customers[i], NULL);
    }

    // Destroy the mutex and semaphores
    pthread_mutex_destroy(&mutex);
    sem_destroy(&waitingRoom);
    sem_destroy(&barberChair);
    sem_destroy(&barberPillow);
    sem_destroy(&seatBelt);

    return 0;
}
