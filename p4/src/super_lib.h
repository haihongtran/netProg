#ifndef __SUPER_LIB_H__
#define __SUPER_LIB_H__

#include "protocol.h"
#include "common.h"

#define HASH_TABLE_SIZE 10000

/* Structure to store file information from child nodes */
typedef struct superFileInfoStruct {
    char fileName[96];
    unsigned int fileSize;
    char ipAddr[20];
    unsigned int portNum;
} superFileInfoStruct;

/* Structure to store all information */
typedef struct fileInformationTable {
    unsigned int len;
    superFileInfoStruct* fileInfoList[HASH_TABLE_SIZE];
} fileInformationTable;

/*
 * Function: hashFunction()
 * Calculate index to store data
 * @key param is file name in this case
 */
int hashFunction(char* key);

/*
 * Function: insertFileInfo()
 * Insert file info into the list
 */
int insertFileInfo(superFileInfoStruct* fileInfo,
    fileInformationTable* fileInfoTable);

/*
 * Function: searchFileInfo()
 * Search file info using fileName
 */
superFileInfoStruct* searchFileInfo(char* fileName,
    superFileInfoStruct** fileInfoList);

/*
 * Function: handleClientFds()
 * Super node handles client fds ready to be read
 */
void handleClientFds(fileDescriptorPool* fdPool, char* otherSuperIpAddr, int* otherSuperPortNum);

/*
 * Function: handleClientRequest()
 * Handle each client request
 */
void handleClientRequest(int clientSock, char* otherSuperIpAddr,
    int* otherSuperPortNum, struct sockaddr_in* clientAddr);

#endif  /*__SUPER_LIB_H__*/
