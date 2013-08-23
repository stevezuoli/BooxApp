// $Id: bs_cmd.h,v 1.2 2008/04/11 06:28:10 hgates Exp $

#ifndef __BS_CMD_H__
#define __BS_CMD_H__

#ifdef ENABLE_EINK_SCREEN

#include "bs_chip.h"

// data type definitions
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;


// broadsheet commands
extern void bs_cmd_init_cmd_set( u16 arg0, u16 arg1, u16 arg2 );
extern void bs_cmd_init_pll_stby( u16 cfg0, u16 cfg1, u16 cfg2 );
extern void bs_cmd_run_sys( void );
extern void bs_cmd_stby( void );
extern void bs_cmd_slp( void );
extern void bs_cmd_init_sys_run( void );
extern void bs_cmd_init_sys_stby( void );
extern void bs_cmd_init_sdram( u16 cfg0, u16 cfg1, u16 cfg2, u16 cfg3 );
extern void bs_cmd_init_dspe_cfg( u16 hsize, u16 vsize, u16 sdcfg, u16 gfcfg, u16 lutidxfmt );
extern void bs_cmd_init_dspe_tmg( u16 fs, u16 fbe, u16 ls, u16 lbe, u16 pixclkcfg );
extern void bs_cmd_set_rotmode( u16 rotmode );

extern u16  bs_cmd_rd_reg( u16 ra );
extern void bs_cmd_wr_reg( u16 ra, u16 wd );

extern int  bs_cmd_rd_irq( void );

extern void bs_cmd_rd_sfm( void );
extern void bs_cmd_wr_sfm( u8 wd );
extern void bs_cmd_end_sfm( void );

extern void bs_cmd_bst_rd_sdr( u32 ma, u32 bc );
extern void bs_cmd_bst_wr_sdr( u32 ma, u32 bc );
extern void bs_cmd_bst_end( void );

extern void bs_cmd_ld_img( u16 dfmt );
extern void bs_cmd_ld_img_area( u16 dfmt, u16 x, u16 y, u16 w, u16 h );
extern void bs_cmd_ld_img_end( void );
extern void bs_cmd_ld_img_wait( void );

extern void bs_cmd_wait_dspe_trg( void );
extern void bs_cmd_wait_dspe_frend( void );
extern void bs_cmd_wait_dspe_lutfree( void );
extern void bs_cmd_wait_dspe_mlutfree( u16 lutmsk );

extern void bs_cmd_rd_wfm_info( u32 ma );

extern void bs_cmd_upd_init( void );
extern void bs_cmd_upd_full( u16 mode, u16 lutn, u16 bdrupd );
extern void bs_cmd_upd_full_area( u16 mode, u16 lutn, u16 bdrupd, u16 x, u16 y, u16 w, u16 h );
extern void bs_cmd_upd_part( u16 mode, u16 lutn, u16 bdrupd );
extern void bs_cmd_upd_part_area( u16 mode, u16 lutn, u16 bdrupd, u16 x, u16 y, u16 w, u16 h );

extern void bs_cmd_gdrv_clr( void );
extern void bs_cmd_upd_set_imgadr( u32 ma );


// software commands
extern void bs_cmd_set_hif_mode_cmd( void ); // switch to the command-mode host interface
extern void bs_cmd_set_hif_mode_reg( void ); // switch to the register-mode host interface

extern void bs_cmd_flag_hif_mode_cmd( void ); // bs_hif_mode_cmd = true
extern void bs_cmd_flag_hif_mode_reg( void ); // bs_hif_mode_cmd = false

extern void bs_cmd_get_disp_sizes( void ); // get the display sizes (bs_hsize and bs_vsize)
extern void bs_cmd_print_disp_timings( void );

extern void bs_cmd_wait_for_bit( int reg, int bitpos, int bitval );

extern void bs_cmd_set_wfm( int addr ); // set waveform
extern void bs_cmd_get_wfm_info( void );   // get the waveform information
extern void bs_cmd_print_wfm_info( void ); // print the waveform information

extern void bs_cmd_clear_gd( void ); // clear gate drivers

extern int  bs_cmd_get_lut_auto_sel_mode( void );  // get lut auto selection mode
extern void bs_cmd_set_lut_auto_sel_mode( int v ); // set lut auto selection mode

extern void bs_cmd_wr_data( int n, u8 * wd );
extern void bs_cmd_ld_img_data( u16 dfmt, u16 w, u16 h, u8 * img );
extern void bs_cmd_ld_img_area_data( u16 dfmt, u16 x, u16 y, u16 w, u16 h, u8 * img );
extern void bs_cmd_ld_img_area_data( u16 dfmt, u16 x, u16 y, u16 w, u16 h, u8 * begin, u16 stride);
extern void bs_cmd_ld_img_area_data( u16 dfmt, u16 x, u16 y, u16 w, u16 h, u8 color, u16 stride);

extern void bs_cmd_rd_sdr( u32 ma, u32 bc, u8 * data );
extern void bs_cmd_wr_sdr( u32 ma, u32 bc, u8 * data );

extern int bs_cmd_rd_temp( void );


// global variables

extern bs_chip bsc; // broadsheet chip

extern bool bs_hif_mode_cmd; // command-mode host interface
extern bool bs_dialog_enb;   // dialog chip enabled
extern bool bs_use_full_update;

extern int  bs_hsize; // horizontal display size (width)
extern int  bs_vsize; // vertical display size (height)

extern int  wfm_fvsn; // waveform format version
extern int  wfm_luts; // waveform lookup table size
extern int  wfm_mc;   // waveform mode count
extern int  wfm_trc;  // waveform temperature region count
extern int  wfm_sb;   // waveform switch byte
extern int  wfm_eb;   // waveform end byte
extern int  wfm_wmta; // waveform mode table address


#endif // #ifdef ENABLE_EINK_SCREEN

#endif // __BS_CMD_H__

// end of file
