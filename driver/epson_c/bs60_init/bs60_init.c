// $Id: bs60_init.cpp,2008/11/13

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bs_chip.h"
#include "bs_cmd.h"
#include "bs60_init.h"

#ifdef USER_SPACE
#define DBG printf
#else
#include <cyg/infra/diag.h>
#define DBG diag_printf
#endif

#ifdef USER_SPACE
int bs60_str2int( const char * str )
{
    int base = 10;
    if ( strlen( str ) > 1 )
    {
        if ( ( str[0] == '0' ) && ( str[1] == 'x' ) ) base = 16;
    }
    return strtol( str, 0, base );
}
#endif

void bs60_ld_value( int v )
{
    // In redboot we are not allowed to allocate 480K in stack!
    unsigned char* tmp = malloc(600 * 800);
    memset(tmp, v & 0xff, 600 * 800);
    bs_cmd_ld_img_area_data_with_stride( 3, 0, 0, 600, 800, tmp, 600);
    free(tmp);
}

void bs60_black( void )
{
    DBG( "[%s] ... displaying black\n", __FUNCTION__ );
    bs60_ld_value( 0x00 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 2, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

void bs60_white( void )
{
    DBG( "[%s] ... displaying white\n", __FUNCTION__ );
    bs60_ld_value( 0xFF );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
    bs_cmd_upd_full( 2, 0, 0 );
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
    DBG( "[%s] ... initializing broadsheet for 6-inch panel\n", __FUNCTION__ );
    bs_chip_init();
    init_gpio();
    bs_cmd_set_hif_mode_cmd( );
    bs_cmd_wr_reg( 0x006, 0x0000 );

    // It's really very important to sleep here to make sure the controller
    // is correctly power on.
    bs_sleep(5);

    bs_cmd_init_sys_run( );
    bs_cmd_wr_reg( 0x0106, 0x0203 );

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

    DBG( "[%s] ... display engine initialized with waveform\n", __FUNCTION__ );

    bs_cmd_clear_gd( );

    bs_cmd_wr_reg( 0x01A, 4 ); // i2c clock divider
    bs_cmd_wr_reg( 0x320, 0 ); // temp auto read on

    bs_cmd_set_lut_auto_sel_mode(0); // make sure auto-lut mode is off
    bs_cmd_set_rotmode(3);  // set rotation mode
    bs60_white();
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
        wfm_addr = bs60_str2int( argv[1] );
    }
    bs60_init( wfm_addr );

    // Disable the flash as it's not necessary.
    bs60_flash( );

    bs_chip_final();
    return 0;
}
#endif

// end of file
