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
 * File Name:    devmscd.c
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *     1.21    08/04/03    SYARRA        Stict checking of
 *                                     usb commands
 *
 * Note: use tab space 4
 *************************************************************/

#include <linux/config.h>
#define    MODULE
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
// #define CONFIG_DETAIL_DEBUG
#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>


#include "pdc_intf.h"
#include "devmscd.h"
#include "mscdbridge.h"
#include "pdc_bus.h"


#define        devmscd_change_state(dev, new_state)    dev->state = new_state


/*--------------------------------------------------------------*
 *               local variables declerations
 *--------------------------------------------------------------*/
struct devmscd_device    devmscd_dev;
struct pdc_urb            read_urb, write_urb, csw_urb;
struct pdc_urb            my_read_urb, my_write_urb;
mscdbridge_req_t        bridge_req;
mscd_req_t                *bridge_read_req, *bridge_write_req;
struct pdc_pipe_desc    bulk_pipe_desc[4];

#if 0
static    char    devmscd_state_name[DEVMSCD_DATA_OUT+1][20] = {
        "DEVMSCD_IDLE    ",
        "DEVMSCD_CBW     ",
        "DEVMSCD_CSW     ",
        "DEVMSCD_DATA_IN ",
        "DEVMSCD_DATA_OUT"
};
#endif



/*--------------------------------------------------------------*
 *               local function declerations
 *--------------------------------------------------------------*/
static    int devmscd_class_req(void *__dev, __u8    *req);
static    int devmscd_configure(void *__dev, unsigned char cfg);
static    void devmscd_reset(struct devmscd_device *dev);
static    void devmscd_read_urb_complete(struct pdc_urb *urb);
static    void devmscd_write_urb_complete(struct pdc_urb *urb);
static    void devmscd_set_command_res(mscdbridge_req_t *req);
static int devmscd_read_data(__u8 *buff, __u32 bytes);
static int devmscd_write_data(__u8 *buff, __u32 bytes);



/*--------------------------------------------------------------*
 *               external function definitions
 *--------------------------------------------------------------*/
extern int devmscd_fn_poll(void *dummy);
extern int fn_poll_running;

/*
 * This function currently supports read, write operations.
 * Stores the request and calls the corresponding read or write function
 * Currently queueing is not implemented
 */
int devmscd_submit_req(mscd_req_t    *req)
{
    int result = MSCD_SUCCESS;

    func_debug(("devmscd_submit_req(%p)\n",req))

    switch(req->req_type) {

        case MSCD_READ:
            /* Bridge wants to receive data from the USB Host */
            if(!bridge_read_req) {
                req->status = MSCD_PENDING;
                bridge_read_req = req;
                return devmscd_read_data(req->data_buff, req->req_data_len);
            }
            break;

        case MSCD_WRITE:
            /* Bridge wants to send data to the USB Host */
            if(!bridge_write_req) {
                req->status = MSCD_PENDING;
                bridge_write_req = req;
                return devmscd_write_data(req->data_buff, req->req_data_len);
            }
            break;

        default:

            result = MSCD_FAILED;
            break;
    }

    return result;
} /* End of devmscd_submit_req */

/*
 * This function currently supports read, write operations.
 * rears the request and calls the corresponding usb device cancel operation
 * Currently queueing is not implemented
 */
int devmscd_cancel_req(mscd_req_t    *req)
{
    int result = MSCD_SUCCESS;

    func_debug(("devmscd_cancel_req(%p)\n",req))

    switch(req->req_type) {

        case MSCD_READ:

            if(bridge_read_req) {
                /* There is a read request pending, so clear this */
                bridge_read_req = NULL;
                if(read_urb.status != PDC_URB_COMPLETE)
                    return pdc_cancel_urb(&read_urb);
                else return MSCD_SUCCESS;
            }
            break;

        case MSCD_WRITE:

            if(bridge_write_req) {
                /* There is a write request pending, so clear this */
                bridge_read_req = NULL;
                if(write_urb.status != PDC_URB_COMPLETE)
                    return pdc_cancel_urb(&write_urb);
                else return MSCD_SUCCESS;
            }
            break;

        default:

            /* Not supported */
            result = MSCD_FAILED;
            break;
    }

    return result;
} /* End of devmscd_cancel_req */

/*--------------------------------------------------------------*
 *               local function definitions
 *--------------------------------------------------------------*/
/*
 * Make the URB req and call the Peripheral controller driver
 * read function
 */
