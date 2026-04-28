/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

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
#include "safeUtil.h"
#include "pdu.h"
#include "pollLib.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void sendToServer(int socketNum);
void recvFromServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void clientControl(int socketNum);


int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);
	socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);

	setupPollSet();
	addToPollSet(STDIN_FILENO);
	addToPollSet(socketNum);

	clientControl(socketNum);
	
	close(socketNum);
	return 0;
}

void sendToServer(int socketNum)
{
	uint8_t buffer[MAXBUF];  	//data buffer
	int sendLen = 0;        	//amount of data to send
	int sent = 0;            	//actual amount of data sent/* get the data and send it   */

	sendLen = readFromStdin(buffer);
	printf("Read: %s, string len: %d (including null)\n", buffer, sendLen);
	
	sent = sendPDU(socketNum, buffer, sendLen);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Socket %d: Message sent to server, Length: %d, msg: %s\n", socketNum, sent, buffer);
}

void recvFromServer(int socketNum)
{
	uint8_t buffer[MAXBUF];   //data buffer
	int recvBytes = 0;        //amount of data received
	
	recvBytes = recvPDU(socketNum, buffer, MAXBUF);
	if (recvBytes < 0)
	{
		perror("recv call");
		exit(-1);
	}
	else if (recvBytes == 0)
	{
		printf("Server has terminated\n");
		close(socketNum);
		removeFromPollSet(socketNum);
		exit(0);
	}
	printf("Socket %d: Message received from server, Length: %d, msg: %s\n", socketNum, recvBytes, buffer);
}

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';

	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 3)
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}
}

void clientControl(int socketNum)
{
	int readySocket = 0;

	while (1)
	{
		readySocket = pollCall(-1);

		if (readySocket == STDIN_FILENO)
		{
			sendToServer(socketNum);
		}
		else if (readySocket == socketNum)
		{
			recvFromServer(socketNum);
		}
		else
		{
			printf("Error: poll returned unknown socket number: %d\n", readySocket);
			exit(-1);
		}
	}
}