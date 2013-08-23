// $Id: pgm.h,v 1.2 2008/04/11 06:28:11 hgates Exp $

#ifndef __PGM_H__
#define __PGM_H__

class pgm
{
 public:

  pgm( void );
  ~pgm( void );

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

 private:

  int _width;
  int _height;
  int _size;
  char * _data;
};


#endif // __PGM_H__

// end of file
