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
 * File Name:    disk_emu.c
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <liberipc/eripcbusyd.h>
#include <liberipc/eripccontentlister.h>

#include "msbridge.h"
#include "disk_emu.h"

#define        RX_BUFFER_SIZE        0x20000
#define        TX_BUFFER_SIZE        0x20000

/*------------------------------------------------*
 *          Global variable definitions           *
 *------------------------------------------------*/

int fd_msdev;                    /* Mass storage device file */
int fd_disk;                    /* Disk emulation file */

unsigned char    disk_emu_state = DISK_EMU_IDLE;    /* Disk emulation state */
unsigned long    disk_emu_tx_residue;    /* Transfer residue */
disk_emu_info_t    disk_emu_info;            /* Status */

/* Transfer and receiver buffers */
unsigned char rcvd_data[RX_BUFFER_SIZE];
unsigned char disk_data[TX_BUFFER_SIZE];

unsigned long    tx_residue = 0;


unsigned char    inquiry_data[INQUIRY_DATA_SIZE] = {
    0x00, 0x80, 0x02, 0x00, 0x33, 0x00, 0x00, 0x00,
    'i', 'R', 'e', 'x', ' ', ' ', ' ', ' ',
    'i', 'L', 'i', 'a', 'd', ' ', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    '1', '.', '0', '0'
};

unsigned char    read_capacity_data[READ_CAPACITY_DATA_SIZE] = {
    ((LAST_LBA >>24) & 0xFF),     /* block address */
    ((LAST_LBA >>16) & 0xFF),
    ((LAST_LBA >>8) & 0xFF),
    (LAST_LBA & 0xFF),
    ((BLOCK_SIZE >> 24) & 0xFF), /* each block size (4 bytes) */
    ((BLOCK_SIZE >> 16) & 0xFF),
    ((BLOCK_SIZE >> 8) & 0xFF),
    (BLOCK_SIZE & 0xFF)
};

unsigned char    mode_sense_data[MODE_SENSE_DATA_SIZE] = {
    0x0B,    /* Length of this sense data */
    0x00,    /* medium type */
    0x00,    /* Device specific parameters */
    0x08,     /* block descriptor length */
    0x00,    /* density code */
    ((LAST_LBA >>16) & 0xFF),    /* Last Block address */
    ((LAST_LBA >>8) & 0xFF),
    (LAST_LBA & 0xFF),
    0x00,    /* Reserved */
    ((BLOCK_SIZE >> 16) & 0xFF), /* Block Size */
    ((BLOCK_SIZE >> 8) & 0xFF),
    (BLOCK_SIZE & 0xFF)
};

unsigned char    request_sense_data[REQUEST_SENSE_DATA_SIZE] = {
            0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00
};

int connected = 0; /* Connection status (as mass storage device) */
erClientChannel_t g_eripcchannel;
erIpcCmd_t  g_eripccmd;

/*-----------------------------------------------------*
 *          Local function declerations                *
 *-----------------------------------------------------*/

static    void disk_emu_prepare_sense_data(unsigned char sense_key,
                                unsigned char asc,
                                unsigned char ascq);
static    void disk_emu_command(void);
static    void disk_emu_sighandler(int signo);
static    void disk_emu_status(unsigned char    status);
static    int    disk_emu_read(unsigned long    data_len);
static    int    disk_emu_write(unsigned long    data_len);
static    void disk_emu_print_cmd(unsigned char *cmd);





/*-----------------------------------------------------*
 *          Local function definitions                 *
 *-----------------------------------------------------*/

