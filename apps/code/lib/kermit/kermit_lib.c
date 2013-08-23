/**
 *  \name kermit_lib.c (kermit API)
 *  \brief User level Kermit EPD  Defines/Globals/Functions
 *
 *  Copyright (C) 2010, Marvell Semiconductors
 *
 *  \author Alice Hsia (ahsia@Marvell.com)
 */

#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#define OUTPUT          stderr
#define PRINT_ERR(format, args...) fprintf (OUTPUT, format, args)

#if 1
    #define PRINT_DEBUG(format, args...) fprintf (OUTPUT, format, args)
#else
    #define PRINT_DEBUG(format, args...)
#endif
#endif
#include "kermit_api.h"
#include "kermit_ioctl.h"




int kermit_fd = 0;

/**
 * \fn kermit_init
 * \brief kermit initialization API
 * \param  none
 * \return none
 */
int kermit_init(void)
{
#ifndef WIN32
    do
    {
        /* attempt to open the Kermit EPD driver */
        if (kermit_fd <= 0)
        {
            if ( (kermit_fd = open("/dev/kermit", O_RDWR) ) < 0 )
            {
                PRINT_ERR("open failed - kermit device fd: %d", kermit_fd);
            }
        }
    }
    while (0);
#endif
    return kermit_fd;
}

/**
 * \fn kermit_exit
 * \brief kermit release API
 * \param  none
 * \return none
 */
void kermit_exit(void)
{
#ifndef WIN32
    do
    {
        if (kermit_fd > 0)
            close(kermit_fd);
        kermit_fd = 0;
    }
    while (0);
#endif
}

/**
 * \fn kermit_mmap
 * \brief map Frame Buffer's memory
 * \param  buffer's size
 * \return none
 */
void * kermit_mmap(unsigned long *size)
{
#ifndef WIN32
    void *vaddr = NULL;
    do
    {
        int ret = 0;
        struct kermit_fb_parms kermit_fb;

    	ret = ioctl(kermit_fd, IOCTL_GET_FB_BUF, &kermit_fb);
        PRINT_DEBUG("IOCTL_GET_FB ret %d, fb.active 0x%x, fb.bytes 0x%x\n",
            ret, kermit_fb.active_fb, kermit_fb.bytes);
        if( ret < 0 || kermit_fb.bytes == 0 )
            break;

        *size = kermit_fb.bytes;
        vaddr = mmap(0, kermit_fb.bytes, PROT_READ | PROT_WRITE, MAP_SHARED, kermit_fd, kermit_fb.active_fb);
        PRINT_DEBUG("kermit_mmap return vaddr = %p\n", vaddr);
    }
    while (0);
    return (vaddr == MAP_FAILED) ? NULL : vaddr;
#endif
    return 0;
}

/**
 * \fn kermit_free
 * \brief unmap Frame Buffer's memory
 * \param  mapped address
 * \param  mapped memories size
 * \return none
 */
void kermit_free(void *vaddr, unsigned long size)
{
#ifndef WIN32
    PRINT_DEBUG("kermit_free:munmap return %d vaddr = %p, size = %ld\n", munmap(vaddr, size), vaddr, size);
#endif
}

/**
 * \fn kermit_set_waveform
 * \brief kermit set waveform's mode API
 * \param  waveform's mode
 * \return none
 */
void kermit_set_waveform(Waveforms_T wf)
{
#ifndef WIN32
    ioctl(kermit_fd, IOCTL_WAVEFORMS_SET, &wf);
#endif
}

/**
 * \fn kermit_flash_screen
 * \brief full flash request API
 * \param  none
 * \return none
 */
void kermit_full_update(void)
{
#ifndef WIN32
    ioctl(kermit_fd, IOCTL_FULL_FLASH);
#endif
}

/**
 * \fn kermit_partial_update
 * \brief partial update request API
 * \param  none
 * \return none
 */
void kermit_partial_update(void)
{
#ifndef WIN32
    // PRINT_DEBUG("kermit_partial_update %d\n", kermit_fd);
    ioctl(kermit_fd, IOCTL_PARTIAL_UPDATE);
#endif
}

/** \fn kermit_versions
 *  \brief returns Kermit's expected and real versions
 *  \param  expected revision id's pointer
 *  \param  real revison id's pointer
 *  \return none
 */
void kermit_versions ( unsigned int * expectedRevId, unsigned int * revisionId )
{
#ifndef WIN32
    struct kermit_vers vers;
    ioctl ( kermit_fd, IOCTL_GET_VERSION, &vers );
    *expectedRevId = vers.revId;
    *revisionId = vers.realRevId;
#endif
}

/** \fn kermit_get_fb_buf
 *  \brief returns Kermit buffers info
 *  \param  pointer to kermit buffers info structure
 *  \return none
 */
void kermit_get_fb_buf (struct kermit_fb_parms * kermit_fb )
{
#ifndef WIN32
    ioctl (kermit_fd, IOCTL_WAVEFORMS_SET, kermit_fb);
#endif
}

/** \fn display_on
 *  \brief  turn kermit display on.
 *  \param  none
 *  \return forward ioctl's return code
 */
int kermit_display_on (void)
{
#ifndef WIN32
    return ioctl ( kermit_fd, IOCTL_DISPLAY_ON );
#endif
    return 0;
}

int kermit_set_grayscale(int gray_scale)
{
#ifndef WIN32
    return ioctl ( kermit_fd, IOCTL_CHANGE_GRAYSCALE, &gray_scale );
#endif
    return 0;
}


/** \fn display_off
 *  \brief turn kermit display off.
 *  \param  none
 *  \return forward ioctl's return code
 */
int kermit_display_off (void)
{
#ifndef WIN32
    return ioctl ( kermit_fd, IOCTL_DISPLAY_OFF );
#endif
    return 0;
}

int kermit_update_done(void)
{
#ifndef WIN32
    return ioctl ( kermit_fd, IOCTL_UPDATE_DONE );
#endif
    return 0;
}

int kermit_is_update_done(void)
{
    unsigned int state = 1;
#ifndef WIN32
    ioctl ( kermit_fd, IOCTL_IS_UPDATE_DONE, &state );
#endif
    return (state != 0);
}

int kermit_panel_on (void)
{
#ifndef WIN32
    return ioctl ( kermit_fd, IOCTL_PANEL_ON );
#endif
    return 0;
}

int kermit_panel_off (void)
{
#ifndef WIN32
    return ioctl ( kermit_fd, IOCTL_PANEL_OFF );
#endif
    return 0;
}

/** \fn kermit_set_vcom
 *  \brief set vcom voltage
 *  \param  unsigned int vcom voltage
 *  \return forward ioctl's return code
 */
int kermit_set_vcom(unsigned int vcom)
{
#ifndef WIN32
    return ioctl ( kermit_fd, IOCTL_SET_VCOM, &vcom );
#endif
    return 0;
}

