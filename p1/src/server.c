#include "protocol.h"

int main(int argc, char const *argv[])
{
    int serverSock, clientSock, fd, optval = 1;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;
    cmd_pkt serverSendPkt;
    data_pkt serverRecvPkt;
    uint8_t* buffer = NULL;
    unsigned int bytes_read = 0;
    unsigned int dataLen;
    char* fileName = NULL;
    bool fileStored = false;
    bool storedError = false;

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
    serverAddr.sin_port         = htons(PORT_NUMBER);
    serverAddr.sin_addr.s_addr  = htonl(INADDR_ANY);

    if (bind(serverSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Binding error");
        return -1;
    }

    /* Start listening to connections */
    if (listen(serverSock, 5) < 0)
    {
        perror("Cannot start listening");
        return -1;
    }

    clientAddrLen = sizeof(clientSock);
    clientSock = accept(serverSock, (struct sockaddr*) &clientAddr, &clientAddrLen);
    if (clientSock < 0)
    {
        perror("[ERROR 0x0005] Cannot accept new client");
        return -1;
    }

    /* Randomize seed to create a random initial sequence number */
    srand(time(NULL));

    /* Ininitilize static fields of application header */
    serverSendPkt.version  =   STATIC_VERSION;
    serverSendPkt.userId   =   STATIC_USER_ID;

    /* Start communication */
    while(1)
    {
        int tmp;
        if ( read(clientSock, &serverRecvPkt, sizeof(serverRecvPkt)) < 0 )
        {
            perror("Cannot read from socket");
            break;
        }
        switch ( ntohs(serverRecvPkt.command) )
        {
            case CLIENT_HELLO:
                /* Construct SERVER_HELLO message */
                serverSendPkt.sequence   = htons(rand() % MAX_SEQUENCE);
                serverSendPkt.length     = htons(HEADER_LENGTH);
                serverSendPkt.command    = htons(SERVER_HELLO);
                /* Send SERVER_HELLO message */
                write(clientSock, &serverSendPkt, HEADER_LENGTH);
                break;
            case DATA_DELIVERY:
                /* Calculate useful length of data */
                dataLen = ntohs(serverRecvPkt.length) - HEADER_LENGTH;
                /* Allocate more memory for buffer to store packet */
                buffer = (uint8_t*) realloc(buffer, bytes_read + dataLen);
                if ( buffer == NULL)
                {
                    perror("Cannot allocate more memory");
                    /* Construct ERROR packet */
                    tmp = ntohs(serverSendPkt.sequence) + 1;
                    serverSendPkt.sequence   = htons(tmp);
                    serverSendPkt.length     = htons(HEADER_LENGTH);
                    serverSendPkt.command    = htons(ERROR);
                    /* Send PKT_RECEIVED packet */
                    write(clientSock, &serverSendPkt, HEADER_LENGTH);
                    storedError = true;
                    break;
                }
                /* Copy received data to buffer */
                memcpy((void*)buffer + bytes_read, (void*)&serverRecvPkt.data, dataLen);
                /* Increase the number of bytes read */
                bytes_read += dataLen;
                /* Construct PKT_RECEIVED packet */
                tmp = ntohs(serverSendPkt.sequence) + 1;
                serverSendPkt.sequence   = htons(tmp);
                serverSendPkt.length     = htons(HEADER_LENGTH);
                serverSendPkt.command    = htons(PKT_RECEIVED);
                /* Send PKT_RECEIVED packet */
                write(clientSock, &serverSendPkt, HEADER_LENGTH);
                break;
            case DATA_STORE:
                /* Calculate useful length of data */
                dataLen = ntohs(serverRecvPkt.length) - HEADER_LENGTH;
                /* Extract the file name from the received data */
                fileName = (char*) malloc(dataLen * sizeof(char));
                memcpy((void*)fileName, (void*)&serverRecvPkt.data, dataLen);
                if ( (fd = open(fileName, O_WRONLY | O_CREAT, 0666)) < 0 )
                {
                    /* Construct STORED_ERROR message and send */
                    tmp = ntohs(serverSendPkt.sequence) + 1;
                    serverSendPkt.sequence   = htons(tmp);
                    serverSendPkt.length     = htons(HEADER_LENGTH);
                    serverSendPkt.command    = htons(STORED_ERROR);
                    write(clientSock, &serverSendPkt, HEADER_LENGTH);
                    printf("[ERROR 0x0008] Cannot creat new file for storing.\n");
                    storedError = true;
                    break;
                }
                write(fd, buffer, bytes_read);
                close(fd);
                free(buffer);
                fileStored = true;
                /* Construct FILE_STORED message and send */
                tmp = ntohs(serverSendPkt.sequence) + 1;
                serverSendPkt.sequence   = htons(tmp);
                serverSendPkt.length     = htons(HEADER_LENGTH);
                serverSendPkt.command    = htons(FILE_STORED);
                write(clientSock, &serverSendPkt, HEADER_LENGTH);
                break;
            case ERROR:
                printf("[ERROR 0x0005] Communication error.\n");
                break;
            default:
                break;
        }
        if (fileStored || storedError)
            break;
    }

    close(clientSock);
    close(serverSock);
    return 0;
}