#include "fileproc.h"
#include "crc.h"
#include "util.h"

void FILE_Processor::process_sdu( M_SDU &sdu ){
	Transport_File file;
	DEBUG("%s", "Processing PDU:\n");
	//for( int i = 0; i < sdu.length+1; i++ ){
	//	printf( "%X ", sdu.data[i] );
	//}
	uint8_t flag = sdu.seq_flag;
	//printf("\n");
	DEBUG( "\n%s","Checking sequence flag\n");
	/*
	 * Somehow putting the pdu.seq_flag directly in the switch caused a bus error on SPARC
	 */
	uint16_t sducrc;
	memcpy( &sducrc, sdu.data + sdu.length+1-2, 2 );
	DEBUG("SDU crc = 0x%X\n", sducrc );
	uint16_t calcedcrc = crcSlow( sdu.data, sdu.length+1-2 );
	DEBUG("Calced crc = 0x%X\n", calcedcrc );

	if( sducrc != calcedcrc ){
		DEBUG("%s","CRC invalid, tossing packet\n");
		return;
	}
       uint16_t apid = sdu.apid;
	
	switch( flag ){
		case SEQ_COMPLETE_FILE:{ // complete file, don't bother adding to the list just output

			      DEBUG("%s","We have a complete file\n");
			      fill_file_info( file, sdu );
			      file.tp_sdu = new unsigned char[file.length];
			      memcpy( file.tp_sdu, sdu.data+10, file.length-10 );
			      // now what? have a complete file, write to disk?
			      this->save_file( file );
			      break;
		      }		
		case SEQ_FIRST_SEGMENT:{					       					       
					       DEBUG("%s","Got first SDU of a file\n" );
						files_by_apid[apid].clear(); // clear out contents if there's an existing list, otherwise this makes a new one
						files_by_apid[apid].push_back( sdu );
					       
					       break;
				       }
		case SEQ_CONTINUATION:{
			DEBUG("%s", "Got continuing SDU of a file\n" );
			if( files_by_apid.count( apid ) == 1 ){
				files_by_apid[apid].push_back( sdu );
			}else{
				DEBUG("%s", "But we haven't started this file yet, ignoring");
			}
			
			break;
		}
		case SEQ_LAST_SEGMENT:{
			if( files_by_apid.count( apid ) == 1 ){
				files_by_apid[apid].push_back( sdu );
			}else{
				break;	
			}
			files_by_apid[apid].push_back( sdu );

			DEBUG("Got last SDU of a file, consisting of %d M_SDUs\n", files_by_apid[apid].size() );
			// build file
			try{
				list<M_SDU>::iterator _sdu = files_by_apid[apid].begin();

				//from the first header, get the base file info
				fill_file_info( file, *_sdu );
				// now build the file from all of the M_SDU packets
				file.tp_sdu = new unsigned char[file.length+1];//throw in the +1, in case of the filler portion
				memcpy(file.tp_sdu, _sdu->data+10, _sdu->length-10-2 ); // -10 this was the first M_SDU that's the transport header, -2 ignore the crc
				_sdu++;
				// note we need to check the sequence count before putting these together. we'll put zeros 
				// in for missing packets, but we need to increase the byte count.
				size_t bytes_stored = _sdu->length-10-2;
				while( _sdu != files_by_apid[apid].end() && bytes_stored < _sdu->length ){
					memcpy(file.tp_sdu+bytes_stored, _sdu->data, _sdu->length );
					bytes_stored += _sdu->length;
					_sdu++;
				}
			

				this->save_file( file );
			}catch(... ){
				printf("Failed to Create file for apid: %d", apid );
			}
			files_by_apid.erase(apid);
			break;
		 }
		 default:{
				 printf("%s:%d unknown packet type\n", __FILE__, __LINE__);
				break;
			 }

	}

	DEBUG("%s", "End process sdu\n"  );

}
void FILE_Processor::build_file(int apid, list<M_SDU> &sdulist){
	//loop through the SDUs and assemble a file together

}

void fill_file_info( Transport_File &file, M_SDU &sdu ){
	/*
      printf("0x");
      for( int i = 2; i < 10; i++ ){
	printf("%0X ", sdu.data[i] ); 
      }

      printf("\n");
      */
      uint64_t tmp=0;
      uint8_t tmp2;
      memcpy( &file.file_counter, sdu.data, sizeof(uint16_t) );
      for( int i = 0,j=56; i < 8; i++, j-=8 ){
		memcpy( &tmp2, sdu.data+2+i, sizeof( uint8_t ) );
		tmp = tmp | ((uint64_t)tmp2 << j);
      }
      file.length = tmp;
      file.length /= 8;
      memcpy( &tmp, sdu.data+2, sizeof(uint64_t) ); 

      DEBUG("File Counter: %d\n*********File Length: %" PRIu64",%"PRIu64 "\n", file.file_counter, tmp,file.length );

}

void FILE_Processor::save_file( Transport_File &file ){
	// open file
	// write contents
	uint8_t tmp8;
	uint16_t tmp16;
	uint32_t tmp32;
	uint64_t tmp64;
	PRIMARY_HEADER_RECORD phr;

	memcpy( &tmp8, file.tp_sdu, 1 );
	phr.header_type = tmp8;
	
	memcpy( &tmp16, file.tp_sdu+1, 2 );
	phr.length = tmp16;
	
	memcpy( &tmp8, file.tp_sdu+3, 1 );
	phr.file_type = tmp8;

	memcpy( &tmp32, file.tp_sdu+4, 4 );
	phr.total_header_length = tmp32;

	memcpy( &tmp64, file.tp_sdu+8, 8 );
	phr.data_field_length = tmp64/8;

	printf("%s:%d Got file with primary header TYPE=%d, LENGTH=%d, FILE CODE =%d, TOTAL HEADER LENGHT=%" PRIu32 ", DATA LENGTH =%" PRIu64 "\n", __FILE__, __LINE__, phr.header_type, phr.length, phr.file_type, phr.total_header_length, phr.data_field_length );
	if( phr.total_header_length > 22 ){
		memcpy( &tmp8, file.tp_sdu+9, 1);
		DEBUG("%s","2nd header type = %d", __FILE__, __LINE__, tmp8 );
	}
}
		




Transport_File::Transport_File(){ 
	tp_sdu = NULL; 
}

Transport_File::~Transport_File(){
	if( tp_sdu != NULL){
		delete[] tp_sdu;
	}

}

