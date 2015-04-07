#ifndef UTIL_H
#define UTIL_H

extern int debug;
#define DEBUG(format,...) if( debug > 0){ printf( "%s:%d " format,__FILE__,__LINE__, __VA_ARGS__ ); }


#endif
