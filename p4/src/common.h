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

/* Function prototypes */
int openNBServerSock(int portNum);
void initPool(int sockfd, fileDescriptorPool *fdPool);
void addClient(int sockfd, struct sockaddr_in* clientAddr, fileDescriptorPool *fdPool);

#endif  /*__COMMON_H__*/
