// $Id: bs80_init.cpp,v 1.2 2008/04/11 06:28:09 hgates Exp $

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
#include "bs80_init.h"

using namespace std;

int bs80_str2int( const char * str )
{
    int base = 10;
    if ( strlen( str ) > 1 ) {
        if ( ( str[0] == '0' ) && ( str[1] == 'x' ) ) base = 16;
    }
    int v = strtol( str, 0, base );
    return v;
}

void bs80_ld_value( int v )
{
    unsigned char tmp[1024 * 768];
    memset(&tmp[0], v & 0xff, 1024 * 768);
    bs_cmd_ld_img_area_data( 3, 0, 0, 768, 1024, &tmp[0], 768);
}

void bs80_black( void )
{
    printf( "[%s] ... displaying black\n", __FUNCTION__ );
    bs80_ld_value( 0x00 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 3, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

void bs80_white( void )
{
    printf( "[%s] ... displaying white\n", __FUNCTION__ );
    bs80_ld_value( 0xFF );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 3, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

void bs80_flash( void )
{
    //printf( "[%s] ... flashing\n", __FUNCTION__ );

    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );

    bs_cmd_upd_full( 0, 0, 0 );
    //bs_cmd_wait_dspe_trg( );
    //bs_cmd_wait_dspe_frend( );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );

    bs80_ld_value( 0xFF );
    bs_cmd_upd_init( );
    bs_cmd_wait_dspe_trg( );
}

void bs80_init( int wa )
{
    printf( "[%s] ... initializing broadsheet for 8-inch panel\n", __FUNCTION__ );

    bsc.init_controller( );
    //bsc.test_gpio( );

    printf( "[%s] ... gpio pins configured and initialized\n", __FUNCTION__ );

    bs_cmd_set_hif_mode_cmd( );

    bs_cmd_wr_reg( 0x006, 0x0000 );
    // It's really very important to sleep here to make sure the controller
    // is correctly power on.
    bsc.sleep( 5 );     /// According to the faq.

    bs_cmd_init_sys_run( );
    bs_cmd_wr_reg( 0x0106, 0x0203 );
    bs_cmd_init_dspe_cfg( BS80_INIT_HSIZE, BS80_INIT_VSIZE,
                          BS80_INIT_SDRV_CFG, BS80_INIT_GDRV_CFG, BS80_INIT_LUTIDXFMT );

    // bs_cmd_init_dspe_tmg( u16 fs, u16 fbe, u16 ls, u16 lbe, u16 pixclkcfg )

    bs_cmd_init_dspe_tmg( BS80_INIT_FSLEN,
        ( BS80_INIT_FELEN << 8 ) | BS80_INIT_FBLEN,
        BS80_INIT_LSLEN,
        ( BS80_INIT_LELEN << 8 ) | BS80_INIT_LBLEN,
        BS80_INIT_PIXCLKDIV );

    int iba = BS80_INIT_HSIZE * BS80_INIT_VSIZE * 2;
    bs_cmd_wr_reg( 0x310, iba & 0xFFFF );
    bs_cmd_wr_reg( 0x312, ( iba >> 16 ) & 0xFFFF );

    bs_cmd_print_disp_timings( );

    bs_cmd_set_wfm( wa );
    bs_cmd_print_wfm_info( );

    printf( "[%s] ... display engine initialized with waveform\n", __FUNCTION__ );

    bs_cmd_clear_gd( );

    bs_cmd_wr_reg( 0x01A, 4 ); // i2c clock divider
    bs_cmd_wr_reg( 0x320, 0 ); // temp auto read on

    bs_cmd_set_lut_auto_sel_mode(0); // make sure auto-lut mode is off
    bs_cmd_set_rotmode(3);  // set rotation mode
}

#ifdef BS80_INIT_MAIN

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
        wfm_addr = bs80_str2int( argv[1] );
    }
    bs80_init( wfm_addr );
    bs80_flash( );
    bs80_black( );
    bs80_white( );
    return 0;
}

#endif // BS80_INIT_MAIN

#endif  // #ifdef ENABLE_EINK_SCREEN

// end of file
