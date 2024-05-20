# Nazwa pliku wykonywalnego
TARGET = program

# Kompilator
CC = gcc

# Flagi kompilatora
CFLAGS = -Wall -pthread

# Pliki źródłowe
SRCS = program.c

# Pliki obiektowe
OBJS = $(SRCS:.c=.o)

# Reguła domyślna
all: $(TARGET)

# Kompilacja pliku wykonywalnego
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Kompilacja plików obiektowych
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Czystość projektu
clean:
	rm -f $(OBJS) $(TARGET)

# Reguła uruchomienia
run: $(TARGET)
	./$(TARGET) 5 # Tutaj można podać domyślną liczbę krzeseł jako argument

# Phony targets
.PHONY: all clean run
