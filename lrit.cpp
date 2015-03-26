#include <cstdio>
#include <cstdlib>
#include "lrit.h"
#include <cstring>
using namespace std;

/*
 * Given a CVCDU data set, create a M_PDU
 * returns 1 if a packet is ready, 0 otherwise
 */
int PDU_Processor::process_packet( struct lrit_packet *packet, M_PDU &pdu ){
	int status = 0; 
	//printf( "Processing Packet from spacecraft %d, packet number %d, vc_id: %d\n", packet->primary_header.spacecraft, packet->primary_header.counter, packet->primary_header.vc_id);

	if( packet->primary_header.vc_id == 63 ){
		printf("****Filler\n");
		return status; // filler packet
	}
	if( packet->primary_header.version != 1 ){
		printf( "Bad Packet\n" );
		return status;
	}

	struct M_PDU_header pdu_header;
	uint16_t tmp;
	memcpy( &tmp, packet->data, 2 );
	pdu_header.spare = (tmp & 0xF800 ) >> 11;
	pdu_header.first_pointer = (tmp & 0x7FF );
	
	
	printf( "First pointer: %d\n", pdu_header.first_pointer );
	if( pdu_header.first_pointer > 884  && pdu_header.first_pointer != 2047){
		printf( "************* outside packet************");
	}

	buffer.push_back( pdu_header );


	
	// add the data to the buffer
	/*
	memcpy( this->buffer+this->buffer_end,packet->data+2,884);
	
	if( pdu_header.first_pointer == 0x7FF ){
		printf( "Adding Data to buffer\n");
		this->buffer_end += 884;
		// this packet doesn't contain a header, just data problably
		return status; // this definately means just pad things
	}

	if( this->first_pointer == -1 ){
		// if this is the first time we've started setup the M_PDU pointer
		// we may have gotten all 0x7FF packets before this
		this->first_pointer = pdu_header.first_pointer+this->buffer_end;
	}
	this->buffer_end += 884;
	this->num_packets_not_processed+=1; 
	M_PDU tmp_pdu = get_pdu_base_data( );

	printf( " Found some actual PDU data (Current buffer length: %d, First Pointer: %d)\n", buffer_end,first_pointer);
	printf( " APID: %d\n Secondary Header: %d\n SeqFlag: %d\n SeqCount: %d\n Length: %d\n", tmp_pdu.apid, tmp_pdu.secondary_header, tmp_pdu.seq_flag, tmp_pdu.seq_count, tmp_pdu.length );


	// now we see if we can build a full packet
	if( this->first_pointer + tmp_pdu.length < this->buffer_end ){
		// we should have a complete packet now
		printf( "Have complete packet\n");
		pdu = tmp_pdu;
		memcpy(pdu.data, this->buffer+this->first_pointer+6, tmp_pdu.length+1 );
		status = 1;

		// something is wrong with this part I think
		size_t bytes_getting_removed = 884*this->num_packets_not_processed; // these always come in 884 byte chunks and first_pointer is referenced to that boundry
		memmove( this->buffer, this->buffer+bytes_getting_removed, this->buffer_end-bytes_getting_removed );
		this->buffer_end -= bytes_getting_removed;
		
		this->first_pointer=pdu_header.first_pointer;
		this->num_packets_not_processed = 0; // reset this counter.
	}
	
	*/
	return status;

	
}

M_PDU PDU_Processor::get_pdu_base_data(){
	M_PDU pdu;
	// copy packet id
	printf("*****0x");
	for(int i = this->first_pointer; i < this->first_pointer+6;i++){
		printf("%0X", buffer[i] );
	}
	printf("*****");
	uint16_t tmp;
	memcpy( &tmp, this->buffer+this->first_pointer, 2);
	pdu.version          = (tmp & 0xE000) >> 13;
	pdu.type             = (tmp & 0x0100) >> 12;
	pdu.secondary_header = (tmp & 0x0800) >> 11;
	pdu.apid             = (tmp & 0x07FF);

	
	// sequence control
	memcpy( &tmp, this->buffer+this->first_pointer+2, 2);
	pdu.seq_flag         = (tmp & 0xC000) >> 14;
	pdu.seq_count        = (tmp & 0x3FFF);

	// length
	memcpy( &tmp, this->buffer+this->first_pointer+2+2, 2);
	pdu.length = tmp;
	return pdu;

}


uint16_t swap_u16( uint16_t val ){
	return (val <<8 ) | (val >> 8 );
}

uint32_t swap_u32( uint32_t val ){
	val = ((val <<8 )&0XFF00FF00) | ( (val >>8) & 0xFF00FF );
	return (val << 16) | (val>>16);
}



