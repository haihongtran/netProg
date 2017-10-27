#include "client_utilities.h"
#include "common_utilities.h"

int main(int argc, char const *argv[])
{
    int clientSock, fd, bytes_read, nextSeq;
    data_pkt clientSendPkt;
    cmd_pkt clientRecvPkt;
    bool fileStored = false, storedError = false;

    /* Check command-line arguments */
    if (argc != 4)
    {
        printf("Please use \'./client server_ip port_number file_name_with_no_space\'\n");
        return -1;
    }

    /* Open required file */
    if ( (fd = open(argv[3], O_RDONLY)) < 0 )
    {
        perror("Cannot open file");
        return -1;
    }

    /* Open and connect client sock */
    clientSock = openClientSock(argv[1], atoi(argv[2]));
    if ( clientSock < 0 )
        return -1;

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
                /* Read data from file */
                bytes_read = read(fd, clientSendPkt.data, DATA_SIZE);
                if ( bytes_read < 0 )
                {
                    perror("Cannot read the file");
                    /* Construct and send ERROR message */
                    nextSeq = ntohs(clientSendPkt.sequence) + 1;
                    sendDataPkt(clientSock, &clientSendPkt, nextSeq, HEADER_LENGTH, ERROR);
                    break;
                }
                /* Construct and send first DATA_DELIVERY packet */
                nextSeq = ntohs(clientSendPkt.sequence) + 1;
                sendDataPkt(clientSock, &clientSendPkt, nextSeq,
                            HEADER_LENGTH + bytes_read, DATA_DELIVERY);
                break;
            case PKT_RECEIVED:
                /* Read data from file */
                bytes_read = read(fd, clientSendPkt.data, DATA_SIZE);
                if (bytes_read == 0) // EOF
                {
                    /* Construct and send DATA_STORE packet */
                    memcpy((void*) clientSendPkt.data, argv[3], strlen(argv[3]) + 1);
                    nextSeq = ntohs(clientSendPkt.sequence) + 1;
                    sendDataPkt(clientSock, &clientSendPkt, nextSeq,
                                HEADER_LENGTH + strlen(argv[3]) + 1, DATA_STORE);
                    break;
                }
                else if (bytes_read < 0)
                {
                    perror("Cannot read the file");
                    /* Construct and send ERROR message */
                    nextSeq = ntohs(clientSendPkt.sequence) + 1;
                    sendDataPkt(clientSock, &clientSendPkt, nextSeq, HEADER_LENGTH, ERROR);
                    break;
                }
                /* Construct and send DATA_DELIVERY packet */
                nextSeq = ntohs(clientSendPkt.sequence) + 1;
                sendDataPkt(clientSock, &clientSendPkt, nextSeq,
                            HEADER_LENGTH + bytes_read, DATA_DELIVERY);
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

    close(fd);
    close(clientSock);
    return 0;
}
