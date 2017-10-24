#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "protocol.h"
#include "common_utilities.h"

/*
 * Function: openClientSock()
 * Open client socket and connect to server
 * Return client socket descriptor for communication
 */
int openClientSock(const char* ipAddr, int portNum);

/*
 * Function: clientSendFile()
 * Client connects and sends text to server
 * Return -1 if the sending fails
 */
int clientSendFile(const char* ipAddr);

#endif  /*__CLIENT_H__*/
