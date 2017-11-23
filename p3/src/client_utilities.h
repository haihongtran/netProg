#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "common.h"

/* Struct of arguments for thread function */
typedef struct argStruct
{
    const char* ipAddr;
    int portNum;
    unsigned int reqNum;
} argStruct;

/*
 * Function: openClientSock()
 * Open client socket and connect to server
 * Return client socket descriptor for communication
 */
int openClientSock(const char* ipAddr, int portNum);

/*
 * Function: clientHttpRequest()
 * Client connects and sends text to server
 * Return -1 if the sending fails
 */
void* clientHttpRequest(void* arg);

#endif  /*__CLIENT_H__*/
