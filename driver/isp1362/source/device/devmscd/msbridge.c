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
 * File Name:    msbridge.c
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/

#include <linux/config.h>
#define MODULE
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>  /* for in_interrupt() */
#undef DEBUG
#include <linux/usb.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>


#include "pdc_intf.h"
#include "devmscd.h"
#include "msbridge.h"

#define        MODULE_NAME            "msbridge"


typedef struct devmscd_intf_info {

    __u8                    notif;            /* Notification type */
    __u8                    use;
    __u8                    notif_pending;
    __u8                    *cmd;            /* mass storage command */

    wait_queue_head_t         read_wq;        /* read wait queue */
    __u8                    read_lock;        /* read lock */
    __u32                    read_bytes;/* read response (from device) bytes */

    wait_queue_head_t         my_read_wq;        /* read wait queue */
    __u8                    my_read_lock;    /* read lock */
    __u32                    my_read_bytes;
    /* read response (from device) bytes */

    wait_queue_head_t         write_wq;        /* Write wait queue (TODO) */
    __u8                    write_lock;        /* Write lock */
    __u32                    write_bytes;    /* Write response bytes */

    struct fasync_struct     *fasync_q;        /* Asynchronous notification */

} devmscd_intf_info_t;




/*------------------------------------------------------------------*
 *                 local function declerations                      *
 *------------------------------------------------------------------*/

static ssize_t devmscd_intf_read(struct file *fp,
char *dest, size_t count, loff_t *ppos);
static ssize_t devmscd_intf_write(struct file *fp,
 const char *buf, size_t count, loff_t *ppos);
static    int    devmscd_intf_open( struct inode *inode, struct file *fp);
static    int    devmscd_intf_close( struct inode *inode, struct file *fp);
static    int devmscd_intf_fasync( int    fd, struct file *fp, int mode);
static    int    devmscd_intf_ioctl(struct inode *inode, struct file *fp,
                    unsigned int    cmd, unsigned long arg);
static void devmscd_disk_write(mscd_req_t *req);
static void devmscd_disk_read(mscd_req_t *req);

extern struct pdc_urb            my_read_urb;
extern struct devmscd_device    devmscd_dev;

/*------------------------------------------------------------------*
 *                 global variable declerations                     *
 *------------------------------------------------------------------*/

static struct file_operations devmscd_intf_fops = {
    owner:        THIS_MODULE,
    read:        devmscd_intf_read,
    write:        devmscd_intf_write,
    poll:        NULL,
    ioctl:        devmscd_intf_ioctl,
    open:        devmscd_intf_open,
    release:    devmscd_intf_close,
    fasync:        devmscd_intf_fasync,
};

struct devmscd_intf_info    intf_info;
struct devmscd_intf_info    *devmscd_intf;
__u8    rcvd_data[0x20000];
__u8    send_data[0x20000];
mscd_req_t        mscd_read_req, mscd_write_req;
mscdbridge_req_t    *mscd_req = NULL;

static int devmscd_connected = 0;
static int fn_poll_running = 0;


/*------------------------------------------------------------------*
 *                 local function definitions                       *
 *------------------------------------------------------------------*/

void devmscd_notify_connected(int connected)
{
    if (connected) {
        devmscd_connected = 1;
        devmscd_intf->notif = MSCD_CONNECT;
        printk("--(= Cable plugged in event\n");
    }
    else {
        devmscd_connected = 0;
        devmscd_intf->notif = MSCD_DISCONNECT;
        printk("=)-- Cable unplugged event\n");
    }
    if(devmscd_intf->fasync_q) {
        kill_fasync( &devmscd_intf->fasync_q, SIGIO, POLL_IN);
    } else {
        devmscd_intf->notif_pending = 1;
    }
}

