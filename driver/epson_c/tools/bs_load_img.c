#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bs_chip.h"
#include "bs_cmd.h"
#include "pgm.h"

#define PPMS 3

void bs_cmd_wr_pgm_data( PgmData* p )
{
  int bc = p->width * p->height;
  u16 v = 0;
  int x = 0;
  int i = 0;
  for ( i = 0; i < bc; i++ ) {
    if ( x == 0 ) v = get_data( p, i );
    else {
      v |= ( (u16) get_data( p, i ) ) << 8;
      wr_data( v );
    }
    x = 1 - x;
  } // for i
  if ( bc & 0x1 ) wr_data( v );
}

void bs_cmd_ld_img_pgm( u16 dfmt, const char * pgmf )
{
  PgmData pgm;
  memset(&pgm, 0, sizeof(PgmData));

  read_file( &pgm, pgmf );
  bs_cmd_ld_img( dfmt );
  bs_cmd_wr_pgm_data( &pgm );
  bs_cmd_ld_img_end( );

  free(pgm.data);
}

int main(int argc, char **argv)
{
  if(argc != 3){
    fprintf(stderr, "Usage: bs_load_img <rotation> <image.pgm>\n\r");
    return(-1);
  }

  bs_chip_init();
  bs_cmd_flag_hif_mode_cmd();                   // required for access to broadsheet commands
  bs_cmd_set_rotmode((atoi((char *)*++argv)));  // set rotation mode
  bs_cmd_ld_img_pgm(PPMS, *++argv);

  // Update screen.
  bs_cmd_wait_for_bit( 0x338, 0, 0 );
  bs_cmd_wait_for_bit( 0x338, 3, 0 );
  bs_cmd_upd_full(3, 15, 0);
  bs_cmd_wait_for_bit( 0x338, 0, 0 );
  bs_cmd_wait_for_bit( 0x338, 3, 0 );

  bs_chip_final();
  return(0);
}

