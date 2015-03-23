#ifndef LRIT_H
#define LRIT_H




struct PRIMARY_HEADER{
	unsigned short version: 2;
	unsigned short spacecraft: 8;
	unsigned short vc_id: 6;
	unsigned int   counter: 24;
	unsigned int   replay: 1;
	unsigned int   spare: 7;
	
};
struct PDU{
	unsigned short spare: 5;
	unsigned short first_pointer: 11;
	unsigned char spu_data[884]; // this packets SPU data

};

struct lrit_packet{

	struct PRIMARY_HEADER primary_header;
	struct PDU pdu;
	unsigned char reed_solomon[128];
};



struct SPU{
	//Packet ID
	unsigned short version: 3;
	unsigned short type:    1;
	unsigned short secondary_header: 1;
	unsigned short apid: 11;
	// Packet Sequence Control
	unsigned short seq_flag: 2;
	unsigned short seq_counter: 14;

	unsigned short length;
	unsigned char *data;
	unsigned short crc;

};


#endif
