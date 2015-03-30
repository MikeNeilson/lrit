#ifndef LRIT_H
#define LRIT_H
#include <inttypes.h>
#include <stdint.h>
#include <cstring>
#include <list>
#include <map>
using namespace std;

const size_t BUFFER_LEN=3*1024*1024;

struct VCDU_header{
	uint8_t version;
	uint8_t spacecraft;
	uint8_t vc_id;
	uint32_t   counter;
	uint8_t   replay;
	uint8_t   spare;
	
};

struct M_PDU{
	uint8_t spare;
	uint16_t first_pointer;
	unsigned char packet_zone[884]; // this packets SPU data

};

struct lrit_packet{

	struct VCDU_header primary_header;
	unsigned char data[886];
	//struct PDU pdu;
	//
	unsigned char reed_solomon[128];
};



struct M_SDU{
	//Packet ID
	uint8_t version;
	uint8_t type;
	uint8_t secondary_header;
	uint16_t apid;
	// Packet Sequence Control
	uint8_t seq_flag;
	uint16_t seq_count;

	uint16_t length;// this length is 0 8191, so you have to add 1 for the correct byte count
	uint8_t data[8192]; // includes CRC
	//unsigned char *data;
	//uint16_t crc;

};

class Transport_File{
	public:
	uint16_t file_counter;
	uint64_t length;
	unsigned char *tp_sdu;

	Transport_File();
	~Transport_File();
};


uint16_t swap_u16( uint16_t val );
uint32_t swap_u32( uint32_t val );

class SDU_Processor{
	public:
	int process_packet( struct lrit_packet *packet, M_SDU &sdu );

	SDU_Processor(){
	/*	memset( buffer, 0, BUFFER_LEN-1 );
		buffer_end = 0;
		first_pointer = -1;
		num_packets_not_processed = 0;*/
	}
	
	
	private:
	/*
	 *  * Given a CVCDU data set, create a M_PDU
	 *   * returns 1 if a packet is ready, 0 otherwise
	 *    */
	/*
	unsigned char buffer[BUFFER_LEN];
	size_t buffer_end;
	size_t first_pointer;
	size_t num_packets_not_processed;
	*/

	map< uint8_t ,list<M_PDU> > buffers; // we need a buffer for each vnc
	
	
	int get_sdu( uint8_t vc_id, M_SDU &sdu );
};


// flags
//
const uint8_t SEQ_COMPLETE_FILE = 3;
const uint8_t SEQ_FIRST_SEGMENT = 1;
const uint8_t SEQ_CONTINUATION  = 0;
const uint8_t SEQ_LAST_SEGMENT  = 2;

const uint16_t APID_FILL = 2047;
const uint16_t PDU_FILL  = 0x7FF;



#endif
