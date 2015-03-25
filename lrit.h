#ifndef LRIT_H
#define LRIT_H
#include <inttypes.h>
#include <stdint.h>



struct PRIMARY_HEADER{
	uint16_t version: 2;
	uint16_t spacecraft: 8;
	uint16_t vc_id: 6;
	uint32_t   counter: 24;
	uint32_t   replay: 1;
	uint32_t   spare: 7;
	
};
struct PDU{
	uint16_t spare: 5;
	uint16_t first_pointer: 11;
	unsigned char spu_data[884]; // this packets SPU data

};

struct lrit_packet{

	struct PRIMARY_HEADER primary_header;
	struct PDU pdu;
	unsigned char reed_solomon[128];
};



struct SPU{
	//Packet ID
	uint16_t version: 3;
	uint16_t type:    1;
	uint16_t secondary_header: 1;
	uint16_t apid: 11;
	// Packet Sequence Control
	uint16_t seq_flag: 2;
	uint16_t seq_count: 14;

	uint16_t length;
	unsigned char *data;
	uint16_t crc;

};


uint16_t swap_u16( uint16_t val );
uint32_t swap_u32( uint32_t val );




#endif