void disk_emu_print_cmd(unsigned char *cmd) {
    char    cmd_name[30];

    switch(cmd[0]) {
        case CMD_TEST_UNIT_READY:
            strcpy(cmd_name,"TEST_UNIT_READY    ");
            break;
        case CMD_REQUEST_SENSE:
            strcpy(cmd_name,"REQUEST_SENSE      ");
            break;
        case CMD_FORMAT_UNIT:
            strcpy(cmd_name,"CMD_FORMAT_UNIT");
            break;
        case CMD_INQUIRY:
            strcpy(cmd_name,"INQUIRY            ");
            break;
        case CMD_RESERVE:
            strcpy(cmd_name,"RESERVE            ");
            break;
        case CMD_RELEASE:
            strcpy(cmd_name,"RELEASE            ");
            break;
        case CMD_MODE_SENSE:
            strcpy(cmd_name,"MODE_SENSE         ");
            break;
        case CMD_START_STOP_UNIT:
            strcpy(cmd_name,"START_STOP_UNIT    ");
            break;
        case CMD_SEND_DIAGNOSTIC:
            strcpy(cmd_name,"SEND_DIAGNOSTIC    ");
            break;
        case CMD_MEDIUM_REMOVAL:
            strcpy(cmd_name,"MEDIUM_REMOVAL     ");
            break;
        case CMD_READ_CAPACITY:
            strcpy(cmd_name,"READ_CAPACITY      ");
            break;
        case CMD_READ10:
            strcpy(cmd_name,"READ10             ");
            break;
        case CMD_WRITE10:
            strcpy(cmd_name,"WRITE10            ");
            break;
        case CMD_VERIFY10:
            strcpy(cmd_name,"VERIFY10           ");
            break;
        default:
            strcpy(cmd_name,"UNKNOWN COMMAND    ");
            break;

    }
//    printf("%s received len = %d\n",cmd_name, cmd[14]);
}



void disk_emu_prepare_sense_data(unsigned char sense_key,
                        unsigned char asc,
                        unsigned char ascq)
{
    request_sense_data[2] = sense_key;
    request_sense_data[12] = asc;
    request_sense_data[13] = ascq;
}


int    disk_emu_read(unsigned long    data_len)
{
    unsigned long    res_len, nbytes;
    unsigned char    *data_ptr;

    /* Read from the emulation disk */
    res_len = read(fd_disk, disk_data, data_len);

    if(res_len <=0) {
        disk_emu_prepare_sense_data(    SENSE_KEY_MEDIUM_ERROR,
                            ASC_ADDR_NOT_FOUND_4_ID_FLD,
                            0x00);
        disk_emu_state = DISK_EMU_STATUS;
        disk_emu_status(MSCD_CMD_RES_FAILED);
        return 0;
    }

    /* Write to the mass storage device */
    data_ptr = disk_data;
    while(res_len) {
        nbytes = write(fd_msdev,data_ptr, res_len);
        data_ptr += nbytes;
        res_len -= nbytes;
        disk_emu_tx_residue -= nbytes;
    }

    if(disk_emu_tx_residue) {
        data_len = ((disk_emu_tx_residue > TX_BUFFER_SIZE) ?
                        TX_BUFFER_SIZE : disk_emu_tx_residue);
        disk_emu_read(data_len);
    } else {
        disk_emu_prepare_sense_data(    SENSE_KEY_NO_SENSE, 0x00, 0x00);
        disk_emu_state = DISK_EMU_STATUS;
        disk_emu_status(MSCD_CMD_RES_SUCCESS);
    }

    return 0;

}/* End of disk_emu_read */

int    disk_emu_write(unsigned long data_len)
{
    unsigned long    res_len;

    /*
     * This call will be a blocked one until we receive the
     * data from the host
     */

    disk_emu_tx_residue -= data_len;

    /* Write to the emuation disk */
    res_len = write(fd_disk, rcvd_data, data_len);
    fsync(fd_disk);

    if(res_len <=0 ) {
        disk_emu_prepare_sense_data(    SENSE_KEY_MEDIUM_ERROR,
                    ASC_ADDR_NOT_FOUND_4_ID_FLD,
                    0x00);
    }

    if(data_len != res_len) {
//        printf("read write error\n");
    }

    if(!disk_emu_tx_residue) {
        disk_emu_state = DISK_EMU_STATUS;

        disk_emu_prepare_sense_data(    SENSE_KEY_NO_SENSE, 0x00,0x00);
        disk_emu_status(MSCD_CMD_RES_SUCCESS);
    } else {
        data_len = ((disk_emu_tx_residue > RX_BUFFER_SIZE) ?
                                RX_BUFFER_SIZE : disk_emu_tx_residue);
        if(data_len) disk_emu_write(data_len);
    }

    return 0;
}/* End of disk_emu_write */

