// $Id: bs_sfm.cpp,v 1.1 2008/05/16 07:30:35 hgates Exp $

#include <cassert>
#include <error.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "bs_sfm.h"

#ifdef ENABLE_EINK_SCREEN

using namespace std;

//#define DEBUG 1


static int sfm_cd;


void bs_sfm_wait_for_bit( int ra, int pos, int val )
{
  int v = BSC_RD_REG( ra );
  while ( ( ( v >> pos ) & 0x1 ) != ( val & 0x1 ) ) v = BSC_RD_REG( ra );
}


void bs_sfm_start( void )
{
  sfm_cd = BSC_RD_REG( 0x0204 ); // spi flash control reg
  BSC_WR_REG( 0x0208, 0 );
  BSC_WR_REG( 0x0204, 0 ); // disable
  int access_mode= 0;
  int enable = 1;
  int v = ( access_mode << 7 ) |
    ( SFM_READ_COMMAND << 6 ) |
    ( SFM_CLOCK_DIVIDE << 3 ) |
    ( SFM_CLOCK_PHASE << 2 ) |
    ( SFM_CLOCK_POLARITY << 1 ) |
    enable;
  BSC_WR_REG( 0x0204, v );
}

void bs_sfm_end( void )
{
  BSC_WR_REG( 0x0204, sfm_cd );
}

void bs_sfm_wr_byte( int data )
{
  int v = ( data & 0xFF ) | 0x100;
  BSC_WR_REG( 0x202, v );
  bs_sfm_wait_for_bit( 0x206, 3, 0 );
}

int bs_sfm_rd_byte( void )
{
  BSC_WR_REG( 0x202, 0 );
  bs_sfm_wait_for_bit( 0x206, 3, 0 );
  int v = BSC_RD_REG( 0x200 );
  return ( v & 0xFF );
}

int bs_sfm_esig( void )
{
#if 0
  BSC_WR_REG( 0x208, 1 );
  bs_sfm_wr_byte( BS_SFM_DP );
  BSC_WR_REG( 0x208, 0 );
#endif
  BSC_WR_REG( 0x208, 1 );
  bs_sfm_wr_byte( BS_SFM_RES );
  bs_sfm_wr_byte( 0 );
  bs_sfm_wr_byte( 0 );
  bs_sfm_wr_byte( 0 );
  int es = bs_sfm_rd_byte( );
  BSC_WR_REG( 0x208, 0 );
  return es;
}

int bs_sfm_read_status( void )
{
  BSC_WR_REG( 0x0208, 1 );
  bs_sfm_wr_byte( BS_SFM_RDSR );
  int s = bs_sfm_rd_byte( );
  BSC_WR_REG( 0x0208, 0 );
  return s;
}


void bs_sfm_write_enable( void )
{
  BSC_WR_REG( 0x0208, 1 );
  bs_sfm_wr_byte( BS_SFM_WREN );
  BSC_WR_REG( 0x0208, 0 );
}

void bs_sfm_write_disable( void )
{
  BSC_WR_REG( 0x0208, 1 );
  bs_sfm_wr_byte( BS_SFM_WRDI );
  BSC_WR_REG( 0x0208, 0 );
}

void bs_sfm_erase( int addr )
{
  printf( "... erasing sector (0x%06x)\n", addr );
  bs_sfm_write_enable( );
  BSC_WR_REG( 0x0208, 1 );
  bs_sfm_wr_byte( BS_SFM_SE );
  bs_sfm_wr_byte( ( addr >> 16 ) & 0xFF );
  bs_sfm_wr_byte( ( addr >> 8 ) & 0xFF );
  bs_sfm_wr_byte( addr & 0xFF );
  BSC_WR_REG( 0x0208, 0 );
  while ( true ) {
    int s = bs_sfm_read_status( );
    if ( ( s & 0x1 ) == 0 ) break;
  } // while
}



void bs_sfm_read( int addr, int size, char * data )
{
  printf( "... reading the serial flash memory (address=0x%06x, size=%d)\n", addr, size );
  BSC_WR_REG( 0x0208, 1 );
  bs_sfm_wr_byte( BS_SFM_READ );
  bs_sfm_wr_byte( ( addr >> 16 ) & 0xFF );
  bs_sfm_wr_byte( ( addr >> 8 ) & 0xFF );
  bs_sfm_wr_byte( addr & 0xFF );
  for ( int i = 0; i < size; i++ ) data[i] = bs_sfm_rd_byte( );
  BSC_WR_REG( 0x0208, 0 );
  printf( "... reading the serial flash memory --- done\n" );
}

