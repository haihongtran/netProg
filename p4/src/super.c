#include "super_lib.h"

int main(int argc, char* argv[]) {
    int serverSock, clientSock;
    struct sockaddr_in clientAddr;
    unsigned int id;
    socklen_t clientAddrLen = sizeof(clientSock);
    fileDescriptorPool fdPool;      /* Information pool about fd sets */
    int i;
    childListeningPort childPorts[CHILD_NUMBER];
    unsigned int portNum = 0, otherSuperPortNum = 0;
    char otherSuperIpAddr[20] = {0};
    int retVal;
    int option_index;
    struct option long_options[] = {
        {"s_ip",   required_argument, 0, 'a'},
        {"s_port", required_argument, 0, 'b'}
    };
    helloSuperToSuperPacket helloSuperToSuperPkt;
    headerPacket helloSuperToSuperReply;
    fileInformationTable fileInfoTable;     // Place to store file information from child nodes
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
                strcpy(otherSuperIpAddr, optarg);
                break;
            case 'b':
                otherSuperPortNum = atoi(optarg);
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
    id = rand() * portNum;

    initFileInfoTable(&fileInfoTable);  // Initialize file information table

    /* Initialize child ports */
    for ( i = 0; i < CHILD_NUMBER; i++ ) {
        childPorts[i].portNum = 0;
    }

    /* Say hello to other super node */
    if ( otherSuperPortNum && otherSuperIpAddr[0] ) {
        /* Connect to the other super node */
        clientSock = openClientSock(otherSuperIpAddr, otherSuperPortNum);
        if ( clientSock < 0 )
            return -1;
        /* Construct hello from super to super packet */
        helloSuperToSuperPkt.hdr.totalLen = htonl(sizeof(helloSuperToSuperPacket));
        helloSuperToSuperPkt.hdr.id = htonl(id);
        helloSuperToSuperPkt.hdr.msgType = htonl(HELLO_SUPER_TO_SUPER);
        helloSuperToSuperPkt.portNum = htonl(portNum);
        /* Send hello packet to other super node */
        write(clientSock, &helloSuperToSuperPkt, sizeof(helloSuperToSuperPacket));
        /* Wait for reply */
        read(clientSock, &helloSuperToSuperReply, HEADER_LEN);
        if ( ntohl(helloSuperToSuperReply.msgType) != HELLO_SUPER_TO_SUPER ) {
            printf("Unexpected message from other super node. Its message type is 0x%08x.\n", ntohl(helloSuperToSuperReply.msgType));
            close(clientSock);
            return -1;
        }
        else
            printf("Received reply from other super node\n");
        /* Close socket */
        close(clientSock);
    }

    /* Open non-blocking server socket */
    serverSock = openNBServerSock(portNum);
    if ( serverSock < 0 )
        return -1;

    /* Initialize file descriptor pool */
    initPool(serverSock, &fdPool);
    printf("Initialized file descriptor pool...\n");

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
            handleClientFds(&fdPool, otherSuperIpAddr,
                &otherSuperPortNum, &fileInfoTable, childPorts, id);
        }
    }

    return 0;
}
