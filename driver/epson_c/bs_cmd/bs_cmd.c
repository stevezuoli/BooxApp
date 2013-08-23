#ifdef CONFIG_ONYX_SPLASH
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/hardware.h>
#define DBG printk
#undef  GPIO3_BASE_ADDR
#define GPIO3_BASE_ADDR IO_ADDRESS(0x53FA4000)
#else

#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#ifdef USER_SPACE
#define DBG printf
#else
#include <cyg/hal/hal_soc.h>
#include <cyg/infra/diag.h>
#define DBG diag_printf
#define __raw_readl  readl
#define __raw_writel writel
#endif

#endif

#include "bs_cmd.h"
#include "bs_chip.h"

#ifdef CONFIG_ONYX_SPLASH
static int bs_hif_mode_cmd = 1;
#else
static int bs_hif_mode_cmd = 0;
#endif

int bs_hsize = 0;
int bs_vsize = 0;

int wfm_fvsn = 0;
int wfm_luts = 0;
int wfm_mc = 0;
int wfm_trc = 0;
int wfm_eb = 0;
int wfm_sb = 0;
int wfm_wmta = 0;

//-----------------------------------------

void bs_cmd_init_cmd_set( u16 arg0, u16 arg1, u16 arg2 )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x00 );
    wr_data( arg0 );
    wr_data( arg1 );
    wr_data( arg2 );
}


void bs_cmd_init_pll_stby( u16 cfg0, u16 cfg1, u16 cfg2 )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x01 );
    wr_data( cfg0 );
    wr_data( cfg1 );
    wr_data( cfg2 );
}

void bs_cmd_run_sys( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x02 );
}


void bs_cmd_stby( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x04 );
}

void bs_cmd_slp( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x05 );
}

void bs_cmd_init_sys_run( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x06 );
}

void bscmd_init_sys_stby( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x07 );
}

void bs_cmd_init_sdram( u16 cfg0, u16 cfg1, u16 cfg2, u16 cfg3 )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x08 );
    wr_data( cfg0 );
    wr_data( cfg1 );
    wr_data( cfg2 );
    wr_data( cfg3 );
}

void bs_cmd_init_dspe_cfg( u16 hsize, u16 vsize, u16 sdcfg, u16 gdcfg, u16 lutidxfmt )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x09 );
    wr_data( hsize );
    wr_data( vsize );
    wr_data( sdcfg );
    wr_data( gdcfg );
    wr_data( lutidxfmt );
    bs_hsize = hsize;
    bs_vsize = vsize;
    DBG( "[%s] ... hsize=%d vsize=%d\n", __FUNCTION__, bs_hsize, bs_vsize );
}

void bs_cmd_init_dspe_tmg( u16 fs, u16 fbe, u16 ls, u16 lbe, u16 pixclkcfg )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x0A );
    wr_data( fs );
    wr_data( fbe );
    wr_data( ls );
    wr_data( lbe );
    wr_data( pixclkcfg );
}

void bs_cmd_set_rotmode( u16 rotmode )
{
    u16 arg;

    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    arg = ( rotmode & 0x3 ) << 8;
    command( 0x0B );
    wr_data( arg );
}

u16 bs_cmd_rd_reg( u16 ra )
{
    if ( !bs_hif_mode_cmd ) return rd_reg( ra );

    command( 0x10 );
    wr_data( ra );
    return rd_data( );
}

void bs_cmd_wr_reg( u16 ra, u16 wd )
{
    if ( !bs_hif_mode_cmd ) { wr_reg( ra, wd ); return; }

    command( 0x11 );
    wr_data( ra );
    wr_data( wd );
}

int bs_cmd_rd_irq( void )
{
    return get_gpio_val( GPIO_HIRQ );
}

void bs_cmd_rd_sfm( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x012 );
}

void bs_cmd_wr_sfm( u8 wd )
{
    u16 data;

    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    data = wd;
    command( 0x13 );
    wr_data( data );
}


void bs_cmd_end_sfm( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x14 );
}

void bs_cmd_bst_rd_sdr( u32 ma, u32 bc )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x1C );
    wr_data( ma & 0xFFFF );
    wr_data( ( ma >> 16 ) & 0xFFFF );
    wr_data( bc & 0xFFFF );
    wr_data( ( bc >> 16 ) & 0xFFFF );
    command( 0x11 );
    wr_data( 0x154 );
}