void disk_emu_command(void)
{
    unsigned char    *cmd = rcvd_data;
    unsigned char    cmd_type = cmd[0];

    unsigned int    data_len = 0;
    unsigned long     block_addr;
    unsigned char    prevent;

    long    res_len = 0;

    disk_emu_print_cmd(cmd);

    disk_emu_state = DISK_EMU_CMD;
//    printf("state = DISK_EMU_CMD\n");

    switch(cmd_type) {

        case CMD_READ10:

          block_addr = ((cmd[2] << 24)|(cmd[3] << 16)|(cmd[4]<<8)|cmd[5]);
            data_len = ((cmd[7] << 8) | cmd[8]) *BLOCK_SIZE;
            disk_emu_tx_residue = data_len;

            if(fd_disk < 0) fd_disk = open(MSDISK_FILE, O_RDWR);

            if(disk_emu_tx_residue && (fd_disk != 1)) {

                /* Need to do more checking */
                lseek(fd_disk, block_addr*BLOCK_SIZE, SEEK_SET);

                /* Some thing to be read from the disk emulation */

                data_len = ((disk_emu_tx_residue > TX_BUFFER_SIZE) ?
                                TX_BUFFER_SIZE : disk_emu_tx_residue);
                disk_emu_state = DISK_EMU_DATA_OUT;
//                printf("state = DISK_EMU_DATA_OUT\n");
                disk_emu_read(data_len);

            } else {

//                printf("read call failed \n");
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_FAILED);

            }
            break;

        case CMD_WRITE10:

            block_addr = ((cmd[2] << 24)|(cmd[3]<<16)|(cmd[4]<<8)|cmd[5]);
            data_len = ((cmd[7] << 8) | cmd[8]) *BLOCK_SIZE;
            disk_emu_tx_residue = data_len;

//            printf("block_addr = %lu, data_len = %d\n", block_addr, data_len);

            lseek(fd_disk, block_addr*BLOCK_SIZE, SEEK_SET);


            disk_emu_state = DISK_EMU_DATA_IN;
//            printf("state = DISK_EMU_DATA_IN\n");

            data_len = ((disk_emu_tx_residue > RX_BUFFER_SIZE) ?
                                    RX_BUFFER_SIZE : disk_emu_tx_residue);
            data_len = read(fd_msdev, rcvd_data, data_len);
//            printf("read nbytes = %x\n", data_len);
            if(data_len) disk_emu_write(data_len);

            break;

        case CMD_REQUEST_SENSE:

            /* Send the sense data */
            res_len = REQUEST_SENSE_DATA_SIZE;
            res_len = write(fd_msdev,request_sense_data, res_len);
            disk_emu_tx_residue -= res_len;

            disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);

            disk_emu_state = DISK_EMU_STATUS;
            disk_emu_status(MSCD_CMD_RES_SUCCESS);

            break;


        case CMD_INQUIRY:

            disk_emu_tx_residue = cmd[4];


            res_len = (cmd[4]<INQUIRY_DATA_SIZE)?cmd[4] : INQUIRY_DATA_SIZE;

            if((cmd[1] & INQUIRY_EVPD) || cmd[2] ) {
                /* If EVPD flags are set or Page code is not zero we are not
                 * supporting it */

                disk_emu_prepare_sense_data(SENSE_KEY_ILLEGAL_REQUEST,
                                ASC_INV_FIELD_IN_CDB,
                                0x00);
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_FAILED);

            } else {

                disk_emu_tx_residue -= write(fd_msdev,inquiry_data, res_len);
                disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_SUCCESS);
            }
            break;

        case CMD_TEST_UNIT_READY:
            if(fd_disk) {
                disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_SUCCESS);
            } else {
                disk_emu_prepare_sense_data(SENSE_KEY_NOT_READY, 0x00, 0x00);
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_FAILED);
            }
            break;
        case CMD_START_STOP_UNIT:
            if(cmd[4]&0x01) {
                /* Start the UNIT */
                if(fd_disk < 0)
            fd_disk = open(MSDISK_FILE, O_RDWR);
            } else {
                /* stop the unit */
                if(fd_disk) close(fd_disk);
                fd_disk = -1;
            }
        case CMD_FORMAT_UNIT:
        case CMD_SEND_DIAGNOSTIC:
        case CMD_VERIFY10:

            disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);
            disk_emu_state = DISK_EMU_STATUS;
            disk_emu_status(MSCD_CMD_RES_SUCCESS);

            break;

        case CMD_MEDIUM_REMOVAL:

            prevent = cmd[4] & 0x01;

            if(prevent && disk_emu_info.prevent) {
                /* problem */

                disk_emu_prepare_sense_data(SENSE_KEY_UNIT_ATTENTION,
                                            ASC_MEDIUM_CHANGE,
                                            0x00);

                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_FAILED);

            } else {
                disk_emu_info.prevent = (cmd[4] & 0x01);

                lseek(fd_disk, LOGICAL_BLOCKS*BLOCK_SIZE, SEEK_SET);
                write(fd_disk, (unsigned char*)(&disk_emu_info),
                 sizeof(disk_emu_info_t));

                disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_SUCCESS);
            }

            break;

        case CMD_READ_CAPACITY:

            if(!((cmd[8])& READ_CAPACITY_RMI)) {

                res_len = READ_CAPACITY_DATA_SIZE;
                disk_emu_tx_residue -= write(fd_msdev,read_capacity_data,
                 res_len);
                disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);

                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_SUCCESS);

            } else {

                res_len = -1;
                disk_emu_prepare_sense_data(    SENSE_KEY_ILLEGAL_REQUEST,
                                    ASC_INV_FIELD_IN_CDB,
                                    0x00);
            }

            break;

        case CMD_MODE_SENSE:

            res_len = MODE_SENSE_DATA_SIZE;
            if(cmd[4] && cmd[4] < res_len) res_len = cmd[4];

            disk_emu_tx_residue -= write(fd_msdev,mode_sense_data, res_len);
            disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);

            disk_emu_state = DISK_EMU_STATUS;
            disk_emu_status(MSCD_CMD_RES_SUCCESS);


            break;


        case CMD_RESERVE:
        case CMD_RELEASE:

            if(cmd[1] & RESERVE_EXTENT) {
                disk_emu_prepare_sense_data(SENSE_KEY_ILLEGAL_REQUEST,
                                ASC_INV_FIELD_IN_CDB, 0x00);
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_FAILED);
            } else {
                disk_emu_prepare_sense_data(SENSE_KEY_NO_SENSE, 0x00, 0x00);
                disk_emu_state = DISK_EMU_STATUS;
                disk_emu_status(MSCD_CMD_RES_SUCCESS);
            }
            break;


        default:


            disk_emu_prepare_sense_data(SENSE_KEY_ILLEGAL_REQUEST,
            ASC_INV_FIELD_IN_CDB,
            0x00);
            disk_emu_state = DISK_EMU_STATUS;

            sleep(1);
            disk_emu_status(MSCD_CMD_RES_FAILED);
            break;

    }
    return;
}

