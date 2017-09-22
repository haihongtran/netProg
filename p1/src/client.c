#include "protocol.h"

int main(int argc, char const *argv[])
{
    int fd;
    int clientSock;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen;
    data_pkt clientSendDataPkt;
    data_pkt clientRecvPkt;
    int bytes_read;
    int read_file_complete = 0;

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
    clientSendDataPkt.version  = STATIC_VERSION;
    clientSendDataPkt.userId   = STATIC_USER_ID;
    clientSendDataPkt.sequence = htons(rand() % MAX_SEQUENCE);
    clientSendDataPkt.length   = htons(HEADER_LENGTH);
    clientSendDataPkt.command  = htons(CLIENT_HELLO);

    /* Send CLIENT_HELLO message */
    write(clientSock, &clientSendDataPkt, HEADER_LENGTH);

    while(1)
    {
        int tmp;
        if ( read(clientSock, &clientRecvPkt, sizeof(clientRecvPkt)) < 0 )
        {
            /* Construct ERROR message */
            tmp = ntohs(clientSendDataPkt.sequence) + 1;
            clientSendDataPkt.sequence  = htons(tmp);
            clientSendDataPkt.length    = htons(HEADER_LENGTH);
            clientSendDataPkt.command   = htons(ERROR);
            /* Send ERROR message to server */
            write(clientSock, &clientSendDataPkt, HEADER_LENGTH);
            perror("Cannot read from socket");
            break;
        }
        switch ( ntohs(clientRecvPkt.command) )
        {
            case SERVER_HELLO:
                bytes_read = read(fd, &clientSendDataPkt.data, DATA_SIZE);
                if ( bytes_read < 0 )
                {
                    /* Construct ERROR message */
                    tmp = ntohs(clientSendDataPkt.sequence) + 1;
                    clientSendDataPkt.sequence  = htons(tmp);
                    clientSendDataPkt.length    = htons(HEADER_LENGTH);
                    clientSendDataPkt.command   = htons(ERROR);
                    /* Send ERROR message to server */
                    write(clientSock, &clientSendDataPkt, HEADER_LENGTH);
                    perror("Cannot read the file");
                    break;
                }
                /* Construct first DATA_DELIVERY packet */
                tmp = ntohs(clientSendDataPkt.sequence) + 1;
                clientSendDataPkt.sequence  = htons(tmp);
                clientSendDataPkt.length    = htons(HEADER_LENGTH + bytes_read);
                clientSendDataPkt.command   = htons(DATA_DELIVERY);
                /* Send first DATA_DELIVERY packet to socket */
                write(clientSock, &clientSendDataPkt, HEADER_LENGTH + bytes_read);
                break;
            case PKT_RECEIVED:
                bytes_read = read(fd, &clientSendDataPkt.data, DATA_SIZE);
                if (bytes_read == 0) // EOF
                {
                    // char* tmpName = "test.txt";
                    char* tmpName = "test.pdf";
                    /* Construct and send DATA_STORE packet */
                    tmp = ntohs(clientSendDataPkt.sequence) + 1;
                    clientSendDataPkt.sequence  = htons(tmp);
                    // clientSendDataPkt.length    = htons(HEADER_LENGTH + strlen(argv[3]));
                    clientSendDataPkt.length    = htons(HEADER_LENGTH + strlen(tmpName) + 1);
                    clientSendDataPkt.command   = htons(DATA_STORE);
                    // memset((char*) &clientSendDataPkt.data, 0, sizeof(clientSendDataPkt.data));
                    // memcpy((void*) &clientSendDataPkt.data, argv[3], strlen(argv[3]));
                    memcpy((void*) &clientSendDataPkt.data, tmpName, strlen(tmpName) + 1);
                    // write(clientSock, &clientSendDataPkt, HEADER_LENGTH + strlen(argv[3]));
                    write(clientSock, &clientSendDataPkt, HEADER_LENGTH + strlen(tmpName) + 1);
                    //TODO: instead of send file and then quit, wait for returned message from server to confirm stored or not
                    read_file_complete++;
                    break;
                }
                else if (bytes_read < 0)
                {
                    /* Construct ERROR message */
                    tmp = ntohs(clientSendDataPkt.sequence) + 1;
                    clientSendDataPkt.sequence  = htons(tmp);
                    clientSendDataPkt.length    = htons(HEADER_LENGTH);
                    clientSendDataPkt.command   = htons(ERROR);
                    /* Send ERROR message to server */
                    write(clientSock, &clientSendDataPkt, HEADER_LENGTH);
                    perror("Cannot read the file");
                    break;
                }
                /* Construct DATA_DELIVERY packet */
                tmp = ntohs(clientSendDataPkt.sequence) + 1;
                clientSendDataPkt.sequence  = htons(tmp);
                clientSendDataPkt.length    = htons(HEADER_LENGTH + bytes_read);
                clientSendDataPkt.command   = htons(DATA_DELIVERY);
                /* Send DATA_DELIVERY packet to socket */
                write(clientSock, &clientSendDataPkt, HEADER_LENGTH + bytes_read);
                break;
            case ERROR:
                //TODO:
                break;
            default:
                break;
        }
        if (read_file_complete)
            break;
    }

    close(fd);
    close(clientSock);
    return 0;
}
