// $Id: bs_sfm.h,v 1.1 2008/05/16 07:30:35 hgates Exp $

#ifndef __BS_SFM_H__
#define __BS_SFM_H__

#include "bs_chip.h"

#define BSC_RD_REG( a )     rd_reg( a )
#define BSC_WR_REG( a, d )  wr_reg( a, d )


// functions

extern void bs_sfm_start( void );
extern void bs_sfm_end( void );

extern void bs_sfm_wr_byte( int data );
extern int  bs_sfm_rd_byte( void );
extern int  bs_sfm_esig( void );

extern void bs_sfm_read( int addr, int size, char * data );
extern void bs_sfm_write( int addr, int size, char * data );
extern void bs_sfm_read_file( int addr, int size, const char * fn );
extern void bs_sfm_write_file( int addr, int size, const char * fn );

extern int  bs_sfm_read_status( void );
extern void bs_sfm_write_enable( void );
extern void bs_sfm_write_disable( void );
extern void bs_sfm_erase( int addr );
extern void bs_sfm_program_page( int pa, int size, char * data );
extern void bs_sfm_program_sector( int sa, int size, char * data );


// parameters

#define SFM_READ_COMMAND   0
#define SFM_CLOCK_DIVIDE   3
#define SFM_CLOCK_PHASE    0
#define SFM_CLOCK_POLARITY 0





// serial flash memory types


#if SFM_M25P10 > 0

#define BS_SFM_WREN 0x06
#define BS_SFM_WRDI 0x04
#define BS_SFM_RDSR 0x05
#define BS_SFM_READ 0x03
#define BS_SFM_PP   0x02
#define BS_SFM_SE   0xD8
#define BS_SFM_RES  0xAB
#define BS_SFM_ESIG 0x10

#define BS_SFM_SECTOR_COUNT 4
#define BS_SFM_SECTOR_SIZE  (32*1024)
#define BS_SFM_PAGE_SIZE    256
#define BS_SFM_PAGE_COUNT   (BS_SFM_SECTOR_SIZE/BS_SFM_PAGE_SIZE)

#endif // SFM_M25P10

#if SFM_M25P20 > 0

#define BS_SFM_WREN 0x06
#define BS_SFM_WRDI 0x04
#define BS_SFM_RDSR 0x05
#define BS_SFM_READ 0x03
#define BS_SFM_PP   0x02
#define BS_SFM_SE   0xD8
#define BS_SFM_RES  0xAB
#define BS_SFM_ESIG 0x11

#define BS_SFM_SECTOR_COUNT 4
#define BS_SFM_SECTOR_SIZE  (64*1024)
#define BS_SFM_PAGE_SIZE    256
#define BS_SFM_PAGE_COUNT   (BS_SFM_SECTOR_SIZE/BS_SFM_PAGE_SIZE)

#endif // SFM_M25P20



#endif // __BS_SFM_H__

// end of file
