#include "client_utilities.h"

int main(int argc, char const *argv[])
{
    /* Check command-line arguments */
    if (argc != 3)
    {
        printf("Please use \'./client [server_ip] [number_of_request]\'\n");
        return -1;
    }

    /* Connect and send file to server */
    if ( clientSendFile(argv[1]) < 0 )
        return -1;

    return 0;
}
