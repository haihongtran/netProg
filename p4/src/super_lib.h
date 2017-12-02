#ifndef __SUPER_LIB_H__
#define __SUPER_LIB_H__

#include "protocol.h"
#include "common.h"

#define HASH_TABLE_SIZE 10000

/* Structure to store file information from child nodes */
typedef struct fileInformation {
    char fileName[256];
    unsigned int fileSize;
    char ipAddr[20];
    unsigned int portNum;
} fileInformation;

/* Structure to store all information */
typedef struct fileInformationTable {
    unsigned int len;
    fileInformation* fileInfoList[HASH_TABLE_SIZE];
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
int insertFileInfo(fileInformation* fileInfo,
    fileInformationTable* fileInfoTable);

/*
 * Function: searchFileInfo()
 * Search file info using fileName
 */
fileInformation* searchFileInfo(char* fileName,
    fileInformation** fileInfoList);

#endif  /*__SUPER_LIB_H__*/
