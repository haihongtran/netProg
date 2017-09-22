#include "protocol.h"

int main(int argc, char const *argv[])
{
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;
    cmd_pkt serverSendCmdPkt;
    data_pkt serverRecvPkt;
    uint8_t* buffer = NULL;
    unsigned int bytes_read = 0;
    unsigned int dataLen;
    char* fileName = NULL;
    int fd;
    //TODO: consider using type bool of stdbool.h
    int stored = 0;

    /* Create socket descriptor */
    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create a socket");
        return -1;
    }
    //TODO: config socket to be used immediately, not to wait for timeout

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
    serverSendCmdPkt.version  =   STATIC_VERSION;
    serverSendCmdPkt.userId   =   STATIC_USER_ID;

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
                serverSendCmdPkt.sequence   = htons(rand() % MAX_SEQUENCE);
                serverSendCmdPkt.length     = htons(HEADER_LENGTH);
                serverSendCmdPkt.command    = htons(SERVER_HELLO);
                /* Send SERVER_HELLO message */
                write(clientSock, &serverSendCmdPkt, HEADER_LENGTH);
                break;
            case DATA_DELIVERY:
                /* Calculate useful length of data */
                dataLen = ntohs(serverRecvPkt.length) - HEADER_LENGTH;
                /* Allocate more memory for buffer to store packet */
                buffer = (uint8_t*) realloc(buffer, bytes_read + dataLen);
                /* Copy received data to buffer */
                memcpy((void*)buffer + bytes_read, (void*)&serverRecvPkt.data, dataLen);
                /* Increase the number of bytes read */
                bytes_read += dataLen;
                /* Construct PKT_RECEIVED packet */
                tmp = ntohs(serverSendCmdPkt.sequence) + 1;
                serverSendCmdPkt.sequence   = htons(tmp);
                serverSendCmdPkt.length     = htons(HEADER_LENGTH);
                serverSendCmdPkt.command    = htons(PKT_RECEIVED);
                /* Send PKT_RECEIVED packet */
                write(clientSock, &serverSendCmdPkt, HEADER_LENGTH);
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
                    tmp = ntohs(serverSendCmdPkt.sequence) + 1;
                    serverSendCmdPkt.sequence   = htons(tmp);
                    serverSendCmdPkt.length     = htons(HEADER_LENGTH);
                    serverSendCmdPkt.command    = htons(STORED_ERROR);
                    perror("[ERROR 0x0008] Cannot creat new file for storing");
                    close(clientSock);
                    close(serverSock);
                    return -1;
                }
                write(fd, buffer, bytes_read);
                close(fd);
                free(buffer);
                stored++;
                //TODO: send message to confirm file stored
                break;
            case ERROR:
                //TODO: handle the case ERROR code is received
                break;
            default:
                break;
        }
        if (stored)
            break;
    }

    close(clientSock);
    close(serverSock);
    return 0;
}