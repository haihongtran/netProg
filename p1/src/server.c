#include "protocol.h"

int main(int argc, char const *argv[])
{
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;

    char buf[1024];

    /* Create socket descriptor */
    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create a socket");
        exit(1);
    }

    /* Bind the socket to a port */
    memset((char*) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(PORT_NUMBER);
    serverAddr.sin_addr.s_addr  = htonl(INADDR_ANY);

    if (bind(serverSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Binding error");
        exit(1);
    }

    /* Start listening to connections */
    if (listen(serverSock, 5) < 0)
    {
        perror("Cannot start listening");
        exit(1);
    }

    clientAddrLen = sizeof(clientSock);
    clientSock = accept(serverSock, (struct sockaddr*) &clientAddr, &clientAddrLen);
    if (clientSock < 0)
    {
        perror("Accepting failed");
        exit(1);
    }

    /* Start communication */
    recv(clientSock, buf, sizeof(buf), 0);
    printf("Data received: %s\n", buf);

    close(clientSock);
    close(serverSock);
    return 0;
}