// $Id: pgm.h,v 1.2 2008/04/11 06:28:11 hgates Exp $

#ifndef __PGM_H__
#define __PGM_H__

typedef struct
{
  int   width;
  int   height;
  int   size;
  char* data;
} PgmData;

void init( PgmData* pgm );

void read_file( PgmData* pgm, const char * fn );
int get_data( const PgmData* pgm, int n );

void write_file( const PgmData* pgm, const char * fn );
void set_width( PgmData* pgm, int w );
void set_height( PgmData* pgm, int h );
void set_data( PgmData* pgm, int n, int d );

#endif // __PGM_H__

// end of file