int devmscd_read_data(__u8 *buff, __u32 bytes)
{
    func_debug(("devmscd_read_data(%p,%x)\n",buff,bytes))

    /* Fill the read URB */
    read_urb.next = NULL;
    read_urb.pipe = devmscd_dev.data_in_pipe;
    read_urb.pipe_type = PDC_PIPE_BULK;
    read_urb.operation = PDC_OPR_READ;
    read_urb.status = PDC_URB_PENDING;
    read_urb.transfer_buffer = buff;
    read_urb.transfer_buffer_length = bytes;
    read_urb.actual_length = 0x00;
    read_urb.complete = devmscd_read_urb_complete;    /* call back function */

    return pdc_submit_urb(&read_urb);
} /* End of devmscd_read_data */

/*
 * Make the URB req and call the Peripheral controller driver
 * write function.
 */
int devmscd_write_data(__u8 *buff, __u32 bytes)
{
    struct pdc_urb    *urb;

    func_debug(("devmscd_write_data(%p,%x)\n",buff,bytes))

    if(bytes == DEVMSCD_CSW_LENGTH)    urb = &csw_urb;
    else urb = &write_urb;

    /* Fill the wrtite URB */
    urb->next = NULL;
    urb->pipe = devmscd_dev.data_out_pipe;
    urb->pipe_type = PDC_PIPE_BULK;
    urb->operation = PDC_OPR_WRITE;
    urb->status = PDC_URB_PENDING;
    urb->transfer_buffer = buff;
    urb->transfer_buffer_length = bytes;
    urb->actual_length = 0x00;
    urb->complete = devmscd_write_urb_complete;

    /* transfer residue will be updated in the call back function */

    return pdc_submit_urb(urb);

} /* End of devmscd_write_data */

/*
 * If the command response is not success, stall the endpoint.
 * Fill the CSW and send it to the host. Go back to IDLE state.
 */
void devmscd_set_command_res(mscdbridge_req_t *req)
{

    struct devmscd_device    *dev = &devmscd_dev;
    __u8    status = req->status;
    struct pdc_pipe_opr        bulk_pipe_opr;

    func_debug(("devmscd_set_command_res(%p)\n",req))

    if(dev->state != DEVMSCD_IDLE) {

        if((status != MSCD_SUCCESS) && (dev->cbw[12]&0x80)) {
                /* Stall Bulk In End Point */
                bulk_pipe_opr.handle = dev->data_out_pipe;
                bulk_pipe_opr.context = (unsigned long) dev;
                bulk_pipe_opr.opr = PDC_PIPE_STALL;
                pdc_pipe_operation(&bulk_pipe_opr);

        } else if(status == MSCD_SUCCESS && dev->tx_residue) {
            if(read_urb.status == PDC_URB_COMPLETE &&
               write_urb.status == PDC_URB_COMPLETE) {
                mdelay(3);
                bulk_pipe_opr.handle = dev->data_out_pipe;
                bulk_pipe_opr.context = (unsigned long) dev;
                bulk_pipe_opr.opr = PDC_PIPE_STALL;
                pdc_pipe_operation(&bulk_pipe_opr);
            } else {
                dev->status_queue = 1;
            }
        }


        /*
         * Copy the transfer residue to the Command status Word
         */
        dev->csw[8] = dev->tx_residue & 0xFF;
           dev->csw[9] = (dev->tx_residue >> 8) & 0xFF;
        dev->csw[10] = (dev->tx_residue >> 16) & 0xFF;
           dev->csw[11] = (dev->tx_residue >> 24) & 0xFF;
           dev->csw[12] = status;

        devmscd_change_state(dev, DEVMSCD_CSW);

        /* If no read or write operations are not in progress
         * send the status to the host, else wait for the read,
         * or write operation to finish
         */
        if((read_urb.status == PDC_URB_COMPLETE &&
            write_urb.status == PDC_URB_COMPLETE) &&
            (dev->status_queue == 0)) {
            devmscd_read_data(dev->cbw,DEVMSCD_CBW_LENGTH);
            devmscd_change_state(dev, DEVMSCD_IDLE);
            devmscd_write_data(dev->csw, DEVMSCD_CSW_LENGTH);
        } else {
            /* Wait for the  data stage to finish */
            dev->status_queue = 1;
        }
    } else {
        /*
         * Command response received in idle state , Might be a reset has done
         * or .... Ignore this
         */
        detail_debug(("Command response received in Invalid state, Ignored\n"))
    }

} /* End of devmscd_set_command_res */




/*
 * read URB complete function.
 * In case of IDLE state, this is a CBW. Other cases it is a data request
 * reurn
 */
