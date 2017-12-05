#include "super_lib.h"

void initFileInfoTable(fileInformationTable* fileInfoTable) {
    int i;
    for ( i = 0; i < HASH_TABLE_SIZE; i++ ) {
        fileInfoTable->fileInfoList[i] = NULL;
    }
    fileInfoTable->len = 0;
}

int hashFunction(char* key) {
    int i, sum = 0;
    for (i = 0; i < strlen(key); i++) {
        sum += key[i];
    }
    return (sum % HASH_TABLE_SIZE);
}

int insertFileInfo(fileInfoStoreStruct* fileInfo,
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

fileInfoStoreStruct* searchFileInfo(char* fileName,
    fileInfoStoreStruct** fileInfoList)
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

void handleClientFds(fileDescriptorPool* fdPool, char* otherSuperIpAddr,
    int* otherSuperPortNum, fileInformationTable* fileInfoTable,
    childListeningPort* childPorts, unsigned int id)
{
    int i, clientSock, nread;
    /* Find the FDs ready to be read */
    for ( i = 0; (i <= fdPool->maxi) && (fdPool->nready > 0); i++ ) {
        clientSock = fdPool->clientfd[i];
        if ( (clientSock > 0) && (FD_ISSET(clientSock, &fdPool->ready_set)) ) {
            fdPool->nready--;
            /* Check if there are data to read from client socket */
            if ( (ioctl(clientSock, FIONREAD, &nread)) < 0 ) {
                perror("ioctl() function has error");
                break;
            }
            if ( nread == 0 ) {  /* Connection close signal from client */
                close(clientSock);
                FD_CLR(clientSock, &fdPool->read_set);
                fdPool->clientfd[i] = -1;
            }
            else {  /* There are data from client */
                handleClientRequest(clientSock, otherSuperIpAddr,
                    otherSuperPortNum, &(fdPool->clientAddr[i]),
                    fileInfoTable, childPorts, id);
            }
        }
    }
}

void handleClientRequest(int clientSock, char* otherSuperIpAddr,
    int* otherSuperPortNum, struct sockaddr_in* clientAddr,
    fileInformationTable* fileInfoTable, childListeningPort* childPorts,
    unsigned int id)
{
    int i;
    unsigned int childPort = 0;
    char buffer[1020] = {0}; /* 1020 is approximately the length of the biggest possible packet: fileInfoPacket */
    int bytes_read;     /* Number of bytes read from client */
    headerPacket hdr;   /* Used to parse packet header */
    fileInfoStoreStruct* fileInfoStore;

    /* Possible packet */
    helloFromChildPacket* helloFromChildPkt = NULL;
    headerPacket* helloFromSuperPkt = NULL;
    helloSuperToSuperPacket* helloSuperToSuperPkt = NULL;
    fileInfoPacket* fileInfoPkt = NULL;
    searchQueryPacket* searchQueryPkt = NULL;
    searchAnsSuccessPacket* searchAnsSuccessPkt = NULL;
    fileInfoSharePacket* fileInfoSharePkt = NULL;
    fileInfoShareSuccessPacket* fileInfoShareSuccessPkt = NULL;
    headerPacket* fileInfoShareErrorPkt = NULL;

    /* Read data from client */
    bytes_read = read(clientSock, buffer, sizeof(buffer));
    if ( bytes_read < 0 ) {
        perror("Cannot read from client socket");
        exit(-1);
    }
    /* Parse header of packet */
    memcpy(&hdr, buffer, HEADER_LEN);
    switch( ntohl(hdr.msgType) ) {
        case HELLO_FROM_CHILD:
            printf("Received HELLO_FROM_CHILD packet\n");
            /* Allocate memory for the packet */
            helloFromChildPkt = (helloFromChildPacket*) malloc(sizeof(helloFromChildPacket));
            /* Fill in data */
            memcpy(helloFromChildPkt, buffer, sizeof(helloFromChildPacket));
            /* Find available place to store child port */
            for ( i = 0; i < CHILD_NUMBER; i++ ) {
                if ( (childPorts[i].portNum) )
                    continue;
                childPorts[i].id = ntohl(helloFromChildPkt->hdr.id);
                childPorts[i].portNum = ntohl(helloFromChildPkt->portNum);
                break;
            }
            free(helloFromChildPkt);
            /* Construct HELLO_FROM_SUPER message */
            helloFromSuperPkt = (headerPacket*) malloc(HEADER_LEN);
            helloFromSuperPkt->totalLen = htonl(HEADER_LEN);
            helloFromSuperPkt->id = htonl(id);
            helloFromSuperPkt->msgType = htonl(HELLO_FROM_SUPER);
            write(clientSock, helloFromSuperPkt, HEADER_LEN);
            free(helloFromSuperPkt);
            break;
        case HELLO_SUPER_TO_SUPER:  //TODO: consider sending back a hello message
            printf("Received HELLO_SUPER_TO_SUPER packet\n");
            /* Allocate memory for the packet */
            helloSuperToSuperPkt = (helloSuperToSuperPacket*) malloc(sizeof(helloSuperToSuperPacket));
            /* Fill in data */
            memcpy(helloSuperToSuperPkt, buffer, sizeof(helloSuperToSuperPacket));
            /* Save port number of other super node */
            *otherSuperPortNum = ntohl(helloSuperToSuperPkt->portNum);
            /* Save IP address of other super node */
            strcpy(otherSuperIpAddr, inet_ntoa(clientAddr->sin_addr));
            /* Free the memory allocated */
            free(helloSuperToSuperPkt);
            break;
        case FILE_INFO:
            printf("Received FILE_INFO packet\n");
            /* Find corersponding client port */
            for ( i = 0; i < CHILD_NUMBER; i++ ) {
                if ( childPorts[i].id == ntohl(hdr.id) ) {
                    childPort = childPorts[i].portNum;
                    break;
                }
            }
            if ( !childPort ) {
                printf("Could not find corersponding port number\n");
                return;
            }
            /* Allocate memory for the packet */
            fileInfoPkt = (fileInfoPacket*) malloc(sizeof(fileInfoPacket));
            /* Fill in data */
            memcpy(fileInfoPkt, buffer, ntohl(hdr.totalLen));
            /* Store file information */
            for ( i = 0; i < ntohl(fileInfoPkt->fileNum); i++ ) {
                /* Construct information */
                fileInfoStore = (fileInfoStoreStruct*) malloc(sizeof(fileInfoStoreStruct));
                strcpy(fileInfoStore->fileName, fileInfoPkt->files[i].fileName);
                fileInfoStore->fileSize = ntohl(fileInfoPkt->files[i].fileSize);
                strcpy(fileInfoStore->ipAddr, inet_ntoa(clientAddr->sin_addr));
                fileInfoStore->portNum = childPort;
                printf("File %d: %s\t%u bytes\tPort: %d\n", i+1, fileInfoStore->fileName, fileInfoStore->fileSize, fileInfoStore->portNum);
                /* Put file information into hash table */
                insertFileInfo(fileInfoStore, fileInfoTable);
            }
            //TODO: share info to other super node
            /* Construct packet to share information to other super node */
            // fileInfoSharePkt = (fileInfoSharePacket*) malloc(sizeof(fileInfoSharePacket));
            // memcpy(fileInfoSharePkt, buffer, ntohl(hdr.totalLen));
            // free(fileInfoSharePkt);
            free(fileInfoPkt);
            break;
        case SEARCH_QUERY:
            break;
        case FILE_INFO_SHARE:
            break;
        case FILE_INFO_SHARE_SUCCESS:
            break;
        case FILE_INFO_SHARE_ERROR:
            break;
        default:
            break;
    }
}
