# Ex7

name : Tal Ornan id :209349356
name : Liel Vaknin id :207323049


## UFS - part 1
		
* Modules:

	myfs.h		File System simulation definitions
	myfs.c		File System simulation implementation
	
### Notes
Before running each tests prepend the statement `export LD_LIBRARY_PATH="put here absolute path of libmyfs.so"` in order to find the right folder - where the libmyfs.so placed: 

> In order to compile run `make all`.
This will build all the test and the ufs system containing all the implementations. 

> make clean		cleans all compilation products


## Mylibc(mystdio) part 2

* Modules:

	mystdio.h		mystdio definitions
	mystdio.c		mystdio implementation
	testmystdio.c	Builds, runing the mystdio and myfs, provides user interface to launch operations: 

> In order to compile run `make all`.
This will build all the test and the ufs system containing all the implementations. 

> make clean		cleans all compilation products


### Notes
	in case the library libmylibc.so not found during building the testmyfs - set environmental variable to folder - where the libmylibc.so placed:

	```
	export LD_LIBRARY_PATH="put here absolute path of libmylibc.so" 

	```
	

## Starting FS:
		
    if( !mymkfs(myfssize) ){
		printf("Error - mymkfs of size %d failed\n",myfssize);
		return -1;
	}

	if( !mymount("myFileSystem.fs", "/",NULL, 0, NULL)) {
		printf("Error - mount of %s to %s failed\n","myFileSystem.fs","/");
		return -1;
	}

## Usage testmystdio:
### Launch: 

> make testmystdio	builds mystdio.o, libmylibc.so and target executable testmystdio

	```
	./testmystdio -s 1000000
	```