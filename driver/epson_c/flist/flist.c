// $Id: flist.cpp,v 1.2 2008/04/11 06:28:10 hgates Exp $

#include <cassert>
#include <error.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include "flist.h"

using namespace std;


const int FILE_SIZE_START = 4;


flist::flist( void ) : _count( 0 ), _size( 0 ), _list( 0 )
{
  // nada
}

flist::~flist( void )
{
  if ( _list ) {
    for ( int i = 0; i < _count; i++ ) {
      if ( _list[i] ) delete [] _list[i];
    } // for i
    delete [] _list;
  }
}

void flist::read( const char * fn )
{
  if ( _list == 0 ) {
    _size = FILE_SIZE_START;
    _list = new char * [ _size ];
    if ( _list == NULL ) error( 1, 0, "!!! ERROR: no memory for _list" );
    _count = 0;
    for ( int i = 0; i < _size; i++ ) _list[i] = 0;
  }

  ifstream ifs( fn );
  if ( !ifs.is_open( ) ) error( 1, 0, "!!! ERROR: failed to open %s", fn );

  char str[100];

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
    int len = strlen( str );
    char * cp = new char [ len+1 ];
    strcpy( cp, str );
    if ( _count >= _size ) expand( );
    _list[_count++] = cp;
  } // while

  ifs.close( );
}

const char * flist::file( int n )
{
  if ( ( n < 0 ) || ( n >= _count ) )
    error( 1, 0, "!!! ERROR: invalid file number (%d)", n );
  return _list[n];
}

void flist::expand( void )
{
  int s = _size * 2;
  char ** lst = new char * [ s ];
  if ( lst == NULL ) error( 1, 0, "!!! ERROR: no memory for _list" );
  for ( int i = 0; i < s; i++ ) {
    if ( i < _size ) lst[i] = _list[i];
    else lst[i] = 0;
  }
  delete [] _list;
  _list = lst;
  _size = s;
}

// end of file
