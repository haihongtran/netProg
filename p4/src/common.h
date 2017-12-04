#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <getopt.h>

/* Information structure for file descriptor (FD) sets */
typedef struct fileDescriptorPool {
    int maxfd;                  /* Max value of FD in all sets */
    fd_set read_set;            /* fd_set of active FDs to read from */
    int clientfd[FD_SETSIZE];   /* Set of active FDs */
    struct sockaddr_in clientAddr[FD_SETSIZE]; /* Address of active clients */
    int maxi;                   /* Max index used in clientfd */
    fd_set ready_set;           /* fd_set of FDs ready to be read */
    int nready;                 /* Number of FDs ready to be read */
} fileDescriptorPool;

/* Function prototypes */
int openNBServerSock(int portNum);
void initPool(int sockfd, fileDescriptorPool *fdPool);
void addClient(int sockfd, struct sockaddr_in* clientAddr, fileDescriptorPool *fdPool);

#endif  /*__COMMON_H__*/
