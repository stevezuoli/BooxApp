// $Id: pgm.cpp,v 1.2 2008/04/11 06:28:11 hgates Exp $

#include <stdlib.h>
#include <string.h>

#include <cassert>
#include <iostream>
#include <fstream>

#include "error.h"
#include "pgm.h"

using namespace std;

pgm::pgm( void ) :  _width( 0 ), _height( 0 ), _size( 0 ), _data( 0 )
{
  // nada
}

pgm::~pgm( void )
{
  if ( _data ) delete [] _data;
}

void pgm::init( void )
{
  if ( _data ) delete [] _data;
  _width = 0;
  _height = 0;
  _size = 0;
}

void pgm::read_file( const char * fn )
{
  std::ifstream ifs( fn );
  if ( !ifs.is_open( ) )
    error( 1, 0, "!!! ERROR: failed to open %s for read", fn );

  int state = 0;
  char str[32];
  int v;
  int max_pix_val = 0;

  while ( state < 4 ) {
    ifs >> str;
    if ( ifs.eof( ) ) error( 1, 0, "!!! ERROR: unexpected EOF in %s", fn );
    if ( str[0] == '#' ) {
      while ( true ) {
        char c = ifs.get( );
        if ( c == '\n' ) break;
      } // while
      continue;
    }
    v = atoi( str );
    switch ( state ) {
    case 0:
      if ( strcmp( str, "P5" ) && strcmp( str, "P6" ) )
	error( 1, 0, "!!! ERROR: invalid ppm/pgm format (%s) in %s", str, fn );
      state = 1;
      break;
    case 1:
      _width = v;
      state = 2;
      break;
    case 2:
      _height = v;
      state = 3;
      break;
    case 3:
      max_pix_val = v;
      state = 4;
      break;
    } // switch
  } // while

  if ( state != 4 ) error( 1, 0, "!!! ERROR: invalid header in %s", fn );

  if ( max_pix_val != 255 )
    error( 1, 0, "!!! ERROR: invalid max pixel value (%d) in %s", max_pix_val, fn );

  while ( true ) {
    char c = ifs.get( );
    if ( c == '\n' ) break;
  } // while

  _size = _width * _height;

  if ( _data ) delete [] _data;

  _data = new char [ _size ];
  if ( _data == NULL ) error( 1, 0, "!!! ERROR: failed to allocate memory for data" );

  ifs.read( _data, _size );
  if ( ifs.gcount( ) != _size )
    error( 1, 0, "!!! ERROR: failed to read data from %s", fn );

  ifs.close( );
}

int pgm::data( int n ) const
{
  if ( ( n < 0 ) || ( n >= _size ) ) error( 1, 0, "!!! ERROR: invalid data number (%d)", n );
  return _data[n];
}


void pgm::write_file( const char * fn )
{
  std::ofstream ofs( fn );
  if ( !ofs.is_open( ) )
    error( 1, 0, "!!! ERROR: failed to open %s for write", fn );

  ofs << "P6" << endl
      << _width << " "
      << _height << endl
      << "255" << endl;
  cout.flush( );
  ofs.write( _data, _size );
  ofs.close( );
}

void pgm::data( int n, int d )
{
  if ( _data == 0 ) {
    _size = _width * _height;
    if ( _size <= 0 ) error( 1, 0, "!!! ERROR: invalid width (%d) or height (%d)", _width, _height );
    if ( _data ) delete [] _data;
    _data = new char [ _size ];
    if ( _data == NULL ) error( 1, 0, "!!! ERROR: failed to allocate memory for data" );
  }
  if ( ( n < 0 ) || ( n >= _size ) ) error( 1, 0, "!!! ERROR: invalid data number (%d)", n );
  _data[n] = char( d );
}

// end of file
