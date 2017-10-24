#include "client_utilities.h"

int main(int argc, char const *argv[])
{
    pthread_t tid;          /* Thread ID */
    argStruct threadArgs;   /* Thread arguments */
    unsigned int numOfThread = atoi(argv[2]);   /* Number of requests */
    unsigned int usecs = 100000;    /* 100ms */
    unsigned int i;

    /* Check command-line arguments */
    if (argc != 3)
    {
        printf("Please use \'./client [server_ip] [number_of_requests]\'\n");
        return -1;
    }

    /* Copy pointer to IP address into thread argument struct */
    threadArgs.ipAddr = argv[1];

    /* Request to server */
    for ( i = 0; i < numOfThread; i++ )
    {
        /* Put thread number into thread argument struct */
        threadArgs.threadNum = i + 1;
        /* Create thread */
        if ( pthread_create(&tid, NULL, clientSendFile, (void*) &threadArgs) != 0 )
        {
            printf("Thread number %d cannot be created.\n", threadArgs.threadNum);
            continue;
        }
        /* Wait for 100ms */
        usleep(usecs);
    }

    return 0;
}