void bs_sfm_program_page( int pa, int size, char * data )
{
  bs_sfm_write_enable( );
  BSC_WR_REG( 0x0208, 1 );
  bs_sfm_wr_byte( BS_SFM_PP );
  bs_sfm_wr_byte( ( pa >> 16 ) & 0xFF );
  bs_sfm_wr_byte( ( pa >> 8 ) & 0xFF );
  bs_sfm_wr_byte( pa & 0xFF );
  for ( int d = 0; d < BS_SFM_PAGE_SIZE; d++ ) {
    bs_sfm_wr_byte( data[d] );
  } // for d
  BSC_WR_REG( 0x0208, 0 );
  while ( true ) {
    int s = bs_sfm_read_status( );
    if ( ( s & 0x1 ) == 0 ) break;
  } // while
}

void bs_sfm_program_sector( int sa, int size, char * data )
{
  printf( "... programming sector (0x%06x)\n", sa );
  int pa = sa;
  for ( int p = 0; p < BS_SFM_PAGE_COUNT; p++ ) {
    int y = p * BS_SFM_PAGE_SIZE;
    bs_sfm_program_page( pa, BS_SFM_PAGE_SIZE, &data[y] );
    pa += BS_SFM_PAGE_SIZE;
  } // for p
}

void bs_sfm_write( int addr, int size, char * data )
{
  printf( "... writing the serial flash memory (address=0x%06x, size=%d)\n", addr, size );

  char * sd = new char [ BS_SFM_SECTOR_SIZE ];
  if ( sd == NULL ) error( 1, 0, "[%s] !!! ERROR: no memory for sd", __FUNCTION__ );

  int s1 = addr / BS_SFM_SECTOR_SIZE;
  int s2 = ( addr + size - 1 ) / BS_SFM_SECTOR_SIZE;

  int x = 0;
  for ( int s = s1; s <= s2; s++ ) {
    int sa = s * BS_SFM_SECTOR_SIZE;
    int start = 0;
    int count = BS_SFM_SECTOR_SIZE;
    if ( s == s1 ) {
      if ( addr > sa ) {
	start = addr - sa;
	bs_sfm_read( sa, start, sd );
      }
    }
    if ( s == s2 ) {
      int limit = addr + size;
      if ( ( sa + BS_SFM_SECTOR_SIZE ) > limit ) {
	count = limit - sa;
	bs_sfm_read( limit, ( sa + BS_SFM_SECTOR_SIZE - limit ), &sd[count] );
      }
    }
    bs_sfm_erase( sa );
    for ( int i = start; i < count; i++ ) {
      assert( x < size );
      sd[i] = data[x++];
    }
    bs_sfm_program_sector( sa, BS_SFM_SECTOR_SIZE, sd );
  } // for s

  bs_sfm_write_disable( );

  delete [] sd;
  
  char * rd = new char [ size ];
  assert( rd != NULL );

  printf( "... verifying the serial flash memory write\n" );

  bs_sfm_read( addr, size, rd );

  for ( int i = 0; i < size; i++ ) {
    if ( rd[i] != data[i] ) {
      printf( "+++++++++++++++ rd[%d]=0x%02x  data[%d]=0x%02x\n", i, rd[i], i, data[i] );
      error( 1, 0, "[%s] !!! ERROR: failed to verify the flash memory write data", __FUNCTION__ );
    }
  } // for i

  delete [] rd;

  printf( "... writing the serial flash memory --- done\n" );
}

void bs_sfm_write_file( int addr, int size, const char * fn )
{
  ifstream ifs( fn );
  if ( !ifs.is_open( ) ) error( 1, 0, "[%s] !!! ERROR: failed to open %s", __FUNCTION__, fn );

  char * data = new char [ size ];
  if ( data == NULL ) error( 1, 0, "[%s] !!! ERROR: no memory for data", __FUNCTION__ );

  ifs.read( data, size );
  if ( ifs.gcount( ) != size )  error( 1, 0, "[%s] !!! ERROR: data read error from %s", __FUNCTION__, fn );

  bs_sfm_write( addr, size, data );

  delete [] data;

  ifs.close( );
}

void bs_sfm_read_file( int addr, int size, const char * fn )
{
  ofstream ofs( fn );
  if ( !ofs.is_open( ) ) error( 1, 0, "[%s] !!! ERROR: failed to open %s", __FUNCTION__, fn );

  char * data = new char [ size ];
  if ( data == NULL ) error( 1, 0, "[%s] !!! ERROR: no memory for data", __FUNCTION__ );

  bs_sfm_read( addr, size, data );

  ofs.write( data, size );

  delete [] data;

  ofs.close( );
}

#endif  // ENABLE_EINK_SCREEN

// end of file
