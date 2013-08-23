


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bs_cmd.h"


int main(int argc, char **argv)
{

#ifdef ENABLE_EINK_SCREEN

    bs_cmd_flag_hif_mode_cmd();                  // required for access to broadsheet commands
    bs_cmd_set_rotmode(3);  // set rotation mode

    unsigned char data[600 * 800] = {0};
    for(int i = 0; i < 800; ++i)
    {
        memset(&data[i * 600], i / 3, 600);
    }


    bs_cmd_ld_img_area_data(3, 0, 0, 600, 800, data, 600);
    bs_cmd_wait_dspe_trg();              /* wait for op trigger to be free */
    bs_cmd_wait_dspe_frend();            /* wait for any in progress update to finish*/
    bs_cmd_upd_full(3,15,0);   /* request update */

#endif

    return(0);
}

