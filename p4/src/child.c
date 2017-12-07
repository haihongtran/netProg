#include "child_lib.h"

int main(int argc, char* argv[]) {
    int clientSock;
    pthread_t tid;
    unsigned int id;
    unsigned int portNum = 0, superPortNum = 0;
    char superIpAddr[20] = {0};
    int retVal;
    int option_index;
    struct option long_options[] = {
        {"s_ip",   required_argument, 0, 'a'},
        {"s_port", required_argument, 0, 'b'}
    };
    helloFromChildPacket helloFromChildPkt;
    headerPacket helloFromSuperPkt;
    fileInfoPacket fileInfoPkt;
    fileInfoRecvSuccessPacket fileInfoRecvSuccessPkt;

    /* Download file */
    char inputBuffer[200], reqFile[FILE_NAME_LEN], destFile[FILE_NAME_LEN_DIR];
    searchQueryPacket* searchQueryPkt;
    searchAnsSuccessPacket* searchAnsSuccessPkt;
    fileReqPacket* fileReqPkt;
    uint8_t* fileBuffer; // Store file data from peer
    bool fileNotFound = false;
    char peerIp[20];
    unsigned int peerPort;
    unsigned int fileSize;
    int storeFd;
    fileResSuccessPacket* fileResSuccessPkt;
    int bytes_read;
    int bytes_stored;
    int dataLen;
    bool fileResFail = false;

    DIR* dir;
    struct dirent* ent;
    unsigned int fileCnt = 0;
    unsigned int pktLen = 0;
    char fileNameWithDir[120] = {0};
    threadArguments threadArgs;
    char* token;
    /* Check command-line arguments */
    if (argc != 7) {
        printf("Please use './child -p portNum --s_ip superNodeIp --s_port superNodePort'\n");
        return -1;
    }
    /* Start parsing arguments */
    while (1) {
        retVal = getopt_long(argc, argv, "p:a:b:", long_options, &option_index);
        /* If all arguments have been parsed, break */
        if ( retVal == -1 )
            break;
        switch (retVal)
        {
            case 'p':
                portNum = atoi(optarg);
                break;
            case 'a':
                strcpy(superIpAddr, optarg);
                break;
            case 'b':
                superPortNum = atoi(optarg);
                break;
            case '?':
                printf("Undefined option\n");
                break;
            default:
                printf("Unknown\n");
                break;
        }
    }

    /* ID of the node */
    id = getRandNum() * portNum;

    /* Connect to corresponding super node */
    clientSock = openClientSock(superIpAddr, superPortNum);
    if ( clientSock < 0 )
        return -1;
    /* Construct HELLO_FROM_CHILD packet */
    helloFromChildPkt.hdr.totalLen = htonl(sizeof(helloFromChildPacket));
    helloFromChildPkt.hdr.id = htonl(id);
    helloFromChildPkt.hdr.msgType = htonl(HELLO_FROM_CHILD);
    helloFromChildPkt.portNum = htonl(portNum);
    /* Send HELLO_FROM_CHILD packet */
    write(clientSock, &helloFromChildPkt, sizeof(helloFromChildPacket));
    /* Wait for HELLO_FROM_SUPER message from super node */
    read(clientSock, &helloFromSuperPkt, HEADER_LEN);
    if ( ntohl(helloFromSuperPkt.msgType) != HELLO_FROM_SUPER ) {
        printf("Function read() returned. Not received HELLO_FROM_SUPER\n");
        return -1;
    }
    else
        printf("Received HELLO_FROM_SUPER. Access granted\n");

    /* Construct file info packet */
    fileInfoPkt.hdr.id = htonl(id);
    fileInfoPkt.hdr.msgType = htonl(FILE_INFO);
    if ( (dir = opendir ("./data")) != NULL ) {
        while ( (ent = readdir (dir)) != NULL ) {
            /* Skipping '.' and '..' files */
            if ( (strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0) )
                continue;
            /* Copy file name and file size into file info packet */
            strcpy(fileInfoPkt.files[fileCnt].fileName, ent->d_name);
            strcpy(fileNameWithDir, "./data/");
            strcat(fileNameWithDir, ent->d_name);
            fileInfoPkt.files[fileCnt].fileSize = htonl(getFileSize(fileNameWithDir));
            fileCnt++;
        }
        closedir (dir);
    }
    else {
        perror ("Could not open directory");
    }
    fileInfoPkt.fileNum = htonl(fileCnt);
    pktLen = HEADER_LEN + sizeof(fileInfoPkt.fileNum) + fileCnt*sizeof(fileInformation);
    fileInfoPkt.hdr.totalLen = htonl(pktLen);
    /* Send file info packet to super node */
    write(clientSock, &fileInfoPkt, pktLen);
    printf("Sending file information to super node\n");
    /* Waiting for reply from super node */
    read(clientSock, &fileInfoRecvSuccessPkt, sizeof(fileInfoRecvSuccessPkt));
    switch ( ntohl(fileInfoRecvSuccessPkt.hdr.msgType) ) {
        case FILE_INFO_RECV_SUCCESS:
            printf("Received FILE_INFO_RECV_SUCCESS packet from super node\n");
            break;
        case FILE_INFO_RECV_ERROR:
            printf("Received FILE_INFO_RECV_ERROR packet from super node\n");
            close(clientSock);
            return -1;
        default:
            printf("Unexpected message from super node. Its message type is 0x%08x.", ntohl(fileInfoRecvSuccessPkt.hdr.msgType));
            close(clientSock);
            return -1;
    }
    close(clientSock);

    /* Thread arguments */
    threadArgs.portNum = portNum;
    threadArgs.id = id;
    /* Run a thread to listen for peer connection */
    if ( pthread_create(&tid, NULL, childServer, (void*)&threadArgs) != 0 ) {
        printf("Cannot create listening thread\n");
        return -1;
    }

    printf("Command format: 'get [file_name] [destination_file_name]'\n");

    while(1) {
        fileNotFound = false;
        /* Get input user request for file */
        printf("Input command: ");  // format: get reqFile destFile
        fgets(inputBuffer, 200, stdin);
        token = strtok(inputBuffer, " \n");  // read 'get'
        if ( strcmp(token, "get") != 0 ) {
            printf("Wrong command. Please enter 'get [file_name] [destination_file_name]'\n");
            continue;
        }
        /* Get reqFile */
        token = strtok(NULL, " \n");
        memset(reqFile, '\0', FILE_NAME_LEN);
        strcpy(reqFile, token);
        /* Get destFile */
        token = strtok(NULL, " \n");
        memset(destFile, '\0', FILE_NAME_LEN_DIR);
        strcpy(destFile, "./download/");
        strcat(destFile, token);
        printf("Request file name is %s\n", reqFile);
        printf("Destination file name is %s\n", destFile);
        /* Connect to super node */
        clientSock = openClientSock(superIpAddr, superPortNum);
        if ( clientSock < 0 ) {
            printf("Could not connect to super node\n");
            continue;
        }
        /* Allocate memory for SEARCH_ANS_SUCCESS packet */
        searchAnsSuccessPkt = (searchAnsSuccessPacket*) malloc(sizeof(searchAnsSuccessPacket));
        /* Construct SEARCH_QUERY packet */
        searchQueryPkt = (searchQueryPacket*) malloc(sizeof(searchQueryPacket));
        searchQueryPkt->hdr.totalLen = htonl(sizeof(searchQueryPacket));
        searchQueryPkt->hdr.id = htonl(id);
        searchQueryPkt->hdr.msgType = htonl(SEARCH_QUERY);
        strcpy(searchQueryPkt->fileName, reqFile);
        /* Send SEARCH_QUERY packet */
        write(clientSock, searchQueryPkt, sizeof(searchQueryPacket));
        printf("Sending SEARCH_QUERY packet for file %s\n", searchQueryPkt->fileName);
        free(searchQueryPkt);
        /* Wait for answer from super node */
        read(clientSock, searchAnsSuccessPkt, sizeof(searchAnsSuccessPacket));
        /* Parse packet received from super node */
        switch( ntohl(searchAnsSuccessPkt->hdr.msgType) ) {
            case SEARCH_ANS_SUCCESS:
                printf("Received SEARCH_ANS_SUCCESS packet from super node\n");
                memset(peerIp, 0, 20);
                strcpy(peerIp, searchAnsSuccessPkt->ipAddr);
                peerPort = ntohl(searchAnsSuccessPkt->portNum);
                fileSize = ntohl(searchAnsSuccessPkt->fileSize);
                printf("The peer having requested file is %s:%u\n", peerIp, peerPort);
                printf("Requested file size is %d\n", fileSize);
                break;
            case SEARCH_ANS_FAIL:
                printf("Received SEARCH_ANS_FAIL packet from super node. File %s not found\n", reqFile);
                fileNotFound = true;
                break;
            default:
                printf("Unexpected message from super node. Its message type is 0x%08x\n", ntohl(searchAnsSuccessPkt->hdr.msgType));
                fileNotFound = true;
                break;
        }
        free(searchAnsSuccessPkt);
        close(clientSock);  /* Close socket to super node */
        /* If file not found, return to the beginning of the loop */
        if (fileNotFound)
            continue;

        /* Connect to peer */
        clientSock = openClientSock(peerIp, peerPort);
        if (clientSock < 0) {
            printf("Could not connect to peer\n");
            continue;
        }
        printf("Connected to peer\n");
        /* Construct FILE_REQ to send to peer */
        fileReqPkt = (fileReqPacket*) malloc(sizeof(fileReqPacket));
        fileReqPkt->hdr.totalLen = htonl(sizeof(fileReqPacket));
        fileReqPkt->hdr.id = htonl(id);
        fileReqPkt->hdr.msgType = htonl(FILE_REQ);
        strcpy(fileReqPkt->fileName, reqFile);
        /* Send FILE_REQ packet */
        write(clientSock, fileReqPkt, sizeof(fileReqPacket));
        printf("Sending FILE_REQ to peer\n");
        free(fileReqPkt);

        bytes_stored = 0;
        fileResFail = false;
        /* Allocate memory for FILE_RES_SUCCESS packet */
        fileResSuccessPkt = (fileResSuccessPacket*) malloc(sizeof(fileResSuccessPacket));
        /* Buffer to store data received from peer */
        fileBuffer = (uint8_t*) malloc(fileSize);
        /* Receive file from peer */
        while(1) {
            bytes_read = read(clientSock, fileResSuccessPkt, sizeof(fileResSuccessPacket));
            if ( bytes_read < 0 ) {
                perror("Cannot read from peer socket");
                break;
            }
            switch ( ntohl(fileResSuccessPkt->hdr.msgType) ) {
                case FILE_RES_SUCCESS:
                    dataLen = ntohl(fileResSuccessPkt->hdr.totalLen) - HEADER_LEN;
                    memcpy(fileBuffer + bytes_stored, fileResSuccessPkt->data, dataLen);
                    bytes_stored += dataLen;
                    break;
                case FILE_RES_FAIL:
                    printf("Received FILE_RES_FAIL packet from peer\n");
                    fileResFail = true;
                    break;
                default:
                    printf("Unexpected message. Its message type is 0x%08x\n", ntohl(fileResSuccessPkt->hdr.msgType));
                    fileResFail = true;
                    break;
            }
            if ( fileResFail )
                break;
            if ( bytes_stored == fileSize )
                break;
        }
        if (fileResFail)
            continue;
        
        if ( (storeFd = open(destFile, O_WRONLY | O_CREAT, 0755)) < 0 ) {
            perror("Could not open file to write");
            free(fileBuffer);
            continue;
        }
        if ( write(storeFd, fileBuffer, fileSize) < 0 ) {
            perror("Could not write new file");
            free(fileBuffer);
            close(storeFd);
            continue;
        }
        printf("File stored\n");
        free(fileBuffer);
        close(storeFd);
    }

    return 0;
}
