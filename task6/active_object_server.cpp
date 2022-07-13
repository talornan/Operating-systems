#include <ctype.h>

#include "active_object_server.h"
#include "active_object.h"

#define BUFFER_SIZE 1024

CQueueHead* socket_queue = NULL;
CQueueHead* message_queue = NULL;
CQueueHead* send_queue = NULL;

typedef struct {
    int fd;
    char data[BUFFER_SIZE];
} Message;

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

void* read_socket(void* argument) {
    if (NULL == argument) {
        return NULL;
    }

    int socket_fd = *(int*)(argument);
    free(argument);
    Message* message = (Message*)calloc(1, sizeof(*message));
    if (NULL == message) {
        printf("Failed to allocate message");
        return NULL;
    }
    message->fd = socket_fd;
    if  (-1 == read(socket_fd, &message->data, BUFFER_SIZE)) {
        printf("Failed to read from socket");
        return NULL;
    }
    return message;
}

void* caesar_cipher(void* argument) {
    if (NULL == argument) {
        return NULL;
    }

    Message* message = (Message*)argument;
    const int key = 1;
    char ch = '\0';

    for (unsigned int i = 0; i < sizeof(message->data); ++i) {
        ch = message->data[i];
        if(ch >= 'a' && ch <= 'z'){
            ch = ch + key;
            if(ch > 'z'){
                ch = ch - 'z' + 'a' - 1;
            }
            message->data[i] = ch;
        }
        else if(ch >= 'A' && ch <= 'Z'){
            ch = ch + key;
            if(ch > 'Z'){
                ch = ch - 'Z' + 'A' - 1;
            }
            message->data[i] = ch;
        }
    }

    enQ(message_queue, message);
    return message;
}

void* invert_case(void* argument) {
    if (NULL == argument) {
        return NULL;
    }
    Message* message = (Message*)argument;

    for (unsigned int i = 0; i < sizeof(message->data); ++i) {
        if (islower(message->data[i])) {
            message->data[i] = toupper(message->data[i]);
        }
        else {
            message->data[i] = tolower(message->data[i]);
        }
    }

    return message;
}

void* enqueue_send(void* argument) {
    if (NULL == argument) {
        return NULL;
    }
    Message* message = (Message*)argument;
    enQ(send_queue, message);
    return message;
}

void* send_message(void* argument) {
    if (NULL == argument) {
        return NULL;
    }
    Message* message = (Message*)argument;
    if (-1 == send(message->fd, (void*)(&message->data), sizeof(message->data), 0)) {
        printf("Failed to send message\n");
        return NULL;
    }
    return message;
}

void* free_message(void* argument) {
    if (NULL == argument) {
        return NULL;
    }
    Message* message = (Message*)argument;
    close(message->fd);
    free(message);
    return NULL;
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
    int* new_fd_pointer = NULL;

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

    socket_queue = createQ();
    message_queue = createQ();
    send_queue = createQ();

    AO* first_ao = newAO(socket_queue, read_socket, caesar_cipher);
    AO* second_ao = newAO(message_queue, invert_case, enqueue_send);
    AO* third_ao = newAO(send_queue, send_message, free_message);
    if (NULL == first_ao) goto Exit;
    if (NULL == second_ao) goto Exit;
    if (NULL == third_ao) goto Exit;


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

        new_fd_pointer = (int*)malloc(sizeof(new_fd));
        *new_fd_pointer = new_fd;
        enQ(socket_queue, new_fd_pointer);
    }

    result = 0;

Exit:
    destroyAO(first_ao);
    destroyAO(second_ao);
    destroyAO(third_ao);
    return result;
}
