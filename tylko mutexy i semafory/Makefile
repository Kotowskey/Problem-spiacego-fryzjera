# Variables
CC = gcc
CFLAGS = -Wall -pthread
TARGET = barber

# Default target
all: $(TARGET)

# Compile the program
$(TARGET): barber.o
	$(CC) $(CFLAGS) -o $(TARGET) barber.o

# Compile the source file into an object file
barber.o: barber.c
	$(CC) $(CFLAGS) -c barber.c

# Clean up build files
clean:
	rm -f *.o $(TARGET)

# Run the program with arguments
run: $(TARGET)
	./$(TARGET) 5 -info

# Phony targets
.PHONY: all clean run
