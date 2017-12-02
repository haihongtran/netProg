#ifndef __SUPER_H__
#define __SUPER_H__

#define HASH_TABLE_SIZE 1000

typedef struct fileInfo {
    char fileName[255];
    unsigned int fileSize;
    char ipAddr[20];
    unsigned int portNum;
} fileInfo;

#endif  /*__SUPER_H__*/
