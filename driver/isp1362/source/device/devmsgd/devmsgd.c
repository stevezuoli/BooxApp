#include <linux/config.h>
#define MODULE
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/tqueue.h>
#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/devfs_fs_kernel.h>

#include "hal_intf.h"
#include "pdc_intf.h"
#include "pdc_bus.h"
#include "devmsgd.h"

// Global variables
static struct pdc_urb    read_urb, write_urb;
static devmsgd_intf_info devmsgd_intf;
static MsgDev            devmsgd_dev;
static __u8              rcvd_data[0x20000];
static __u8              send_data[0x20000];

// Interface functions
int devmsgd_intf_open(struct inode* inode, struct file* fp)
{
    init_waitqueue_head(&(devmsgd_intf.read_wq));
    init_waitqueue_head(&(devmsgd_intf.write_wq));

    MOD_INC_USE_COUNT;
    return 0;
}

void cb_read_complete(struct pdc_urb* urb)
{
    // urb->actual_length is the actual data length we read just now.
    devmsgd_intf.read_bytes = urb->actual_length;

    // Wake up applications
    devmsgd_intf.read_lock = 0;
    wake_up_interruptible(&(devmsgd_intf.read_wq));
}

ssize_t devmsgd_intf_read(struct file *fp, char *dest, size_t count,
loff_t *ppos)
{
    devmsgd_intf.read_lock = 1;

    /* Fill the read URB */
    read_urb.next = NULL;
    read_urb.pipe = devmsgd_dev.data_in_pipe;
    read_urb.pipe_type = PDC_PIPE_BULK;
    read_urb.operation = PDC_OPR_READ;
    read_urb.status = PDC_URB_PENDING;
    read_urb.transfer_buffer = rcvd_data;
    read_urb.transfer_buffer_length = count;
    read_urb.actual_length = 0x00;
    read_urb.complete = cb_read_complete; /* call back function */

    pdc_submit_urb(&read_urb);

    while (devmsgd_intf.read_lock)
    {
        interruptible_sleep_on(&(devmsgd_intf.read_wq));
        if (signal_pending(current))
        {
            return -ERESTARTSYS;
        }
    }

    memcpy(dest, rcvd_data, devmsgd_intf.read_bytes);
    return devmsgd_intf.read_bytes;
}

void cb_write_complete(struct pdc_urb* urb)
{
    // urb->actual_length is the actual data length we wrote just now.
    devmsgd_intf.write_bytes = urb->actual_length;

    // Wake up applications
    devmsgd_intf.write_lock = 0;
    wake_up_interruptible(&(devmsgd_intf.write_wq));
}

ssize_t devmsgd_intf_write(struct file *fp, const char *buf, size_t count,
loff_t *ppos)
{
    memcpy(send_data, buf, count);
    devmsgd_intf.write_lock = 1;

    /* Fill the wrtite URB */
    write_urb.next = NULL;
    write_urb.pipe = devmsgd_dev.data_out_pipe;
    write_urb.pipe_type = PDC_PIPE_BULK;
    write_urb.operation = PDC_OPR_WRITE;
    write_urb.status = PDC_URB_PENDING;
    write_urb.transfer_buffer = send_data;
    write_urb.transfer_buffer_length = count;
    write_urb.actual_length = 0x00;
    write_urb.complete = cb_write_complete;

    pdc_submit_urb(&write_urb);

    while (devmsgd_intf.write_lock)
    {
        interruptible_sleep_on(&(devmsgd_intf.write_wq));
        if (signal_pending(current))
        {
            return -ERESTARTSYS;
        }
    }

    return devmsgd_intf.write_bytes;
}

int devmsgd_intf_close(struct inode *inode, struct file *fp)
{
    MOD_DEC_USE_COUNT;
    return 0;
}

static struct file_operations fops =
{
    owner:   THIS_MODULE,
    open:    devmsgd_intf_open,
    read:    devmsgd_intf_read,
    write:   devmsgd_intf_write,
    release: devmsgd_intf_close,
    poll:    NULL,
    ioctl:   NULL,
    fasync:  NULL
};

