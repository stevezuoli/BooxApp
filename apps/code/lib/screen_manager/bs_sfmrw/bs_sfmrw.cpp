// $Id: bs_sfmrw.cpp,v 1.1 2008/05/16 07:30:35 hgates Exp $

#include <cassert>
#include <error.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include "bs_sfm.h"

#ifdef ENABLE_EINK_SCREEN

using namespace std;

char * pname;

bs_chip bsc;

const int INIT_PWR_SAVE_MODE = 0x0000;

const int INIT_PLL_CFG_0 = 0x0004;
const int INIT_PLL_CFG_1 = 0x5949;
const int INIT_PLL_CFG_2 = 0x0040;
const int INIT_CLK_CFG   = 0x0000;

const int INIT_SPI_FLASH_ACC_MODE = 0; // access mode select
const int INIT_SPI_FLASH_RDC_MODE = 0; // read command select
const int INIT_SPI_FLASH_CLK_DIV  = 3;  // clock divider
const int INIT_SPI_FLASH_CLK_PHS  = 0; // clock phase select
const int INIT_SPI_FLASH_CLK_POL  = 0; // clock polarity select
const int INIT_SPI_FLASH_ENB      = 1; // enable
const int INIT_SPI_FLASH_CTL =
  ( INIT_SPI_FLASH_ACC_MODE << 7 ) |
  ( INIT_SPI_FLASH_RDC_MODE << 6 ) |
  ( INIT_SPI_FLASH_CLK_DIV << 3 ) |
  ( INIT_SPI_FLASH_CLK_PHS << 2 ) |
  ( INIT_SPI_FLASH_CLK_POL << 1 ) |
  INIT_SPI_FLASH_ENB;

const int INIT_SPI_FLASH_CS_ENB = 1;
const int INIT_SPI_FLASH_CSC = INIT_SPI_FLASH_CS_ENB;


void init_pll( void )
{
  BSC_WR_REG( 0x006, 0x0000 );
  BSC_WR_REG( 0x010, INIT_PLL_CFG_0 );
  BSC_WR_REG( 0x012, INIT_PLL_CFG_1 );
  BSC_WR_REG( 0x014, INIT_PLL_CFG_2 );
  BSC_WR_REG( 0x016, INIT_CLK_CFG );
  int v = BSC_RD_REG( 0x00A );
  while ( ( v & 0x1 ) == 0 ) v = BSC_RD_REG( 0x00A );
  v = BSC_RD_REG( 0x006 );
  BSC_WR_REG( 0x006, v & ~0x1 );
}

void init_spi( void )
{
  BSC_WR_REG( 0x204, INIT_SPI_FLASH_CTL );
  BSC_WR_REG( 0x208, INIT_SPI_FLASH_CSC );
}

void test_regs( void )
{
  int d = BSC_RD_REG( 0x000 );
  printf( "[%s] ... REG[0x000] = 0x%04x\n", __FUNCTION__, d );
  d = BSC_RD_REG( 0x0002 );
  printf( "[%s] ... REG[0x002] = 0x%04x\n", __FUNCTION__, d );
  d = BSC_RD_REG( 0x0004 );
  printf( "[%s] ... REG[0x004] = 0x%04x\n", __FUNCTION__, d );
  BSC_WR_REG( 0x304, 0x0123 );
  BSC_WR_REG( 0x30A, 0x4567 );
  d = BSC_RD_REG( 0x304 );
  assert( d == 0x0123 );
  d = BSC_RD_REG( 0x30A );
  assert( d == 0x4567 );
}

void init( void )
{
  bsc.init_controller( );
  sleep( 1 );
  init_pll( );
  BSC_WR_REG( 0x106, 0x0203 );
  init_spi( );
  test_regs( );
}

int get_value( const char * str )
{
  int base = 10;
  if ( strlen( str ) > 1 ) {
    if ( ( str[0] == '0' ) && ( str[1] == 'x' ) ) base = 16;
  }
  int v = strtol( str, 0, base );
  return v;
}

void usage( void )
{
  cout << "Usage: " << pname << "  <read/write> <address> <byte_count> <bin_file>"
       << endl;
  exit( 1 );
}

int main( int argc, char * argv[] )
{
  pname = argv[0];
  if ( argc != 5 ) usage( );

  int mode = 0;

  int a = 1;
  if ( !strcmp( argv[a], "read" ) ) mode = 0;
  else if ( !strcmp( argv[a], "write" ) ) mode = 1;
  else error( 1, 0, "[%s] !!! ERROR: invalid mode argument (%s)", pname, argv[a] );
  a++;

  int addr = get_value( argv[a++] );
  int size = get_value( argv[a++] );

  init( );

  bs_sfm_start( );
  printf( "... staring sfm access\n" );

  int es = bs_sfm_esig( );
  // Need to support both M25P20 and M25P40
  if ( es != BS_SFM_ESIG && es != 0x12 && es != 0x11 ) error( 1, 0,
                  "[%s] !!! ERROR: invalid sfm electronic signature (0x%02x) --- 0x%02x expected\n",
                  pname, es, BS_SFM_ESIG );

  printf( "... sfm esig=0x%02x\n", es );

  if ( mode == 0 ) bs_sfm_read_file( addr, size, argv[a] );
  else if ( mode == 1 ) bs_sfm_write_file( addr, size, argv[a] );

  printf( "... ending sfm access\n" );
  bs_sfm_end( );

  return 0;
}

#endif // #ifdef ENABLE_EINK_SCREEN

// end of file
