#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "lrit.h"
#include "fileproc.h"
#include "util.h"

int run = 1;
int debug = 0;
void handler(int i){
	run = 0;
	//printf("Closing Proram\n");
	DEBUG("%s", "Closing Program\n" );
}

int main( int argc, char* argv[] ){

	if( argc != 3 ){
		printf( "Usage: %s address port\n", argv[0] );
		return 1;
	}

	
	char *serverAddressStr = argv[1];
	in_port_t serverPort = atoi( argv[2] );
	
	int sock = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	if( sock < 0 ){
		printf("Failed to create socket");
		return 1;
	}
	
	struct sockaddr_in serverAddress;
	memset( &serverAddress, 0, sizeof(serverAddress) );
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons( serverPort );
	
	int retval = inet_pton( AF_INET, serverAddressStr, &serverAddress.sin_addr.s_addr);
	if( retval < 0 ){
		printf( "Address Invalid" );
		return 1;
	}
	retval = connect( sock, (struct sockaddr*)&serverAddress,sizeof(serverAddress) );
	if( retval < 0 ){
		printf( "Failed to connect to %s:%d", argv[1], serverPort );
		return 1;
	}
	
	signal(SIGINT,handler);
	SDU_Processor sdu_proc;
	FILE_Processor file_proc;
	//char start_pattern[4] = {0x1A,0xCF, 0xFC, 0x1D};
	//char buffer[5] = {0};
	//int i = 0;
	unsigned int start_pattern = 0x1ACFFC1D;
	unsigned int received  = 0;
	while(run){
		unsigned char buffer;
		int numbytes = recv(sock,&buffer,1, 0 );
		if( numbytes < 0){
			printf("recieve failed: %d\n", numbytes);
			run = 0;
			continue;
		}else if( numbytes == 0 ){
			continue;
		}
		received = (received << 8) + buffer;

		
		if( received == start_pattern ){
			DEBUG( "%s", "Found packet\n" );
			received=0; // clear the little buffer
			int totalbytes = 0;
			struct lrit_packet packet;
			M_SDU sdu;
			memset( &sdu, 0, sizeof(sdu) );
			uint8_t buf2[1024] = {0};
			while( totalbytes < 1020 ){
				numbytes = recv(sock,&(buf2[totalbytes]),1020-totalbytes,0);
				if( numbytes < 0 ){
					printf("Packet Recieve Failed: %d\n ", numbytes);
					perror(NULL);
					run = 0;
					break;
				}
				totalbytes+=numbytes;
			}
			if( numbytes >= 0 && totalbytes == 1020){
				//memcpy( (unsigned char*)&packet, buf2, 1020 );
				// NOTE, below is probably not endian safe
				uint16_t tmp;
				memcpy( &tmp, buf2, 2);
				packet.primary_header.version   = (tmp & 0xC000) >> 14;
				packet.primary_header.spacecraft= (tmp & 0x3FC0) >>  6;
				packet.primary_header.vc_id     = (tmp & 0x003F);
				
				uint32_t tmp2;
				memcpy( &tmp2, &buf2[2],4);
				packet.primary_header.counter   = (tmp2 & 0xFFFFFF00) >>  8;
				packet.primary_header.replay    = (tmp2 & 0x00000080) >>  7;
				packet.primary_header.spare     = (tmp2 & 0x0000007F);
				memcpy( packet.data, (buf2+6), 886 );

				DEBUG( "%s","Processing Packet from spacecraft %d, packet number %d, vc_id: %d\n", packet.primary_header.spacecraft, packet.primary_header.counter, packet.primary_header.vc_id);
				int status = sdu_proc.process_packet( &packet, sdu );
				if( status ){
					DEBUG("%s","We have a complete M_PDU packet\n");
					file_proc.process_sdu( sdu );
					DEBUG("%s","Packet Processed\n");
				}
				// check status, do something
			}
		}
		

	}

	
	close(sock);
	

	return 0;
}