/*
 * Send command response to the mass storage device
 */
void disk_emu_status(unsigned char    status)
{
    mscd_cmd_res_t    cmd_res;

    cmd_res.status = status;
    cmd_res.residue = disk_emu_tx_residue;
    ioctl(fd_msdev, MSCD_IOC_SET_CMD_RES, &cmd_res);
    disk_emu_state = DISK_EMU_IDLE;
}


/*
 * Signal handler
 * This processes the signals received from the mass storage device
 */
void disk_emu_sighandler(int signo) {
    unsigned int status, nbytes;
    mscd_notif_t     notif_sts;
    int ret;

    /*
     * Get the Signal/notification from the device
     */

    ret = ioctl(fd_msdev, MSCD_IOC_GET_NOTIF, &notif_sts);
      status = notif_sts.notif;


//      printf("Signal received %x \n",status);
      switch (status) {

        case MSCD_CMD:
            //printf("MSCD_DATA_RCVD\n");

        if (!connected){
            connected=1;
            printf("*** Connected to pc (Delayed)***\n");
            strcpy(g_eripccmd.name, "MSDiskSetConnected");
            g_eripccmd.cc = ccClMSDiskSetConnected;
            g_eripccmd.nArg = 1;
            strcpy(g_eripccmd.arg[0], "1");
            erIpcSndCommand(g_eripcchannel, &g_eripccmd);
        }
            if(disk_emu_state == DISK_EMU_IDLE) {

                ret = ioctl(fd_msdev, MSCD_IOC_GET_COMMAND, rcvd_data);
                disk_emu_command();

            } else if(disk_emu_state == DISK_EMU_DATA_IN) {
                nbytes = read(fd_msdev,rcvd_data, disk_emu_tx_residue);

//                printf("received bytes %d\n", nbytes);
                disk_emu_write(nbytes);
        }

        break;

    case MSCD_RESET:
        printf("MSCD_RESET\n");
        /* This is reset mass storage device, reset the FSM */
        disk_emu_info.prevent = 0;
        lseek(fd_disk, LOGICAL_BLOCKS*BLOCK_SIZE, SEEK_SET);
        nbytes = write(fd_disk, (unsigned char*)(&disk_emu_info),
         sizeof(disk_emu_info_t));

        disk_emu_state = DISK_EMU_IDLE;

        break;
    case MSCD_CONNECT:
        if (!connected){
            connected=1;
            printf("*** Connected to pc ***\n");
            strcpy(g_eripccmd.name, "MSDiskSetConnected");
            g_eripccmd.cc = ccClMSDiskSetConnected;
            g_eripccmd.nArg = 1;
            strcpy(g_eripccmd.arg[0], "1");
            erIpcSndCommand(g_eripcchannel, &g_eripccmd);
        }
    break;
    case MSCD_DISCONNECT:
        if (connected){
            connected = 0;
            printf("*** Disconnected from pc ***\n");
            strcpy(g_eripccmd.name, "MSDiskSetConnected");
            g_eripccmd.cc = ccClMSDiskSetConnected;
            g_eripccmd.nArg = 1;
            strcpy(g_eripccmd.arg[0], "0");
            erIpcSndCommand(g_eripcchannel, &g_eripccmd);
        }
    break;
      default :
        printf("unknown status %d\n", status);
        break;
    }

    return;
}



