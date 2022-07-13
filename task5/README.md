# Ex5
name :
Liel Vaknin 207323049
Tal Ornan 209349356

## How to run?

In order to compile the program run the following:

```
make all
Enter make run_server
Enter make run_client - you can run as many clients as you want - the IP the client connects to is the localhost - 
this will happen automatically as we defined in the makefile.

``` 

There are 3 executables which are compiled:
1. `server` - A multi-processing server which runs the stack and can be accessed by multiple clients simultaneously.
2. `client` - A program which takes input from the user sends it to the server and prints the server's response. Pass the Server's IP as argument.
3. `main_stack` - A single threaded program which runs the stack locally.


## Testing
In order to test first compile by running `make test_stack` and running `./test_stack
