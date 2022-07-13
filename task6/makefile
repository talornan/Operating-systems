CC = gcc
FLAGS = -Wall -g

all: queue_test active_object_test active_object_server guard_test singleton_test selectserver selectclient ex6.so

queue.o: queue.cpp queue.h
	$(CC) $(FLAGS) -c queue.cpp -fPIC -pthread -o queue.o

active_object.o: active_object.cpp active_object.h
	$(CC) $(FLAGS) -c active_object.cpp -fPIC -pthread -o active_object.o

active_object_server.o: active_object_server.cpp active_object_server.h
	$(CC) $(FLAGS) -c active_object_server.cpp -fPIC -pthread -o active_object_server.o

guard.o: guard.cpp guard.h
	$(CC) $(FLAGS) -c guard.cpp -fPIC -o guard.o

reactor.o: Reactor.cpp Reactor.h
	$(CC) $(FLAGS) -c Reactor.cpp -fPIC -std=c++17 -o reactor.o

ex6.so: reactor.o guard.o active_object_server.o queue.o active_object.o
	$(CC) $(FLAGS) --shared -fPIC -pthread reactor.o guard.o active_object_server.o queue.o active_object.o -o ex6.so

queue_test: queue_test.cpp ex6.so
	$(CC) $(FLAGS) queue_test.cpp queue.o -L. -l:ex6.so -std=c++17 -lstdc++ -o queue_test -pthread

active_object_test: active_object_test.cpp ex6.so
	$(CC) $(FLAGS) active_object_test.cpp -L. -l:ex6.so -std=c++17 -lstdc++ -o active_object_test -pthread

active_object_server: ex6.so
	$(CC) $(FLAGS) -pthread -L. -l:ex6.so -std=c++17 -lstdc++ -o active_object_server

guard_test: guard_test.cpp ex6.so
	$(CC) $(FLAGS) -pthread guard_test.cpp -L. -l:ex6.so -std=c++17 -lstdc++ -o guard_test

singleton_test: singleton_test.cpp Singleton.h ex6.so
	$(CC) $(FLAGS) -pthread singleton_test.cpp -L. -l:ex6.so -std=c++17 -lstdc++ -o singleton_test

selectserver: selectserver.cpp ex6.so
	$(CC) $(FLAGS) -pthread selectserver.cpp -L. -l:ex6.so -std=c++17 -lstdc++ -o selectserver

selectclient: selectclient.cpp ex6.so
	$(CC) $(FLAGS) -pthread selectclient.cpp -L. -l:ex6.so -std=c++17 -lstdc++ -o selectclient


.PHONY: clean build
clean:
	rm -f *.o *.a queue_test active_object_test active_object_server guard_test singleton_test selectserver selectclient ex6.so

build: queue_test active_object_test
