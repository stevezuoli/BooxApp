// $Id: pgm.cpp,v 1.2 2008/04/11 06:28:11 hgates Exp $

#include <stdio.h>
#include <stdlib.h>

#include "pgm.h"

#ifdef KERNEL_SPACE
#define DBG diag_DBG
#else
#define DBG printf
#endif

void init( PgmData* pgm )
{
    if (pgm->data)
    {
        free(pgm->data);
    }

    pgm->width = 0;
    pgm->height = 0;
    pgm->size = 0;
    pgm->data = 0;
}

void read_file( PgmData* pgm, const char * fn )
{
  FILE* fp = fopen(fn, "rb");
  if (fp == NULL)
  {
      DBG("!!! ERROR: failed to open %s for read.\n", fn );
      return;
  }

  int state = 0;
  char str[32];
  int max_pix_val = 0;

  while ( state < 3 ) {
    fgets(str, 32, fp);
    if ( feof(fp) ) DBG( "!!! ERROR: unexpected EOF in %s\n", fn );
    if ( str[0] == '#' ) {
      while ( 1 ) {
        char c = fgetc(fp);
        if ( c == '\n' ) break;
      } // while
      continue;
    }
    switch ( state ) {
    case 0:
      if ( strncmp( str, "P5", 2 ) && strncmp( str, "P6", 2 ) )
    DBG( "!!! ERROR: invalid ppm/pgm format (%s) in %s\n", str, fn );
      state = 1;
      break;
    case 1:
      sscanf(str, "%d %d", &pgm->width, &pgm->height);
      state = 2;
      break;
    case 2:
      max_pix_val = atoi(str);
      state = 3;
      break;
    } // switch
  } // while

  if ( state != 3 ) DBG( "!!! ERROR: invalid header in %s", fn );

  if ( max_pix_val != 255 )
    DBG( "!!! ERROR: invalid max pixel value (%d) in %s", max_pix_val, fn );

  pgm->size = pgm->width * pgm->height;

  if ( pgm->data ) free(pgm->data);

  pgm->data = malloc(pgm->size);
  if ( pgm->data == NULL ) DBG( "!!! ERROR: failed to allocate memory for data" );

  size_t bytes_read = fread(pgm->data, 1, pgm->size, fp);
  if (bytes_read != pgm->size)
    DBG( "!!! ERROR: failed to read data from %s", fn );

  fclose(fp);
}

int get_data( const PgmData* pgm, int n )
{
  if ( ( n < 0 ) || ( n >= pgm->size ) ) DBG( "!!! ERROR: invalid data number (%d)", n );
  return pgm->data[n];
}

void write_file( const PgmData* pgm, const char * fn )
{
  FILE* fp = fopen(fn, "w");
  if (fp == NULL)
  {
      DBG( "!!! ERROR: failed to open %s for write", fn );
  }

  char buf[256];
  int bytes_to_write = sprintf(buf, "P6\n""%d %d\n""255\n", pgm->width, pgm->height);
  fwrite(buf, 1, bytes_to_write, fp);

  fwrite(pgm->data, 1, pgm->size, fp);
  fclose(fp);
}

void set_width( PgmData* pgm, int w )
{
    pgm->width = w;
}

void set_height( PgmData* pgm, int h )
{
    pgm->height = h;
}

void set_data( PgmData* pgm, int n, int d )
{
  if ( pgm->data == 0 ) {
    pgm->size = pgm->width * pgm->height;
    if ( pgm->size <= 0 ) DBG( "!!! ERROR: invalid width (%d) or height (%d)", pgm->width, pgm->height );
    if ( pgm->data ) free(pgm->data);
    pgm->data = malloc(pgm->size);
    if ( pgm->data == NULL ) DBG( "!!! ERROR: failed to allocate memory for data" );
  }
  if ( ( n < 0 ) || ( n >= pgm->size ) ) DBG( "!!! ERROR: invalid data number (%d)", n );
  pgm->data[n] = d;
}

// end of file
