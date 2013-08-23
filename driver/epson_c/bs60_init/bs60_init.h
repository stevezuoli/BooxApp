// $Id: bs60_init.h,v 1.2 2008/11/13

#ifndef __BS60_INIT_H__
#define __BS60_INIT_H__

#define BS60_INIT_HSIZE 800
#define BS60_INIT_VSIZE 600
#define BS60_INIT_FSLEN 4
#define BS60_INIT_FBLEN 4
#define BS60_INIT_FELEN 10
#define BS60_INIT_LSLEN 10
#define BS60_INIT_LBLEN 4
#define BS60_INIT_LELEN 74
#define BS60_INIT_PIXCLKDIV 6
#define BS60_INIT_SDRV_CFG ( 100 | ( 1 << 8 ) | ( 1 << 9 ) )
#define BS60_INIT_GDRV_CFG  0x2
#define BS60_INIT_LUTIDXFMT ( 4 | ( 1 << 7 ) )

extern void bs60_init( int wfmaddr );
extern void bs60_flash( void );
extern void bs60_white( void );
extern void bs60_black( void );
extern void bs60_ld_value( int v );
#ifdef USER_SPACE
extern int  bs60_str2int( const char * str );
#endif

#endif // __BS60_INIT_H__

// end of file
