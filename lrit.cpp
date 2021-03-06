#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include "lrit.h"
#include "util.h"
#include <cstring>
using namespace std;

/*
 * Given a CVCDU data set, create a M_PDU
 * returns 1 if a packet is ready, 0 otherwise
 */
int SDU_Processor::process_packet( struct lrit_packet *packet, M_SDU &sdu ){
	int status = 0; 
	//printf( "Processing Packet from spacecraft %d, packet number %d, vc_id: %d\n", packet->primary_header.spacecraft, packet->primary_header.counter, packet->primary_header.vc_id);
	//printf("****Buffer Size %d Packets\n", buffers[packet->primary_header.vc_id].size() );
	if( packet->primary_header.vc_id == 63 ){
		DEBUG("%s","****Filler\n");
		return status; // filler packet
	}
	if( packet->primary_header.version != 1 ){
		printf( "Bad Packet\n" );
		return status;
	}

	struct M_PDU pdu;
	uint16_t tmp;
	memcpy( &tmp, packet->data, 2 );
	pdu.spare = (tmp & 0xF800 ) >> 11;
	pdu.first_pointer = (tmp & 0x7FF );
	memcpy( pdu.packet_zone, packet->data+2, 884 );	
	
	DEBUG("%s","First pointer: %d\n", pdu.first_pointer );
	if( pdu.first_pointer > 884  && pdu.first_pointer != 2047){
		printf( "************* outside packet************\n");
	}
	
	buffers[packet->primary_header.vc_id].push_back( pdu );
	
	status = this->get_sdu( packet->primary_header.vc_id, sdu );

	
	return status;

	
}

/*
 * attempt to build a PDU
 */
int SDU_Processor::get_sdu(uint8_t vcid, M_SDU &sdu){

	list<M_PDU> &buffer = buffers[vcid];
	list<M_PDU>::iterator pdu = buffer.begin();
	// first lets move forwards from starting the stream in the middle of a file
	// this should only happen if we started the connection in the middle of a PDU
	while( pdu->first_pointer == 0x7FF ){
		pdu = buffer.erase( pdu );
		if( buffer.size() == 0 ){
			return 0;
		}
	}
	// copy pdu id
	DEBUG( "%s", "\n" );
	/*
	printf("*****0x");
	for(int i = pdu->first_pointer; i < pdu->first_pointer+6;i++){
		printf("%0X ", pdu->packet_zone[i] );
	}
	printf("*****\n");
	*/
	uint16_t tmp=0;
	memcpy( &tmp, pdu->packet_zone+pdu->first_pointer, 2);
	sdu.version          = (tmp & 0xE000) >> 13;
	sdu.type             = (tmp & 0x0100) >> 12;
	sdu.secondary_header = (tmp & 0x0800) >> 11;
	sdu.apid             = (tmp & 0x07FF);

	// sequence control
	tmp = 0;
	memcpy( &tmp, pdu->packet_zone+pdu->first_pointer+2, 2);
	sdu.seq_flag         = (tmp & 0xC000) >> 14;
	sdu.seq_count        = (tmp & 0x3FFF);

	// length
	tmp=0;
	memcpy( &tmp, pdu->packet_zone+pdu->first_pointer+4, 2);
	sdu.length = tmp;
	

	
	DEBUG("Have a pdu of length: %d\n", sdu.length+1 );

	// for now were's just checking the above logic
	
	if( pdu == buffer.end() && (sdu.length+1+pdu->first_pointer) > 884 || sdu.length<5 ){
		// we don't have all of the pdu yet
		return 0;
	} 

	size_t pkgdata = 0;
	list<M_PDU>::iterator start = pdu;
	while( pkgdata < sdu.length+1 && pdu != buffer.end()  ){
		DEBUG("Cur First Pointer: %d\n", pdu->first_pointer );
		DEBUG("Current data found: %d bytes\n", pkgdata  );
		if ( pdu->first_pointer == PDU_NO_HEADER || pdu->first_pointer > 880 ){
			//middle PDU for SDU
			// we just copy the whole thing
			DEBUG("Adding full packet zone to M_SDU (884 bytes)\n", __FILE__, __LINE__ );
			size_t bytes_remaining = 8192-pkgdata;
			if( bytes_remaining >= 884){
				memcpy( sdu.data+pkgdata, pdu->packet_zone, 884 );
				pkgdata += 884;
			} else{
				// this probably means a packet got missed, but this will keep
				// the softwrae from seg fauling and allow the CRC check to fix it
				memcpy( sdu.data+pkgdata, pdu->packet_zone, bytes_remaining );
				pkgdata += bytes_remaining;
			}
		} else{
			// we have to determine if we're at the start
			if( pkgdata == 0){
				/*
				 * PDU length is max 884, so copy min( sdu.length+1, 884-first_pointer )
				 */
				size_t num_bytes_to_copy = min( sdu.length+1, 884-(pdu->first_pointer+6) );
				
				DEBUG( "Adding %d bytes from packet zone to M_SDU (first pdu)\n", num_bytes_to_copy );
				if( num_bytes_to_copy > 0 ){
					// have to skip the first 6 bytes, first pointer points to the header of the sdu
					memcpy( sdu.data+pkgdata, pdu->packet_zone+pdu->first_pointer+6, num_bytes_to_copy );
					pkgdata+=num_bytes_to_copy;
				}else{
					DEBUG("%s", "Negative number of bytes requested\n" );
				}
				

			// first PDU for SDU
			} else{
			// last PDU for SDU
			//  so copy up to  first pointer, then break so this pdu doesn't get remvoe ( there's more data after first pointer
			//  TODO: may also need to check this using sdu;
				DEBUG("Adding %d bytes from packet zone to M_SDU (middle/end pdu)\n", pdu->first_pointer );
					if( pdu->first_pointer < 884 ){
					memcpy( sdu.data+pkgdata, pdu->packet_zone, pdu->first_pointer);
					pkgdata+=pdu->first_pointer;
					break; // get out of the loop now, we should have everything and we don't want this packet removed
				}
			}
		
		
		}
		pdu++;
	}
	DEBUG( "Have pulled %d bytes for packet of size %d bytes\n", pkgdata, sdu.length+1 );	
	if( pkgdata == sdu.length+1 ){
		DEBUG("%s", "full PDU found\n" );
		buffer.erase( start,pdu );
		return 1;
	}
	
	return 0;
}


