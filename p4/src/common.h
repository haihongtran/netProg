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

#define LISTENQ             1024
#define HEADER_LEN          12
#define MAX_FILES           100
#define FILE_NAME_LEN       96
#define FILE_NAME_LEN_DIR   110

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
    char fileName[FILE_NAME_LEN];
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
    char fileNames[MAX_FILES][FILE_NAME_LEN];
} fileInfoRecvSuccessPacket;

typedef struct searchQueryPacket {
    headerPacket hdr;
    char fileName[FILE_NAME_LEN];
} searchQueryPacket;

typedef struct searchAnsSuccessPacket {
    headerPacket hdr;
    char ipAddr[20];
    unsigned int portNum;
    unsigned int fileSize;
} searchAnsSuccessPacket;

typedef struct fileReqPacket {
    headerPacket hdr;
    char fileName[FILE_NAME_LEN];
} fileReqPacket;

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

/*
 * Function: getRandNum()
 * Return a random number
 */
int getRandNum();

#endif  /*__COMMON_H__*/
