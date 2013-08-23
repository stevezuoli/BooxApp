// $Id: flist.h,v 1.2 2008/04/11 06:28:10 hgates Exp $

#ifndef __FLIST_H__
#define __FLIST_H__


class flist
{
 public:

  flist( void );
  ~flist( void );

 public:

  void read( const char * fn );
  int  count( void ) const { return _count; }
  const char * file( int n );

 private:

  void expand( void );

 private:

  int _count;
  int _size;
  char ** _list;
};

#endif // __FLIST_H__

// end of file
