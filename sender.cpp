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
#include <time.h>
#include <sys/time.h>


#define BUFSIZE 1024

#define TIMEOUT 250

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
  int max_seq;
  float crpt_rate, loss_rate;
   srand (time(NULL));
   fd_set fds;
   struct timeval timeout;
   int rc;
   timeout.tv_sec = TIMEOUT/1000;
   timeout.tv_usec = 0;
   FD_ZERO(&fds);
   
  /* 
   * check command line arguments 
   */
  if (argc != 5) {
    fprintf(stderr, "usage: %s <port> <corrupt_rate> <loss_rate> <window_size>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);
  crpt_rate = atof(argv[2]);
  loss_rate = atof(argv[3]);
  int ws = atoi(argv[4]);
  
  printf("crpt_rate: %f\n", crpt_rate);
  

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
      sendto: echo the input back to the client 
     */
	 /*
	 */
	 char bug[TOTAL_SIZE];
	 int fd;
	 char c;
	 int a;
	 int base = 1, count = ws;
	 max_seq = ws+2;
	 packet pack = packet(&(buf[0]));
	 packet initial = packet(false, false, true, max_seq);
	 initial.make_string(bug);
	  n = sendto(sockfd, bug, strlen(bug), 0, 
							(struct sockaddr *) &clientaddr, clientlen);
	  char buffer[PAYLOAD_SIZE];
	  bzero(buffer, PAYLOAD_SIZE);
	 packet to_send = packet(false, false, false, 1);
	 printf("\nfile asked for %s\n", pack.payload);
	 bool once_more = false;
	 if((fd = open(pack.payload, O_RDONLY)) >= 0)
	 {
		  printf("file found\n");
		 list<packet> window;
		 int bytes = 0;
		 int seq_num = 1;
		 bool is_wrong;
		 n=read(fd,&c, 1);
		 packet ackpack = packet(false, false, false, 1, false);
		 while( n== 1 || once_more)
		 {
			 
			 if (n < 0) error("ERROR reading from file");	
			 // read PAYLOAD_SIZE bytes into buffer, put it into payload, push it to the list... and once i have window size or eof.. send them 1 by 1..
			 //
			 if(!once_more)
				buffer[bytes++] = c;
			 if(bytes == PAYLOAD_SIZE -1 || once_more) // have 1 packet..
			 {
				 buffer[bytes] = '\0';
				 //printf("\n buffer is %s\n", buffer);
				 if(rand()%100+1 >= 100*crpt_rate)
					to_send = packet(false, true, false, seq_num);
				 else
					to_send = packet(false, true, false, seq_num, true);
				
				 seq_num  = (seq_num+1)%max_seq;
				 seq_num += (!seq_num)*1;
				 to_send.put_payload(buffer);
				 window.push_back(to_send);
				 bytes = 0;
				 bzero(buffer, PAYLOAD_SIZE);
				 
				  if( window.size() >= 1 && window.size() <= ws) 
			 {
				 list<packet>::reverse_iterator i;
				 printf("\nabout to send\n");
				 //send the packets..
				 for(i = window.rbegin(); i != window.rend() && !((*i).sent); i++)
				 {
					 (*i).make_string(bug);
					 if(rand()%100+1 >= 100*loss_rate)
					 {
					 printf("\n sending packet  %d which is crp: %d \n", (*i).seq_num, (*i).crp);
					 sendto(sockfd, bug, strlen(bug), 0, 
							(struct sockaddr *) &clientaddr, clientlen);
					 }
					 else 
					  printf("losing packet %d\n", (*i).seq_num);
					 (*i).sent = true;
				 }
				 //wait to receive something...
				
			 }//window size < ws...can make more packets and sent.. but anytime i have a packet ill send it..
			  if(window.size() >= ws || once_more)
				{
					while(window.size() > 0)
					{
						is_wrong = false;
					rc = 0;
				
					while(rc != 1 || is_wrong)
					{
					if(!is_wrong)
					{
							timeout.tv_sec = 0;
							timeout.tv_usec = TIMEOUT * 1000;
					}
					FD_ZERO(&fds);
					FD_SET(sockfd, &fds);
					 rc = select(sockfd+1, &fds, NULL, NULL, &timeout);
					 if (timeout.tv_usec == 0)
						 timeout.tv_usec = TIMEOUT * 1000;
					 if(rc == 0) //timeout
					 {
						 
							list<packet>::iterator i;
							printf("\nabout to RETRANSMIT\n");
						//send the packets..
						for(i = window.begin(); i != window.end()  ; i++)
							{
							if(rand()%100+1 >= 100*crpt_rate)
								(*i).crp = false;
							else
								(*i).crp = true;
							(*i).make_string(bug);
							if(rand()%100+1 >= 100*loss_rate)
								{
									printf("\n sending packet  %d which is crp: %d \n", (*i).seq_num, (*i).crp);
									sendto(sockfd, bug, strlen(bug), 0, 
										(struct sockaddr *) &clientaddr, clientlen);
								}
							else 
									printf("losing packet %d\n", (*i).seq_num);
							(*i).sent = true;
							}
					 }
					 else if(rc == 1)
					 {
						 recvfrom(sockfd, buf, BUFSIZE, 0,
						(struct sockaddr *) &clientaddr, &clientlen);
						ackpack = packet(buf);
						printf("received ack for %d\n", ackpack.seq_num);
						if((ackpack.seq_num == window.front().seq_num-1) || (ackpack.seq_num == ws+1 && window.front().seq_num == 1) || ackpack.crp)
						{
							printf("this ack was old : so crpt or some pkt was lost\n");
							is_wrong = true;
							printf("time remaining = %d\n", timeout.tv_usec);
						}
						else
						is_wrong = false;
						
					 }
					 
					}
					 
					while(ackpack.seq_num != window.front().seq_num)
						window.pop_front();
					window.pop_front();
						
					
						if(!once_more)
							break;
					}
				}
			 }
			
			if(once_more)
				break;
			n=read(fd,&c, 1);
			if(n != 1)
				once_more = true;
		 }
		 
	 }
	 close(fd);
	 packet endpack = packet(false, false, false, 2);
	 printf("%d %d %d\n", endpack.ack, endpack.data,  endpack.init );
	 endpack.make_string(bug);
	 printf("\n sending\n %s \n", bug);
	// pack.make_string(bug);
    n = sendto(sockfd, bug, strlen(bug), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
		  
    if (n < 0) 
      error("ERROR in sendto");
	
  }
}
