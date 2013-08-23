// $Id: ptm.h,v 1.2 2008/04/11 06:28:11 hgates Exp $

#ifndef __PTM_H__
#define __PTM_H__

class ptm
{
 public:

  ptm( void );
  ~ptm( void );

 public:

  void init( void );

 public:

  void read_file( const char * fn );
  int width( void ) const { return _width; }
  int height( void ) const { return _height; }
  int data( int n ) const;

 public:

  void write_file( const char * fn );
  void width( int w ) { _width = w; }
  void height( int h ) { _height = h; }
  void data( int n, int d );

 public:

  int  get_value( const char * str );
  int  repeat_count( int x, int d );

 private:

  int _width;
  int _height;
  int _size;
  char * _data;

};

#endif // __PTM_H__

// end of file
