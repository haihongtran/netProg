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
        perror("[ERROR 0x0005] Cannot connect to server");
        return -1;
    }
    return clientSock;
}
