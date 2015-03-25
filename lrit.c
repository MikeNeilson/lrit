#include <stdio.h>
#include <stdlib.h>
#include "lrit.h"

#define BUFFER_LEN 3*1024*1024

void process_packet( struct lrit_packet *packet ){
	static unsigned char buffer[BUFFER_LEN];
	static unsigned int buffer_end = 0;
	static unsigned int first_pointer = 0;
	printf( "Processing Packet from spacecraft %d, packet number %d, vc_id: %d\n", packet->primary_header.spacecraft, packet->primary_header.counter, packet->primary_header.vc_id);

	if( packet->primary_header.vc_id == 63 ){
		printf("****Filler");
		return; // filler packet
	}
	if( packet->primary_header.version != 1 ){
		printf( "Bad Packet" );
	}

	printf( "First pointer: %d\n", packet->pdu.first_pointer );
		
	unsigned char buf[2] = {0};
	memcpy( buf, (unsigned char*)&packet->pdu, 2 );
	printf( "****%X%X\n", buf[0], buf[1] );
	
	
	
	struct SPU sdu;

	if( packet->pdu.first_pointer <  (884-6) ){ // for now, just wait for the first packet that starts within the packet we have.
		memcpy( (unsigned char*)&sdu, &packet->pdu.spu_data[packet->pdu.first_pointer], 6 );


		printf( " Found some actual SPU data\n");
		printf( " APID: %d\n Secondary Header: %d\n SeqFlag: %d\n SeqCount: %d\n Length: %d\n", sdu.apid, sdu.secondary_header, sdu.seq_flag, sdu.seq_count, sdu.length );

	}
	
	
	

	
}


uint16_t swap_u16( uint16_t val ){
	return (val <<8 ) | (val >> 8 );
}

uint32_t swap_u32( uint32_t val ){
	val = ((val <<8 )&0XFF00FF00) | ( (val >>8) & 0xFF00FF );
	return (val << 16) | (val>>16);
}



