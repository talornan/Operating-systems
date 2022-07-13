CC = gcc -Wall -g

all: signals
signals: signals.o
	$(CC) -o signals signals.o
signals.o: signals.c
	$(CC) -c signals.c
.PHONY: all clean
clean:
	rm -f *.o signals