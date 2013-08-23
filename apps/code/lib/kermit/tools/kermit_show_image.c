
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../kermit_api.h"
#include <sys/stat.h>
#include <string.h>

/// Show image
static void show_usage()
{
    printf("kermit_show_image x y width height file waveform_mode\n");
}

static long fsize(const char *const name)
{
    struct stat stbuf;
    return (stat(name, &stbuf) == -1) ? -1 : stbuf.st_size;
}

int main(int argc, char *argv[])
{
    static const int MAX = 1024;
    static const int BUFFER_SIZE = 600 * 800 / 2;
    static const int ROW = 400;
    int x, y, width, height;
    char file_name[MAX];
    Waveforms_T wf_mode = WF_GC16;
    unsigned char buffer[600 * 800 / 2] = {0};
    FILE * fp = 0;
    unsigned char * dst = 0;
    unsigned char * src = 0;
    int i;

    if (argc != 7)
    {
        show_usage();
        return -1;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);
    width = atoi(argv[3]);
    height = atoi(argv[4]);
    memset((void *)file_name, 0, MAX);
    strncpy((void *)file_name, argv[5], MAX - 1);
    wf_mode = (Waveforms_T)(atoi(argv[6]));


    int ret = kermit_init();
    printf("kermit_init %d\n", ret);

    kermit_display_off();
    kermit_panel_on();
    int kermit_buffer_size_ = 0;
    unsigned char *kermit_buffer_ = (unsigned char *)kermit_mmap((long unsigned int *)&kermit_buffer_size_);
    printf("kermit buffer size %d\n", kermit_buffer_size_);

    // Load data
    fp = fopen(file_name, "rb");
    if (fp == NULL)
    {
        printf("Could not open file %s\n", file_name);
        return -1;
    }
    fread(&buffer, sizeof(unsigned char), BUFFER_SIZE, fp);

    // Copy data to framebuffer directly.
    dst = kermit_buffer_ + y * ROW + x / 2;
    src = &buffer[0];
    for(i = 0; i < height; ++i)
    {
         memcpy((void *)dst, src, width / 2);
         dst += ROW;
         src += width / 2;
    }

    kermit_set_waveform(wf_mode);

    kermit_full_update();
    printf("kermit_full_update\n");

    kermit_display_on();
    kermit_panel_on();
    printf("kermit_display_on \n");

    kermit_update_done();

    kermit_display_off();

    kermit_exit();
    printf("kermit_exit \n");
    return 0;
}
