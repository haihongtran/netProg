#include "client_utilities.h"

int openClientSock(const char* ipAddr, int portNum)
{
    int clientSock;
    struct sockaddr_in serverAddr;

    /* Create socket descriptor */
    if ( (clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Failed to create a socket");
        return -1;
    }

    /* Connect to server */
    memset((char*) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(portNum);
    serverAddr.sin_addr.s_addr  = inet_addr(ipAddr);

    if (connect(clientSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("[ERROR 0x0005] Cannot connect to server");
        return -1;
    }
    return clientSock;
}

void* clientSendFile(void* arg)
{
    argStruct threadArg = *(argStruct*) arg;
    int clientSock, nextSeq;
    data_pkt clientSendPkt;
    cmd_pkt clientRecvPkt;
    bool fileStored = false, storedError = false;
    char threadNum[7], fileName[10], message[20];

    /* Thread reaps itself after termination */
    pthread_detach(pthread_self());

    /* Open and connect client sock */
    clientSock = openClientSock(threadArg.ipAddr, PORT_NUMBER);
    if ( clientSock < 0 )
        return NULL;

    /* Convert thread number from integer into a string and store */
    snprintf(threadNum, sizeof(threadNum), "%u", threadArg.threadNum);

    /* Construct data to send */
    strcpy(message, "I am thread #");
    strcat(message, threadNum);

    /* Construct file name to send */
    strcpy(fileName, threadNum);
    strcat(fileName, ".txt");

    /* Randomize seed to create a random initial sequence number */
    srand(time(NULL));

    /* Initialize static fields of sending packet */
    clientSendPkt.version  = STATIC_VERSION;
    clientSendPkt.userId   = STATIC_USER_ID;

    /* Construct and send CLIENT_HELLO */
    sendDataPkt(clientSock, &clientSendPkt, rand() % MAX_SEQUENCE,
                HEADER_LENGTH, CLIENT_HELLO);

    while(1)
    {
        /* Read packet from server */
        if ( read(clientSock, &clientRecvPkt, sizeof(clientRecvPkt)) < 0 )
        {
            perror("Cannot read from socket");
            /* Construct and send ERROR message */
            nextSeq = ntohs(clientSendPkt.sequence) + 1;
            sendDataPkt(clientSock, &clientSendPkt, nextSeq, HEADER_LENGTH, ERROR);
            break;
        }
        /* Check command of server */
        switch ( ntohs(clientRecvPkt.command) )
        {
            case SERVER_HELLO:
                /* Copy message into data field of send packet */
                strcpy(clientSendPkt.data, message);

                /* Construct and send DATA_DELIVERY packet */
                nextSeq = ntohs(clientSendPkt.sequence) + 1;
                sendDataPkt(clientSock, &clientSendPkt, nextSeq,
                            HEADER_LENGTH + strlen(message), DATA_DELIVERY);
                break;
            case PKT_RECEIVED:
                /* Copy file name into data field of send packet */
                strcpy(clientSendPkt.data, fileName);

                /* Construct and send DATA_STORE packet */
                nextSeq = ntohs(clientSendPkt.sequence) + 1;
                sendDataPkt(clientSock, &clientSendPkt, nextSeq,
                            HEADER_LENGTH + strlen(fileName) + 1, DATA_STORE);
                break;
            case ERROR:
                printf("[ERROR 0x0005] Server cannot store data to buffer.\n");
                storedError = true;
                break;
            case FILE_STORED:
                fileStored = true;
                break;
            case STORED_ERROR:
                printf("Server cannot store requested data.\n");
                storedError = true;
                break;
            default:
                printf("Unrecognized command: 0x%04x\n", clientRecvPkt.command);
                break;
        }
        if (fileStored || storedError)
            break;
    }

    close(clientSock);
    return NULL;
}
