#include "protocol.h"

int main(int argc, char const *argv[])
{
    int clientSock;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen;

    char buffer[1024];

    /* Create socket descriptor */
    if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create a socket");
        exit(1);
    }

    /* Connect to server */
    memset((char*) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(PORT_NUMBER);
    serverAddr.sin_addr.s_addr  = inet_addr("127.0.0.1");

    serverAddrLen = sizeof(serverAddr);
    if (connect(clientSock, (struct sockaddr*) &serverAddr, serverAddrLen) < 0)
    {
        perror("Connecting failed");
        exit(1);
    }

    /* Start communication */
    strcpy(buffer, "Hello\n");
    send(clientSock, buffer, 6, 0);
    printf("Data sent: %s\n", buffer);

    close(clientSock);
    return 0;
}
