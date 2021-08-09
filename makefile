CC=gcc

CFLAGS=-I . -Wall --pedantic

all: dwnld

dwnld.o: dwnld.c
	$(CC) $(CFLAGS) -c dwnld.c -o dwnld.o

dwnld: main.c dwnld.o
	$(CC) $(CFLAGS) main.c dwnld.o -o dwnld

clean:
	rm dwnld dwnld.o
