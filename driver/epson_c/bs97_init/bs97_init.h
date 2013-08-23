// $Id: bs97_init.h,v 1.1 2008/04/11 06:28:09 hgates Exp $

#ifndef __BS97_INIT_H__
#define __BS97_INIT_H__

#define BS97_INIT_HSIZE 1200
#define BS97_INIT_VSIZE 825

#define BS97_INIT_FSLEN 0
#define BS97_INIT_FBLEN 4
#define BS97_INIT_FELEN 4
#define BS97_INIT_LSLEN 4
#define BS97_INIT_LBLEN 10
#define BS97_INIT_LELEN 60
#define BS97_INIT_PIXCLKDIV 3

#define BS97_INIT_SDRV_CFG ( 100 | ( 1 << 8 ) | ( 1 << 9 ) )
#define BS97_INIT_GDRV_CFG  0x2
#define BS97_INIT_LUTIDXFMT ( 4 | ( 1 << 7 ) )

extern void bs97_init( int wfmaddr );
extern void bs97_flash( void );
extern void bs97_white( void );
extern void bs97_black( void );
extern void bs97_ld_value( int v );
extern int  bs97_str2int( const char * str );

#endif // __BS97_INIT_H__

// end of file
