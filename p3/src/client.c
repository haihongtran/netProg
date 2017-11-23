#include "client_utilities.h"

int main(int argc, char const* argv[]) {
    unsigned int threadsNum = atoi(argv[3]);    /* Number of threads */
    pthread_t tid[threadsNum];                  /* Thread ID */
    argStruct threadArgs;                       /* Thread arguments */
    unsigned int i;

    /* Check command-line arguments */
    if (argc != 5)
    {
        printf("Please use \'./client [server_ip] [server_port] [threads_number] [requests_per_thread]\'\n");
        return -1;
    }

    /* Copy pointer to IP address into thread argument struct */
    threadArgs.ipAddr   = argv[1];
    threadArgs.portNum  = atoi(argv[2]);
    threadArgs.reqNum   = atoi(argv[4]);

    printf("Creating %d threads. Each thread sends %d HTTP requests to server.\n", threadsNum, threadArgs.reqNum);

    /* Creating threads */
    for ( i = 0; i < threadsNum; i++ )
    {
#ifdef DEBUG
        printf("Creating threads %d\n", i+1);
#endif
        if ( pthread_create(&tid[i], NULL, clientHttpRequest, (void*) &threadArgs) != 0 )
        {
            printf("Thread number %d cannot be created.\n", i + 1);
            continue;
        }
    }

    /* Wait for all other threads to join */
    for ( i = 0; i < threadsNum; i++ ) {
#ifdef DEBUG
        printf("Waiting for threads #%d to join...\n", i + 1);
#endif
        pthread_join(tid[i], NULL);
    }

    printf("All threads finished works!\n");

    return 0;
}
