#ifndef __MULTI_SERVER_H__
#define __MULTI_SERVER_H__

#include <signal.h>
#include <sys/wait.h>

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

/*
 * Function: sigchldHandler()
 * Handle SIGCHLD signal
 * Reap all zombie processes
 */
void sigchldHandler(int sig);

#endif  /*__MULTI_SERVER_H__*/