/* poll the DCframenumber register to see if we are still connected */
int devmscd_fn_poll(void *dummy)
{
    int oldfn1, oldfn2, newfn;
    struct task_struct *curtask = current;

    newfn = pdc_read_fn();
    oldfn1 = newfn;
    oldfn2 = newfn;

    daemonize();

    strcpy(current->comm, "usbplugd");

    while(fn_poll_running)
    {
        newfn=pdc_read_fn();
        // printk("fn = %d, %d, %d\n", newfn, oldfn1, oldfn2);

        if ((newfn == oldfn1) && (newfn == oldfn2))
        {
            if (devmscd_connected)
            {
                devmscd_notify_connected(0);
            }
        }
        else
        {
            if (!devmscd_connected)
            {
                devmscd_notify_connected(1);
            }
        }
        oldfn2 = oldfn1;
        oldfn1 = newfn;
        current->state = TASK_INTERRUPTIBLE;
        schedule_timeout(HZ);
    }
    printk("Exiting usbplugd kernel thread\n");
    return 0;
}

void cb_read_complete(struct pdc_urb* urb)
{
    // urb->actual_length is the actual data length we read just now.
    devmscd_intf->my_read_bytes = urb->actual_length;

    // Wake up applications
    devmscd_intf->my_read_lock = 0;
    wake_up_interruptible(&(devmscd_intf->my_read_wq));
}

ssize_t devmsgd_intf_read(char *dest, size_t count)
{
    devmscd_intf->my_read_lock = 1;

    /* Fill the read URB */
    my_read_urb.next = NULL;
    my_read_urb.pipe = devmscd_dev.my_read_pipe;
    my_read_urb.pipe_type = PDC_PIPE_BULK;
    my_read_urb.operation = PDC_OPR_READ;
    my_read_urb.status = PDC_URB_PENDING;
    my_read_urb.transfer_buffer = dest;
    my_read_urb.transfer_buffer_length = count;
    my_read_urb.actual_length = 0x00;
    my_read_urb.complete = cb_read_complete; /* call back function */

    pdc_submit_urb(&my_read_urb);

    while (devmscd_intf->my_read_lock)
    {
        interruptible_sleep_on(&(devmscd_intf->my_read_wq));
        if (signal_pending(current))
        {
            return -ERESTARTSYS;
        }
    }

    return devmscd_intf->my_read_bytes;
}

/*
 * Send read command to the class driver.
 */
ssize_t devmscd_intf_read(struct file *fp, char *dest, size_t count,
loff_t *ppos) {
    devmscd_intf_info_t    *intf = (devmscd_intf_info_t*)fp->private_data;

    intf->read_lock = 1;

//    devmscd_read_data(rcvd_data, count);
    mscd_fill_req((&mscd_read_req), MSCD_READ,
            rcvd_data, count,
            devmscd_disk_write, NULL);
    devmscd_submit_req(&mscd_read_req);

    while(intf->read_lock) {
        interruptible_sleep_on(&intf->read_wq);
        if (signal_pending(current))
             return -ERESTARTSYS;
    }

    memcpy(dest, rcvd_data, intf->read_bytes);
    return intf->read_bytes;

}

/*
 * Send read command to the class driver.
 */
ssize_t devmscd_intf_write(struct file *fp, const char *buf, size_t count,
 loff_t *ppos)
{
    devmscd_intf_info_t    *intf = (devmscd_intf_info_t*)fp->private_data;

    memcpy(send_data,buf, count);
    intf->write_lock = 1;
//    devmscd_write_data(send_data, count);
    mscd_fill_req((&mscd_write_req), MSCD_WRITE,
            send_data, count,
            devmscd_disk_read, NULL);
    devmscd_submit_req(&mscd_write_req);

    while(intf->write_lock) {
        interruptible_sleep_on(&intf->write_wq);
        if (signal_pending(current))
             return -ERESTARTSYS;
    }

    return intf->write_bytes;
}

/*
 * Intialize the wait queues
 */
int    devmscd_intf_open( struct inode *inode, struct file *fp)
{

    fp->private_data = devmscd_intf; /* set the OTG controller data in file */

    init_waitqueue_head(&devmscd_intf->read_wq);
    init_waitqueue_head(&devmscd_intf->write_wq);
    init_waitqueue_head(&devmscd_intf->my_read_wq);

    devmscd_intf->use++;

    MOD_INC_USE_COUNT;                    /* Increment the module count */

    return 0;
}

/*
 * cancel any asynchronous notification
 */
int    devmscd_intf_close( struct inode *inode, struct file *fp)
{

    MOD_DEC_USE_COUNT;                    /* Decrement module count */

    if(devmscd_intf->use) devmscd_intf->use--;

    devmscd_intf_fasync(-1,fp, 0);
    /* cancel asynchronous notification to application */
    return 0;
}

