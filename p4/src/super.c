#include "super_lib.h"

int main(int argc, char const* argv[]) {
    unsigned int portNum1 = 0, portNum2 = 0;
    char ipAddr[20] = {0};
    int retVal;
    int option_index;
    struct option long_options[] = {
        {"s_ip",    required_argument, 0, 'a'},
        {"s_port",  required_argument, 0, 'b'}
    };
    /* Start parsing arguments */
    while (1) {
        retVal = getopt_long(argc, argv, "p:a:b:", long_options, &option_index);
        /* If all arguments have been parsed, break */
        if ( retVal == -1 )
            break;
        switch (retVal)
        {
            case 'p':
                portNum1 = atoi(optarg);
                break;
            case 'a':
                strcpy(ipAddr, optarg);
                break;
            case 'b':
                portNum2 = atoi(optarg);
                break;
            case '?':
                printf("Not defined option\n");
                break;
            default:
                printf("Unknown\n");
                break;
        }
    }
    return 0;
}
