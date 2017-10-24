#include "client_utilities.h"
#include "common_utilities.h"

int main(int argc, char const *argv[])
{
    int clientSock, nextSeq;
    data_pkt clientSendPkt;
    cmd_pkt clientRecvPkt;
    bool fileStored = false, storedError = false;
    const char* msg = "I am thread #1";
    const char* fileName = "1.txt";

    /* Check command-line arguments */
    if (argc != 3)
    {
        printf("Please use \'./client [server_ip] [number_of_request]\'\n");
        return -1;
    }

    /* Open and connect client sock */
    clientSock = openClientSock(argv[1], PORT_NUMBER);
    if ( clientSock < 0 )
        return -1;

    /* Randomize seed to create a random initial sequence number */
    srand(time(NULL));

    /* Initialize static fields of sending packet */
    clientSendPkt.version  = STATIC_VERSION;
    clientSendPkt.userId   = STATIC_USER_ID;
    /* Construct and send CLIENT_HELLO */
    sendDataPkt(clientSock, &clientSendPkt, rand() * MAX_SEQUENCE,
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
                /* Copy msg into data field of send packet */
                strcpy(clientSendPkt.data, msg);

                /* Construct and send DATA_DELIVERY packet */
                nextSeq = ntohs(clientSendPkt.sequence) + 1;
                sendDataPkt(clientSock, &clientSendPkt, nextSeq,
                            HEADER_LENGTH + strlen(msg), DATA_DELIVERY);
                break;
            case PKT_RECEIVED:
                /* Copy file name into data field of send packet */
                strcpy(clientSendPkt.data, fileName);

                /* Construct and send DATA_STORE packet */
                nextSeq = ntohs(clientSendPkt.sequence) + 1;
                sendDataPkt(clientSock, &clientSendPkt, nextSeq,
                            HEADER_LENGTH + strlen(fileName), DATA_STORE);
                break;
            case ERROR:
                printf("[ERROR 0x0005] Server cannot store file to buffer.\n");
                storedError = true;
                break;
            case FILE_STORED:
                printf("The requested file is successfully transfered and stored.\n");
                fileStored = true;
                break;
            case STORED_ERROR:
                printf("The requested file cannot be stored.\n");
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
    return 0;
}