/*
 *  Set/reset asynchronous notification to the application
 */
int devmscd_intf_fasync( int    fd, struct file *fp, int mode)
{
    int result;

    devmscd_intf_info_t    *intf = (devmscd_intf_info_t*)fp->private_data;


    result =  fasync_helper(fd, fp, mode, &intf->fasync_q);

    if(result >= 0 && intf->notif_pending) {
        if(intf->fasync_q) {
                kill_fasync( &intf->fasync_q, SIGIO, POLL_IN);
                intf->notif_pending = 0;
        }
    }

    return result;
}

/*
 *  perform the IO control operations for the application
 */
int    devmscd_intf_ioctl(struct inode *inode, struct file *fp,
                    unsigned int    cmd, unsigned long arg) {

    devmscd_intf_info_t    *intf = (devmscd_intf_info_t*)fp->private_data;
    int                ret = 0;
    mscd_cmd_res_t    cmd_res;
    msgd_cmd_read_data read_buf;

    switch(cmd) {
        case MSCD_IOC_GET_NOTIF:

            copy_to_user((mscd_notif_t *)arg, &intf->notif,
             sizeof(mscd_notif_t));
        break;

        case MSCD_IOC_GET_COMMAND:

            if(mscd_req) {
                copy_to_user((mscd_command_t *)arg, intf->cmd,
                sizeof(mscd_command_t));
            } else {
                return -EINVAL;
            }
        break;

        case MSCD_IOC_SET_CMD_RES:
            copy_from_user(&cmd_res, (mscd_cmd_res_t *)arg,
            sizeof(mscd_cmd_res_t));

            if(mscd_req && mscd_req->complete) {
                mscd_req_t    *req = mscd_req;
                req->status = cmd_res.status;
                mscd_req = NULL;
                req->complete(req);
            }
//            devmscd_set_command_res(cmd_res.status);

            break;
        case MSGD_IOC_READ_DATA:
            ret = devmsgd_intf_read(read_buf.data, 0x40);
            copy_to_user((msgd_cmd_read_data *)arg, read_buf.data,
             sizeof(msgd_cmd_read_data));
            break;

    }

    return ret;
}

/*
 * Wake up any pending read requests
 */
void devmscd_disk_read(mscd_req_t *req)
{

    devmscd_intf->write_bytes = req->res_data_len;
    devmscd_intf->write_lock = 0;

    wake_up_interruptible(&devmscd_intf->write_wq);

    return;
}

/*
 * Wake up any pending write requests
 */
void devmscd_disk_write(mscd_req_t *req)
{

    devmscd_intf->read_bytes = req->res_data_len;
    devmscd_intf->read_lock = 0;

    wake_up_interruptible(&devmscd_intf->read_wq);

    return;
}

int mscdbridge_submit_req(mscdbridge_req_t *req)
{
    int    result = MSCD_SUCCESS;

    switch(req->req_type) {

        case MSCDBRIDGE_DEINIT:
            req->status = MSCD_SUCCESS;
            devfs_unregister_chrdev(DEVMSCD_MAJOR,DRIVER_NAME);
            devmscd_intf  = NULL;
            break;
        case MSCDBRIDGE_INIT:
            req->status = MSCD_SUCCESS;
            devmscd_intf  = &intf_info;

            devmscd_intf->use = 0;
            devmscd_intf->notif_pending = 0;

            result = devfs_register_chrdev(DEVMSCD_MAJOR, DRIVER_NAME,
            &devmscd_intf_fops);
            break;

        case MSCD_COMMAND:
            /* Store the request */
            mscd_req = req;
            devmscd_intf->cmd = req->data_buff;
        case MSCDBRIDGE_RESET:
            /*
             * Send notification to the mass storage device
             */
            req->status = MSCD_SUCCESS;
            devmscd_intf->notif = req->req_type;

            if(devmscd_intf->fasync_q) {
                kill_fasync( &devmscd_intf->fasync_q, SIGIO, POLL_IN);
            } else {
                devmscd_intf->notif_pending = 1;
            }
            break;

    }

    return result;
}
