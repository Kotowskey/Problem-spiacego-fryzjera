# Problem śpiącego fryzjera

Salon fryzjerski składa się z gabinetu z jednym fotelem (zasób wymagający synchronizacji) oraz z poczekalni zawierającej n krzeseł. W danym momencie w gabinecie może być strzyżony tylko jeden klient (wątek), reszta czeka na wolnych krzesłach w poczekalni. Fryzjer po skończeniu strzyżenia (może być stały czas) prosi do gabinetu kolejnego klienta, lub ucina sobie drzemkę, jeśli poczekalnia jest pusta. Nowy klient budzi fryzjera jeśli ten śpi, lub siada na wolne miejsce w poczekalni jeśli fryzjer jest zajęty. Jeśli poczekalnia jest pełna, to klient nie wchodzi do niej i rezygnuje z wizyty.





# Funkcja marnująca czas

```c
#define MAX_ITERATIONS 1000000000

void busy_wait(int iterations) {
    for (volatile int i = 0; i < iterations; i++);
}

busy_wait(rand() % MAX_ITERATIONS); // przykład użycia
