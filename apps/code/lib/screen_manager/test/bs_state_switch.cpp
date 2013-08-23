


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "bs_cmd.h"

// Switch between sleep, standby and run.mode.
int main(int argc, char **argv)
{
#ifdef ENABLE_EINK_SCREEN

    bs_cmd_flag_hif_mode_cmd();     // required for access to broadsheet commands
    bs_cmd_set_rotmode(3);          // set rotation mode

    unsigned char data[600 * 800] = {0};
    for(int i = 0; i < 800; ++i)
    {
        memset(&data[i * 600], i / 3, 600);
    }

    int page = 0;
    while (1)
    {
        printf("change to run mode\n");
        bs_cmd_run_sys();

        bs_cmd_ld_img_area_data(3, 0, 0, 600, 800, data, 600);
        printf("copy page %d\n", ++page);

        bsc.wait_until_ready();
        printf("ready done!\n");

        bs_cmd_upd_full(3, 15, 0);
        printf("update full done!\n");

        bs_cmd_wait_dspe_trg();
        bs_cmd_wait_dspe_frend();

        printf("change to standby mode!\n");
        bsc.wait_until_ready();
        bs_cmd_stby();
        printf("standby mode now!\n");
    }
#endif

    return(0);
}

