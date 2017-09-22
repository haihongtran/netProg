#include "protocol.h"

int main(int argc, char const *argv[])
{
    int clientSock, fd, bytes_read;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen;
    data_pkt clientSendPkt;
    data_pkt clientRecvPkt;
    bool fileStored = false;
    bool storedError = false;

    if (argc != 4)
    {
        printf("Please use \'./client server_ip port_number transfer_file\'\n");
        return -1;
    }

    if ( (fd = open(argv[3], O_RDONLY)) < 0 )
    {
        perror("Cannot open file");
        return -1;
    }

    /* Create socket descriptor */
    if ( (clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Failed to create a socket");
        return -1;
    }

    /* Connect to server */
    memset((char*) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr  = inet_addr(argv[1]);

    serverAddrLen = sizeof(serverAddr);
    if (connect(clientSock, (struct sockaddr*) &serverAddr, serverAddrLen) < 0)
    {
        perror("[ERROR 0x0005] Cannot connect to server");
        return -1;
    }

    /* Randomize seed to create a random initial sequence number */
    srand(time(NULL));

    //TODO: create a sendPkt function which has arguments cmd, pkt_len, etc.
    /* Construct CLIENT_HELLO message */
    clientSendPkt.version  = STATIC_VERSION;
    clientSendPkt.userId   = STATIC_USER_ID;
    clientSendPkt.sequence = htons(rand() % MAX_SEQUENCE);
    clientSendPkt.length   = htons(HEADER_LENGTH);
    clientSendPkt.command  = htons(CLIENT_HELLO);

    /* Send CLIENT_HELLO message */
    write(clientSock, &clientSendPkt, HEADER_LENGTH);

    while(1)
    {
        int tmp;
        if ( read(clientSock, &clientRecvPkt, sizeof(clientRecvPkt)) < 0 )
        {
            /* Construct ERROR message */
            tmp = ntohs(clientSendPkt.sequence) + 1;
            clientSendPkt.sequence  = htons(tmp);
            clientSendPkt.length    = htons(HEADER_LENGTH);
            clientSendPkt.command   = htons(ERROR);
            /* Send ERROR message to server */
            write(clientSock, &clientSendPkt, HEADER_LENGTH);
            perror("Cannot read from socket");
            break;
        }
        switch ( ntohs(clientRecvPkt.command) )
        {
            case SERVER_HELLO:
                bytes_read = read(fd, &clientSendPkt.data, DATA_SIZE);
                if ( bytes_read < 0 )
                {
                    /* Construct ERROR message */
                    tmp = ntohs(clientSendPkt.sequence) + 1;
                    clientSendPkt.sequence  = htons(tmp);
                    clientSendPkt.length    = htons(HEADER_LENGTH);
                    clientSendPkt.command   = htons(ERROR);
                    /* Send ERROR message to server */
                    write(clientSock, &clientSendPkt, HEADER_LENGTH);
                    perror("Cannot read the file");
                    break;
                }
                /* Construct first DATA_DELIVERY packet */
                tmp = ntohs(clientSendPkt.sequence) + 1;
                clientSendPkt.sequence  = htons(tmp);
                clientSendPkt.length    = htons(HEADER_LENGTH + bytes_read);
                clientSendPkt.command   = htons(DATA_DELIVERY);
                /* Send first DATA_DELIVERY packet to socket */
                write(clientSock, &clientSendPkt, HEADER_LENGTH + bytes_read);
                break;
            case PKT_RECEIVED:
                bytes_read = read(fd, &clientSendPkt.data, DATA_SIZE);
                if (bytes_read == 0) // EOF
                {
                    /* User enters file name */
                    char* fileName = (char*) malloc(MAX_NAME_LEN);
                    printf("Enter file name: ");
                    fgets(fileName, MAX_NAME_LEN, stdin);
                    /* Remove trailing newline, if there */
                    if ((strlen(fileName) > 0) && (fileName[strlen(fileName) - 1] == '\n'))
                        fileName[strlen(fileName) - 1] = '\0';
                    /* Construct and send DATA_STORE packet */
                    tmp = ntohs(clientSendPkt.sequence) + 1;
                    clientSendPkt.sequence  = htons(tmp);
                    clientSendPkt.length    = htons(HEADER_LENGTH + strlen(fileName) + 1);
                    clientSendPkt.command   = htons(DATA_STORE);
                    memcpy((void*) &clientSendPkt.data, fileName, strlen(fileName) + 1);
                    write(clientSock, &clientSendPkt, HEADER_LENGTH + strlen(fileName) + 1);
                    break;
                }
                else if (bytes_read < 0)
                {
                    perror("Cannot read the file");
                    /* Construct ERROR message */
                    tmp = ntohs(clientSendPkt.sequence) + 1;
                    clientSendPkt.sequence  = htons(tmp);
                    clientSendPkt.length    = htons(HEADER_LENGTH);
                    clientSendPkt.command   = htons(ERROR);
                    /* Send ERROR message to server */
                    write(clientSock, &clientSendPkt, HEADER_LENGTH);
                    break;
                }
                /* Construct DATA_DELIVERY packet */
                tmp = ntohs(clientSendPkt.sequence) + 1;
                clientSendPkt.sequence  = htons(tmp);
                clientSendPkt.length    = htons(HEADER_LENGTH + bytes_read);
                clientSendPkt.command   = htons(DATA_DELIVERY);
                /* Send DATA_DELIVERY packet to socket */
                write(clientSock, &clientSendPkt, HEADER_LENGTH + bytes_read);
                break;
            case ERROR:
                printf("[ERROR 0x0005] Server's memory is full.\n");
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
                break;
        }
        if (fileStored || storedError)
            break;
    }

    close(fd);
    close(clientSock);
    return 0;
}
