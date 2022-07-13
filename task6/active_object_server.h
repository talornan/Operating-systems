#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>


#define PORT "3490"  // the port users will be connecting to
#define MAX_SIZE 50
#define BACKLOG 10   // how many pending connections queue will hold

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
void *handle_connection(void *arg);
#endif //SERVER_H