void devmscd_read_urb_complete(struct pdc_urb *urb)
{
    struct devmscd_device    *dev = &devmscd_dev;
    mscd_req_t                *req;

    func_debug(("devmscd_read_urb_complete(%p)\n",urb))

    switch(dev->state) {
        case DEVMSCD_IDLE:

            /* This is a CBW packet */
            dev->tx_residue = (dev->cbw[8] | (dev->cbw[9] << 8));
               dev->tx_residue |= ((dev->cbw[10] << 16) | (dev->cbw[11] << 24));


            /* Copy the CBW tag to the CSW tag */
            dev->csw[4] = dev->cbw[4];
            dev->csw[5] = dev->cbw[5];
            dev->csw[6] = dev->cbw[6];
            dev->csw[7] = dev->cbw[7];

            devmscd_change_state(dev, DEVMSCD_CBW);

            /* send this command to the mass storage bridge */
            mscd_fill_req(    (&bridge_req), MSCD_COMMAND,
                            (&dev->cbw[DEVMSCD_CBW_WRAPPER]),
                            (DEVMSCD_CBW_LENGTH - DEVMSCD_CBW_WRAPPER),
                            devmscd_set_command_res, NULL);
            mscdbridge_submit_req(&bridge_req);

            if(dev->cbw[12] & DEVMSCD_CBWFLAG_IN) {
                /* Data to Host, so Data out from Device */
                devmscd_change_state(dev, DEVMSCD_DATA_OUT);
            } else {
                /* Data from Host, so Data in to Device */
                devmscd_change_state(dev, DEVMSCD_DATA_IN);
            }

            break;

        case DEVMSCD_DATA_IN:
               dev->tx_residue -= urb->actual_length;
            req = bridge_read_req;
            if(req) {
                /* Call the call back function */
                req->res_data_len = urb->actual_length;
                req->status = MSCD_SUCCESS;
                bridge_read_req = NULL;
                if(req->complete)    req->complete(req);
            }
            break;
    }
    urb->status = PDC_URB_COMPLETE;
} /* End of devmscd_read_urb_complete */

/*
 *  Call the call back function of the bridge request
 *  If the status stage is already sent by the bridge, finish the status
 *  stage
 */
void devmscd_write_urb_complete(struct pdc_urb *urb)
{
    struct devmscd_device    *dev = &devmscd_dev;
    mscd_req_t                *req;
    struct pdc_pipe_opr        bulk_pipe_opr;

    func_debug(("devmscd_write_urb_complete(%p)\n",urb))

    dev->tx_residue -= urb->actual_length;

    if(dev->status_queue) {
        if(dev->tx_residue) {
            /* Send a stall */
            bulk_pipe_opr.handle = dev->data_out_pipe;
            bulk_pipe_opr.context = (unsigned long) dev;
            bulk_pipe_opr.opr = PDC_PIPE_STALL;
            mdelay(3);
            pdc_pipe_operation(&bulk_pipe_opr);

            dev->csw[8] = dev->tx_residue & 0xFF;
               dev->csw[9] = (dev->tx_residue >> 8) & 0xFF;
            dev->csw[10] = (dev->tx_residue >> 16) & 0xFF;
               dev->csw[11] = (dev->tx_residue >> 24) & 0xFF;
        }

        /* Already status is received, so send the status */
        devmscd_read_data(dev->cbw,DEVMSCD_CBW_LENGTH);
        devmscd_write_data(dev->csw, DEVMSCD_CSW_LENGTH);
        devmscd_change_state(dev, DEVMSCD_IDLE);
        dev->status_queue = 0;
    }

    if(devmscd_dev.state != DEVMSCD_CSW) {
        /* Update the transfer residue */
//        devmscd_dev.tx_residue -= urb->actual_length;
        req = bridge_write_req;
        if(req) {
            /* call the bridge call back function */
            req->res_data_len = urb->actual_length;
            req->status = MSCD_SUCCESS;
            bridge_write_req = NULL;
            if(req->complete)    req->complete(req);
        }
    }
    urb->status = PDC_URB_COMPLETE;

} /* End of devmscd_write_urb_complete */

/*
 * Bulk Only Trnasport, Mass storage reset.
 * Go back to Idle state, no transfer residue.
 */
