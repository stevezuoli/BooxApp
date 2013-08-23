// $Id: bs80_init.cpp,v 1.2 2008/04/11 06:28:09 hgates Exp $

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bs_chip.h"
#include "bs_cmd.h"
#include "bs80_init.h"

#ifdef USER_SPACE
#define DBG printf
#else
#include <cyg/infra/diag.h>
#define DBG diag_printf
#endif

#ifdef USER_SPACE
int bs80_str2int( const char * str )
{
    int base = 10;
    if ( strlen( str ) > 1 )
    {
        if ( ( str[0] == '0' ) && ( str[1] == 'x' ) ) base = 16;
    }
    return strtol( str, 0, base );
}
#endif

void bs80_ld_value( int v )
{
    // In redboot we are not allowed to allocate 768K in stack!
    unsigned char* tmp = malloc(768 * 1024);
    memset(tmp, v & 0xff, 768 * 1024);
    bs_cmd_ld_img_area_data_with_stride( 3, 0, 0, 768, 1024, tmp, 768);
    free(tmp);
}

void bs80_black( void )
{
    DBG( "[%s] ... displaying black\n", __FUNCTION__ );
    bs80_ld_value( 0x00 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 2, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

void bs80_white( void )
{
    DBG( "[%s] ... displaying white\n", __FUNCTION__ );
    bs80_ld_value( 0xFF );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 2, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

void bs80_flash( void )
{
    //DBG( "[%s] ... flashing\n", __FUNCTION__ );

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
    DBG( "[%s] ... initializing broadsheet for 8-inch panel\n", __FUNCTION__ );
    bs_chip_init();
    init_gpio();
    bs_cmd_set_hif_mode_cmd( );
    bs_cmd_wr_reg( 0x006, 0x0000 );

    // It's really very important to sleep here to make sure the controller
    // is correctly power on.
    bs_sleep(5);

    bs_cmd_init_sys_run( );
    bs_cmd_wr_reg( 0x0106, 0x0203 );

    bs_cmd_init_dspe_cfg( BS80_INIT_HSIZE,
                          BS80_INIT_VSIZE,
                          BS80_INIT_SDRV_CFG,
                          BS80_INIT_GDRV_CFG,
                          BS80_INIT_LUTIDXFMT );

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

    DBG( "[%s] ... display engine initialized with waveform\n", __FUNCTION__ );

    bs_cmd_clear_gd( );

    bs_cmd_wr_reg( 0x01A, 4 ); // i2c clock divider
    bs_cmd_wr_reg( 0x320, 0 ); // temp auto read on

    bs_cmd_set_lut_auto_sel_mode(0); // make sure auto-lut mode is off
    bs_cmd_set_rotmode(3);  // set rotation mode
    bs80_white();
}

#ifdef USER_SPACE
void usage( const char * pn )
{
    DBG( "Usage: %s"" <wfm_addr>\n", pn);
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

    // Disable the flash as it's not necessary.
    bs80_flash( );

    bs_chip_final();
    return 0;
}
#endif

// end of file
