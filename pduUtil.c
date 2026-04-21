#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "pduUtil.h"

int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData){
    int total_len = lengthOfData + 2;
    uint16_t net_len = htons(total_len);

    uint8_t *temp_buffer = malloc(total_len);
    memcpy(temp_buffer, &net_len, 2);
    memcpy(temp_buffer + 2, dataBuffer, lengthOfData);

    if (send(clientSocket, temp_buffer, total_len, 0) < 0) {
        free(temp_buffer);
        perror("send call");
        return -1;
    }
    
    free(temp_buffer);
    return lengthOfData;
}

int recvPDU(int socketNumber, uint8_t *dataBuffer, int bufferSize){
    uint16_t net_len;
    int recv_len = 0;

    // Receive the length of the incoming PDU (2 bytes)
    recv_len = recv(socketNumber, &net_len, sizeof(net_len), MSG_WAITALL);
    if (recv_len < 0) {
        perror("recv call");
        return -1;
    } else if (recv_len == 0) {
        // Connection closed by the peer
        return 0;
    }

    uint16_t pdu_len = ntohs(net_len);
    if (pdu_len > bufferSize) {
        fprintf(stderr, "Buffer size is too small for incoming PDU\n");
        return -1;
    }

    // Receive the actual PDU data based on the length received
    recv_len = recv(socketNumber, dataBuffer, pdu_len - 2, MSG_WAITALL);
    if (recv_len < 0) {
        perror("recv call");
        return -1;
    } else if (recv_len == 0) {
        // Connection closed by the peer
        return 0;
    }

    return recv_len; // Return the length of the data received
}
