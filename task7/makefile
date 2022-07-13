CC = gcc
W = -Wall

.PHONEY : clean all

all: test

test:libmyfs.so libmylibc.so  testmystdio.o
	$(CC) $(W) -o  testmystdio testmystdio.o ./libmyfs.so ./libmylibc.so

testmystdio: libmylibc.so libmyfs.so
	$(CC) $(W) -L/mnt/c/systemOpertionTesks/task7 -o testmystdio testmystdio.c -lmyfs -lmylibc

libmylibc.so: mystdio.o
	$(CC) $(W) --shared -o libmylibc.so mystdio.o

mystdio.o: mystdio.c
	$(CC) $(W) -fPIC -c mystdio.c

libmyfs.so: myfs.o
	$(CC) $(W) --shared -o libmyfs.so myfs.o

myfs.o: myfs.c
	$(CC) $(W) -fPIC -c myfs.c

clean:
	@echo "Cleaning up..."
	rm -rvf *.o
	rm -rvf 'myfs.o'
	rm -rvf 'mystdio.o'
	rm -rvf 'testmystdio.o'
	rm -rvf *.so
	rm -rvf 'libmyfs.so'
	rm -rvf 'libmylibc.so'
	rm -rf testmystdio
	rm -rf *.o test

