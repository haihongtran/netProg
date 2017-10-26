#include "multi_server_utilities.h"

int openServerSock(int portNum)
{
    int serverSock, optval = -1;
    struct sockaddr_in serverAddr;

    /* Create socket descriptor */
    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
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
    if (listen(serverSock, LISTENQ) < 0)
    {
        perror("Cannot start listening");
        return -1;
    }
    return serverSock;
}

void handleClientRequest(int clientSock)
{
    int fd, nextSeq;
    cmd_pkt serverSendPkt;
    data_pkt serverRecvPkt;
    uint8_t* buffer = NULL; // Buffer for storing data from client
    unsigned int bytes_read = 0; // Total number of bytes read from client
    unsigned int dataLen;   // Number of data bytes in each packet
    char* fileName = NULL;
    bool fileStored = false, storedError = false, communicationError = false;

    /* Randomize seed to create a random initial sequence number */
    srand(time(NULL));

    /* Ininitilize static fields of application header */
    serverSendPkt.version  =   STATIC_VERSION;
    serverSendPkt.userId   =   STATIC_USER_ID;

    /* Start communication */
    while(1)
    {
        /* Read packet from client */
        if ( read(clientSock, &serverRecvPkt, sizeof(serverRecvPkt)) < 0 )
        {
            perror("Cannot read from socket");
            break;
        }
        /* Check command of client */
        switch ( ntohs(serverRecvPkt.command) )
        {
            case CLIENT_HELLO:
                /* Construct and send SERVER_HELLO */
                sendCmdPkt(clientSock, &serverSendPkt, rand() % MAX_SEQUENCE,
                            HEADER_LENGTH, SERVER_HELLO);
                break;
            case DATA_DELIVERY:
                /* Calculate length of useful data */
                dataLen = ntohs(serverRecvPkt.length) - HEADER_LENGTH;
                /* Allocate more memory for buffer to store packet */
                buffer = (uint8_t*) realloc(buffer, bytes_read + dataLen);
                if ( buffer == NULL)
                {
                    perror("Cannot allocate more memory");
                    /* Construct and send ERROR packet */
                    nextSeq = ntohs(serverSendPkt.sequence) + 1;
                    sendCmdPkt(clientSock, &serverSendPkt, nextSeq, HEADER_LENGTH, ERROR);
                    storedError = true;
                    break;
                }
                /* Copy received data to buffer */
                memcpy((void*)buffer + bytes_read, (void*)serverRecvPkt.data, dataLen);
                /* Increase the number of bytes read from client*/
                bytes_read += dataLen;
                /* Construct and send PKT_RECEIVED packet */
                nextSeq = ntohs(serverSendPkt.sequence) + 1;
                sendCmdPkt(clientSock, &serverSendPkt, nextSeq, HEADER_LENGTH, PKT_RECEIVED);
                break;
            case DATA_STORE:
                /* Calculate length of useful data */
                dataLen = ntohs(serverRecvPkt.length) - HEADER_LENGTH;
                /* Extract the file name from the received data */
                fileName = (char*) malloc(dataLen * sizeof(char));
                memcpy((void*)fileName, (void*)serverRecvPkt.data, dataLen);
                /* Create new file for writing */
                if ( (fd = open(fileName, O_WRONLY | O_CREAT, 0666)) < 0 )
                {
                    perror("[ERROR 0x0008] Cannot create new file for storing");
                    storedError = true;
                    /* Construct and send STORED_ERROR */
                    nextSeq = ntohs(serverSendPkt.sequence) + 1;
                    sendCmdPkt(clientSock, &serverSendPkt, nextSeq, HEADER_LENGTH, STORED_ERROR);
                    break;
                }
                /* Write data into file */
                if ( write(fd, buffer, bytes_read) < 0 )
                {
                    perror("[ERROR 0x0008] Cannot write into the new file");
                    storedError = true;
                    /* Construct and send STORED_ERROR */
                    nextSeq = ntohs(serverSendPkt.sequence) + 1;
                    sendCmdPkt(clientSock, &serverSendPkt, nextSeq, HEADER_LENGTH, STORED_ERROR);
                    break;
                }
                close(fd);
                free(buffer);
                fileStored = true;
                /* Construct FILE_STORED message and send */
                nextSeq = ntohs(serverSendPkt.sequence) + 1;
                sendCmdPkt(clientSock, &serverSendPkt, nextSeq, HEADER_LENGTH, FILE_STORED);
                break;
            case ERROR:
                printf("[ERROR 0x0005] Communication error.\n");
                communicationError = true;
                break;
            default:
                printf("Unrecognized command: 0x%04x\n", serverRecvPkt.command);
                break;
        }
        if (fileStored || storedError || communicationError)
            break;
    }
}

void sigchldHandler(int sig)
{
    while ( waitpid(-1, 0, WNOHANG) > 0 )
        ;
    return;
}
