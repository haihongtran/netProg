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

/* Uncomment this to see the logs used for debugging */
// #define DEBUG

#define DATA_SIZE           4096
#define HTTP_RES_HDR_SIZE   19
#define HTML_FILE_SIZE      1421253

#endif  /*__COMMON_H__*/
