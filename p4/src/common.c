#include "common.h"

int openNBServerSock(int portNum)
{
    int serverSock, optval = -1;
    struct sockaddr_in serverAddr;

    /* Create socket descriptor */
    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create a socket");
        return -1;
    }

    /* Eliminate "Address already in use" error from bind */
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR,
                    (const void*) &optval, sizeof(int)) < 0)
    {
        perror("Cannot change options of server socket");
        return -1;
    }

    /* Set the socket to non-blocking */
    if ( (optval = fcntl(serverSock, F_GETFL)) < 0 ) {
        perror("Cannot get current options of socket");
    }
    optval = (optval | O_NONBLOCK);
    if ( fcntl(serverSock, F_SETFL, optval) ) {
        perror("Cannot set new options for socket");
    }

    /* Bind the socket to a port */
    memset((char*) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(portNum);
    serverAddr.sin_addr.s_addr  = htonl(INADDR_ANY);

    if (bind(serverSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Binding error");
        return -1;
    }

    /* Start listening to connections */
    if (listen(serverSock, LISTENQ) < 0) {
        perror("Cannot start listening");
        return -1;
    }
    return serverSock;
}

void initPool(int sockfd, fileDescriptorPool *fdPool) {
    int i;
    for ( i = 0; i < FD_SETSIZE; i++ )
        fdPool->clientfd[i] = -1;
    fdPool->maxfd = sockfd;
    FD_ZERO(&fdPool->read_set);
    FD_SET(sockfd, &fdPool->read_set);
}

void addClient(int sockfd, struct sockaddr_in* clientAddr, fileDescriptorPool *fdPool) {
    int i;
    fdPool->nready--;
    /* Find an available slot */
    for ( i = 0; i < FD_SETSIZE; i++ ) {
        if ( fdPool->clientfd[i] < 0 ) {
            /* Add connected socket to the pool */
            fdPool->clientfd[i] = sockfd;
            /* Add the socket descriptor to read set */
            FD_SET(sockfd, &fdPool->read_set);
            /* Copy the client address into the pool */
            memcpy(&fdPool->clientAddr[i], clientAddr, sizeof(struct sockaddr_in));
            /* Update max descriptor */
            if ( sockfd > fdPool->maxfd )
                fdPool->maxfd = sockfd;
            /* Update max index used */
            if ( i > fdPool->maxi )
                fdPool->maxi = i;
            break;
        }
    }
    if ( i == FD_SETSIZE ) {
        printf("ERROR: No empty slots to add more clients");
    }
}
