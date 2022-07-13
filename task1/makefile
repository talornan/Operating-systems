FLAGS =-Wall -g
CC = gcc #if we want to change compiler

all: server main

main: main.o
	$(CC) $(FLAGS) -o main main.o

server: server.o
	$(CC) $(FLAGS) -o server server.o

main.o: main.c main.h
	$(CC) $(FLAGS) -c main.c 

server.o: server.c server.h
	$(CC) $(FLAGS) -c server.c 

.PHONY: clean all
	
run: all
	./main

clean:
	rm -f *.o *.a *.so server main

