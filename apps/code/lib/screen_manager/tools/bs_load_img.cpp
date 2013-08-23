


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ENABLE_EINK_SCREEN
#include "bs_cmd.h"
#include "ptm.h"
#include "pgm.h"

void bs_cmd_wr_pgm_data( pgm & p )
{
    int bc = p.width( ) * p.height( );
    u16 v = 0;
    int x = 0;
    for ( int i = 0; i < bc; i++ ) {
        if ( x == 0 ) v = p.data( i );
        else {
            v |= ( (u16) p.data( i ) ) << 8;
            bsc.data(v );
        }
        x = 1 - x;
    } // for i
    if ( bc & 0x1 ) bsc.data(v );
}

void bs_cmd_ld_img_pgm( u16 dfmt, const char * pgmf )
{
#if 1
    pgm a_pgm;
    a_pgm.read_file( pgmf );
    bs_cmd_ld_img( dfmt );
    bs_cmd_wr_pgm_data( a_pgm );
    bs_cmd_ld_img_end( );
#endif
#if 0
    bs_cmd_ld_img_area_pgm( dfmt, rot, 0, 0, pgmf );
    return;

    pgm a_pgm;
    a_pgm.read_file( pgmf );
    int w = a_pgm.width( );
    int h = a_pgm.height( );
    int bc = w * h;
    int xx = bs_cmd_rd_reg( 0x14C );
    int yy = bs_cmd_rd_reg( 0x14E );
    int ww = bs_cmd_rd_reg( 0x150 );
    int hh = bs_cmd_rd_reg( 0x152 );
    bs_cmd_wr_reg( 0x14C, 0 );
    bs_cmd_wr_reg( 0x14E, 0 );
    bs_cmd_wr_reg( 0x150, w );
    bs_cmd_wr_reg( 0x152, h );
    bs_cmd_ld_img( dfmt, rot, w, h );
    bs_cmd_wr_pgm_data( bc, a_pgm );
    bs_cmd_ld_img_end( );
    bs_cmd_wr_reg( 0x14C, xx );
    bs_cmd_wr_reg( 0x14E, yy );
    bs_cmd_wr_reg( 0x150, ww );
    bs_cmd_wr_reg( 0x152, hh );
#endif
}

void bs_cmd_ld_img_area_pgm( u16 dfmt, u16 x, u16 y, const char * pgmf )
{
    pgm a_pgm;
    a_pgm.read_file( pgmf );
    int w = a_pgm.width( );
    int h = a_pgm.height( );
    bs_cmd_ld_img_area( dfmt, x, y, w, h );
    bs_cmd_wr_pgm_data( a_pgm );
    bs_cmd_ld_img_end( );
}


void bs_cmd_wr_ptm_data( ptm & p )
{
    int bc = p.width( ) * p.height( );
    u16 v = 0;
    int x = 0;
    for ( int i = 0; i < bc; i++ ) {
        if ( x == 0 ) v = p.data( i );
        else {
            v |= ( (u16) p.data( i ) ) << 8;
            bsc.data( v );
        }
        x = 1 - x;
    } // for i
    if ( bc & 0x1 ) bsc.data( v );
}

void bs_cmd_ld_img_ptm( u16 dfmt, const char * ptmf )
{
    ptm a_ptm;
    a_ptm.read_file( ptmf );
    bs_cmd_ld_img( dfmt );
    bs_cmd_wr_ptm_data( a_ptm );
    bs_cmd_ld_img_end( );
}

void bs_cmd_ld_img_area_ptm( u16 dfmt, u16 x, u16 y, const char * ptmf )
{
    ptm a_ptm;
    a_ptm.read_file( ptmf );
    int w = a_ptm.width( );
    int h = a_ptm.height( );
    bs_cmd_ld_img_area( dfmt, x, y, w, h );
    bs_cmd_wr_ptm_data( a_ptm );
    bs_cmd_ld_img_end( );
}

#endif

#define PPMS 3

int main(int argc, char **argv){


    if(argc != 3){
        printf("Usage: bs_load_img <rotation> <image.pgm>\n\r");
        return(-1);
    }

#ifdef ENABLE_EINK_SCREEN
    bs_cmd_flag_hif_mode_cmd();                  // required for access to broadsheet commands
    bs_cmd_set_rotmode((atoi((char *)*++argv)));  // set rotation mode
    bs_cmd_ld_img_pgm(PPMS, *++argv);

    bs_cmd_wait_dspe_trg();              /* wait for op trigger to be free */
    bs_cmd_wait_dspe_frend();            /* wait for any in progress update to finish*/
    bs_cmd_upd_full(WAVEFORM_16_MODE, 15, 0);   /* request update */


#endif

    return(0);
}

