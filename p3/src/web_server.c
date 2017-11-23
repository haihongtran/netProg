#include "server_utilities.h"

int main() {
    int serverSock, clientSock;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientSock);
    fileDescriptorPool fdPool;      /* Information pool about fd sets */
    threadPool* thrPool = NULL;     /* Thread pool */

    /* Open non-blocking server socket */
    serverSock = openNBServerSock(PORT_NUMBER);
    if ( serverSock < 0 )
        return -1;

    /* Initialize thread pool and file descriptor pool */
    initPool(serverSock, &fdPool);
    thrPool = threadPoolInit(THR_NUM);

    printf("Web server is ready to serve!\n");

    while(1) {
        fdPool.ready_set = fdPool.read_set;
        fdPool.nready = select(fdPool.maxfd + 1, &fdPool.ready_set, NULL, NULL, NULL);
        if ( fdPool.nready < 0 ) {
            perror("Select function has error");
            continue;
        }
#ifdef DEBUG
        printf("select() returns: number of socket descriptors having data is %d\n", fdPool.nready);
#endif
        /* New client */
        if ( FD_ISSET(serverSock, &fdPool.ready_set) ) {
            if ( (clientSock = accept(serverSock, (struct sockaddr*) &clientAddr,
                    &clientAddrLen)) < 0 ) {
                perror("Cannot accept new client");
            }
            else {
#ifdef DEBUG
                printf("Adding client...\n");
#endif
                addClient(clientSock, &clientAddr, &fdPool);
            }
        }
        handleClientFds(&fdPool, thrPool);
    }

    return 0;
}
