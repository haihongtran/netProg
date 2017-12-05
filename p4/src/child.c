#include "child_lib.h"

int main(int argc, char* argv[]) {
    int clientSock;
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
    DIR* dir;
    struct dirent* ent;
    unsigned int fileCnt = 0;
    unsigned int pktLen = 0;
    char fileNameWithDir[120] = {0};
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
        printf("Access to super node is denied\n");
        return -1;
    }
    else
        printf("Access granted\n");

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

    return 0;
}
