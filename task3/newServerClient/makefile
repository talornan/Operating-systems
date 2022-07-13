FLAGS =-Wall -g
CC = gcc #if we want to change compiler
AR = ar

all: threadServer client

client: client.o
	$(CC) $(FLAGS) -o client client.o

threadServer: serverLib.a 
	$(CC) $(FLAGS) -pthread -o threadServer serverLib.a

serverLib.a:newServer.o
	$(AR) -rcs serverLib.a newServer.o

client.o: client.c client.h
	$(CC) $(FLAGS) -c client.c 

newServer.o: newServer.c newServer.h
	$(CC) $(FLAGS) -pthread -c newServer.c 	


.PHONY: clean
clean:
	rm -f *.o *.a *.so threadServer

run:
	./threadServer
