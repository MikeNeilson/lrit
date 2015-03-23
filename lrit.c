#include <stdio.h>
#include <stdlib.h>
#include "lrit.h"

#define BUFFER_LEN 3*1024*1024
unsigned char buffer[BUFFER_LEN];
unsigned int buffer_end;


void process_packet( struct lrit_packet *packet ){

	if( packet->primary_header.vc_id == 63 ){
		return; // filler packet
		printf("Filler");
	}
	printf( "Processing Packet from spacecraft %d, packet number %d, vc_id: %d\n", packet->primary_header.spacecraft, packet->primary_header.counter, packet->primary_header.vc_id);
	if( packet->primary_header.version != 1 ){
		printf( "Bad Packet" );
	}

	printf( "First pointer: %d\n", packet->pdu.first_pointer );
		
}