void bs_cmd_bst_wr_sdr( u32 ma, u32 bc )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x1D );
    wr_data( ma & 0xFFFF );
    wr_data( ( ma >> 16 ) & 0xFFFF );
    wr_data( bc & 0xFFFF );
    wr_data( ( bc >> 16 ) & 0xFFFF );
    command( 0x11 );
    wr_data( 0x154 );
}


void bs_cmd_bst_end( void )
{
    if ( !bs_hif_mode_cmd ) {
        wr_reg( 0x0142, 2 );
        return;
    }

    command( 0x1E );
}

void bs_cmd_ld_img( u16 dfmt )
{
    u16 arg;
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    arg = ( dfmt & 0x3 ) << 4;
    command( 0x20 );
    wr_data( arg );
    command( 0x11 );
    wr_data( 0x154 );

#if 0
    u16 v = ( ( dfmt & 0x3 ) << 4 ) | ( ( rot & 0x3 ) << 8 );
    command( 0x20 );
    wr_data( v );
    command( 0x11 );
    wr_data( 0x0154 );

    bs_cmd_wr_reg( 0x140, 0x8000 ); // reset
    u16 mat = 0; // memory access type - packed pixel
    u16 mrw = 0; // memory write
    u16 pps = dfmt & 0x3; // packed pixel type
    u16 rotation = rot & 0x3; // rotation
    u16 v = mat | ( mrw << 2 ) | ( pps << 4 ) | ( rotation << 8 );
    bs_cmd_wr_reg( 0x140, v ); // write
    bs_cmd_wr_reg( 0x14C, 0 ); // x start for packed pixels
    bs_cmd_wr_reg( 0x14E, 0 ); // y start for packed pixels
    bs_cmd_wr_reg( 0x150, w ); // width for packed pixels
    bs_cmd_wr_reg( 0x152, h ); // height for packed pixels
    bs_cmd_wr_reg( 0x142, 1 ); // trigger start

    command( 0x11 );
    wr_data( 0x154 );
#endif
}

void bs_cmd_ld_img_area( u16 dfmt, u16 x, u16 y, u16 w, u16 h )
{
    u16 arg;
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    arg = ( dfmt & 0x3 ) << 4;
    command( 0x22 );
    wr_data( arg );
    wr_data( x );
    wr_data( y );
    wr_data( w );
    wr_data( h );
    command( 0x11 );
    wr_data( 0x0154 );

#if 0
    u16 v = ( ( dfmt & 0x3 ) << 4 ) | ( ( rot & 0x3 ) << 8 );
    command( 0x22 );
    wr_data( v );
    wr_data( x );
    wr_data( y );
    wr_data( w );
    wr_data( h );
    command( 0x11 );
    wr_data( 0x0154 );

    bs_cmd_wr_reg( 0x140, 0x8000 ); // reset
    u16 mat = 0; // memory access type - packed pixel
    u16 mrw = 0; // memory write
    u16 pps = dfmt & 0x3; // packed pixel type
    u16 rotation = rot & 0x3;
    u16 v = mat | ( mrw << 2 ) | ( pps << 4 ) | ( rotation << 8 );
    bs_cmd_wr_reg( 0x140, v ); // write
    bs_cmd_wr_reg( 0x14C, x );
    bs_cmd_wr_reg( 0x14E, y );
    bs_cmd_wr_reg( 0x150, w );
    bs_cmd_wr_reg( 0x152, h );
    bs_cmd_wr_reg( 0x142, 1 ); // trigger start

    command( 0x11 );
    wr_data( 0x0154 );
#endif
}

void bs_cmd_ld_img_end( void )
{
    if ( !bs_hif_mode_cmd ) { wr_reg( 0x0142, 2 ); return; }

    command( 0x23 );
}

void bs_cmd_ld_img_wait( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x24 );
}

void bs_cmd_wait_dspe_trg( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x28 );
}

void bs_cmd_wait_dspe_frend( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x29 );
}

void bs_cmd_wait_dspe_lutfree( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x2A );
}

void bs_cmd_wait_dspe_mlutfree( u16 lutmsk )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x2B );
    wr_data( lutmsk );
}

void bs_cmd_rd_wfm_info( u32 ma )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x30 );
    wr_data( ma & 0xFFFF );
    wr_data( ( ma >> 16 ) & 0xFFFF );
}

void bs_cmd_upd_init( void )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x32 );
}

void bs_cmd_upd_full( u16 mode, u16 lutn, u16 bdrupd )
{
    u16 arg;
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    arg = ( ( mode & 0xF ) << 8 ) | ( ( lutn & 0xF ) << 4 ) | ( ( bdrupd & 0x1 ) << 14 );
    command( 0x33 );
    wr_data( arg );
}

