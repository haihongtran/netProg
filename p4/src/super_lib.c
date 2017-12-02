#include "super_lib.h"

int hashFunction(char* key) {
    int i, sum = 0;
    for (i = 0; i < strlen(key); i++) {
        sum += key[i];
    }
    return (sum % HASH_TABLE_SIZE);
}

int insertFileInfo(fileInformation* fileInfo,
    fileInformationTable* fileInfoTable)
{
    int index;
    /* Check if the table is full */
    if ( fileInfoTable->len == HASH_TABLE_SIZE ) {
        printf("There is no available space to add info\n");
        return -1;
    }
    /* Get the index */
    index = hashFunction(fileInfo->fileName);
    /* Move in the list until an empty cell is encountered */
    while ( fileInfoTable->fileInfoList[index] != NULL ) {
        index += 1;     /* Go to the next cell */
        index %= HASH_TABLE_SIZE;   /* Wrap around the list */
    }
    /* Store information */
    fileInfoTable->fileInfoList[index] = fileInfo;
    fileInfoTable->len += 1;
    return 0;
}

fileInformation* searchFileInfo(char* fileName,
    fileInformation** fileInfoList)
{
    /* Get the hash */
    int index = hashFunction(fileName);
    /* Find the file info */
    while ( fileInfoList[index] != NULL ) {
        /* Check if the keys match */
        if ( strncmp(fileInfoList[index]->fileName, fileName, strlen(fileName)) == 0 )
            return fileInfoList[index];
        index += 1;     /* Move to next cell */
        index %= HASH_TABLE_SIZE;   /* Wrap around the list */
    }
    /* Not found */
    return NULL;
}
