#include "server.h"
//Q4.2
// taken from https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C/blob/main/server.c
int main()
{    
    int sockfd;
    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Cannot create socket");
        return 0;
    }

    printf("open socket successfully.\n");
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(ServerPort);
    serverAddr.sin_addr.s_addr = inet_addr(localHost);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Cannot bind socket");
        return 0;
    }
    printf("Binded to Port number %d.\n", ServerPort);

    if (listen(sockfd, 10) < 0)
    {
        perror("Listen error");
        return 0;
    }
    printf("\nWaiting for new TCP connections \n");

    int clientSocket;
    struct sockaddr_in clientAdd;
    socklen_t addressLen;
    char buffer[1024];

    while (1)
    {
        memset(&clientAdd, 0, sizeof(clientAdd)); // memset() is used to fill a block of memory with a particular value (reset).
        printf("waiting for client\n");
        addressLen = sizeof(clientAdd);
        clientSocket = accept(sockfd, (struct sockaddr *)&clientAdd, &addressLen); // The accept() system call is used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET).
        printf("incomming connecting \n");

        int inBytes = 0;
        while (1)
        {
            inBytes = recv(clientSocket, buffer, 1024, 0);
            if (inBytes > 0)
            {
                buffer[inBytes] = '\0';
                if (!strcmp(buffer, "LOCAL\n"))
                {
                    close(clientSocket);
                    break;
                }
                printf("%s\n", buffer);
                inBytes = 0;
            }
        }
    }

    close(sockfd);

    return 0;
}


