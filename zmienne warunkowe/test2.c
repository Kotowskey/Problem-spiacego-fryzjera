#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define MAX_ITERATIONS 10000

pthread_mutex_t mutex;
pthread_cond_t cond_barber;
pthread_cond_t cond_customer;

int rezygnacje = 0;
bool barber_chair = false;
int num_chairs;
bool print_info = false;
int current_customer = -1; // Zmienna do przechowywania ID klienta aktualnie obsługiwanego

typedef struct Node {
    int id;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* front;
    Node* rear;
    int size;
} Queue;

Queue waiting_queue;
Queue resigned_queue;

void queue_init(Queue* q) {
    q->front = q->rear = NULL;
    q->size = 0;
}

bool queue_is_empty(Queue* q) {
    return q->size == 0;
}

void enqueue(Queue* q, int id) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->id = id;
    new_node->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = new_node;
    } else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->size++;
}

int dequeue(Queue* q) {
    if (queue_is_empty(q)) {
        return -1;
    }
    Node* temp = q->front;
    int id = temp->id;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return id;
}

void print_status() {
    printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n", rezygnacje, waiting_queue.size, num_chairs, current_customer);
    if (print_info) {
        printf("Kolejka oczekujących: ");
        Node* current = waiting_queue.front;
        while (current != NULL) {
            printf("%d ", current->id);
            current = current->next;
        }
        printf("\nLista rezygnacji: ");
        current = resigned_queue.front;
        while (current != NULL) {
            printf("%d ", current->id);
            current = current->next;
        }
        printf("\n");
    }
}

void busy_wait(int iterations) {
    for (volatile int i = 0; i < iterations; i++) {
        int sum = sqrt(i) / sin(i);
    }
}

void* barber(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (queue_is_empty(&waiting_queue)) {
            printf("Fryzjer ucina sobie drzemkę i czeka na klienta\n");
            pthread_cond_wait(&cond_barber, &mutex);
        }

        current_customer = dequeue(&waiting_queue);
        barber_chair = true;
        pthread_cond_signal(&cond_customer);
        print_status();
        pthread_mutex_unlock(&mutex);

        busy_wait(rand() % MAX_ITERATIONS);

        pthread_mutex_lock(&mutex);
        printf("Fryzjer zakończył strzyżenie klienta %d.\n", current_customer);
        barber_chair = false;
        current_customer = -1;
        pthread_cond_broadcast(&cond_customer);
        print_status();
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* customer(void* arg) {
    int id = *(int*)arg;
    pthread_mutex_lock(&mutex);

    if (waiting_queue.size < num_chairs) {
        enqueue(&waiting_queue, id);
        pthread_cond_signal(&cond_barber);
        print_status();

        while (barber_chair && current_customer != id) {
            pthread_cond_wait(&cond_customer, &mutex);
        }
    } else {
        enqueue(&resigned_queue, id);
        rezygnacje++;
        print_status();
    }

    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        printf("Podaj prawidłową liczbę argumentów!\n");
        return EXIT_FAILURE;
    }

    num_chairs = atoi(argv[1]);
    if (num_chairs <= 0 || num_chairs > 10) {
        printf("Bledna liczba krzesel!\n");
        return EXIT_FAILURE;
    }

    if (argc == 3 && strcmp(argv[2], "-info") == 0) {
        print_info = true;
    }

    pthread_t barber_thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_barber, NULL);
    pthread_cond_init(&cond_customer, NULL);
    queue_init(&waiting_queue);
    queue_init(&resigned_queue);

    pthread_create(&barber_thread, NULL, barber, NULL);

    int customer_id = 1;
    while (1) {
        pthread_t customer_thread;
        int* id_ptr = malloc(sizeof(int));
        *id_ptr = customer_id;
        pthread_create(&customer_thread, NULL, customer, id_ptr);
        customer_id++;

        // Klienci przychodzą w losowych odstępach czasu
        busy_wait((rand() % (MAX_ITERATIONS / 10)) + (MAX_ITERATIONS / 10));

        free(id_ptr);
    }

    pthread_cancel(barber_thread);
    pthread_join(barber_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_barber);
    pthread_cond_destroy(&cond_customer);

    return EXIT_SUCCESS;
}
