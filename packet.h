#ifndef PACKET_H
#define PACKET_H

#include <string.h>

#define PAYLOAD_SIZE 1001
#define TOTAL_SIZE 1037
//"ACK:0 INIT:0 DATA:0 CRP:0 SEQ:1111 \n"
// 123456789012345678901234567890123456
class packet{
	public:
	bool ack;
	bool data;
	bool init;
	bool crp;
	long int seq_num;
	char payload[PAYLOAD_SIZE];
	packet(bool isack, bool isdata, bool isinit, int seq_n, bool crpt=false);
	int make_string(char s[]);
	packet(char s[]);
	int size;
	bool sent;
	void put_payload(char s[]);
};

#endif