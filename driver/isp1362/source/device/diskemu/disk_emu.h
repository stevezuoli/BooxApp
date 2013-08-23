/*************************************************************
 * Philips Mass Storage disk emulation
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
 * File Name:    disk_emu.h
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/
#ifndef __DISK_EMU_H__
#define __DISK_EMU_H__

/* Disk Emulation file */
#ifdef CONFIG_1362_PXA250
#ifdef CONFIG_PXA250_REV4
#define        MSDISK_FILE                        "/dev/tffsa6"
#else
#define        MSDISK_FILE                        "/mnt/ramdisk/otgdisk"
#endif
#else
#define        MSDISK_FILE                        "/dev/usb/otg/otgdisk"
#endif /* CONFIG_1362_PXA250 */

#ifdef _8MB_SIZE_
#define   LOGICAL_BLOCKS          0x00004000
#define   LINUX_UNIT_SIZE         16
#else /* 128MB size */
#define   LOGICAL_BLOCKS          0x00040000
#define   LINUX_UNIT_SIZE         32
#endif /* _8MB_SIZE_ */

#define        BLOCK_SIZE                        0x00000200
#define        LAST_LBA                        (LOGICAL_BLOCKS-1)

#define        INQUIRY_DATA_SIZE                0x24
#define        READ_CAPACITY_DATA_SIZE            0x08
#define        MODE_SENSE_DATA_SIZE            0x0C
#define        REQUEST_SENSE_DATA_SIZE            0x12

#define        CMD_TEST_UNIT_READY                0x00
#define        CMD_REQUEST_SENSE                0x03
#define        CMD_FORMAT_UNIT                    0x04
#define        CMD_INQUIRY                        0x12
#define        CMD_RESERVE                        0x16
#define        CMD_RELEASE                        0x17
#define        CMD_MODE_SENSE                    0x1A
#define        CMD_START_STOP_UNIT                0x1B
#define        CMD_SEND_DIAGNOSTIC                0x1D
#define        CMD_MEDIUM_REMOVAL                0x1E
#define        CMD_READ_CAPACITY                0x25
#define        CMD_READ10                        0x28
#define        CMD_WRITE10                        0x2A
#define        CMD_VERIFY10                    0x2F

/*
 * Definitions for INQUIRY CMD fields
 */
#define        INQUIRY_EVPD                    0x01

/*
 * Definitions of READ_CAPACITY fields
 */
#define        READ_CAPACITY_RMI                0x01

/*
 * Definition of RESERVE RELEASE fields
 */
#define        RESERVE_EXTENT                    0x01


/* REQUEST_SENSE */
/*
 * Sense Key definitions
 */
#define        SENSE_KEY_NO_SENSE                0x00
#define        SENSE_KEY_RECOVERED_ERROR        0x01
#define        SENSE_KEY_NOT_READY                0x02
#define        SENSE_KEY_MEDIUM_ERROR            0x03
#define        SENSE_KEY_HARDWARE_ERROR        0x04
#define        SENSE_KEY_ILLEGAL_REQUEST        0x05
#define        SENSE_KEY_UNIT_ATTENTION        0x06
#define        SENSE_KEY_DATA_PROTECT            0x07

/*
 * Additional Sense Codes (ASC)
 */
#define        ASC_ADDR_NOT_FOUND_4_ID_FLD        0x12
#define        ASC_ADDR_NOT_FOUND_4_DATA_FLD    0x13
#define        ASC_INV_FIELD_IN_CDB            0x24
#define        ASC_MEDIUM_CHANGE                0x28

/*
 * Additional Sense Code Qualifiers (ASCQ)
 */
#define        ASCQ_INV_FIELD_IN_CDB            0x00



#define        MS_DEV_FILE_NAME            "/dev/otgdev0"

/* OTG ms disk status */
typedef struct disk_emu_info {
    unsigned char    prevent;            /* Allow/prevent medium removal */
    unsigned char    sense_key;            /* Sense key */
    unsigned char    asc;                /* Additional sense code */
    unsigned char    ascq;                /* Additional sense code qualifier */
} disk_emu_info_t;

/*
 * Disk emulation states
 */
#define        DISK_EMU_IDLE            0x00
#define        DISK_EMU_CMD            0x01
#define        DISK_EMU_STATUS            0x02
#define        DISK_EMU_DATA_IN        0x03
#define        DISK_EMU_DATA_OUT        0x04

#endif /* __DISK_EMU_H__ */
