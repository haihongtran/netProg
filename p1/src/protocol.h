#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/* Header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define CLIENT_HELLO    0x0001
#define SERVER_HELLO    0x0002
#define DATA_DELIVERY   0x0003
#define DATA_STORE      0x0004
#define ERROR           0x0005

#define PORT_NUMBER     12345

typedef struct appHeader {
    unsigned char   version;
    unsigned char   userId;
    unsigned int    sequence;
    unsigned int    length;
    unsigned int    command;
} appHeader;

#endif