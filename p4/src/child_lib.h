#ifndef __CHILD_LIB_H__
#define __CHILD_LIB_H__

#include "protocol.h"
#include "common.h"

#include <sys/stat.h>
#include <dirent.h>

typedef struct threadArguments {
    unsigned int portNum;
    unsigned int id;
} threadArguments;

typedef struct handleClientReqArg {
    int clientSock;
    unsigned int id;
} handleClientReqArg;

/*
 * Function: getFileSize()
 * Return file size in long int
 */
off_t getFileSize(const char *fileName);

/*
 * Function: childServer()
 * Open a server to serve other peers
 */
void* childServer(void* arg);

void handleClientFds(fileDescriptorPool* fdPool, unsigned int id);
// void handleClientRequest(int clientSock, unsigned int id);
void* handleClientRequest(void* arg);

#endif  /*__CHILD_LIB_H__*/
