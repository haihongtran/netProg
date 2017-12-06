#include "child_lib.h"

int main(int argc, char* argv[]) {
    int clientSock;
    pthread_t tid;
    const unsigned int id = rand(); //TODO: rand() cannot generate random number
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
    /* Download file */
    char inputBuffer[200], reqFile[96], destFile[110];
    searchQueryPacket* searchQueryPkt;
    searchAnsSuccessPacket* searchAnsSuccessPkt;
    fileReqPacket* fileReqPkt;
    headerPacket* hdrFileResponse;
    uint8_t* receiveBuffer; // Store file data from peer
    bool fileNotFound = false;
    char peerIp[20];
    unsigned int peerPort;
    unsigned int fileSize;
    int storeFd;

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
    if ((dir = opendir ("./data")) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
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
    close(clientSock);

    //TODO: uncomment this to run the listening thread
    /* Thread arguments */
    // threadArgs.portNum = portNum;
    // threadArgs.id = id;
    // /* Run a thread to listen for peer connection */
    // if ( pthread_create(&tid, NULL, childServer, (void*)&threadArgs) != 0 ) {
    //     printf("Cannot create listening thread\n");
    //     return -1;
    // }

    while(1) {
        fileNotFound = false;
        /* Get input user request for file */
        printf("Input command: ");  // format: get reqFile destFile
        fgets(inputBuffer, 200, stdin);
        token = strtok(inputBuffer, " \n");  // read 'get'
        /* Get reqFile */
        token = strtok(NULL, " \n");
        memset(reqFile, '\0', 96);
        strcpy(reqFile, token);
        /* Get destFile */
        token = strtok(NULL, " \n");
        memset(destFile, '\0', 110);
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

        printf("The peer having requested file is %s:%u\n", peerIp, peerPort);
        continue;   //TODO: remove this continue to start download file

        /* Connect to peer */
        clientSock = openClientSock(peerIp, peerPort);
        if (clientSock < 0) {
            printf("Could not connect to peer\n");
            continue;
        }
        /* Allocate memory to receive file from peer */
        receiveBuffer = (uint8_t*) malloc(HEADER_LEN + fileSize);
        /* Construct FILE_REQ to send to peer */
        fileReqPkt = (fileReqPacket*) malloc(sizeof(fileReqPacket));
        fileReqPkt->hdr.totalLen = htonl(sizeof(fileReqPacket));
        fileReqPkt->hdr.id = htonl(id);
        fileReqPkt->hdr.msgType = htonl(FILE_REQ);
        strcpy(fileReqPkt->fileName, reqFile);
        /* Send FILE_REQ packet */
        write(clientSock, fileReqPkt, sizeof(fileReqPacket));
        free(fileReqPkt);
        /* Wait for reply from peer */
        read(clientSock, &receiveBuffer, HEADER_LEN + fileSize);
        /* Parse header of received packet */
        hdrFileResponse = (headerPacket*) malloc(HEADER_LEN);
        memcpy(hdrFileResponse, receiveBuffer, HEADER_LEN);
        switch ( ntohl(hdrFileResponse->msgType) ) {
            case FILE_RES_SUCCESS:
                if ( ntohl(hdrFileResponse->totalLen) != (HEADER_LEN + fileSize) ) {
                    printf("Total len is %u, while (HEADER_LEN + fileSize) = %u\n", ntohl(hdrFileResponse->totalLen), HEADER_LEN + fileSize);
                    break;
                }
                if ( (storeFd = open(destFile, O_WRONLY | O_CREAT, 0666)) < 0 ) {
                    perror("Could not open file to write");
                    break;
                }
                if ( write(storeFd, receiveBuffer + HEADER_LEN, fileSize) < 0 ) {
                    perror("Could not write new file");
                    close(storeFd);
                    break;
                }
                close(storeFd);
                break;
            case FILE_RES_FAIL:
                printf("Peer does not have the file %s\n", reqFile);
                break;
            default:
                printf("Unexpected message from peer. Its message type is 0x%08x\n", ntohl(hdrFileResponse->msgType));
                break;
        }
        free(hdrFileResponse);
        free(receiveBuffer);
    }

    // Delete this after implement the above while loop
    // pthread_join(tid, NULL);

    return 0;
}
