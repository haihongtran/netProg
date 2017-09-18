#include "protocol.h"

int main(int argc, char const *argv[])
{
    FILE* fp;
    int clientSock;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen;

    char buffer[1024];

    if (argc != 4)
    {
        printf("Please use \'./client server_ip port_number transfer_file\'\n");
        exit(1);
    }

    if ( !(fp = fopen(argv[3], "r")) )
    {
        perror("File does not exist");
        exit(1);
    }

    /* Create socket descriptor */
    if ( (clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        fclose(fp);
        perror("Failed to create a socket");
        exit(1);
    }

    /* Connect to server */
    memset((char*) &serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr  = inet_addr(argv[1]);

    serverAddrLen = sizeof(serverAddr);
    if (connect(clientSock, (struct sockaddr*) &serverAddr, serverAddrLen) < 0)
    {
        perror("Connecting failed");
        exit(1);
    }

    /* Start communication */
    strcpy(buffer, "Hello");
    send(clientSock, buffer, 5, 0);
    printf("Data sent: %s\n", buffer);

    fclose(fp);
    close(clientSock);
    return 0;
}
