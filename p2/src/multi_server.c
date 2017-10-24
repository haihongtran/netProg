#include "server_utilities.h"

int main(int argc, char const *argv[])
{
    int serverSock, clientSock;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientSock);

    /* Handling SIGCHLD signal to reap all zombie processes*/
    signal(SIGCHLD, sigchldHandler);

    /* Open server socket */
    serverSock = openServerSock(PORT_NUMBER);
    if ( serverSock < 0 )
        return -1;

    while(1)
    {
        clientSock = accept(serverSock, (struct sockaddr*) &clientAddr, &clientAddrLen);
        if (clientSock < 0)
        {
            perror("[ERROR 0x0005] Cannot accept new client");
            continue;
        }
        if ( fork() == 0 ) /* Child process */
        {
            /* Child closes its copy of server listening socket inherited from parent*/
            close(serverSock);
            /* Child handles request from client */
            handleClientRequest(clientSock);
            /* Child closes connection with client after use */
            close(clientSock);
            /* Child exits */
            exit(0);
        }
        close(clientSock);  /* Parent closes its copy of connected socket */
    }
}
