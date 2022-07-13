/*
** client.c -- a stream socket client demo
*/

#include <arpa/inet.h>         // for inet_ntop
#include <netdb.h>             // for addrinfo, freeaddrinfo, gai_strerror
#include <netinet/in.h>        // for INET6_ADDRSTRLEN, sockaddr_in, sockadd...
#include <stdio.h>             // for fprintf, perror, printf, stderr, NULL
#include <stdlib.h>            // for exit
#include <string.h>            // for memset
#include <sys/socket.h>        // for connect, socket, AF_INET, AF_UNSPEC
#include <unistd.h>            // for close
#include "Reactor.h"

#define STDIN 0  // file descriptor for standard input
#define PORT "9034" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

int sockfd;

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void handle_stdin(int fd) {
    char buf[MAXDATASIZE] = {};
    ssize_t numbytes;

    if (-1 == (numbytes = read(fd, &buf, MAXDATASIZE-1))) {
        perror("read");
        exit(1);
    }

    buf[numbytes] = '\0';

    if (-1 == (numbytes = send(sockfd, buf, numbytes, 0))) {
        perror("send");
        exit(1);
    }
}

void handle_connection(int fd) {
    ssize_t numbytes;
    char buf[MAXDATASIZE] = {};
    if ((numbytes = recv(fd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    } else if (0 == numbytes) {
        close(fd);
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("%s", buf);
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
            -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        freeaddrinfo(servinfo);
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
              sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    Reactor reactor;
    reactor.InstallHandler(STDIN, handle_stdin);
    reactor.InstallHandler(sockfd, handle_connection);

    return 0;
}
