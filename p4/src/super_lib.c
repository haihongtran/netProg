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
    int i, socketOtherSuper = 0;
    unsigned int pktLen = 0;
    unsigned int childPort = 0;
    char recvBuff[1020] = {0}; /* 1020 is approximately the of the biggest possible packet: fileInfoPacket */
    int bytes_read;     /* Number of bytes read from client */
    headerPacket hdr;   /* Used to parse packet header */
    fileInfoStoreStruct* fileInfoStore;

    /* Possible packet */
    helloFromChildPacket* helloFromChildPkt = NULL;
    headerPacket* helloFromSuperPkt = NULL;
    helloSuperToSuperPacket* helloSuperToSuperPkt = NULL;
    headerPacket* helloSuperToSuperReply = NULL;
    fileInfoPacket* fileInfoPkt = NULL;
    searchQueryPacket* searchQueryPkt = NULL;
    searchAnsSuccessPacket* searchAnsSuccessPkt = NULL;
    headerPacket* searchAnsFailPkt = NULL;
    fileInfoSharePacket* fileInfoSharePkt = NULL;
    fileInfoShareSuccessPacket* fileInfoShareSuccessPkt = NULL;
    headerPacket* fileInfoShareErrorPkt = NULL;
    headerPacket* hdrInfoShare; /* Used to parse return packet of FILE_INFO_SHARE message */
    char bufferInfoShare[1260] = {0};   /* 1260 is approximately length of FILE_INFO_SHARE message */

    /* Read data from client */
    bytes_read = read(clientSock, recvBuff, sizeof(recvBuff));
    if ( bytes_read < 0 ) {
        perror("Cannot read from client socket");
        return;
    }
    /* Parse header of packet */
    memcpy(&hdr, recvBuff, HEADER_LEN);
    switch( ntohl(hdr.msgType) ) {
        case HELLO_FROM_CHILD:
            printf("Received HELLO_FROM_CHILD packet\n");
            /* Allocate memory for the packet */
            helloFromChildPkt = (helloFromChildPacket*) malloc(sizeof(helloFromChildPacket));
            /* Fill in data */
            memcpy(helloFromChildPkt, recvBuff, sizeof(helloFromChildPacket));
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
            printf("Sending HELLO_FROM_SUPER packet to child node\n");
            free(helloFromSuperPkt);
            break;
        case HELLO_SUPER_TO_SUPER:
            printf("Received HELLO_SUPER_TO_SUPER packet\n");
            /* Allocate memory for the packet */
            helloSuperToSuperPkt = (helloSuperToSuperPacket*) malloc(sizeof(helloSuperToSuperPacket));
            /* Fill in data */
            memcpy(helloSuperToSuperPkt, recvBuff, sizeof(helloSuperToSuperPacket));
            /* Save port number of other super node */
            *otherSuperPortNum = ntohl(helloSuperToSuperPkt->portNum);
            /* Save IP address of other super node */
            strcpy(otherSuperIpAddr, inet_ntoa(clientAddr->sin_addr));
            /* Free the memory allocated */
            free(helloSuperToSuperPkt);
            /* Construct reply */
            helloSuperToSuperReply = (headerPacket*) malloc(HEADER_LEN);
            helloSuperToSuperReply->totalLen = ntohl(HEADER_LEN);
            helloSuperToSuperReply->id = ntohl(id);
            helloSuperToSuperReply->msgType = ntohl(HELLO_SUPER_TO_SUPER);
            /* Send reply */
            write(clientSock, helloSuperToSuperReply, HEADER_LEN);
            printf("Replying HELLO_SUPER_TO_SUPER to other super node\n");
            free(helloSuperToSuperReply);
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
            /* Allocate memory to hold data of the FILE_INFO message */
            fileInfoPkt = (fileInfoPacket*) malloc(sizeof(fileInfoPacket));
            /* Fill in data */
            memcpy(fileInfoPkt, recvBuff, ntohl(hdr.totalLen));
            /* Construct FILE_INFO_SHARE message */
            fileInfoSharePkt = (fileInfoSharePacket*) malloc(sizeof(fileInfoSharePacket));
            fileInfoSharePkt->hdr.id = htonl(id);
            fileInfoSharePkt->hdr.msgType = htonl(FILE_INFO_SHARE);
            pktLen = HEADER_LEN + sizeof(unsigned int) + ntohl(fileInfoPkt->fileNum)*sizeof(fileInfoStoreStruct);
            fileInfoSharePkt->hdr.totalLen = htonl(pktLen);
            fileInfoSharePkt->fileNum = fileInfoPkt->fileNum;   // Copy directly the network format (Big Endian)
            /* Store file information */
            for ( i = 0; i < ntohl(fileInfoPkt->fileNum); i++ ) {
                /* Construct information */
                fileInfoStore = (fileInfoStoreStruct*) malloc(sizeof(fileInfoStoreStruct));
                strcpy(fileInfoStore->fileName, fileInfoPkt->files[i].fileName);
                fileInfoStore->fileSize = ntohl(fileInfoPkt->files[i].fileSize);
                strcpy(fileInfoStore->ipAddr, inet_ntoa(clientAddr->sin_addr));
                fileInfoStore->portNum = childPort;
                /* Put file information into hash table */
                insertFileInfo(fileInfoStore, fileInfoTable);
                /* Copy data to FILE_INFO_SHARE message */
                strcpy(fileInfoSharePkt->files[i].fileName, fileInfoStore->fileName);
                fileInfoSharePkt->files[i].fileSize = htonl(fileInfoStore->fileSize);
                strcpy(fileInfoSharePkt->files[i].ipAddr, fileInfoStore->ipAddr);
                fileInfoSharePkt->files[i].portNum = htonl(fileInfoStore->portNum);
            }
            //TODO: send FILE_INFO_RECV_SUCCESS back to child
            free(fileInfoPkt);
            //TODO: consider sending FILE_INFO_RECV_ERROR back to child
            /* Connect to other super node */
            socketOtherSuper = openClientSock(otherSuperIpAddr, *otherSuperPortNum);
            if ( socketOtherSuper < 0 ) {
                printf("Cannot connect to other super node to share file info\n");
                free(fileInfoSharePkt);
                break;
            }
            /* Send FILE_INFO_SHARE packet */
            write(socketOtherSuper, fileInfoSharePkt, pktLen);
            printf("Sending FILE_INFO_SHARE packet to other super node\n");
            free(fileInfoSharePkt);
            /* Waiting for reply */
            bytes_read = read(socketOtherSuper, bufferInfoShare, sizeof(bufferInfoShare));
            if ( bytes_read < 0 ) {
                perror("Cannot read from client socket");
                close(socketOtherSuper);
                exit(-1);
            }
            hdrInfoShare = (headerPacket*) malloc(HEADER_LEN);
            memcpy(hdrInfoShare, bufferInfoShare, HEADER_LEN);
            if ( ntohl(hdrInfoShare->msgType) == FILE_INFO_SHARE_SUCCESS )
                printf("Received FILE_INFO_SHARE_SUCCESS from other super node\n");
            else if ( ntohl(hdrInfoShare->msgType) == FILE_INFO_SHARE_ERROR )
                printf("Received FILE_INFO_SHARE_ERROR from other super node\n");
            else
                printf("Unknown message. It msgType file is %u\n", ntohl(hdrInfoShare->msgType));
            free(hdrInfoShare);
            close(socketOtherSuper);
            break;
        case SEARCH_QUERY:
            printf("Received SEARCH_QUERY packet from peer\n");
            /* Allocate memory for the packet */
            searchQueryPkt = (searchQueryPacket*) malloc(sizeof(searchQueryPacket));
            /* Fill in data */
            memcpy(searchQueryPkt, recvBuff, ntohl(hdr.totalLen));
            /* Search information of the requested file */
            fileInfoStore = searchFileInfo(searchQueryPkt->fileName,
                                           fileInfoTable->fileInfoList);
            free(searchQueryPkt);
            if ( !fileInfoStore ) {
                printf("Cannot find the file %s\n", searchQueryPkt->fileName);
                /* Construct SEARCH_ANS_FAIL packet */
                searchAnsFailPkt = (headerPacket*) malloc(HEADER_LEN);
                searchAnsFailPkt->totalLen = htonl(HEADER_LEN);
                searchAnsFailPkt->id = htonl(id);
                searchAnsFailPkt->msgType = htonl(SEARCH_ANS_FAIL);
                /* Send SEARCH_ANS_FAIL packet to child node */
                write(clientSock, searchAnsFailPkt, HEADER_LEN);
                printf("Sending SEARCH_ANS_FAIL packet to child node\n");
                free(searchAnsFailPkt);
                break;
            }
            /* Construct SEARCH_ANS_SUCCESS packet */
            searchAnsSuccessPkt = (searchAnsSuccessPacket*) malloc(sizeof(searchAnsSuccessPacket));
            searchAnsSuccessPkt->hdr.totalLen = htonl(sizeof(searchAnsSuccessPacket));
            searchAnsSuccessPkt->hdr.id = htonl(id);
            searchAnsSuccessPkt->hdr.msgType = htonl(SEARCH_ANS_SUCCESS);
            strcpy(searchAnsSuccessPkt->ipAddr, fileInfoStore->ipAddr);
            searchAnsSuccessPkt->portNum = htonl(fileInfoStore->portNum);
            searchAnsSuccessPkt->fileSize = htonl(fileInfoStore->fileSize);
            /* Send SEARCH_ANS_SUCCESS packet to child node */
            write(clientSock, searchAnsSuccessPkt, sizeof(searchAnsSuccessPacket));
            printf("Sending back SEARCH_ANS_SUCCESS packet to child node\n");
            free(searchAnsSuccessPkt);
            break;
        case FILE_INFO_SHARE:
            printf("Received FILE_INFO_SHARE packet from other super node\n");
            if ( ntohl(hdr.totalLen) > bytes_read ) {
                printf("Not receiving all contents from other super node. Sending back FILE_INFO_SHARE_ERROR message\n");
                fileInfoShareErrorPkt = (headerPacket*) malloc(HEADER_LEN);
                fileInfoShareErrorPkt->totalLen = htonl(HEADER_LEN);
                fileInfoShareErrorPkt->id = htonl(id);
                fileInfoShareErrorPkt->msgType = htonl(FILE_INFO_SHARE_ERROR);
                write(clientSock, &fileInfoShareErrorPkt, HEADER_LEN);
                printf("Sending FILE_INFO_SHARE_ERROR packet\n");
                free(fileInfoShareErrorPkt);
                break;
            }
            /* Allocate memory for the packet */
            fileInfoSharePkt = (fileInfoSharePacket*) malloc(sizeof(fileInfoSharePacket));
            /* Fill in data */
            memcpy(fileInfoSharePkt, recvBuff, ntohl(hdr.totalLen));
            fileInfoShareSuccessPkt = (fileInfoShareSuccessPacket*) malloc(sizeof(fileInfoShareSuccessPacket));
            fileInfoShareSuccessPkt->hdr.totalLen = fileInfoSharePkt->hdr.totalLen; // Copy directly network format
            fileInfoShareSuccessPkt->hdr.id = htonl(id);
            fileInfoShareSuccessPkt->hdr.msgType = htonl(FILE_INFO_SHARE_SUCCESS);
            fileInfoShareSuccessPkt->fileNum = fileInfoSharePkt->fileNum;   // Copy directly network format
            for ( i = 0; i < ntohl(fileInfoSharePkt->fileNum); i++) {
                /* Construct information */
                fileInfoStore = (fileInfoStoreStruct*) malloc(sizeof(fileInfoStoreStruct));
                strcpy(fileInfoStore->fileName, fileInfoSharePkt->files[i].fileName);
                fileInfoStore->fileSize = ntohl(fileInfoSharePkt->files[i].fileSize);
                strcpy(fileInfoStore->ipAddr, fileInfoSharePkt->files[i].ipAddr);
                fileInfoStore->portNum = ntohl(fileInfoSharePkt->files[i].portNum);
                /* Put file information into hash table */
                insertFileInfo(fileInfoStore, fileInfoTable);
                /* Update FILE_INFO_SUCCESS packet */
                strcpy(fileInfoShareSuccessPkt->files[i].fileName, fileInfoStore->fileName);
                fileInfoShareSuccessPkt->files[i].fileSize = htonl(fileInfoStore->fileSize);
                strcpy(fileInfoShareSuccessPkt->files[i].ipAddr, fileInfoStore->ipAddr);
                fileInfoShareSuccessPkt->files[i].portNum = htonl(fileInfoStore->portNum);
            }
            /* Reply to the other super node with FILE_INFO_SUCCESS message */
            write(clientSock, fileInfoShareSuccessPkt, ntohl(fileInfoShareSuccessPkt->hdr.totalLen));
            printf("Sending FILE_INFO_SHARE_SUCCESS packet back to other super node\n");
            free(fileInfoSharePkt);
            break;
        default:
            printf("Unexpected message. Its message type is 0x%08x\n", ntohl(hdr.msgType));
            break;
    }
}
