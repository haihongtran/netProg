#include "child_lib.h"

off_t getFileSize(const char *fileName) {
    struct stat st;
    /* Get file stats */
    if (stat(fileName, &st) == 0)
        return st.st_size;
    perror("Cannot get file stats");
    return -1;
}

void* childServer(void* arg) {
    threadArguments threadArgs = *(threadArguments*) arg;
    int serverSock, clientSock;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientSock);
    fileDescriptorPool fdPool;      /* Information pool about fd sets */

    /* Open non-blocking server socket */
    serverSock = openNBServerSock(threadArgs.portNum);
    if ( serverSock < 0 )
        exit(-1);

    /* Initialize file descriptor pool */
    initPool(serverSock, &fdPool);

    while(1) {
        fdPool.ready_set = fdPool.read_set;
        fdPool.nready = select(fdPool.maxfd + 1, &fdPool.ready_set, NULL, NULL, NULL);
        if ( fdPool.nready < 0 ) {
            perror("Select function has error");
            continue;
        }
        /* New client */
        if ( FD_ISSET(serverSock, &fdPool.ready_set) ) {
            if ( (clientSock = accept(serverSock, (struct sockaddr*) &clientAddr,
                    &clientAddrLen)) < 0 ) {
                perror("Cannot accept new client");
            }
            else {
                addClient(clientSock, &clientAddr, &fdPool);
            }
        }
        /* Connections in other sockets */
        if ( fdPool.nready ) {
            handleClientFds(&fdPool, threadArgs.id);
        }
    }
}

void handleClientFds(fileDescriptorPool* fdPool, unsigned int id) {
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
                handleClientRequest(clientSock, id);
            }
        }
    }
}

void handleClientRequest(int clientSock, unsigned int id) {
    fileReqPacket fileReqPkt;
    int fd, bytes_read, fileSize;
    char reqFileName[FILE_NAME_LEN_DIR] = {0};    /* ./data/<fileName> */
    unsigned int pktLen, usecs = 10000;
    headerPacket* fileResFailPkt;
    fileResSuccessPacket* fileResSuccessPkt;

    /* Read data from peer */
    bytes_read = read(clientSock, &fileReqPkt, sizeof(fileReqPacket));
    if ( bytes_read < 0 ) {
        perror("Cannot read from peer socket");
        return;
    }
    /* Parse message type */
    switch( ntohl(fileReqPkt.hdr.msgType) ) {
        case FILE_REQ:
            strcpy(reqFileName, "./data/");
            strcat(reqFileName, fileReqPkt.fileName);
            fileSize = getFileSize(reqFileName);
            /* Send back FILE_RES_FAIL if file not found */
            if ( fileSize < 0 ) {
                fileResFailPkt = (headerPacket*) malloc(HEADER_LEN);
                fileResFailPkt->totalLen = htonl(HEADER_LEN);
                fileResFailPkt->id = htonl(id);
                fileResFailPkt->msgType = htonl(FILE_RES_FAIL);
                write(clientSock, fileResFailPkt, HEADER_LEN);
                free(fileResFailPkt);
                break;
            }
            /* Open requested file */
            if ( (fd = open(reqFileName, O_RDONLY)) < 0 )
            {
                fileResFailPkt = (headerPacket*) malloc(HEADER_LEN);
                fileResFailPkt->totalLen = htonl(HEADER_LEN);
                fileResFailPkt->id = htonl(id);
                fileResFailPkt->msgType = htonl(FILE_RES_FAIL);
                write(clientSock, fileResFailPkt, HEADER_LEN);
                free(fileResFailPkt);
                break;
            }
            /* Construct FILE_RES_SUCCESS packet */
            fileResSuccessPkt = (fileResSuccessPacket*) malloc(sizeof(fileResSuccessPacket));
            fileResSuccessPkt->hdr.id = htonl(id);
            fileResSuccessPkt->hdr.msgType = htonl(FILE_RES_SUCCESS);
            while (1) {
                bytes_read = read(fd, fileResSuccessPkt->data, DATA_SIZE);
                if ( bytes_read == 0 ) { // EOF
                    break;
                }
                pktLen = HEADER_LEN + bytes_read;
                fileResSuccessPkt->hdr.totalLen = htonl(pktLen);
                write(clientSock, fileResSuccessPkt, pktLen);
                usleep(usecs);
            }
            free(fileResSuccessPkt);
            close(fd);
            break;
        default:
            // printf("Unexpected message. Its message type is 0x%08x\n", ntohl(fileReqPkt.hdr.msgType));
            break;
    }
}
