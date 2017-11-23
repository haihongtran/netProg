#include "server_utilities.h"

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

void addClient(int sockfd, fileDescriptorPool *fdPool) {
    int i;
    fdPool->nready--;
    /* Find an available slot */
    for ( i = 0; i < FD_SETSIZE; i++ ) {
        if ( fdPool->clientfd[i] < 0 ) {
            /* Add connected socket to the pool */
            fdPool->clientfd[i] = sockfd;
            /* Add the socket descriptor to read set */
            FD_SET(sockfd, &fdPool->read_set);
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

void handleClientFds(fileDescriptorPool* fdPool, threadPool* thrPool)
{
    int i, clientSock, nread;
    /* Find the FDs ready to be read */
    for ( i = 0; (i <= fdPool->maxi) && (fdPool->nready > 0); i++ ) {
        clientSock = fdPool->clientfd[i];
        if ( (clientSock > 0) && (FD_ISSET(clientSock, &fdPool->ready_set)) ) {
            fdPool->nready--;
            /* Check if there are data to read from client socket */
            if ( (ioctl(clientSock, FIONREAD, &nread)) < 0 ) {
                perror("ioctl() function has error");
                break;
            }
            if ( nread == 0 ) {  /* Connection close signal from client */
#ifdef DEBUG
                printf("Closing client connection...\n");
#endif
                close(clientSock);
                FD_CLR(clientSock, &fdPool->read_set);
                fdPool->clientfd[i] = -1;
            }
            else {  /* There are data from client */
                /* Read data from client */
                char buffer[nread];
                read(clientSock, buffer, nread);
                /* Handle client request */
                handleClientRequest(clientSock, thrPool);
            }
        }
    }
}

void handleClientRequest(int clientSock, threadPool* thrPool) {
    /* Allocate memory for client socket descriptor to put in task queue */
    int* clientSd = (int*) malloc(sizeof(int));
    *clientSd = clientSock;
#ifdef DEBUG
    printf("Enqueueing task...\n");
#endif
    /* Enqueue task */
    threadPoolEnqueue(thrPool, (void*)httpResponse, (void*)clientSd);
}

void httpResponse(void* sockfd) {
    int clientSock = *(int*) sockfd;
    int fd, bytesRead, bytesWrite;
    char* httpRes = "HTTP/1.1 200 OK\r\n\r\n";
    char sendBuf[DATA_SIZE];

    /* Open index.html file */
    if ( (fd = open("../res/index.html", O_RDONLY)) < 0 )
    {
        perror("Cannot find the required HTML file");
        exit(-1);
    }

    /* Write http response hdr to socket */
    bytesWrite = write(clientSock, httpRes, strlen(httpRes));
    if ( bytesWrite < 0 ) {
        perror("Writing HTTP header to socket has error");
        close(fd);
        exit(-1);
    }

    /* Send index.html file to client */
    while(1) {
        bytesRead = read(fd, sendBuf, DATA_SIZE);
        if ( bytesRead < 0 ) {  /* Read error */
            perror("Cannot read the HTML file");
            exit(-1);
        }
        else if ( bytesRead > 0 ) {
            bytesWrite = write(clientSock, sendBuf, bytesRead);
            if ( bytesWrite < 0 ) {
                perror("Writing to socket has error");
            }
        }
        else    /* End of file */
            break;
    }
    /* Close file after sending */
    close(fd);
}