void bs_cmd_upd_full_area( u16 mode, u16 lutn, u16 bdrupd, u16 x, u16 y, u16 w, u16 h )
{
    u16 arg;
    u16 dlgenb = 0;
    u16 bdr_upd = bdrupd;
    u16 rect_mode = 2; // xs, ys, xe, ye
    u16 wfm_mode = mode;
    u16 lut_num = lutn;
    u16 op_mode = 3; // full display update
    u16 val;

    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

#if 1
    arg = ( ( mode & 0xF ) << 8 ) | ( ( lutn & 0xF ) << 4 ) | ( ( bdrupd & 0x1 ) << 14 );
    command( 0x34 );
    wr_data( arg );
    wr_data( x );
    wr_data( y );
    wr_data( w );
    wr_data( h );
    return;
#endif

    bs_cmd_wr_reg( 0x0338, 0xFFFF );     // clear
    bs_cmd_wr_reg( 0x033A, 0xFFFF ); // clear

    bs_cmd_wr_reg( 0x0340, x );
    bs_cmd_wr_reg( 0x0342, y );
    bs_cmd_wr_reg( 0x0344, x + w - 1 );
    bs_cmd_wr_reg( 0x0346, y + h - 1 );


    val =
        ( ( dlgenb & 0x1 ) << 15 ) |
        ( ( bdr_upd & 0x1 ) << 14 ) |
        ( ( rect_mode & 0x3 ) << 12 ) |
        ( ( wfm_mode & 0xF ) << 8 ) |
        ( ( lut_num & 0xF ) << 4 ) |
        ( ( op_mode & 0x7 ) << 1 ) |
        1;
    bs_cmd_wr_reg( 0x0334, val );

}

void bs_cmd_upd_part( u16 mode, u16 lutn, u16 bdrupd )
{
    u16 arg;
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    arg = ( ( mode & 0xF ) << 8 ) | ( ( lutn & 0xF ) << 4 ) | ( ( bdrupd & 0x1 ) << 14 );
    command( 0x35 );
    wr_data( arg );
}

void bs_cmd_upd_part_area( u16 mode, u16 lutn, u16 bdrupd, u16 x, u16 y, u16 w, u16 h )
{
    u16 arg;
    u16 dlgenb = 0;
    u16 bdr_upd = bdrupd;
    u16 rect_mode = 2; // xs, ys, xe, ye
    u16 wfm_mode = mode;
    u16 lut_num = lutn;
    u16 op_mode = 4; // partial display update
    u16 val;

    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

#if 1
    arg = ( ( mode & 0xF ) << 8 ) | ( ( lutn & 0xF ) << 4 ) | ( ( bdrupd & 0x1 ) << 14 );
    command( 0x36 );
    wr_data( arg );
    wr_data( x );
    wr_data( y );
    wr_data( w );
    wr_data( h );
    return;
#endif

    bs_cmd_wr_reg( 0x0338, 0xFFFF );     // clear
    bs_cmd_wr_reg( 0x033A, 0xFFFF ); // clear

    bs_cmd_wr_reg( 0x0340, x );
    bs_cmd_wr_reg( 0x0342, y );
    bs_cmd_wr_reg( 0x0344, x + w - 1 );
    bs_cmd_wr_reg( 0x0346, y + h - 1 );

    val =
        ( ( dlgenb & 0x1 ) << 15 ) |
        ( ( bdr_upd & 0x1 ) << 14 ) |
        ( ( rect_mode & 0x3 ) << 12 ) |
        ( ( wfm_mode & 0xF ) << 8 ) |
        ( ( lut_num & 0xF ) << 4 ) |
        ( ( op_mode & 0x7 ) << 1 ) |
        1;
    bs_cmd_wr_reg( 0x0334, val );
}

void bs_cmd_gdrv_clr( void )
{
    if ( !bs_hif_mode_cmd ) {
        int v = ( 5 << 1 ) | 1;
        wr_reg( 0x0334, v );
        return;
    }

    command( 0x37 );
}

void bs_cmd_upd_set_imgadr( u32 ma )
{
    if ( !bs_hif_mode_cmd ) DBG( "[%s] !!! DBG: not in hif_mode_cmd", __FUNCTION__ );

    command( 0x38 );
    wr_data( ma & 0xFFFF );
    wr_data( ( ma >> 16 ) & 0xFFFF );
}


//-----------------------------------------


void bs_cmd_set_hif_mode_cmd( void )
{
    DBG( "[%s] ... setting hif mode to cmd\n", __FUNCTION__ );
    ifmode( BS_IFM_CMD );
    bs_hif_mode_cmd = 1;
    // bs_sleep(1000);
}

