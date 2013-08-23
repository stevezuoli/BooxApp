// $Id: ptm.cpp,v 1.2 2008/04/11 06:28:11 hgates Exp $

#include <cassert>
#include "error.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "ptm.h"

using namespace std;


ptm::ptm( void ) :  _width( 0 ), _height( 0 ), _size( 0 ), _data( 0 )
{
  // nada
}

ptm::~ptm( void )
{
  if ( _data ) delete [] _data;
}

void ptm::init( void )
{
  if ( _data ) delete [] _data;
  _width = 0;
  _height = 0;
  _size = 0;
}

void ptm::read_file( const char * fn )
{
  std::ifstream ifs( fn );
  if ( !ifs.is_open( ) ) error( 1, 0, "[%s] !!! ERROR: failed to open %s for read", __FUNCTION__, fn );

  int state = 0;
  char str[32];
  int v;
  int max_pix_val = 0;

  while ( state < 4 ) {
    ifs >> str;
    if ( ifs.eof( ) ) error( 1, 0, "[%s] !!! ERROR: unexpected EOF in %s", __FUNCTION__, fn );
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
      if ( strcmp( str, "PTM" ) )
        error( 1, 0, "[%s] !!! ERROR: invalid ppm/ptm format (%s) in %s", __FUNCTION__, str, fn );
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
      if ( max_pix_val != 255 ) error( 1, 0, "[%s] !!! ERROR: invalid max pixel value (%d) in %s",
                       __FUNCTION__, max_pix_val, fn );
      state = 4;
      break;
    } // switch
  } // while

  if ( state != 4 )
    error( 1, 0, "[%s] !!! ERROR: invalid header in %s", __FUNCTION__, fn );

  while ( true ) {
    char c = ifs.get( );
    if ( c == '\n' ) break;
  } // while

  _size = _width * _height;

  if ( _data ) delete [] _data;

  _data = new char [ _size ];
  if ( _data == NULL )
    error( 1, 0, "[%s] !!! ERROR: failed to allocate memory for data", __FUNCTION__ );

  int rlenc = 1;
  int var = 0;
  int count = 0;
  int value = 0;
  int n = 0;

  while ( true ) {
    ifs >> str;
    if ( ifs.eof( ) ) break;
    if ( str[0] == '#' ) {
      while ( true ) {
        char c = ifs.get( );
        if ( c == '\n' ) break;
      } // while
      continue;
    }
    if ( rlenc ) {
      if ( var == 0 ) {
    if ( !strcmp( str, "@" ) ) { rlenc = 0; continue; }
    count = get_value( str );
    var = 1;
      }
      else {
    value = get_value( str );
    var = 0;
    if ( n >= _size )
      error( 1, 0, "[%s] !!! ERROR: too many data in %s", __FUNCTION__, fn );
    for ( int i = 0; i < count; i++ ) _data[n++] = char( value );
      }
    }
    else {
      if ( !strcmp( str, "@" ) ) { rlenc = 1; continue; }
      value = get_value( str );
      if ( n >= _size )
    error( 1, 0, "[%s] !!! ERROR: too many data in %s", __FUNCTION__, fn );
      _data[n++] = char( value );
    }
  } // while

  ifs.close( );

  if ( ( rlenc == 0 ) || ( var != 0 ) )
    error( 1, 0, "[%s] !!! ERROR: invalid data structure in %s", __FUNCTION__, fn );
  if ( n != _size ) {
    error( 1, 0, "[%s] !!! ERROR: data count error in %s", __FUNCTION__, fn );
  }
}

int ptm::get_value( const char * str )
{
  int len = strlen( str );
  int base = 10;
  if ( len > 1 ) {
    if ( ( str[0] == '0' ) && ( str[1] == 'x' ) ) base = 16;
  }
  int v = strtol( str, 0, base );
  return v;
}

int ptm::data( int n ) const
{
  if ( ( n < 0 ) || ( n >= _size ) )
    error( 1, 0, "[%s] !!! ERROR: invalid data number (%d)", __FUNCTION__, n );
  return _data[n];
}


int ptm::repeat_count( int x, int d )
{
  int n = 0;
  int y = x;
  while ( y < _size ) {
    int v = int( _data[y] ) & 0xFF;
    if ( v != d ) break;
    n++;
    y++;
  } // while
  return n;
}

void ptm::write_file( const char * fn )
{
  std::ofstream ofs( fn );
  if ( !ofs.is_open( ) ) error( 1, 0, "[%s] !!! ERROR: failed to open %s for write", __FUNCTION__, fn );
  ofs << "PTM" << endl
      << _width << " "
      << _height << endl
      << "255" << endl;
  char str[30];
  int rlenc = 1;
  int x = 0;
  int m = 0;
  while ( x < _size ) {
    int d = int( _data[x] ) & 0xFF;
    sprintf( str, "0x%02x", d );
    int n = repeat_count( x, d );
    if ( n < 3 ) {
      if ( rlenc == 1 ) {
    ofs << "@ # " << x << endl;
    m = 0;
    rlenc = 0;
      }
      for ( int i = 0; i < n; i++ ) {
    ofs << str << " ";
    m++;
    if ( m >= 16 ) { ofs << endl; m = 0; }
      } // for i
    }
    else /* if ( n >= 3 ) */ {
      if ( rlenc == 0 ) {
    if ( m > 0 ) { ofs << endl; m = 0; }
    ofs << "@ # " << x << endl;
    rlenc = 1;
      }
      ofs << n << " " << str << endl;
    }
    x += n;
  } // while
  assert( x == _size );
  ofs << endl
      << "# end of file" << endl;
  ofs.close( );
}

void ptm::data( int n, int d )
{
  if ( _data == 0 ) {
    _size = _width * _height;
    if ( _size <= 0 )
      error( 1, 0, "[%s] !!! ERROR: invalid width (%d) or height (%d)", __FUNCTION__, _width, _height );
    _data = new char [ _size ];
    if ( _data == NULL )
      error( 1, 0, "[%s] !!! ERROR: failed to allocate memory for data", __FUNCTION__ );
  }
  if ( ( n < 0 ) || ( n >= _size ) )
    error( 1, 0, "[%s] !!! ERROR: invalid data number (%d)", __FUNCTION__, n );
  _data[n] = char( d );
}

// end of file
