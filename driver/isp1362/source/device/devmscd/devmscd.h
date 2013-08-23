/*************************************************************
 * Philips USB Mass Storage Class driver (Device side)
 *
 * (c) 2002 Koninklijke Philips Electronics N.V., All rights reserved
 *
 * This  source code and any compilation or derivative thereof is the
 * proprietary information of Koninklijke Philips Electronics N.V.
 * and is confidential in nature.
 * Under no circumstances is this software to be exposed to or placed
 * under an Open Source License of any type without the expressed
 * written permission of Koninklijke Philips Electronics N.V.
 *
 * File Name:   devmscd.h
 *
 * History:
 *
 *  Version Date        Author      Comments
 * -------------------------------------------------
 *  1.0     09/23/02    SYARRA      Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/

#ifndef __DEVMSCD_H__
#define    __DEVMSCD_H__

/*
 * Mass Storage (BOT) class commands
 */
#define        DEVMSCD_GET_MAX_LUN            0xFE
#define        DEVMSCD_MS_RESET            0xFF

/*
 * Mass storage class sates
 * State Transistion
 *                           | --------------------
 *                          IDLE                  |
 *                           |                    |
 *                          CBW                   |
 *                           |                    |
 *                     ----------------           |
 *                     |              |           |
 *                  DATA_IN        DATA_OUT       |
 *                     |              |           |
 *                     ----------------           |
 *                            |                   |
 *                           CSW                  |
 *                            |--------------------
 */
#define        DEVMSCD_IDLE                0x00        /* Idle, no command */
#define        DEVMSCD_CBW                    0x01     /* Received command */
#define        DEVMSCD_CSW                    0x02     /* Status Stage */
#define        DEVMSCD_DATA_IN                0x03     /* DATA IN stage */
#define        DEVMSCD_DATA_OUT            0x04        /* DATA OUT State */

/*
 * message lengths
 */
#define        DEVMSCD_CBW_LENGTH            0x1F       /* CBW length */
#define        DEVMSCD_CSW_LENGTH            0x0D       /* CSW length */
#define        DEVMSCD_CBW_WRAPPER            0x0F      /* CBWCB start point */

/*
 * Signatures (little Indian)
 */
#define        DEVMSCD_CBWSIGNATURE        0x43425355      /* CBW Signature */
#define        DEVMSCD_CSWSIGNATURE        0x53425355      /* CSW Signature */

/*
 * CBW Flag vlaues
 */
#define        DEVMSCD_CBWFLAG_OUT            0x00     /* Data out from Host */
#define        DEVMSCD_CBWFLAG_IN             0x80     /* Data In to Host */

typedef struct devmscd_device {
    __u8        state;                    /* Mass storage BOT state */
    __u8        data_in_pipe;            /* Bulk In pipe (DATA IN) */
    __u8        data_out_pipe;            /* Bulk Out Pipe (DATA OUT) */
    __u8        my_read_pipe;
    __u8        my_write_pipe;
    __u32        tx_residue;                /* Transfer residue */
    __u8        status_queue;            /* command status queue flag */
    __u8        config;                    /* USB Configuration # */

    __u8        cbw[DEVMSCD_CBW_LENGTH];    /* Command Block Word */
    __u8        csw[DEVMSCD_CSW_LENGTH];    /* Command Status Word */
} devmscd_dev_t;


/*
 * Driver definitions
 */
#define DRIVER_AUTHOR         "Philips Semiconductors"
#define    DRIVER_DESC            "USB Mass storage Class Driver (Device)"
#define DEVMSCD_VERSION     "1.0"
#define    DRIVER_NAME            "devmscd"

#include "hal_intf.h"

#endif /* __DEVMSCD_H__ */
