#include "client.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd;
    char recv_buffer[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    char * line = NULL;
    size_t line_size = 0;

    if (argc != 2) {
        fprintf(stderr,"ERROR: usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "ERROR: getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("ERROR: client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("ERROR: client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "ERROR: client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    freeaddrinfo(servinfo); // all done with this structure

    do {
        if (getline(&line, &line_size, stdin) == -1) {
            goto Exit;
        }

        if (-1 == send(sockfd, line, line_size, 0)) {
            perror("ERROR: send");
            goto Exit;
        }

        memset(recv_buffer, '\0', sizeof(recv_buffer));
        if (-1 == recv(sockfd, (void*)recv_buffer, sizeof(recv_buffer), 0)) {
            perror("ERROR: recv");
            goto Exit;
        }
        if (strlen(recv_buffer)) {
            printf("OUTPUT: %s\n", recv_buffer);
        }
    } while (1);

Exit:
    free(line);
    close(sockfd);
    return 0;
}
