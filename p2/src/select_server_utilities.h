#ifndef __SELECT_SERVER_H__
#define __SELECT_SERVER_H__

#include <sys/select.h>
#include <sys/ioctl.h>
#include "common_utilities.h"

#define LISTENQ 1024

/* Information structure for file descriptor (FD) sets */
typedef struct s_pool {
    int maxfd;                  /* Max value of FD in all sets */
    fd_set read_set;            /* fd_set of active FDs to read from */
    int clientfd[FD_SETSIZE];   /* Set of active FDs */
    int maxi;                   /* Max index used in clientfd */
    fd_set ready_set;           /* fd_set of FDs ready to be read */
    int nready;                 /* Number of FDs ready to be read */
} fdPool;

/*
 * Function: openNBServerSock()
 * Open non-blocking socket and listen for client request
 * Return non-blocking listening socket descriptor
 */
int openNBServerSock(int portNum);

/*
 * Function: initPool()
 * Initialize pool structure
 */
void initPool(int sockfd, fdPool* p);

/*
 * Function: addClient()
 * Add client to read fd set
 */
void addClient(int sockfd, fdPool* p);

/*
 * Function: handleClientRequest()
 * Handle each client request
 */
void handleClientRequest(int clientSock, uint8_t** buffer,
        uint16_t* lastSeqNum, unsigned int* bytes_read);

/*
 * Function: handleClientFds()
 * Handle client fds ready to be read
 */
void handleClientFds(fdPool* p, uint8_t** buffer,
        uint16_t* lastSeqNum, unsigned int* bytes_read);

#endif  /*__SELECT_SERVER_H__*/
