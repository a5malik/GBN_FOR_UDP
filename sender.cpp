/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "packet.h"
#include <list>
#include <sys/stat.h> 
#include <fcntl.h>
#include <iostream>

#define BUFSIZE 1024

using namespace std;

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
	printf("%s", buf);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    //printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
    
    /* 
     * sendto: echo the input back to the client 
     */
	 char bug[TOTAL_SIZE];
	 int fd;
	 char c;
	 int ws = 1;
	 int a;
	 packet pack = packet(&(buf[0]));
	 packet to_send = packet(false, false, false, 1);
	 printf("\nfile asked for %s\n", pack.payload);
	 if((fd = open(pack.payload, O_RDONLY)) >= 0)
	 {
		  printf("file found\n");
		 list<packet> window;
		 int bytes = 0;
		 int seq_num = 1;
		 char buffer[PAYLOAD_SIZE];
		 while((n=read(fd,&c, 1) == 1))
		 {
			 if (n < 0) error("ERROR reading from file");	
			 // read PAYLOAD_SIZE bytes into buffer, put it into payload, push it to the list... and once i have window size or eof.. send them 1 by 1..
			 //
			 buffer[bytes++] = c;
			 if(bytes == PAYLOAD_SIZE -1)
			 {
				 buffer[bytes] = '\0';
				 //printf("\n buffer is %s\n", buffer);
				 to_send = packet(false, true, false, seq_num++);
				 to_send.put_payload(buffer);
				 window.push_back(to_send);
				 bytes = 0;
				 bzero(buffer, PAYLOAD_SIZE);
			 }
			 if(window.size() == ws)
			 {
				 list<packet>::iterator i;
				 printf("\nabout to send\n");
				 for(i = window.begin(); i != window.end(); i++)
				 {
					 (*i).make_string(bug);
					 printf("\n sending\n %s \n", bug);
					  n = sendto(sockfd, bug, strlen(bug), 0, 
							(struct sockaddr *) &clientaddr, clientlen);
				 }
				 while(window.size()>0)
				 {
					 printf("waiting for ack \n");
					 n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
					packet ackpack = packet(buf);
					 printf("%d %d %d\n", ackpack.ack, ackpack.data,  ackpack.init );
					printf("received ack for %d\n", ackpack.seq_num);
					if(window.front().seq_num == ackpack.seq_num)
						window.pop_front();
					cin >> a;
				 }
				
			 }
		 
		 }
		 
				 buffer[bytes] = '\0';
				 //printf("\n buffer is %s\n", buffer);
				 to_send = packet(false, true, false, seq_num++);
				 to_send.put_payload(buffer);
				 window.push_back(to_send);
				 bytes = 0;
				 bzero(buffer, PAYLOAD_SIZE);
				 if(window.size() == ws)
			 {
				 list<packet>::iterator i;
				 printf("\nabout to send\n");
				 for(i = window.begin(); i != window.end(); i++)
				 {
					 (*i).make_string(bug);
					 printf("\n sending\n %s \n", bug);
					  n = sendto(sockfd, bug, strlen(bug), 0, 
							(struct sockaddr *) &clientaddr, clientlen);
				 }
				 while(window.size()>0)
				 {
					 printf("waiting for ack \n");
					 n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
					packet ackpack = packet(buf);
					printf("received ack for %d\n", ackpack.seq_num);
					if(window.front().seq_num == ackpack.seq_num)
						window.pop_front();
					cin >> a;
				 }
			 }
		 
	 }
	 close(fd);
	 packet endpack = packet(false, false, false, 2);
	 printf("%d %d %d\n", endpack.ack, endpack.data,  endpack.init );
	 endpack.make_string(bug);
	 printf("\n sending\n %s \n", bug);
	 cin >> a;
	// pack.make_string(bug);
    n = sendto(sockfd, bug, strlen(bug), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
		  
    if (n < 0) 
      error("ERROR in sendto");
	
  }
}
