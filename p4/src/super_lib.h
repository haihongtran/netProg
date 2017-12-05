#ifndef __SUPER_LIB_H__
#define __SUPER_LIB_H__

#include "protocol.h"
#include "common.h"

#define HASH_TABLE_SIZE 100
#define CHILD_NUMBER 50

/* Structure to store file information from child nodes */
typedef struct fileInfoStoreStruct {
    char fileName[96];
    unsigned int fileSize;
    char ipAddr[20];
    unsigned int portNum;
} fileInfoStoreStruct;

/* Structure to store all information */
typedef struct fileInformationTable {
    unsigned int len;
    fileInfoStoreStruct* fileInfoList[HASH_TABLE_SIZE];
} fileInformationTable;

/* Structure to store port information from child node */
typedef struct childListeningPort {
    unsigned int id;
    unsigned int portNum;
} childListeningPort;

/*
 * Function: initHashTable()
 * Initialize file information table
 */
void initFileInfoTable(fileInformationTable* fileInfoTable);

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
int insertFileInfo(fileInfoStoreStruct* fileInfo,
    fileInformationTable* fileInfoTable);

/*
 * Function: searchFileInfo()
 * Search file info using fileName
 */
fileInfoStoreStruct* searchFileInfo(char* fileName,
    fileInfoStoreStruct** fileInfoList);

/*
 * Function: handleClientFds()
 * Super node handles client fds ready to be read
 */
void handleClientFds(fileDescriptorPool* fdPool, char* otherSuperIpAddr,
    int* otherSuperPortNum, fileInformationTable* fileInfoTable,
    childListeningPort* childPorts, unsigned int id);

/*
 * Function: handleClientRequest()
 * Handle each client request
 */
void handleClientRequest(int clientSock, char* otherSuperIpAddr,
    int* otherSuperPortNum, struct sockaddr_in* clientAddr,
    fileInformationTable* fileInfoTable, childListeningPort* childPorts,
    unsigned int id);

#endif  /*__SUPER_LIB_H__*/
