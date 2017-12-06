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
#include <sys/ioctl.h>
#include <getopt.h>
#include <stdint.h>

#define LISTENQ     1024
#define DATA_SIZE   100000
#define HEADER_LEN  12
#define MAX_FILES   10

/* Communication packets between super node and child node */
typedef struct headerPacket {
    uint32_t totalLen;
    uint32_t id;
    uint32_t msgType;
} headerPacket;

typedef struct helloFromChildPacket {
    headerPacket hdr;
    unsigned int portNum;
} helloFromChildPacket;

typedef struct fileInformation {
    char fileName[96];
    unsigned int fileSize;
} fileInformation;

typedef struct fileInfoPacket {
    headerPacket hdr;
    unsigned int fileNum;
    fileInformation files[MAX_FILES];
} fileInfoPacket;

typedef struct fileInfoRecvSuccessPacket {
    headerPacket hdr;
    unsigned int fileNum;
    char fileNames[MAX_FILES][96];
} fileInfoRecvSuccessPacket;

typedef struct searchQueryPacket {
    headerPacket hdr;
    char fileName[96];
} searchQueryPacket;

typedef struct searchAnsSuccessPacket {
    headerPacket hdr;
    char ipAddr[20];
    unsigned int portNum;
    unsigned int fileSize;  //TODO: check if this is allowed
} searchAnsSuccessPacket;

typedef struct fileReqPacket {
    headerPacket hdr;
    char fileName[96];
} fileReqPacket;

typedef struct fileResSuccessPacket {
    headerPacket hdr;
    uint8_t data[DATA_SIZE];
} fileResSuccessPacket;

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

/*
 * Function: openClientSock()
 * Open client socket and connect to server
 * Return client socket descriptor for communication
 */
int openClientSock(const char* ipAddr, int portNum);

#endif  /*__COMMON_H__*/
