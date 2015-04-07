#ifndef FILE_PROC_H
#define FILE_PROC_H
#include <map>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <string>
#include <list>
#include <inttypes.h>
#include "lrit.h"
using namespace std;

class FILE_Processor{


	public:
		FILE_Processor(){}


		void process_sdu( M_SDU &sdu );
		
	private:

		void build_file(int apid, list<M_SDU> &sdulist );
		void save_file( Transport_File &file );
		
		// all M_SDU packets for a given APID
		map< uint16_t, list<M_SDU> > files_by_apid;

		string directory; // we are we going to store the files	

};

void fill_file_info( Transport_File &file, M_SDU &sdu );


#endif
