#include "select_server_utilities.h"

int main()
{
    int serverSock, clientSock;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientSock);
    fdPool pool;                /* Information pool about fd sets */
    uint8_t* buffer[FD_SETSIZE]         = { NULL }; /* Data buffer for clients */
    uint16_t lastSeqNum[FD_SETSIZE]     = { 0 };    /* Sequece number of last packet */
    unsigned int bytes_read[FD_SETSIZE] = { 0 };    /* Bytes read from each client */

    /* Open non-blocking server socket */
    serverSock = openNBServerSock(PORT_NUMBER);
    if ( serverSock < 0 )
        return -1;

    /* Initialize information pool */
    initPool(serverSock, &pool);

    while(1) {
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);
        if ( FD_ISSET(serverSock, &pool.ready_set) ) {
            if ( (clientSock = accept(serverSock, (struct sockaddr*) &clientAddr,
                    &clientAddrLen)) < 0 ) {
                perror("[ERROR 0x0005] Cannot accept new client");
            }
            else {
                addClient(clientSock, &pool);
            }
        }
        handleClientFds(&pool, buffer, lastSeqNum, bytes_read);
    }
}