int main(int argc, char **argv)
{
    int ret;
    struct sigaction action;
    int    nbytes;
    int i;
    unsigned char buf[0x40] = {0};
    int bytes_read = 0;

    /* Open the mass storage device file, Try this 10 times before giving up */
    fd_msdev = -1;
    i = 0;
    while((i < 10) && (fd_msdev < 0))
    {
        printf("\nOpening %s Try %d...", MS_DEV_FILE_NAME, i+1);
        fd_msdev=open(MS_DEV_FILE_NAME, O_RDWR);
        sleep(1);
        i++;
    }
    if(fd_msdev < 0)
    {
        printf("\nError in opening file: %s\n",MS_DEV_FILE_NAME);
        exit(0);
    }
    else
    {
        printf("Success\n");
    }

    /* Open the mass storage disk emulation file */
    fd_disk = open(MSDISK_FILE, O_RDWR);

    if(fd_disk < 0) {
        printf("disk emulation file is not found %s\n",MSDISK_FILE);
        close(fd_msdev);
        exit(0);
    }

    /* Initalize the status from the mass storage emulation file */

    lseek(fd_disk, LOGICAL_BLOCKS*BLOCK_SIZE, SEEK_SET);
    nbytes = read(fd_disk, (unsigned char*)(&disk_emu_info),
    sizeof(disk_emu_info_t));

    if(nbytes != sizeof(disk_emu_info_t)) {
        printf("Init file reading failed\n");
        close(fd_disk);
        close(fd_msdev);
        exit(0);
    }

    close(fd_disk);        /* Close the file temporarly */
    fd_disk = -1;


    /*
     * Set signal action to receive asynchronous notifications from the
     * mass storage device file
     */
    memset(&action, 0, sizeof(action));
    action.sa_handler = disk_emu_sighandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    erIpcStartClient(ER_CONTENTLISTER_CHANNEL, &g_eripcchannel);

    sigaction(SIGIO, &action, NULL);

    ret= fcntl(fd_msdev, F_SETOWN, getpid());

    ret=fcntl(fd_msdev,F_GETFL);

    fcntl(fd_msdev, F_SETFL, ret | FASYNC);

    /*
     * Wait for ever and process the things in signal handler
     */
    while (1)
    {
        sleep(100);
    }

    if(fd_disk > 0) close(fd_disk);
    close(fd_msdev);
}
