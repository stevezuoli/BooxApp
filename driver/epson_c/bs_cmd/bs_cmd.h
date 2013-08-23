#ifndef __BS_CMD_H__
#define __BS_CMD_H__

// data type definitions
#ifndef CONFIG_ONYX_SPLASH // Avoid redefinition in kernel
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
#endif

// broadsheet commands
void bs_cmd_init_cmd_set( u16 arg0, u16 arg1, u16 arg2 );
void bs_cmd_init_pll_stby( u16 cfg0, u16 cfg1, u16 cfg2 );
void bs_cmd_run_sys( void );
void bs_cmd_stby( void );
void bs_cmd_slp( void );
void bs_cmd_init_sys_run( void );
void bs_cmd_init_sys_stby( void );
void bs_cmd_init_sdram( u16 cfg0, u16 cfg1, u16 cfg2, u16 cfg3 );
void bs_cmd_init_dspe_cfg( u16 hsize, u16 vsize, u16 sdcfg, u16 gfcfg, u16 lutidxfmt );
void bs_cmd_init_dspe_tmg( u16 fs, u16 fbe, u16 ls, u16 lbe, u16 pixclkcfg );
void bs_cmd_set_rotmode( u16 rotmode );

u16  bs_cmd_rd_reg( u16 ra );
void bs_cmd_wr_reg( u16 ra, u16 wd );

int  bs_cmd_rd_irq( void );

void bs_cmd_rd_sfm( void );
void bs_cmd_wr_sfm( u8 wd );
void bs_cmd_end_sfm( void );

void bs_cmd_bst_rd_sdr( u32 ma, u32 bc );
void bs_cmd_bst_wr_sdr( u32 ma, u32 bc );
void bs_cmd_bst_end( void );

void bs_cmd_ld_img( u16 dfmt );
void bs_cmd_ld_img_area( u16 dfmt, u16 x, u16 y, u16 w, u16 h );
void bs_cmd_ld_img_end( void );
void bs_cmd_ld_img_wait( void );

void bs_cmd_wait_dspe_trg( void );
void bs_cmd_wait_dspe_frend( void );
void bs_cmd_wait_dspe_lutfree( void );
void bs_cmd_wait_dspe_mlutfree( u16 lutmsk );

void bs_cmd_rd_wfm_info( u32 ma );

void bs_cmd_upd_init( void );
void bs_cmd_upd_full( u16 mode, u16 lutn, u16 bdrupd );
void bs_cmd_upd_full_area( u16 mode, u16 lutn, u16 bdrupd, u16 x, u16 y, u16 w, u16 h );
void bs_cmd_upd_part( u16 mode, u16 lutn, u16 bdrupd );
void bs_cmd_upd_part_area( u16 mode, u16 lutn, u16 bdrupd, u16 x, u16 y, u16 w, u16 h );

void bs_cmd_gdrv_clr( void );
void bs_cmd_upd_set_imgadr( u32 ma );


// software commands
void bs_cmd_set_hif_mode_cmd( void ); // switch to the command-mode host interface
void bs_cmd_set_hif_mode_reg( void ); // switch to the register-mode host interface

void bs_cmd_flag_hif_mode_cmd( void ); // bs_hif_mode_cmd = true
void bs_cmd_flag_hif_mode_reg( void ); // bs_hif_mode_cmd = false

void bs_cmd_get_disp_sizes( void ); // get the display sizes (bs_hsize and bs_vsize)
void bs_cmd_print_disp_timings( void );

void bs_cmd_wait_for_bit( int reg, int bitpos, int bitval );

void bs_cmd_set_wfm( int addr ); // set waveform
void bs_cmd_get_wfm_info( void );   // get the waveform information
void bs_cmd_print_wfm_info( void ); // print the waveform information

void bs_cmd_clear_gd( void ); // clear gate drivers

int  bs_cmd_get_lut_auto_sel_mode( void );  // get lut auto selection mode
void bs_cmd_set_lut_auto_sel_mode( int v ); // set lut auto selection mode

void bs_cmd_wr_data( int n, u8 * wd );
void bs_cmd_ld_img_data( u16 dfmt, u16 w, u16 h, u8 * img );
void bs_cmd_ld_img_area_data( u16 dfmt, u16 x, u16 y, u16 w, u16 h, u8 * img );
void bs_cmd_ld_img_area_data_with_stride( u16 dfmt, u16 x, u16 y, u16 w, u16 h, u8 * begin, u16 stride);

void bs_cmd_rd_sdr( u32 ma, u32 bc, u8 * data );
void bs_cmd_wr_sdr( u32 ma, u32 bc, u8 * data );
#endif // __BS_CMD_H__
