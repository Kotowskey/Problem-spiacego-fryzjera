#!/bin/bash

# Kompilacja programu
gcc -pthread -o barber_shop barber.c

# Sprawdzenie, czy kompilacja się powiodła
if [ $? -ne 0 ]; then
    echo "Błąd kompilacji programu."
    exit 1
fi

# Uruchomienie programu z argumentami w tle
./barber_shop 5 -info &
BARBER_PID=$!

# Czekanie 20 sekund
sleep 20

# Zatrzymanie programu
kill $BARBER_PID

# Sprawdzenie, czy kill się powiódł
if [ $? -ne 0 ]; then
    echo "Błąd podczas zatrzymywania programu."
    exit 1
fi

# Wyświetlenie komunikatu o zakończeniu testu
echo "Test zakończony pomyślnie."
