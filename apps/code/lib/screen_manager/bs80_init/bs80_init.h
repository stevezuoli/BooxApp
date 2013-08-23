// $Id: bs80_init.h,v 1.2 2008/04/11 06:28:09 hgates Exp $

#ifndef __BS80_INIT_H__
#define __BS80_INIT_H__

#define BS80_INIT_HSIZE 1024
#define BS80_INIT_VSIZE 768

#define BS80_INIT_FSLEN 0
#define BS80_INIT_FBLEN 4
#define BS80_INIT_FELEN 80
#define BS80_INIT_LSLEN 10
#define BS80_INIT_LBLEN 20
#define BS80_INIT_LELEN 80
#define BS80_INIT_PIXCLKDIV 3

#define BS80_INIT_SDRV_CFG ( 100 | ( 0 << 8 ) | ( 1 << 9 ) )
#define BS80_INIT_GDRV_CFG  0x2
#define BS80_INIT_LUTIDXFMT ( 4 | ( 1 << 7 ) )

extern void bs80_init( int wfmaddr );
extern void bs80_flash( void );
extern void bs80_white( void );
extern void bs80_black( void );
extern void bs80_ld_value( int v );
extern int  bs80_str2int( const char * str );

#endif // __BS80_INIT_H__

// end of file
