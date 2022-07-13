/*
** server.c -- a stream socket server demo
*/

#include "server.h"
#include "stack.h"

static stack_st * stack = NULL;

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void sig_handler(int signum){

    //Return type of the handler function should be void
    printf("\nInside handler function %d\n", signum);
    exit(1);
}

void handle_connection(int fd){
    char argument[1024] = {};
    char command[1024] = {};
    char top_result[1024] = {};

    signal(SIGSEGV,sig_handler); // Register signal handler

    do {
        memset(top_result, '\0', sizeof(top_result));
        if (-1 == recv(fd, (void*)command, sizeof(command), 0)) {
            perror("ERROR: recv");
            goto Exit;
        }

        switch(get_command_type(command)) {
            case PUSH:
                get_argument(command, argument);
                printf("DEBUG: Pushing \"%s\"\n", argument);
                stack_push(stack, argument);
                break;
            case POP:
                printf("DEBUG: Popping\n");
                stack_pop(stack);
                break;
            case TOP:
                if (stack_top(stack, top_result) == 0) {
                    printf("DEBUG: Top \"%s\"\n", top_result);
                } else {
                    printf("ERROR: Failed top\n");
                }
                break;
            default:
                printf("DEBUG: Unknown command.\n");
                break;
        }

        if (send(fd, top_result, sizeof(top_result), 0) == -1){
            perror("ERROR: send");
            goto Exit;
        }
    } while(1);

Exit:
    printf("DEBUG: Exiting handle_connection(fd=%d)\n", fd);
    return;
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int result = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "ERROR: getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("ERROR: server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("ERROR: setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("ERROR: server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "ERROR: server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("ERROR: listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("ERROR: sigaction");
        exit(1);
    }

    printf("DEBUG: Creating stack...\n");
    if (0 != stack_create(&stack)){
        perror("ERROR: Failed to create stack");
        exit(1);
    }

    printf("DEBUG: server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("ERROR: accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("DEBUG: server: got connection from %s\n", s);

        switch (fork()) {
            case -1:
                puts("ERROR: Fork failed");
                goto Exit;
                break;
            case 0:
                close(sockfd); // child doesn't need the listener
                handle_connection(new_fd);
                close(new_fd);
                exit(0);
                break;
            default:
                // Parent - child is created
                close(new_fd);  // parent doesn't need this
                break;
        }
    }

    result = 0;

Exit:
    stack_free(stack);
    return result;
}