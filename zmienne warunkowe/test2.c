#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h> //klasyczne pliki naglowkowe
#include <time.h> //Do pseudolosowosci funkcji marnujacej czas
#include <math.h> //Do funkcji marnującej czas

#define MAX_ITERATIONS 100000000 //Zmiana wartosci przyspieszy/spowolni przychodzenie klientow i czas strzyżenia

pthread_mutex_t mutex;
pthread_cond_t cond_barber;
pthread_cond_t cond_customer; //mutexy

int rezygnacje = 0; //Zmienna pomocnicza do sledzenia ilosci rezygnacji
bool barber_chair = false; //Zmienna pomocnicza do sledzenia czy fryzjer pracuje
int num_chairs; //Ilosc krzesel podawana przez uzytkownika przy uruchomieniu programu
bool print_info = false; //Pomocnicze do spelnienia zalozen projektu
int current_customer = -1; // Zmienna do przechowywania ID klienta aktualnie obsługiwanego

typedef struct Node {
    int id;
    struct Node* next;
} Node; //Struktura przechowujaca id aktualnego klienta

typedef struct Queue {
    Node* front;
    Node* rear;
    int size;
} Queue; //Kolejka FIFO do prawidlowego wyswietlania komunikatow

Queue waiting_queue;
Queue resigned_queue;
//Poniezej funkcje pomocnicze do kolejki FIFO
void queue_init(Queue* q) {
    q->front = q->rear = NULL;
    q->size = 0;
}
//Upewniamy sie ze kolejka jest pusta
bool queue_is_empty(Queue* q) {
    return q->size == 0;
}
//Dodajemy do kolejki
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
//Czyscimy kolejke
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
//Wyswietlanie statusu, rowniez z parametrem -info
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

void busy_wait(int iterations) { //Proste marnowanie czasu
    for (volatile int i = 0; i < iterations; i++) {
        int sum = sqrt(i) / sin(i);
    }
}
//Obsluga fryzjera
void* barber(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (queue_is_empty(&waiting_queue)) {//Fryzjer moze odpoczac
            printf("Fryzjer ucina sobie drzemkę i czeka na klienta\n");
            pthread_cond_wait(&cond_barber, &mutex);
        }

        current_customer = dequeue(&waiting_queue);
        barber_chair = true;
        pthread_cond_signal(&cond_customer); //Rozpoczynamy ciecie
        print_status();
        pthread_mutex_unlock(&mutex);

        busy_wait(rand() % MAX_ITERATIONS); //Fryzjer wykonuje swoja prace w pseudolosowym czasie

        pthread_mutex_lock(&mutex);
        printf("Fryzjer zakończył strzyżenie klienta %d.\n", current_customer); //Konczymy strzyzenie, odblokowujemy mutexy, zwalniamy fotel
        barber_chair = false;
        pthread_cond_broadcast(&cond_customer);
        print_status();
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
//Obsluga klientow
void* customer(void* arg) {
    int id = *(int*)arg;
    pthread_mutex_lock(&mutex);

    if (waiting_queue.size < num_chairs) {
        enqueue(&waiting_queue, id); //Dodanie klienta do kolejki
        pthread_cond_signal(&cond_barber); //Przekazanie klienta do Fryzjerka
        print_status();

        while (barber_chair && current_customer != id) { //Oczekiwanie na swoja kolej
            pthread_cond_wait(&cond_customer, &mutex);
        }
    } else { //Rezygnacja z uslugi
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