void devmscd_reset(struct devmscd_device *dev)
{

    func_debug(("devmscd_reset(%p)\n",dev))

    devmscd_change_state(dev, DEVMSCD_IDLE);
    dev->tx_residue = 0x00;
    dev->status_queue = 0;
    read_urb.status = PDC_URB_COMPLETE;
    write_urb.status = PDC_URB_COMPLETE;

    /* Initialize the CSW signature */
    dev->csw[0] = 0x55;
    dev->csw[1] = 0x53;
    dev->csw[2] = 0x42;
    dev->csw[3] = 0x53;
} /* End of devmscd_reset */

int devmscd_configure(void *__dev, unsigned char cfg)
{
    struct devmscd_device    *dev = (struct devmscd_device *)__dev;

    if(dev) {

        if(cfg == 1) {

            /* Un configure the device first */
            if(dev->config) devmscd_configure(__dev, 0);

            devmscd_reset(dev);
            dev->data_in_pipe = PDC_INV_PIPE_HANDLE;
            dev->data_out_pipe = PDC_INV_PIPE_HANDLE;
            dev->my_read_pipe = PDC_INV_PIPE_HANDLE;
            dev->my_write_pipe = PDC_INV_PIPE_HANDLE;

            /* Configure the device */
            dev->data_in_pipe = pdc_open_pipe(&bulk_pipe_desc[0]);

            if(dev->data_in_pipe != PDC_INV_PIPE_HANDLE) {
                dev->data_out_pipe = pdc_open_pipe(&bulk_pipe_desc[1]);
                if(dev->data_out_pipe != PDC_INV_PIPE_HANDLE) {
                    /* Start waiting for CBW data */
                    devmscd_read_data(dev->cbw,DEVMSCD_CBW_LENGTH);
                } else {
                    pdc_close_pipe(dev->data_in_pipe);
                    dev->data_in_pipe = PDC_INV_PIPE_HANDLE;
                }
            }

            dev->my_read_pipe = pdc_open_pipe(&bulk_pipe_desc[2]);
            if (dev->my_read_pipe == PDC_INV_PIPE_HANDLE)
            {
                printk("pdc_open_pipe failed for dev->my_read_pipe!\n");
            }

            dev->my_write_pipe = pdc_open_pipe(&bulk_pipe_desc[3]);
            if (dev->my_write_pipe == PDC_INV_PIPE_HANDLE)
            {
                printk("pdc_open_pipe failed for dev->my_write_pipe!\n");
            }

            dev->config = cfg;

        } else if(cfg == 0) {
                /* Cancel all data requests to the device */
                if(read_urb.status == PDC_URB_PENDING)
                    pdc_cancel_urb(&read_urb);
                if(my_read_urb.status == PDC_URB_PENDING)
                    pdc_cancel_urb(&my_read_urb);

                /* Un configure the device */
                if(dev->data_in_pipe != PDC_INV_PIPE_HANDLE) {
                    pdc_close_pipe(dev->data_in_pipe);
                    dev->data_in_pipe = PDC_INV_PIPE_HANDLE;
                }
                if(dev->data_out_pipe != PDC_INV_PIPE_HANDLE) {
                    pdc_close_pipe(dev->data_out_pipe);
                    dev->data_out_pipe = PDC_INV_PIPE_HANDLE;
                }
                if(dev->my_read_pipe != PDC_INV_PIPE_HANDLE) {
                    pdc_close_pipe(dev->my_read_pipe);
                    dev->my_read_pipe = PDC_INV_PIPE_HANDLE;
                }
                if(dev->my_write_pipe != PDC_INV_PIPE_HANDLE) {
                    pdc_close_pipe(dev->my_write_pipe);
                    dev->my_write_pipe = PDC_INV_PIPE_HANDLE;
                }

                devmscd_reset(dev);

                dev->config = cfg;
        }

    }

    return 0;
}

/*
 * The class request can have the following return values
 * == 0 success and no data to send/receive
 * >  0 success data stage is needed through dev urb
 * < 0  failure of the command, send stall
 *
 */
int devmscd_class_req(void *__dev, __u8    *req)
{
    int        result;
    struct devmscd_device    *dev = (struct devmscd_device *)__dev;
    struct pdc_bus_ctrlrequest *usb_req = (struct pdc_bus_ctrlrequest*)req;

    func_debug(("devmscd_class_req(%p)\n",req))

    switch(usb_req->bRequest) {

        case DEVMSCD_MS_RESET:

            detail_debug(("DEVMSCD_RESET\n"))

            if((usb_req->wValue != 0x00) ||
               (usb_req->wIndex != 0x00) ||
               (usb_req->wLength != 0x00))  {
                    result = -1;
                    break;
            }

            /* Inform the mass storage bridge about the reset */
            mscd_fill_req(&bridge_req, MSCDBRIDGE_RESET, NULL, 0x00,
                          NULL, NULL);
            mscdbridge_submit_req(&bridge_req);

            /* Reset internal variables */
            devmscd_reset(dev);

            result =  0;

            break;

        case DEVMSCD_GET_MAX_LUN:

            /*
             * This command is not supported by this class driver
             */
            result =  -1;
            break;

        default:
            result =  -1;
            break;
    }

    return result;
} /* devmscd_class_req */




