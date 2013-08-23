
/*************************************************************
 * Philips USB Mass Storage Class driver Interface
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
 * File Name:    msbridge.h
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/

#ifndef __DEVMSCD_INTF_H__
#define    __DEVMSCD_INTF_H__



#define        MSCD_CMD_LEN    0x10


#define        MSCD_CMD        MSCD_COMMAND
#define        MSCD_RESET        MSCDBRIDGE_RESET
#define     MSCD_CONNECT    42
#define     MSCD_DISCONNECT 41

typedef struct mscd_notif {
    unsigned char    notif;                /* notification type */
} mscd_notif_t;


/*
 * Mass storage command
 */
typedef struct mscd_command {
    unsigned char    cmd[MSCD_CMD_LEN];
} mscd_command_t;

#define        MSCD_CMD_RES_SUCCESS    MSCD_SUCCESS
#define        MSCD_CMD_RES_FAILED        MSCD_FAILED
#define        MSCD_CMD_RES_ERROR        MSCD_ERROR

typedef struct mscd_cmd_res {
    unsigned char    status;        /* response of the command */
    unsigned long    residue;    /* data residue */
} mscd_cmd_res_t;

typedef struct
{
    unsigned char data[0x40];
} msgd_cmd_read_data;

#define        DEVMSCD_MAJOR            18
#define        MSCD_IOC_MAGIC            'o'

#define   MSCD_IOC_GET_NOTIF     _IOR(MSCD_IOC_MAGIC, 0x10, mscd_notif_t)
#define   MSCD_IOC_GET_COMMAND   _IOR(MSCD_IOC_MAGIC, 0x11, mscd_command_t)
#define   MSCD_IOC_SET_CMD_RES   _IOW(MSCD_IOC_MAGIC, 0x12, mscd_cmd_res_t)
#define   MSGD_IOC_READ_DATA     _IOR(MSCD_IOC_MAGIC, 0x13, msgd_cmd_read_data)

#include "mscdbridge.h"

#endif    /* __DEVMSCD_INTF_H__ */
