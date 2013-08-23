
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../kermit_api.h"
#include <cstring>


int main(int argc, char *argv[])
{
    Waveforms_T wf_mode = WF_GC8;
    int fill = 0xff;

    if (argc >= 2)
    {
        wf_mode = static_cast<Waveforms_T>(atoi(argv[1]));
    }

    if (argc >= 3)
    {
        wf_mode = static_cast<Waveforms_T>(atoi(argv[1]));
        fill = atoi(argv[2]);
    }

    int ret = kermit_init();
    printf("kermit_init %d\n", ret);

    // get kermit revision id 
    unsigned int   expectedId = 0, revId = 0;
    kermit_versions(&expectedId, &revId);
    printf("kermit_versions expectedId 0x%x, revId 0x%x\n", expectedId, revId);

    kermit_display_off();
    kermit_panel_on();
    int kermit_buffer_size_ = 0;
    unsigned char *kermit_buffer_ = (unsigned char *)kermit_mmap((long unsigned int *)&kermit_buffer_size_);
    printf("kermit buffer size %d\n", kermit_buffer_size_);


    for(int i = 0; i < 600; ++i)
    {
         memset(kermit_buffer_ + i * 400, fill, 400);
    }

    kermit_set_waveform(wf_mode);

    kermit_full_update();
    printf("kermit_full_update\n");

    kermit_display_on();
    printf("kermit_display_on \n");

    kermit_update_done();
    printf("Ensure update done\n");

    kermit_display_off();
    printf("Turn off controller\n");

    if (fill >= 255)
    {
        kermit_panel_off();
        printf("Turn off panel \n");
    }

    kermit_exit();
    printf("kermit_exit \n");
    return 0;
}