void bs_cmd_set_hif_mode_reg( void )
{
    DBG( "[%s] ... setting hif mode to reg\n", __FUNCTION__ );
    ifmode( BS_IFM_REG );
    bs_hif_mode_cmd = 0;
    // bs_sleep(1000);
}

void bs_cmd_flag_hif_mode_cmd( void )
{
    DBG( "[%s] ... flagging cmd hif mode\n", __FUNCTION__ );
    bs_hif_mode_cmd = 1;
}

void bs_cmd_flag_hif_mode_reg( void )
{
    DBG( "[%s] ... flagging reg hif mode\n", __FUNCTION__ );
    bs_hif_mode_cmd = 0;
}

void bs_cmd_get_disp_sizes( void )
{
    bs_hsize = bs_cmd_rd_reg( 0x306 ); // line data length
    bs_vsize = bs_cmd_rd_reg( 0x300 ); // frame data length
    DBG( "[%s] ... hsize=%d vsize=%d\n", __FUNCTION__, bs_hsize, bs_vsize );
}

void bs_cmd_print_disp_timings( void )
{
    int vsize = bs_cmd_rd_reg( 0x300 );
    int vsync = bs_cmd_rd_reg( 0x302 );
    int vblen = bs_cmd_rd_reg( 0x304 );
    int velen = ( vblen >> 8 ) & 0xFF;

    int hsize = bs_cmd_rd_reg( 0x306 );
    int hsync = bs_cmd_rd_reg( 0x308 );
    int hblen = bs_cmd_rd_reg( 0x30A );
    int helen = ( hblen >> 8 ) & 0xFF;

    vblen &= 0xFF;
    hblen &= 0xFF;
    DBG( "[%s] disp_timings: vsize=%d vsync=%d vblen=%d velen=%d\n", __FUNCTION__, vsize, vsync, vblen, velen );
    DBG( "[%s] disp_timings: hsize=%d hsync=%d hblen=%d helen=%d\n", __FUNCTION__, hsize, hsync, hblen, helen );
}

void bs_cmd_wait_for_bit( int reg, int bitpos, int bitval )
{
    while ( 1 ) {
        int d = bs_cmd_rd_reg( reg );
        int v = ( d >> bitpos ) & 0x1;
        if ( v == ( bitval & 0x1 ) ) break;
    } // while
}

void bs_cmd_set_wfm( int addr )
{
    bs_cmd_rd_wfm_info( addr );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
}

void bs_cmd_get_wfm_info( void )
{
    u16 a = bs_cmd_rd_reg( 0x354 );
    u16 b = bs_cmd_rd_reg( 0x356 );
    u16 c = bs_cmd_rd_reg( 0x358 );
    u16 d = bs_cmd_rd_reg( 0x35C );
    u16 e = bs_cmd_rd_reg( 0x35E );
    wfm_fvsn = a & 0xFF;
    wfm_luts = ( a >> 8 ) & 0xFF;
    wfm_trc = ( b >> 8 ) & 0xFF;
    wfm_mc = b & 0xFF;
    wfm_sb = ( c >> 8 ) & 0xFF;
    wfm_eb = c & 0xFF;
    wfm_wmta = d | ( e << 16 );
}

void bs_cmd_print_wfm_info( void )
{
    bs_cmd_get_wfm_info( );
    DBG( "[%s] wfm: fvsn=%d luts=%d mc=%d trc=%d eb=0x%02x sb=0x%02x wmta=%d\n",
        __FUNCTION__, wfm_fvsn, wfm_luts, wfm_mc, wfm_trc, wfm_eb, wfm_sb, wfm_wmta );
}

void bs_cmd_clear_gd( void )
{
    bs_cmd_gdrv_clr( );
    bs_cmd_wait_for_bit( 0x338, 0, 0 );
    bs_cmd_wait_for_bit( 0x338, 3, 0 );
}

int bs_cmd_get_lut_auto_sel_mode( void )
{
    int v = bs_cmd_rd_reg( 0x330 );
    return ( ( v >> 7 ) & 0x1 );
}

void bs_cmd_set_lut_auto_sel_mode( int v )
{
    int d = bs_cmd_rd_reg( 0x330 );
    if ( v & 0x1 ) d |= 0x80;
    else           d &= ~0x80;
    bs_cmd_wr_reg( 0x330, d );
}


