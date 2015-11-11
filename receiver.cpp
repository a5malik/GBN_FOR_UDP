#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "packet.h"
#include <iostream>
#define BUFSIZE 1024
#define MAX_FILENAME 500

using namespace std;

/* 

 * error - wrapper for perror

 */

void error(char *msg) {
    perror(msg);
    exit(0);
}

/*

how to proceed:
need to ask for a file... 
need to ack packet with seq number

*/

int main(int argc, char **argv) {
   int sockfd, portno, n;
    socklen_t serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
	char filename[MAX_FILENAME];
    /* check command line arguments */

    if (argc != 4) {
       fprintf(stderr,"usage: %s <hostname> <port> <file>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
	strcpy(filename, argv[3]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    bzero(buf, BUFSIZE);
   //printf("Please enter msg: ");
    //fgets(buf, BUFSIZE, stdin);
	
	packet pack = packet(false, true, true, 5);
	pack.put_payload(filename);
	int size = pack.make_string(buf);
    /* send the message to the server */
    serverlen = sizeof(serveraddr);
	printf("%d\n", strlen(buf));
    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");

	
    /* print the server's reply */
    bzero(buf, BUFSIZE);
	char recv[TOTAL_SIZE];
	char send_string[TOTAL_SIZE];
	printf("\n waiting to receive 1st pack\n");
    n = recvfrom(sockfd, recv, sizeof(recv), 0, (struct sockaddr *)&serveraddr, &serverlen);
	packet recvd = packet(recv);
	packet to_send = packet(true, false, false, 1);
	int i = 0;
	while((recvd.ack || recvd.data || recvd.init))
	{
		printf("packet %d: \n", recvd.seq_num);
		printf("\n%s\n", recvd.payload);
		to_send.seq_num  = recvd.seq_num;
		to_send.make_string(send_string);
		 n = sendto(sockfd, send_string, strlen(send_string), 0, (struct sockaddr *) &serveraddr, serverlen);
			if (n < 0) 
				error("ERROR in sendto");
			printf("waiting to receive..\n");
		bzero(recv, sizeof(recv));
		n = recvfrom(sockfd, recv, sizeof(recv), 0, (struct sockaddr *)&serveraddr, &serverlen);
	    i++;
		recvd = packet(recv);
		printf("%d %d %d\n", recvd.ack, recvd.data,  recvd.init );
	}
	printf("all done\n");
    return 0;

}