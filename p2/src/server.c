#include "server_utilities.h"

int main(int argc, char const *argv[])
{
    int serverSock, clientSock;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen;

    /* Open server socket */
    serverSock = openServerSock(PORT_NUMBER);
    if ( serverSock < 0 )
        return -1;

    /* Wait and accept a client request */
    clientAddrLen = sizeof(clientSock);
    clientSock = accept(serverSock, (struct sockaddr*) &clientAddr, &clientAddrLen);
    if (clientSock < 0)
    {
        perror("[ERROR 0x0005] Cannot accept new client");
        return -1;
    }

    /* Handle client request */
    handleClientRequest(clientSock);

    /* Close sockets after use */
    close(clientSock);
    close(serverSock);

    return 0;
}
