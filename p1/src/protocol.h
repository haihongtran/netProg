#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/* Header files */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#define CLIENT_HELLO    0x0001
#define SERVER_HELLO    0x0002
#define DATA_DELIVERY   0x0003
#define DATA_STORE      0x0004
#define ERROR           0x0005

/* More commands from server */
//TODO: explain these commands in readme.txt
#define PKT_RECEIVED    0x0006
#define FILE_STORED     0x0007
#define STORED_ERROR    0x0008

#define PORT_NUMBER     12345
#define MAX_SEQUENCE    65535

#define STATIC_VERSION  0x04
#define STATIC_USER_ID  0x08

#define DATA_SIZE       1400
#define HEADER_LENGTH   8

#define MAX_NAME_LEN    256

typedef struct cmd_pkt
{
    uint8_t     version;
    uint8_t     userId;
    uint16_t    sequence;
    uint16_t    length;
    uint16_t    command;
} cmd_pkt;

typedef struct data_pkt
{
    uint8_t     version;
    uint8_t     userId;
    uint16_t    sequence;
    uint16_t    length;
    uint16_t    command;
    uint8_t     data[DATA_SIZE];
} data_pkt;

#endif