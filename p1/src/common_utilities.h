#include "protocol.h"

void sendCmdPkt(int sockfd, cmd_pkt* cmdPkt, unsigned int seqNum,
            unsigned int pktLen, int cmd);

void sendDataPkt(int sockfd, data_pkt* dataPkt, unsigned int seqNum,
            unsigned int pktLen, int cmd);