/**
 * \name kermit_ioctl.h (proprietary kermit driver)
 * \brief Kermit ioctl interface
 *
 * Copyright (C) 2009, FirstPaper LLC
 *
 * The source code in this file can't be used, adapted,
 * and redistributed in source or binary form, without 
 * FirstPaper LLC permission
 *
 * \author Genisim tsilker (gtsilker@firstpaper.com) Alice Hsia (ahsia@firstpaper.com)
 */
#ifndef _KERMIT_IOCTL_H
#define _KERMIT_IOCTL_H

#define IOCTL_KERMIT_MAGIC              'K'

struct kermit_fb_parms {
    unsigned int active_fb;
    unsigned int backup_fb;
    unsigned int bytes;
};

struct kermit_vers {
    unsigned short revId;
    unsigned short realRevId;
};

#define IOCTL_DISPLAY_ON                            _IO(IOCTL_KERMIT_MAGIC,  0)
#define IOCTL_DISPLAY_OFF                           _IO(IOCTL_KERMIT_MAGIC,  1)
#define IOCTL_WAVEFORMS_SET                         _IOW(IOCTL_KERMIT_MAGIC, 2, int *)
#define IOCTL_GET_FB_BUF                            _IOR(IOCTL_KERMIT_MAGIC, 3, struct kermit_fb_parms)
#define IOCTL_GET_VERSION                           _IOR(IOCTL_KERMIT_MAGIC, 4, struct kermit_vers)
#define IOCTL_FULL_FLASH                            _IO(IOCTL_KERMIT_MAGIC,  5)
#define IOCTL_PARTIAL_UPDATE                        _IO(IOCTL_KERMIT_MAGIC,  6)
#define IOCTL_SET_VCOM                              _IOW(IOCTL_KERMIT_MAGIC, 7, int *)
#define IOCTL_PANEL_ON                              _IO(IOCTL_KERMIT_MAGIC,  8)
#define IOCTL_PANEL_OFF                             _IO(IOCTL_KERMIT_MAGIC,  9)
#define IOCTL_UPDATE_DONE                           _IO(IOCTL_KERMIT_MAGIC, 10)
#define IOCTL_IS_UPDATE_DONE                        _IO(IOCTL_KERMIT_MAGIC, 11)
#define IOCTL_CHANGE_GRAYSCALE                      _IO(IOCTL_KERMIT_MAGIC, 12)
#define IOCTL_KERMIT_MAXNR                          13

#endif


