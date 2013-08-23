#include <stdio.h>
#include <stdlib.h>
#include "bs_cmd.h"
#include "bs_chip.h"

#define MSG_WIDTH     600
#define MSG_HEIGHT     18

static void update_area(int wfm_mode, u16 x, u16 y, u16 width, u16 height)
{
    wait_for_ready();
    bs_cmd_upd_full_area(wfm_mode, 15, 0, x, y, width, height);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <msg_file>\n", argv[0]);
        return -1;
    }

    int screen_width = 600, screen_height = 800, grayscale = 8;

    // video = "600*800:8"
    char* disp_info = getenv("video");
    if (disp_info)
    {
        sscanf(disp_info, "%d*%d:%d", &screen_width, &screen_height, &grayscale);
    }
    int wfm_mode = grayscale == 8 ? 3 : 2;

    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Can't open %s for reading!\n", argv[1]);
        return -1;
    }

    unsigned char bmp[MSG_WIDTH * MSG_HEIGHT];
    if (fread(bmp, 1, MSG_WIDTH * MSG_HEIGHT, fp) != MSG_WIDTH * MSG_HEIGHT)
    {
        fprintf(stderr, "Invalid message file!\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    // Initialize broadsheet.
    bs_chip_init();
    bs_cmd_flag_hif_mode_cmd();

    // Update message area.
    bs_cmd_ld_img_area_data(3, 0, screen_height - 2 - MSG_HEIGHT, MSG_WIDTH, MSG_HEIGHT, bmp);
    update_area(wfm_mode, 0, screen_height - 2 - MSG_HEIGHT, MSG_WIDTH, MSG_HEIGHT);

    bs_chip_final();
    return 0;
}

