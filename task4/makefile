FLAGS =-Wall -g
CC = gcc #if we want to change compiler
AR = ar

all: server client main_stack test_stack

client: client.o
	$(CC) $(FLAGS) -o client client.o

server: serverLib.a stack.o malloc.o
	$(CC) $(FLAGS) -pthread -o server serverLib.a stack.o malloc.o

serverLib.a: server.o
	$(AR) -rcs serverLib.a server.o

client.o: client.cpp client.h
	$(CC) $(FLAGS) -c client.cpp

server.o: server.cpp server.h
	$(CC) $(FLAGS) -pthread -c server.cpp  stack.cpp

stack.o: stack.cpp stack.h
	$(CC) $(FLAGS) -c stack.cpp -o stack.o

malloc.o: malloc.cpp malloc.h
	$(CC) $(FLAGS) -c malloc.cpp -o malloc.o

main_stack.o: main_stack.cpp
	$(CC) $(FLAGS) -c main_stack.cpp -o main_stack.o

test_stack.o: test_stack.cpp
	$(CC) $(FLAGS) -c test_stack.cpp -o test_stack.o

main_stack: malloc.o stack.o main_stack.o
	$(CC) $(FLAGS) malloc.o stack.o main_stack.o -o main_stack

test_stack: malloc.o stack.o test_stack.o
	$(CC) $(FLAGS) malloc.o stack.o test_stack.o -o test_stack

.PHONY: clean
clean:
	rm -f *.o *.a *.so server client main_stack test_stack

run_client: client
	./client 127.0.0.1

run_server: server
	./server

stack: main_stack
	./main_stack
