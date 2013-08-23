
#include <stdio.h>
#include <stdlib.h>
#include "bs_cmd.h"


int main(int argc, char **argv){

  int lut = 15;
  if(argc != 2){
    printf("Usage: bs_disp_full <mode>\n\r");
    return(-1);
  }

#ifdef ENABLE_EINK_SCREEN
    bs_cmd_flag_hif_mode_cmd();          /* required for access to broadsheet commands */
    bs_cmd_wait_dspe_trg();              /* wait for op trigger to be free */
    bs_cmd_wait_dspe_frend();            /* wait for any in progress update to finish*/
    bs_cmd_upd_full(atoi(*++argv),lut,0);   /* request update */
#endif
  return(0);
}
