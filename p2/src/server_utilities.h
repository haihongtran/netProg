#ifndef __SERVER_H__
#define __SERVER_H__

#include "protocol.h"
#include "common_utilities.h"

#define LISTENQ 1024

/*
 * Function: openServerSock()
 * Open socket and listen for client request
 * Return server listening socket descriptor
 */
int openServerSock(int portNum);

/*
 * Function: handleClientRequest()
 * Handle client request
 */
void handleClientRequest(int clientSock);

#endif  /*__SERVER_H__*/
