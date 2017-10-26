#include "common_utilities.h"

void sendCmdPkt(int sockfd, cmd_pkt* cmdPkt, unsigned int seqNum,
            unsigned int pktLen, int cmd)
{
    cmdPkt->sequence    = htons(seqNum);
    cmdPkt->length      = htons(pktLen);
    cmdPkt->command     = htons(cmd);
    write(sockfd, cmdPkt, pktLen);
}

void sendDataPkt(int sockfd, data_pkt* dataPkt, unsigned int seqNum,
            unsigned int pktLen, int cmd)
{
    dataPkt->sequence   = htons(seqNum);
    dataPkt->length     = htons(pktLen);
    dataPkt->command    = htons(cmd);
    write(sockfd, dataPkt, pktLen);
}
