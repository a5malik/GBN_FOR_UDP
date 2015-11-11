#ifndef PACKET_H
#define PACKET_H

#include <string.h>

#define PAYLOAD_SIZE 1001
#define TOTAL_SIZE 1032
//"ACK:0 INIT:0 DATA:0 SEQ:1111 \n"
class packet{
	public:
	bool ack;
	bool data;
	bool init;
	long int seq_num;
	char payload[PAYLOAD_SIZE];
	packet(bool isack, bool isdata, bool isinit, int seq_n);
	int make_string(char s[]);
	packet(char s[]);
	int size;
	void put_payload(char s[]);
};

#endif