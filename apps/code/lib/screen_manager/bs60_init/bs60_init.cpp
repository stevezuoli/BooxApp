// $Id: bs60_init.cpp,2008/11/13

#ifdef ENABLE_EINK_SCREEN

#include <cassert>
#include <error.h>
#include <cstdlib>
#include <ctype.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include "bs_chip.h"
#include "bs_cmd.h"
#include "bs60_init.h"

using namespace std;

int bs60_str2int( const char * str )
{
    int base = 10;
    if ( strlen( str ) > 1 ) {
        if ( ( str[0] == '0' ) && ( str[1] == 'x' ) ) base = 16;
    }
    int v = strtol( str, 0, base );
    return v;
}

void bs60_ld_value( int v )
{
    unsigned char tmp[600 * 800];
    memset(&tmp[0], v & 0xff, 600 * 800);
    bs_cmd_ld_img_area_data( 3, 0, 0, 600, 800, &tmp[0], 600);
}

void bs60_black( void )
{
    printf( "[%s] ... displaying black\n", __FUNCTION__ );
    bs60_ld_value( 0x00 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 3, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

void bs60_white( void )
{
    printf( "[%s] ... displaying white\n", __FUNCTION__ );
    bs60_ld_value( 0xFF );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 3, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

void bs60_flash( void )
{
    //printf( "[%s] ... flashing\n", __FUNCTION__ );

    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );

    bs_cmd_upd_full( 0, 0, 0 );
    //bs_cmd_wait_dspe_trg( );
    //bs_cmd_wait_dspe_frend( );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );

    bs60_ld_value( 0xFF );
    bs_cmd_upd_init( );
    bs_cmd_wait_dspe_trg( );
}

void bs60_init( int wa )
{
    printf( "[%s] ... initializing broadsheet for 6-inch panel with reset support and auto select . \n", __FUNCTION__ );

    bsc.reset();
    bsc.init_controller();

    // printf("before bs_cmd_set_hif_mode_cmd!\n");

    bs_cmd_set_hif_mode_cmd( );

    // printf("after bs_cmd_set_hif_mode_cmd!\n");
    bs_cmd_wr_reg( 0x006, 0x0000 );

    // printf("after bs_cmd_wr_reg!\n");

    // It's really very important to sleep here to make sure the controller
    // is correctly power on.
    bsc.sleep( 5 );     /// According to the faq.

    // printf("after bs_sleep!\n");

    bs_cmd_init_sys_run( );

    // printf("after init sys run!\n");

    bs_cmd_wr_reg( 0x0106, 0x0203 );
    // printf("after     bs_cmd_wr_reg( 0x0106, 0x0203 )!\n");

    bs_cmd_init_dspe_cfg( BS60_INIT_HSIZE,
                          BS60_INIT_VSIZE,
                          BS60_INIT_SDRV_CFG,
                          BS60_INIT_GDRV_CFG,
                          BS60_INIT_LUTIDXFMT );

    bs_cmd_init_dspe_tmg( BS60_INIT_FSLEN,
                          ( BS60_INIT_FELEN << 8 ) | BS60_INIT_FBLEN,
                          BS60_INIT_LSLEN,
                          ( BS60_INIT_LELEN << 8 ) | BS60_INIT_LBLEN,
                          BS60_INIT_PIXCLKDIV );
    bs_cmd_print_disp_timings( );

    bs_cmd_set_wfm( wa );
    bs_cmd_print_wfm_info( );

    printf( "[%s] ... display engine initialized with waveform\n", __FUNCTION__ );

    bs_cmd_clear_gd( );

    bs_cmd_wr_reg( 0x01A, 4 ); // i2c clock divider
    bs_cmd_wr_reg( 0x320, 0 ); // temp auto read on, see pag 213. Turn off the temp sensor.

    bs_cmd_set_lut_auto_sel_mode(1); // make sure auto-lut mode is off
    bs_cmd_set_rotmode(3);  // set rotation mode
}

#ifdef BS60_INIT_MAIN

void usage( const char * pn )
{
    cout << "Usage: " << pn
        << " <wfm_addr>" << endl;

    exit( 1 );
}

int main( int argc, char * argv[] )
{
    int wfm_addr = 0x886;
    if ( argc == 2 )
    {
        wfm_addr = bs60_str2int( argv[1] );
    }
    bs60_init( wfm_addr );

    // Disable the flash as it's not necessary.
    // bs60_flash( );

    return 0;
}

#endif // BS60_INIT_MAIN

#endif // ENABLE_EINK_SCREEN

// end of file