void bs_cmd_wr_data( int n, u8 * wd )
{
    u16 v = 0;
    int x = 0;
    int i = 0;
    for ( i = 0; i < n; i++ )
    {
        if ( x == 0 )
        {
            v = wd[i];
        }
        else
        {
            v |= ( wd[i] << 8 );
            wr_data( v );
        }
        x = 1 - x;
    }
    if ( n & 0x1 ) wr_data( v );
}

void bs_cmd_rd_data( int n, u8 * rd )
{
    int n2 = n / 2;
    int x = 0;
    int i = 0;
    for ( i = 0; i < n2; i++ ) {
        u16 v = rd_data( );
        rd[x++] = v & 0xFF;
        rd[x++] = ( v >> 8 ) & 0xFF;
    } // for i
    if ( n & 0x1 ) rd[x++] = rd_data( );
}

void bs_cmd_ld_img_data( u16 dfmt, u16 w, u16 h, u8 * img )
{
    bs_cmd_ld_img( dfmt );
    bs_cmd_wr_data( w*h, img );
    bs_cmd_ld_img_end( );
}

void bs_cmd_ld_img_area_data( u16 dfmt, u16 x, u16 y, u16 w, u16 h, u8 * img )
{
    bs_cmd_ld_img_area( dfmt, x, y, w, h );
    bs_cmd_wr_data( w*h, img );
    bs_cmd_ld_img_end( );
}

void bs_cmd_ld_img_area_data_with_stride( u16 dfmt, u16 x, u16 y, u16 w, u16 h, u8 * begin, u16 stride)
{
#ifdef USER_SPACE
    volatile unsigned int * data_addr = NULL;
#endif
    u8 * p = NULL;
    int i = 0;
    int j = 0;

    // For debug:
    DBG("fast dfmt %d x %d y %d w %d h %d begin %p stride %d\n", dfmt, x, y, w, h, begin, stride);

    // Make sure it's ready.
    wait_until_ready();

    bs_cmd_ld_img_area( dfmt, x, y, w, h );

    // To get better performance. Move bit shift out of loop.
#ifdef USER_SPACE
    data_addr = get_data_addr();
#endif

    // Make sure it's output direction.
    gpio_hdb_dir(1);

    // read data
    p = begin + y * stride + x;
    for(i = y; i < y + h; ++i)
    {
        u16 * wd = (u16*)p;
        int bound = w >> 1;
        for(j = 0; j < bound; ++j)
        {
#ifdef USER_SPACE
            // hdc 1
            *data_addr |= VAL_HDC;

            // hcs_l 0 / hwe_l 0
            *data_addr &= VAL_CS_WE_0;

            // data
            *data_addr = (*data_addr & 0xffff0000) | *wd;
            ++wd;

            // hcs_l 1 / hwe_l 1
            *data_addr |= VAL_CS_WE_1;
#else
            // hdc 1
            unsigned int x = __raw_readl(GPIO3_BASE_ADDR);
            __raw_writel(x | VAL_HDC, GPIO3_BASE_ADDR);

            // hcs_l 0 / hwe_l 0
            x = __raw_readl(GPIO3_BASE_ADDR);
            __raw_writel(x & VAL_CS_WE_0, GPIO3_BASE_ADDR);

            // data
            x = __raw_readl(GPIO3_BASE_ADDR);
            __raw_writel((x & 0xffff0000) | *wd, GPIO3_BASE_ADDR);
            ++wd;

            // hcs_l 1 / hwe_l 1
            x = __raw_readl(GPIO3_BASE_ADDR);
            __raw_writel(x | VAL_CS_WE_1, GPIO3_BASE_ADDR);
#endif
        } // for j
        if ( w & 0x1 ) wr_data( *wd );
        p += stride;
    }

    bs_cmd_ld_img_end( );
}

void bs_cmd_rd_sdr( u32 ma, u32 bc, u8 * data )
{
    int n = 0;
    int i = 0;

    bs_cmd_bst_rd_sdr( ma, bc );
    for ( i = 0; i < (int)bc / 2; i++ ) {
        u16 d = rd_data( );
        data[n++] = d & 0xFF;
        data[n++] = ( d >> 8 ) & 0xFF;
    } // for i
    bs_cmd_bst_end( );
}

void bs_cmd_wr_sdr( u32 ma, u32 bc, u8 * data )
{
    int n = 0;
    int i = 0;

    bs_cmd_bst_wr_sdr( ma, bc );
    for ( i = 0; i < (int)bc / 2; i++ ) {
        u16 d = data[n++];
        d |= ( data[n++] << 8 );
        wr_data( d );
    } // for i
    bs_cmd_bst_end( );
}