struct pdc_class_drv devmscd_drv = {
    name:            DRIVER_NAME,
    class_vendor:    devmscd_class_req,
    set_config:        devmscd_configure,
    set_intf:        NULL,
    priv_data:        &devmscd_dev,
};

/*
 * module initialization function
 * Initialize the physical disk.
 * Register the class driver with the functional interface to the
 * peripheral controller driver and wait for the it to call connect
 * function
 */
static int __init devmscd_module_init (void)
{
    int    result;

    func_debug(("devmscd_module_init(void)\n"))

    bridge_read_req = NULL;
    bridge_write_req = NULL;
    bulk_pipe_desc[0].ep = 1;
    bulk_pipe_desc[0].ep_dir = PDC_EP_DIR_OUT;
    bulk_pipe_desc[0].context = (unsigned long)(&devmscd_drv);
    bulk_pipe_desc[0].priv = (void*)(&devmscd_drv);
    bulk_pipe_desc[0].notify = NULL;

    bulk_pipe_desc[1].ep = 2;
    bulk_pipe_desc[1].ep_dir = PDC_EP_DIR_IN;
    bulk_pipe_desc[1].context = (unsigned long)(&devmscd_drv);
    bulk_pipe_desc[1].priv = (void*)(&devmscd_drv);
    bulk_pipe_desc[1].notify = NULL;

    bulk_pipe_desc[2].ep = 3;
    bulk_pipe_desc[2].ep_dir = PDC_EP_DIR_OUT;
    bulk_pipe_desc[2].context = (unsigned long)(&devmscd_drv);
    bulk_pipe_desc[2].priv = (void*)(&devmscd_drv);
    bulk_pipe_desc[2].notify = NULL;

    bulk_pipe_desc[3].ep = 4;
    bulk_pipe_desc[3].ep_dir = PDC_EP_DIR_IN;
    bulk_pipe_desc[3].context = (unsigned long)(&devmscd_drv);
    bulk_pipe_desc[3].priv = (void*)(&devmscd_drv);
    bulk_pipe_desc[3].notify = NULL;

    /* Initialize mass storage bridge */
    mscd_fill_req(&bridge_req, MSCDBRIDGE_INIT, NULL, 0x00, NULL, NULL);
    result = mscdbridge_submit_req(&bridge_req);

    if(result < 0) return result;

    devmscd_dev.data_in_pipe = PDC_INV_PIPE_HANDLE;
    devmscd_dev.data_out_pipe = PDC_INV_PIPE_HANDLE;

    /* register to the device controller driver */
    result =  pdc_register_class_drv(&devmscd_drv);

    if(result == 0) {
        isp1362_printk(KERN_INFO __FILE__ ": %s Initialization Success \n",
        devmscd_drv.name);
    } else {
        isp1362_printk(KERN_INFO __FILE__
        ": %s Iinitialization Failed (error = %d)\n",devmscd_drv.name,result);
    }

    /* Start kernel thread that polls the framenumber register to verify an
       active USB connection */
    fn_poll_running = 1;
    kernel_thread(devmscd_fn_poll, NULL, CLONE_SIGHAND);

    return  result;

} /* End of devmscd_module_init */

/*
 * module close function
 * shutdown the physical disk
 * unregister the class driver from the peripheral controller driver.
 */
static void __exit devmscd_module_cleanup (void)
{

    func_debug(("devmscd_module_cleanup(void)\n"))

    /* Stop the kernel thread that polls the framenumber register */
    fn_poll_running = 0;

    /* de-register to the device controller driver */
    pdc_deregister_class_drv(&devmscd_drv);

    /* Notify the bridge that we are going off */
    mscd_fill_req(&bridge_req, MSCDBRIDGE_DEINIT, NULL, 0x00, NULL, NULL);

    mscdbridge_submit_req(&bridge_req);

    return;
} /* End of devmscd_module_cleanup */

module_init (devmscd_module_init);
module_exit (devmscd_module_cleanup);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR(DRIVER_AUTHOR);