int devmsgd_class_req(void* priv, unsigned char* buf)
{
    MsgDev* dev = (MsgDev *)priv;
    struct pdc_bus_ctrlrequest *usb_req = (struct pdc_bus_ctrlrequest*)buf;

    printk("usb_req->bRequest = %d\n", usb_req->bRequest);
    return 0;
}

int devmsgd_config(void* priv, unsigned char config);

struct pdc_class_drv devmsgd_drv =
{
    name :        DRIVER_NAME,
    class_vendor: devmsgd_class_req,
    set_config:   devmsgd_config,
    set_intf:     NULL,
    priv_data:    &devmsgd_dev,
};

void unconfig_dev(MsgDev* dev)
{
    if (read_urb.status == PDC_URB_PENDING)
    {
        pdc_cancel_urb(&read_urb);
    }

    if (dev->data_in_pipe != PDC_INV_PIPE_HANDLE)
    {
        pdc_close_pipe(dev->data_in_pipe);
        dev->data_in_pipe = PDC_INV_PIPE_HANDLE;
    }

    if (dev->data_out_pipe != PDC_INV_PIPE_HANDLE)
    {
        pdc_close_pipe(dev->data_out_pipe);
        dev->data_out_pipe = PDC_INV_PIPE_HANDLE;
    }

    dev->bConfig = 0;
}

int devmsgd_config(void* priv, unsigned char config)
{
    MsgDev* dev = (MsgDev *)priv;
    struct pdc_pipe_desc bulk_pipe_desc[2];

    printk("devmsgd_config: config = %d\n", config);

    if (dev == NULL)
    {
        return 0;
    }

    if (config == 1)
    {
        if (dev->bConfig == 1)
        {
            unconfig_dev(dev);
        }

        dev->data_in_pipe = PDC_INV_PIPE_HANDLE;
        dev->data_out_pipe = PDC_INV_PIPE_HANDLE;

        bulk_pipe_desc[0].ep = 3;
        bulk_pipe_desc[0].ep_dir = PDC_EP_DIR_OUT;
        bulk_pipe_desc[0].context = (unsigned long)(&devmsgd_drv);
        bulk_pipe_desc[0].priv = (void*)(&devmsgd_drv);
        bulk_pipe_desc[0].notify = NULL;

        bulk_pipe_desc[1].ep = 4;
        bulk_pipe_desc[1].ep_dir = PDC_EP_DIR_IN;
        bulk_pipe_desc[1].context = (unsigned long)(&devmsgd_drv);
        bulk_pipe_desc[1].priv = (void*)(&devmsgd_drv);
        bulk_pipe_desc[1].notify = NULL;

        dev->data_in_pipe = pdc_open_pipe(&bulk_pipe_desc[0]);
        if (dev->data_in_pipe == PDC_INV_PIPE_HANDLE)
        {
            printk("pdc_open_pipe failed for dev->data_in_pipe!\n");
        }
        dev->data_out_pipe = pdc_open_pipe(&bulk_pipe_desc[1]);
        if (dev->data_out_pipe == PDC_INV_PIPE_HANDLE)
        {
            printk("pdc_open_pipe failed for dev->data_out_pipe!\n");
        }

        dev->bConfig = 1;
    }
    else if (config == 0)
    {
        unconfig_dev(dev);
    }
    return 0;
}

static int __init devmsgd_init(void)
{
    int result;

    result = devfs_register_chrdev(DEVMSGD_MAJOR, DRIVER_NAME, &fops);
    if (result == 0)
    {
        printk(KERN_INFO __FILE__ ": devfs_register_chrdev successfully.\n");
    }
    else
    {
        printk(KERN_INFO __FILE__ ": devfs_register_chrdev Failed.\n");
        return result;
    }

    result = pdc_register_class_drv(&devmsgd_drv);
    if (result == 0)
    {
        printk(KERN_INFO __FILE__ ": pdc_register_class_drv successfully.\n");
    }
    else
    {
        printk(KERN_INFO __FILE__ ": pdc_register_class_drv failed.\n");
    }

    return result;
}

static void __exit devmsgd_cleanup(void)
{
    printk("devmsgd_cleanup(void)\n");
    devfs_unregister_chrdev(DEVMSGD_MAJOR, DRIVER_NAME);
    pdc_deregister_class_drv(&devmsgd_drv);
}

module_init(devmsgd_init);
module_exit(devmsgd_cleanup);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

