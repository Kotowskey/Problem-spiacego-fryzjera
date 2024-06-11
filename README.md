# Funkcja marnująca czas

```c
#define MAX_ITERATIONS 1000000000

void busy_wait(int iterations) {
    for (volatile int i = 0; i < iterations; i++);
}

busy_wait(rand() % MAX_ITERATIONS); // przykład użycia
