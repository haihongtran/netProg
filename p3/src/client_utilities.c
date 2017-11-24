#include "client_utilities.h"

int openClientSock(const char* ipAddr, int portNum)
{
    int clientSock;
    struct sockaddr_in serverAddr;

    /* Create socket descriptor */
    if ( (clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Failed to create a socket");
        return -1;
    }

    /* Connect to server */
    memset((char*) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(portNum);
    serverAddr.sin_addr.s_addr  = inet_addr(ipAddr);

    if (connect(clientSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Cannot connect to server");
        return -1;
    }
    return clientSock;
}

void* clientHttpRequest(void* arg) {
    argStruct threadArg = *(argStruct*) arg;
    int i, clientSock;
    char* httpReqMsg = "GET /index.html/ HTTP/1.1\r\nHost: index.html\r\n\r\n";
    unsigned int msgLen = strlen(httpReqMsg);
    unsigned int allBytesRead, bytesRead;
    unsigned int responseSize;
    char recvBuff[DATA_SIZE] = {0};

    /* Open and connect client sock */
    clientSock = openClientSock(threadArg.ipAddr, threadArg.portNum);
    if ( clientSock < 0 )
        return NULL;

    /* Assume client has parsed HTTP response to get the expected size */
    responseSize = HTTP_RES_HDR_SIZE + HTML_FILE_SIZE;

    /* Send HTTP request and receive HTTP response */
    for (i = 0; i < threadArg.reqNum; i++) {
        /* Send HTTP request to server */
        write(clientSock, httpReqMsg, msgLen);
        allBytesRead = 0;
        while (1) {
            /* Read HTTP response from server */
            memset(recvBuff, 0, DATA_SIZE);
            bytesRead = read(clientSock, recvBuff, sizeof(recvBuff));
            if ( bytesRead < 0 ) {
                perror("Cannot read from socket");
                break;
            }
            /* Increase receive bytes */
            allBytesRead += bytesRead;
            if ( allBytesRead >= responseSize ) {
#ifdef DEBUG
                printf("Received the whole file! allBytesRead is now %u bytes.\n", allBytesRead);
#endif
                break;
            }
        }
    }
    /* Close client socket after use */
    close(clientSock);
}
