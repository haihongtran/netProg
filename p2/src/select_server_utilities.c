#include "select_server_utilities.h"

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

void initPool(int sockfd, fdPool *p) {
    int i;
    for ( i = 0; i < FD_SETSIZE; i++ )
        p->clientfd[i] = -1;
    p->maxfd = sockfd;
    FD_ZERO(&p->read_set);
    FD_SET(sockfd, &p->read_set);
}

void addClient(int sockfd, fdPool *p) {
    int i;
    p->nready--;
    /* Find an available slot */
    for ( i = 0; i < FD_SETSIZE; i++ ) {
        if ( p->clientfd[i] < 0 ) {
            /* Add connected socket to the pool */
            p->clientfd[i] = sockfd;
            /* Add the socket descriptor to read set */
            FD_SET(sockfd, &p->read_set);
            /* Update max descriptor */
            if ( sockfd > p->maxfd )
                p->maxfd = sockfd;
            /* Update max index used */
            if ( i > p->maxi )
                p->maxi = i;
            break;
        }
    }
    if ( i == FD_SETSIZE ) {
        printf("ERROR: No empty slots to add more clients");
    }
}

void handleClientRequest(int clientSock, uint8_t** buffer,
        uint16_t* lastSeqNum, unsigned int* bytes_read)
{
    int fd, seqNum;
    cmd_pkt serverSendPkt;
    data_pkt serverRecvPkt;
    unsigned int dataLen;   // Number of data bytes in each packet
    char* fileName = NULL;

    /* Randomize seed to create a random initial sequence number */
    srand(time(NULL));

    /* Ininitilize static fields of application header */
    serverSendPkt.version  =   STATIC_VERSION;
    serverSendPkt.userId   =   STATIC_USER_ID;

    /* Read packet from client */
    if ( read(clientSock, &serverRecvPkt, sizeof(serverRecvPkt)) < 0 )
    {
        perror("Cannot read from socket");
        exit(-1);
    }
    /* Check command of client */
    switch ( ntohs(serverRecvPkt.command) )
    {
        case CLIENT_HELLO:
            /* Construct and send SERVER_HELLO */
            seqNum = rand() % MAX_SEQUENCE;
            *lastSeqNum = seqNum;
            sendCmdPkt(clientSock, &serverSendPkt, seqNum,
                        HEADER_LENGTH, SERVER_HELLO);
            break;
        case DATA_DELIVERY:
            /* Calculate length of useful data */
            dataLen = ntohs(serverRecvPkt.length) - HEADER_LENGTH;
            /* Allocate more memory for buffer to store packet */
            *buffer = (uint8_t*) realloc(*buffer, *bytes_read + dataLen);
            /* Update sequence number */
            seqNum = *lastSeqNum + 1;
            *lastSeqNum = seqNum;
            if ( (*buffer) == NULL )
            {
                perror("Cannot allocate more memory");
                /* Construct and send ERROR packet */
                sendCmdPkt(clientSock, &serverSendPkt, seqNum, HEADER_LENGTH, ERROR);
                break;
            }
            /* Copy received data to buffer */
            memcpy((void*)(*buffer) + *bytes_read, (void*)serverRecvPkt.data, dataLen);
            /* Increase the number of bytes read from client*/
            *bytes_read += dataLen;
            /* Construct and send PKT_RECEIVED packet */
            sendCmdPkt(clientSock, &serverSendPkt, seqNum, HEADER_LENGTH, PKT_RECEIVED);
            break;
        case DATA_STORE:
            /* Calculate length of useful data */
            dataLen = ntohs(serverRecvPkt.length) - HEADER_LENGTH;

            /* Extract the file name from the received data */
            fileName = (char*) malloc(dataLen * sizeof(char));
            memcpy((void*)fileName, (void*)serverRecvPkt.data, dataLen);

            /* Update sequence number */
            seqNum = *lastSeqNum + 1;
            *lastSeqNum = seqNum;

            /* Create new file for writing */
            if ( (fd = open(fileName, O_WRONLY | O_CREAT, 0666)) < 0 )
            {
                perror("[ERROR 0x0008] Cannot create new file for storing");
                /* Construct and send STORED_ERROR */
                sendCmdPkt(clientSock, &serverSendPkt, seqNum, HEADER_LENGTH, STORED_ERROR);
                break;
            }
            /* Write data into file */
            if ( write(fd, *buffer, *bytes_read) < 0 )
            {
                perror("[ERROR 0x0008] Cannot write into the new file");
                /* Construct and send STORED_ERROR */
                sendCmdPkt(clientSock, &serverSendPkt, seqNum, HEADER_LENGTH, STORED_ERROR);
                break;
            }
            close(fd);
            free(*buffer);
            /* Construct FILE_STORED message and send */
            sendCmdPkt(clientSock, &serverSendPkt, seqNum, HEADER_LENGTH, FILE_STORED);
            break;
        case ERROR:
            printf("[ERROR 0x0005] Communication error.\n");
            break;
        default:
            printf("Unrecognized command: 0x%04x\n", serverRecvPkt.command);
            break;
    }
}

void handleClientFds(fdPool* p, uint8_t** buffer,
        uint16_t* lastSeqNum, unsigned int* bytes_read)
{
    int i, clientSock, nread;
    /* Find the FDs ready to be read */
    for ( i = 0; (i <= p->maxi) && (p->nready > 0); i++ ) {
        clientSock = p->clientfd[i];
        if ( (clientSock > 0) && (FD_ISSET(clientSock, &p->ready_set)) ) {
            p->nready--;
            /* Check if there are data to read from client socket */
            ioctl(clientSock, FIONREAD, &nread);
            if ( nread == 0 ) {  /* No data to read */
                close(clientSock);
                FD_CLR(clientSock, &p->read_set);
                p->clientfd[i] = -1;
                buffer[i] = NULL;
                lastSeqNum[i] = 0;
                bytes_read[i] = 0;
            }
            else {  /* Read data from socket */
                handleClientRequest(clientSock, &buffer[i], &lastSeqNum[i], &bytes_read[i]);
            }
        }
    }
}
