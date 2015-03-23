#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "lrit.h"

int run = 1;

void handler(int i){
	run = 0;
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

	//char start_pattern[4] = {0x1A,0xCF, 0xFC, 0x1D};
	//char buffer[5] = {0};
	//int i = 0;
	unsigned int start_pattern = 0x1ACFFC1D;
	unsigned int received  = 0;
	while(run){
		unsigned char buffer;
		int numbytes = recv(sock,&buffer,1, 0 );
		if( numbytes < 0){
			printf("recieve failed");
			run = 0;
			continue;
		}else if( numbytes == 0 ){
			continue;
		}
		received = (received << 8) + buffer;

		
		if( received == start_pattern ){
			printf( "Found packet\n" );
			received=0; // clear the little buffer
			int totalbytes = 0;
			struct lrit_packet packet;
			while( totalbytes < sizeof(packet) ){
				numbytes = recv(sock,&((char*)&packet)[totalbytes],sizeof(packet)-totalbytes,0);
				if( numbytes < 0 ){
					printf("Packet Recieve Failed");
					run = 0;
					continue;
				}
				totalbytes+=numbytes;
			}
			
			process_packet( &packet );
			printf("Packet Processed\n");
		}

	}

	
	close(sock);
	

	return 0;
}
