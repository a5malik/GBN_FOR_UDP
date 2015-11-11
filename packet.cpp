#include "packet.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
using namespace std;
packet::packet(bool isack, bool isdata, bool isinit, int seq_n)
	: ack(isack), data(isdata), init(isinit), seq_num(seq_n)
{
	this->payload[0] = '\0';
}

int packet::make_string(char s[])
{
	stringstream ss;
	ss << this->seq_num;
	string temp = ss.str();
	char* num = (char*)temp.c_str();
	sprintf(s, "ACK:%c INIT:%c DATA:%c SEQ:%s \n%s", (this->ack)?'1':'0', (this->init)?'1':'0', (this->data)?'1':'0', num, this->payload);
	return 26+strlen(num)+strlen(this->payload);
	
}

void packet::put_payload(char s[])
{
	strcpy(this->payload, s);
	this->payload[strlen(s)]='\0';
}

 packet::packet(char s[])
{
	ack = (s[4]=='0')?false:true;
	init = (s[11]=='0')?false:true;
	//printf("%c %c %c %c %c\n", s[16], s[17], s[18], s[19], s[20]);
	data = (s[18]=='0')?false:true;
	char num[7];
	int i = 0, j = 24;
	while(s[j]!=' ')
	{
		num[i] = s[j];
		j++;i++;
	}
	num[i] = '\0';
	seq_num = atoi(num);
	char* pos = strstr(s,"\n");
	pos++;
	if(data)
	{
		put_payload(pos);
	}
}