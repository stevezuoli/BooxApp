/*************************************************************
 * Philips USB peripheral controller driver
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
 * File Name:    usb_pdc.c
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SBANSAL        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/

/* Philips Peripheral Controller Driver
 *
 * The mapping of the endpoint is simple: it basically behaves as a device,
 * which can be opened, closed, and have reads and writes performed
 * on it. Due to the high bandwidth of the USB (12Mbit) we maintain local
 * buffers to ensure that we don't get starved of data during transmissions -
 * and have a receive buffer to allow the process dealing with USB to read in
 * bigger blocks than the packet size.
 *
 * The implementation is designed to be pretty transparent: this is for a
 * number of reasons, this is same protocol over both the USB and the serial
 * ports on the customers unit.Implementing the endpoint as a simple
 * 'open/close' device
 * as opposed to a more complex network-style interface also means that we can
 * do froody stuff like run PPP over a 12Mbit usb link (host permitting, of
 * course...). To this end, there is limited control over the way the USB
 * device works - endpoint 0 is handled totally under software control, and
 * only a limited number of events are passed though for the user-side task
 * to worry about (like connection/disconnection of the USB cable).
 *
 */

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

#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>

#include "pdc_intf.h"
#include "usb_pdc.h"



#define DRIVER_AUTHOR         "Philips Semiconductors"
#define    DRIVER_DESC            "ISP1362 Peripheral Controller Driver"
#define PDC_DRIVER_VERSION     "1.0"
#define    DRIVER_NAME            "usb-pdc"

int usb_pdc_connected;

/*--------------------------------------------------------------*
 *               external functions
 *--------------------------------------------------------------*/
extern    int    pdc_bus_init(struct pdc_dev    *pdc);
extern    void     pdc_bus_deinit(void);


/*--------------------------------------------------------------*
 *          Device Controller endpoint related functions
 *--------------------------------------------------------------*/
static void writeendpoint(int endpoint, unsigned char *buffer, int length);
static __inline__ int checkendpoint(int endpoint);
static void pdc_configure_eps(struct pdc_dev    *pdc);
static    int pdc_submit_control_urb(struct pdc_dev    *pdc,
struct pdc_urb *urb_req);

/*--------------------------------------------------------------*
 *         Interrupt Service Routine related functions
 *--------------------------------------------------------------*/
void pdc_isr(struct isp1362_dev *dev, void *isr_data);
static void tx_data(struct pdc_dev *pdc, __u8 ep, int kick);
static void rx_data(struct pdc_dev *pdc, __u8 ep);
static void rx_command(struct pdc_dev *pdc);
static void tx_command(struct pdc_dev *pdc);


/*--------------------------------------------------------------*
 *               initialization function declerations
 *--------------------------------------------------------------*/
static int __init pdc_module_init (void);
static void __exit pdc_module_cleanup (void);
static int     pdc_probe (struct isp1362_dev    *dev);
static void pdc_remove (struct isp1362_dev    *dev);
static void pdc_init(struct isp1362_dev    *dev);
static void pdc_connect(void);

/*--------------------------------------------------------------*
 *               local variable definitions
 *--------------------------------------------------------------*/

struct     pdc_dev             usb_devices[1];        /* local interface */
static     struct isp1362_dev    *isp1362_dc_dev;    /* HOSAL interface */
static     spinlock_t             pdc_rdwr_lock = SPIN_LOCK_UNLOCKED;
struct     pdc_pipe            pdc_eps[PDC_MAX_PIPES];
struct     pdc_ep_desc            pdc_ctrl_ep_desc[2];

/*--------------------------------------------------------------*
 *          Device Controller register access functions
 *--------------------------------------------------------------*/
static __inline__ void pdc_command(__u16    cmd)
{
    isp1362_command(cmd,isp1362_dc_dev);
    isp1362_udelay(1);
}

static __inline__ __u16    pdc_cread(void)
{
    return isp1362_read16(isp1362_dc_dev);
}

static __inline__ void pdc_cwrite(__u16    data)
{
    isp1362_write16(data,isp1362_dc_dev);
    return;
}

static __inline__ __u16    pdc_read16(__u16 reg)
{
    __u16    data;

    isp1362_reg_read16(isp1362_dc_dev, reg, data);

    return data;
}

static __inline__ void pdc_write16(__u16    reg, __u16    data)
{
    isp1362_reg_write16(isp1362_dc_dev, reg, data);
}

static __inline__ __u32    pdc_read32(__u16    reg)
{
    __u32    data;

    isp1362_reg_read32(isp1362_dc_dev, reg, data);

    return data;
}

static __inline__ void pdc_write32(__u16 reg, __u32    data)
{
    isp1362_reg_write32(isp1362_dc_dev, reg, data);

    return;
}



#define    pdc_set_mps_reg_value(mps,reg_mps,iso)                            \
    if(iso){                                                            \
        if(mps <= 16)    {reg_mps = EPCONFIG_ISO_FIFO16; mps =16;}        \
        else if(mps <= 32)    {reg_mps = EPCONFIG_ISO_FIFO32;    mps = 32;}    \
        else if(mps <= 48)    {reg_mps = EPCONFIG_ISO_FIFO48; mps = 48;}    \
        else if(mps <= 64)    {reg_mps = EPCONFIG_ISO_FIFO64;    mps = 64;}    \
        else if(mps <= 96)    {reg_mps = EPCONFIG_ISO_FIFO96;    mps = 96;}    \
        else if(mps <= 128)    {reg_mps = EPCONFIG_ISO_FIFO128; mps = 128;}\
        else if(mps <= 160)    {reg_mps = EPCONFIG_ISO_FIFO160; mps = 160;}\
        else if(mps <= 192)    {reg_mps = EPCONFIG_ISO_FIFO192; mps = 192;}\
        else if(mps <= 256)    {reg_mps = EPCONFIG_ISO_FIFO256; mps = 256;}\
        else if(mps <= 320)    {reg_mps = EPCONFIG_ISO_FIFO320; mps = 320;}\
        else if(mps <= 384)    {reg_mps = EPCONFIG_ISO_FIFO384; mps = 384;}\
        else if(mps <= 512)    {reg_mps = EPCONFIG_ISO_FIFO512; mps = 512;}\
        else if(mps <= 640)    {reg_mps = EPCONFIG_ISO_FIFO640; mps = 640;}\
        else if(mps <= 768)    {reg_mps = EPCONFIG_ISO_FIFO768; mps = 768;}\
        else if(mps <= 896)    {reg_mps = EPCONFIG_ISO_FIFO896; mps = 896;}\
        else {reg_mps = EPCONFIG_ISO_FIFO1023; mps = 1023;}                \
    } else {                                                            \
        if(mps <= 8)    {reg_mps = EPCONFIG_FIFO8; mps = 8;}            \
        else if(mps <= 16)    {reg_mps = EPCONFIG_FIFO16;    mps = 16;}        \
        else if(mps <= 32)    {reg_mps = EPCONFIG_FIFO32;    mps = 32;}        \
        else {reg_mps = EPCONFIG_FIFO64; mps = 64;}                        \
    }



/*--------------------------------------------------------------*
 *          Device Controller endpoint related functions
 *--------------------------------------------------------------*/
/* Check to see if endpoint is full */
static __inline__ int checkendpoint(int endpoint)
{
    /* Possible CMD_CHECKEPSTATUS? */
    return(pdc_read16(CMD_CHECKEPSTATUS+endpoint)&
           (STATUS_EPFULL0|STATUS_EPFULL1));
}


/* Read a packet out of a FIFO */
static int readendpoint(int endpoint, unsigned char *buffer, int length)
{
    /* Any data? */
    if (!checkendpoint(endpoint)) return(0);

    isp1362_buff_read(isp1362_dc_dev, (CMD_READEP+endpoint), buffer, length);

    /* Now we've read it, clear the buffer */
    pdc_command(CMD_CLEAREP+endpoint);

    /* Return bytes read */
    return length;
}

/* Write a packet into the fifo */
static void writeendpoint(int endpoint, unsigned char *buffer, int length)
{
    isp1362_buff_write(isp1362_dc_dev, (CMD_WRITEEP+endpoint), buffer, length);
    /* Validate the buffer so the chip will send it */
    pdc_command(CMD_VALIDATEEP+endpoint);
}

/*
 * Configure the Endpoints in the Hardware for those
 * pipes that are configured. For others Just diables
 * them. This function also adjusts the Endpoint max packet size to the
 * neareds available max packet size of the hardware.
 */
void pdc_configure_eps(struct pdc_dev    *pdc)
{
    __u16    ep_config = 0;
    __u8    handle;
    struct pdc_pipe    *pipe;
    __u8    mps;
    __u32    int_en;


    func_debug(("pdc_configure_eps(pdc=%p)\n",pdc))

    int_en = pdc_read32(CMD_READIRQENABLE);

    /* Configure all the Pipes (endpoint registers) in the device */
    for(handle=0;handle<PDC_MAX_PIPES;handle++) {

        pipe = pdc->ep_pipes+handle;        /* Get the pipe data structure */

        ep_config = 0;

        if(pipe->ep_state != PDC_PIPE_UNCONFIG) {
            /* pipe is configured */
            /* MPS, DBL BUFF, DIR, enable,disable */
            /* Use double buffering for non control pipes */

            ep_config |= EPCONFIG_FIFOEN;
            if(pipe->ep_desc->ep_dir) ep_config |= EPCONFIG_EPDIR;
            if(handle > EP0IN) ep_config |= EPCONFIG_DBLBUF;

            /*
             * Set the maximum pkt size and get the device configuration
             * maximum packet size
             */
            if(pipe->ep_desc->attributes != PDC_EP_ISOCHRONOUS) {
                pdc_set_mps_reg_value((pipe->ep_desc->max_pkt_size), mps, 0);
            } else {
                pdc_set_mps_reg_value((pipe->ep_desc->max_pkt_size), mps, 1);
            }

            ep_config |= mps;
            int_en |= (IE_EP0OUT << handle);
        }

        /* Configure the end point register */
        pdc_write16(CMD_WRITEEPCONFIG+handle, ep_config);
    }

    pdc_write32(CMD_WRITEIRQENABLE,int_en);

} /* End of pdc_configure_eps() */

void pdc_dev_control(unsigned long    opr)
{

    __u32        data;
    __u16        data_u16;
        struct pdc_dev         *pdc = usb_devices;

    switch(opr) {

        case PDC_ENABLE:

            data = pdc_read32(CMD_READIRQ);

            /* Bus is already in suspend state before enabling the DC
             * If the interrupt is already cleared, then we never get any
             * suspend interrupt later. So inform upper layers that the
             * bus is suspended */
            if((data & BUSTATUS) && pdc->pdc_bus && pdc->pdc_bus->notif)
                pdc->pdc_bus->notif(pdc->pdc_bus->context, PDC_BUSTATUS);


            /* Set device address & enable */
            pdc_write16(CMD_WRITEADDRESS,ADDRESS_DEVEN|0);


            /* Enable interrupts */
            pdc_write32(CMD_WRITEIRQENABLE,IE_EP0OUT|IE_EP0IN|IE_SUSP|IE_RST);

            /* Connect to the bus */
            pdc_write16(CMD_WRITEMODE,MODE_SOFTCT|MODE_INTENA);
            break;

        case PDC_DISABLE:

            /* Go off bus */
            pdc_write16(CMD_WRITEADDRESS,0);

            /* Turn off IRQs */
            pdc_write32(CMD_WRITEIRQENABLE,0);

            /* Global IRQ disable & turn off softconnect */
#if 0
            data_u16 = pdc_read16(CMD_READMODE);
            data_u16 &= (~MODE_INTENA);
            pdc_write16(CMD_WRITEMODE,data_u16);
#else
            pdc_write16(CMD_WRITEMODE,0);
#endif

            break;

        case PDC_CONNECT:

            /* Connect it to the USB bus */
            data_u16 = pdc_read16(CMD_READMODE);
            data_u16 |= MODE_SOFTCT;
            pdc_write16(CMD_WRITEMODE, data_u16);

            break;

        case PDC_DISCONNECT:

            /* Disconnect it from the USB bus */
            data_u16 = pdc_read16(CMD_READMODE);
            data_u16 &= (~MODE_SOFTCT);
            pdc_write16(CMD_WRITEMODE, data_u16);

            break;
    }

    return;
} /* End of pdc_dev_control()  */



/*
 * Deal with the received data on the USB pipe
 *  @pdc: pdc device data structure
 * @handle: USB pipe handle
 */
void rx_data(struct pdc_dev *pdc, pdc_pipe_handle_t    handle)
{
    struct pdc_pipe    *pipe = pdc->ep_pipes + handle;
    struct pdc_urb    *urb = pipe->urb;
    __u16            status;
    __u8            fifos = 0;
    __u32            bytes, rcvd_bytes;

    func_debug(("rx_data(pdc=%p,handle=%x)\n",pdc,handle))

    if(in_interrupt()) {
        status = pdc_read16(CMD_READEPSTATUS+handle);
        pipe->ep_status = status;
    } else {
        status = pipe->ep_status;
    }

    if((status & STATUS_EPFULL) == STATUS_EPFULL) {
            fifos = 2;
    } else if(status&STATUS_EPFULL) {
            fifos = 1;
    }

    if (!(status & STATUS_EPFULL) || (status & STATUS_EPSTAL)) {
        return ;
    }
    while((fifos--) && urb) {

        /* How many bytes do we need? */
        bytes = urb->transfer_buffer_length - urb->actual_length;

        if(bytes > pipe->ep_desc->max_pkt_size) bytes =
            pipe->ep_desc->max_pkt_size;

        rcvd_bytes = readendpoint(handle,
        &(((__u8*)urb->transfer_buffer)[urb->actual_length]),bytes);

        urb->actual_length += rcvd_bytes;

        /* Clear the full bits */
        if(pipe->ep_status & STATUS_EPFULL0) {
            pipe->ep_status &= ~(STATUS_EPFULL0);
        } else if(pipe->ep_status & STATUS_EPFULL1) {
            pipe->ep_status &= ~(STATUS_EPFULL1);
        }

        /* Did we receive a short packet?? */
        if(rcvd_bytes < bytes) {
            urb->status = PDC_SHORT_PACKET;
            detail_debug(("Short packet\n"))
        }


        /* Complete the urb if all the bytes are received from host or
         * terminated because of short packet
         */

        if(urb->transfer_buffer_length == urb->actual_length||
             urb->status == PDC_SHORT_PACKET) {
            pipe->urb = urb->next;
            if(urb->status == PDC_URB_PENDING) {
                urb->status = PDC_URB_COMPLETE;
            }
            if(urb->complete) urb->complete(urb);
            urb = pipe->urb;
        }
    } //while(fifo--)
} /* End of rx_data() */




/*
 * Deal with the transmit data on a USB pipe
 * @pdc : pdc device data structure
 * @pipe :  USB pipe
 */
void tx_data(struct pdc_dev *pdc, pdc_pipe_handle_t    handle, int kick)
{
    int txstat,tofill=1;
    struct pdc_pipe *pipe = pdc->ep_pipes + handle;
    struct pdc_urb    *ep_urb = pipe->urb;
    int             usb_txsize;

    func_debug(("tx_data(handle=%x, kick=%x)\n", handle, kick))

    if (kick == 2) {

        /* BUG: sending NULL data packet after stall is not valid .....*/
        /* Send NULL packet if DATA PID is not cleared by the HW
         * This case assumed that tx_data(2) is used only for endpoint EP2 */
        if((pdc_read16(CMD_READEPSTATUS+handle)) & 0x10) {
            writeendpoint(handle,0,0);
        }
        pipe->txrx_idle = 1;


        if(ep_urb) {

            ep_urb->actual_length = 0;
            if(ep_urb->next)    ep_urb->next->actual_length = 0;
            kick = 1;
        }
        else return;
    }

    if (!kick) {
        /* Get status/clear IRQ */
        txstat=pdc_read16(CMD_READEPSTATUS+handle);

        /* Successfully sent some stuff: bump counts & reset buffer */
        ep_urb = pipe->urb;

        /* If last packet was short, and there's nothing in the buffer
           to send, then just stop here with TX disabled */
        /* XXX: when the data transfer is over, go to silent mode */
        if(ep_urb && (ep_urb->actual_length == ep_urb->transfer_buffer_length)
             && !(txstat & (STATUS_EPFULL))) {
            /* URB Complete */
            pipe->urb = ep_urb->next;
            ep_urb->actual_length = ep_urb->transfer_buffer_length;
            ep_urb->status = PDC_URB_COMPLETE;
            if(ep_urb->complete)    ep_urb->complete(ep_urb);
            ep_urb = pipe->urb;
        }

        if(!ep_urb) {
            pipe->txrx_idle = 1;
            return;
        }
    }

    ep_urb = pipe->urb;

    /* Work out how many FIFOs we can fill */
    tofill = 2;
//    txstat = pdc_read16(CMD_READEPSTATUS+handle);
    txstat = pdc_read16(CMD_CHECKEPSTATUS+handle);
    if(txstat & STATUS_EPFULL0) tofill--;
    if(txstat & STATUS_EPFULL1) tofill--;

    /* XXX The problem with this for BOT protocol is in the IN data stage
     * after sending the data, host expects status (IN) data and we are
     * sending a NULL packet which is causing whole device to re-enumerate
     * In this case the data is multiple of MaxPacketSize
     * So only if tx_used is there then send the data */

    /* While we can send stuff... (this will fill both FIFOs) */
    while( (ep_urb) && (tofill>0) &&
            ep_urb->actual_length != ep_urb->transfer_buffer_length)
    {
        /* Fill local packet buffer from TX buffer: if there's nothing
           to send (there might be: we need to be able to send zero
           length packets to terminate a transfer of an exact multiple
           of the buffer size), then we'll be sending a 0 byte
           packet */

        usb_txsize = ep_urb->transfer_buffer_length - ep_urb->actual_length;

        if ((usb_txsize)> pipe->ep_desc->max_pkt_size)
             usb_txsize=pipe->ep_desc->max_pkt_size;

        writeendpoint(handle, (((__u8*)ep_urb->transfer_buffer)+
        ep_urb->actual_length), usb_txsize);
        ep_urb->actual_length += usb_txsize;

        /* Not idle anymore */
        pipe->txrx_idle=0;

        /* Filled another buffer */
        tofill--;

    }

} /* End of tx_data() */

/* Deal with incoming packet on the control endpoint */
void rx_command(struct pdc_dev *pdc)
{
    /* Get receiver status */
    unsigned char command[8];
    int eplast;
    unsigned long    bytes, rcvd_bytes;

    struct pdc_pipe *pipe;
    struct pdc_urb  *urb;

    /* Get last transaction status (clears IRQ) */
    eplast=pdc_read16(CMD_READEPSTATUS+EP0OUT);

    /* Is this a setup packet? */
    if(eplast&STATUS_SETUPT) {
        int i;

        /* Read the packet into our buffer */
        if ((i=readendpoint(EP0OUT,command,sizeof(command)))!=sizeof(command)) {
            pdc_command(CMD_STALLEP+EP0IN);
            pdc_command(CMD_STALLEP+EP0OUT);
            detail_debug(("pdc:Short USB control read (%d bytes)!\n",i))
            return;
        }

        /* Acknowledge out endpoints, then clear; the
           arrival of the setup packet disables the clear command
           which is issued in readendpoint() */
        pdc_command(CMD_ACKSETUP);
        pdc_command(CMD_CLEAREP+EP0OUT);

        /* Clear all the previous data transfers on the IN endpoint */
        pipe = pdc->ep_pipes + EP0OUT ;

        urb = pipe->urb;

        /* For each URB complete the URB */
        while(urb) {

            urb->status = PDC_SETUP_OVERRITE;
            pipe->urb = urb->next;
            if(urb->complete)    urb->complete(urb);
            urb = pipe->urb;
        }


        /* Call the notification function of Configuration driver */
        pipe = pdc->ep_pipes + EP0OUT;

        if((pipe->pipe_desc) && (pipe->pipe_desc->notify))    {
                pipe->pipe_desc->notify(PDC_SETUP_COMMAND,
                                        pipe->pipe_desc->priv,
                                        command);
        }

    } else {
        /* If not a setup packet, it must be an OUT packet */

        pipe = pdc->ep_pipes + EP0OUT ;

        urb = pipe->urb;

        if(urb) {
            /* How many bytes do we need? */
            bytes = urb->transfer_buffer_length - urb->actual_length;

            rcvd_bytes = readendpoint(EP0OUT,
            &(((__u8*)urb->transfer_buffer)[urb->actual_length]),bytes);

            urb->actual_length += rcvd_bytes;

            /* Did we receive a short packet?? */
            if(rcvd_bytes < bytes) urb->status = PDC_SHORT_PACKET;

     /* Complete the urb if all the bytes are received from host or terminated
      * because of short packet
      */
            if(urb->transfer_buffer_length == urb->actual_length||
                 urb->status == PDC_SHORT_PACKET) {
                pipe->urb = urb->next;
                if(urb->status == PDC_URB_PENDING) {
                    urb->status = PDC_URB_COMPLETE;
                }
                if(urb->complete) urb->complete(urb);
                urb = pipe->urb;
            }
        } else {
            /* Clear the buffer, Do we need to wait for the appl?? let us see */
            readendpoint(EP0OUT,NULL,0);
        }

    }
} /* End of rx_command */

/* TX command */
void tx_command(struct pdc_dev *pdc)
{
    struct pdc_pipe        *pipe = pdc->ep_pipes + EP0IN;
    int txstatus;

    struct pdc_urb        *pipe_urb;
    __u16                len;

    func_debug(("tx_command(void)\n"))

    /* Read last status & discard */
    txstatus=pdc_read16(CMD_READEPSTATUS+EP0IN);

    pipe_urb = pipe->urb;

    if(pipe_urb) {

        len = pipe_urb->transfer_buffer_length - pipe_urb->actual_length;

        if(len > pipe->ep_desc->max_pkt_size)
               len = pipe->ep_desc->max_pkt_size;

        writeendpoint(EP0IN,
        &(((__u8*)pipe_urb->transfer_buffer)[pipe_urb->actual_length]), len);

        pipe_urb->actual_length += len;

        if(pipe_urb->actual_length == pipe_urb->transfer_buffer_length) {
            pipe_urb->status = PDC_URB_COMPLETE;
            pipe->urb = pipe_urb->next;
            if(pipe_urb->complete) pipe_urb->complete(pipe_urb);
        }

    }

    if(!pipe->urb) pipe->txrx_idle = 1;

} /* End of tx_command */

/* Interrupt Service Routine */
void pdc_isr(struct isp1362_dev *dev, void *isr_data)
{

    struct pdc_dev             *pdc = (struct pdc_dev*)isr_data;
    unsigned int             evnt, non_ctrl_ep_event, ep_event, data;
    unsigned int check_bustatus = 0;
    struct pdc_config_dev    *bus = pdc->pdc_bus;
    pdc_pipe_handle_t        handle;
    struct pdc_pipe            *pipe;

    while(1) {

        if(check_bustatus == 1)
        {
            if(dev->int_reg & BUSTATUS)
            {
                /* Inform the configuration driver  */
                if(bus && bus->notif)    bus->notif(bus->context, PDC_SUSPEND);
                dev->req_suspend = 1;
            }
            else
            {
                printk("USB DC: Left SUSPEND state during ISR\n");
            }
            check_bustatus = 0;
        }

        /* Read IRQ status, masking with possible valid IRQs */
        evnt= dev->int_reg&
            (IE_EP0OUT|IE_EP0IN|IE_NON_CTRL_EP_MASK|IE_RST|IE_SUSP);
        /* No irq? */
        if (evnt==0) {
            return;
        }

        non_ctrl_ep_event = evnt & IE_NON_CTRL_EP_MASK;

        handle = EP1;

        while(non_ctrl_ep_event) {

            ep_event = IE_EP0OUT << handle;

            if( non_ctrl_ep_event & ep_event) {

                pipe = pdc->ep_pipes + handle;

                if(pipe->ep_desc && pipe->pipe_desc) {
                    if(pipe->ep_desc->ep_dir) {
                        /* In pipe */
                        tx_data(pdc, handle, 0);
                    } else {
                        /* Out pipe */
                        rx_data(pdc, handle);
                    }

                } else {
                }

                non_ctrl_ep_event &= ~ep_event;
            }

            handle++;
        }

        if (evnt&IE_EP0OUT) {
            rx_command(pdc);    /* Control Out, SETUP */
        }

        if (evnt&IE_EP0IN) {
            tx_command(pdc);    /* COntrol IN */
        }

        if (evnt&IE_RST) {
            /*
             * Inform the configuration driver
             * initialize the controller and connect to the bus
             */
            if(bus && bus->notif)    bus->notif(bus->context, PDC_RESET);
            /* notify the application */
            pdc_init(dev);
            pdc_connect();
        }

        if (evnt&IE_RESM)
        {
             /* Inform the configuration driver  */
            printk("USB DC: RESM IRQ\n");
            if(bus && bus->notif)    bus->notif(bus->context, PDC_RESUME);
        }

        if (evnt&IE_SUSP)
        {
            printk("USB DC: SUSP IRQ\n");
            check_bustatus = 1;
        }

        dev->int_reg =pdc_read32(CMD_READIRQ);
    }

} /* End of pdc_isr */





/*------------------------------------------------------------------------*
 *                  External Interface functions                          *
 *------------------------------------------------------------------------*/

/*
 * Get the current frame number on the USB bus
 * @cd  class driver identifier
 */
unsigned long   pdc_get_frame_number(struct pdc_class_drv *cd)
{
    unsigned long flags;
    unsigned long frame_number;


    func_debug(("pdc_get_frame_number(cd=%p)\n",cd))

    spin_lock_irqsave(&pdc_rdwr_lock, flags);

    frame_number = pdc_read16(CMD_READFRAME);
    frame_number &= 0x7FF;

    spin_unlock_irqrestore(&pdc_rdwr_lock, flags);

    return frame_number;
} /* End of pdc_get_frame_number() */


/*
 * Set the address of the device
 * @context is the device context
 * @address is the device address on the bus
 */
int pdc_set_device_address(unsigned long context,unsigned short address)
{
    func_debug(("pdc_set_device_address(context=%x,address=%x\n",context,
     address))

    pdc_write16(CMD_WRITEADDRESS, (ADDRESS_DEVEN|address));

    return 0;

} /* End of pdc_set_device_address() */

/*
 * Configure the device endpoints and Notification function
 */
int pdc_configure_device(struct pdc_config_dev *config)
{
    struct pdc_ep_desc    *ep_desc = config->ep_desc;
    struct pdc_pipe        *pipe;
    struct pdc_dev         *pdc = usb_devices;
    pdc_pipe_handle_t    handle;
    __u8                index;


    func_debug(("pdc_configure_device(config=%p)\n",config))


    /* Initialize all pipe varaiables except the control ones */
    for(handle=2; handle < PDC_MAX_PIPES; handle++) {
        pipe = pdc->ep_pipes + handle;
        pipe->ep_desc = NULL;
        pipe->pipe_desc = NULL;
        pipe->ep_state = PDC_PIPE_UNCONFIG;
    }

    /* Go through all the end points in the list */
    for(index = 0; index < config->num_eps; index++) {

        if(ep_desc->ep_num != 0 && ep_desc->ep_num < PDC_MAX_EPS) {

            /* Get the pipe handle for the Endpoint */
            handle = pdc_usb_to_epreg((ep_desc->ep_num),(ep_desc->ep_dir));

            pipe = pdc->ep_pipes + handle;    /* Get the pipe descriptor */

            pipe->ep_desc = ep_desc;

            pipe->ep_state = PDC_PIPE_CONFIG;
        }

        ep_desc++;
    }


    /* Copy the configuration driver information */
    pdc->pdc_bus = config;

    /* Configure the endpoints in the Hardware */
    pdc_configure_eps(pdc);

    return 0;
} /* End of pdc_configure_device() */


/*
 * Open a data transfer pipe
 * @pipe_desc:    pipe information
 */
pdc_pipe_handle_t pdc_open_pipe(struct pdc_pipe_desc *pipe_desc)
{
    struct pdc_pipe        *pipe;
    pdc_pipe_handle_t    handle = PDC_INV_PIPE_HANDLE;
    struct pdc_dev         *pdc=usb_devices;

    func_debug(("pdc_open_pipe(pipe_desc=%p)\n",pipe_desc))

    if(pipe_desc->ep < PDC_MAX_EPS)
    {
        handle = pdc_usb_to_epreg((pipe_desc->ep),(pipe_desc->ep_dir));
        pipe = pdc->ep_pipes + handle;

        /* If pipe is configured before, then open the pipe */
        if(pipe->ep_state == PDC_PIPE_CONFIG) {
            /* Only configured pipes can be opened */
            pipe->ep_state = PDC_PIPE_OPEN;
            pipe->pipe_desc = pipe_desc;
        } else {
            /* else return invalid handle */
            handle = PDC_INV_PIPE_HANDLE;
        }

        if(pipe_desc->ep == 0) {
       /*
        * This is a control end point to the user and control out pipe for us,
        * so open a control in pipe also
        */

            pipe = pdc->ep_pipes + handle + 1;

            pipe->ep_state = PDC_PIPE_OPEN;
            pipe->pipe_desc = pipe_desc;
        }
    }


    return handle;
} /* End of pdc_open_pipe() */

/*
 * Close an already opened pipe
 * @pipe_handle:    pipe handle
 */
void pdc_close_pipe(pdc_pipe_handle_t   pipe_handle)
{
    __u8    ep;
    struct pdc_pipe *pipe;
    struct pdc_dev         *pdc=usb_devices;

    func_debug(("pdc_close_pipe(pipe_handle=%x)\n",pipe_handle))

    if(pipe_handle < PDC_MAX_PIPES) {

        pipe = pdc->ep_pipes + pipe_handle;

        /* If the pipe is opened then cleanup the stuff */
        if(pipe->ep_state == PDC_PIPE_OPEN) {
            pipe->pipe_desc =  NULL;
            pipe->ep_state = PDC_PIPE_CONFIG;
        }

        /*
         * Get the endpoint and if it is control ep then close
         * the control IN pipe also
         */
           ep = pdc_pipe_to_epreg(pipe_handle);

        if(ep == 0) {
            pdc_close_pipe(pipe_handle + 1);
        }
    }
} /* End of pdc_close_pipe() */

int pdc_submit_control_urb(struct pdc_dev    *pdc, struct pdc_urb *urb_req)
{
    struct pdc_pipe    *pipe;
    struct pdc_urb    *urb;
    unsigned long    length;
    pdc_pipe_handle_t    handle = urb_req->pipe;

    func_debug(("pdc_submit_control_urb(urb=%p)\n",urb_req))


    if(urb_req->operation == PDC_OPR_WRITE) handle += 1;  /* Control IN pipe */

    pipe = pdc->ep_pipes + handle;

    urb = pipe->urb;

    /* Insert the URB in to the pipe URB queue (FIFO) */
    if(!urb) {
        pipe->urb  = urb_req;
    } else {
        while(urb) {
            if(urb->next != NULL)    urb = urb->next;
            else urb->next = urb_req;
        }
    }

    /* set the output parameters */
    urb_req->actual_length = 0;
    urb_req->next = NULL;

    if(urb_req->operation == PDC_OPR_WRITE) {

        if(pipe->urb == urb_req)    {
            length = urb_req->transfer_buffer_length;
            if(urb_req->transfer_buffer_length > pipe->ep_desc->max_pkt_size) {
                length = pipe->ep_desc->max_pkt_size;
            }
            writeendpoint(handle,urb_req->transfer_buffer,length);
            urb_req->actual_length += length;
        }

        if(urb_req->actual_length == urb_req->transfer_buffer_length) {
            urb_req->status = PDC_URB_COMPLETE;
            pipe->urb = urb_req->next;
            if(urb_req->complete) urb_req->complete(urb);
        }

    } else if(urb_req->operation == PDC_OPR_READ) {
        /* TODO */
    }

    return 0;
}

/*
 * Submit an urb for data transfer
 * @urb: urb request block
 */
int pdc_submit_urb(struct pdc_urb *urb_req)
{
    struct pdc_pipe    *pipe;
    struct pdc_urb    *urb;
    unsigned long flags;
    struct pdc_dev         *pdc=usb_devices;

    func_debug(("pdc_submit_urb(urb=%p)\n",urb_req))

    if(urb_req->pipe > PDC_MAX_PIPES)    return -EINVAL;

    if(urb_req->pipe < 2)    return pdc_submit_control_urb(pdc, urb_req);

    pipe = pdc->ep_pipes + urb_req->pipe;

    spin_lock_irqsave(&pdc_rdwr_lock, flags);

    urb = pipe->urb;

    /* Insert the URB in to the pipe URB queue (FIFO) */
    if(!urb) {
        pipe->urb  = urb_req;
    } else {
        while(urb) {
            if(urb->next != NULL)    urb = urb->next;
            else urb->next = urb_req;
        }
    }

    /* set the output parameters */
    urb_req->actual_length = 0;
    urb_req->next = NULL;

    if(urb_req->operation == PDC_OPR_READ) {

        /*
         * handle the read request, if there are no URB's,
         * call the rx_data function
         */
        if(pipe->urb == urb_req)    {
            rx_data(pdc, urb_req->pipe);
        }

    } else if(urb_req->operation == PDC_OPR_WRITE) {

        /*
         * Handle the write request.
         * If the Pipe is not stalled and the transmission is idle
         * start the transmission
         */
        pipe->ep_status = (pdc_read16(CMD_CHECKEPSTATUS+(urb_req->pipe))
        & STATUS_EPSTAL) ? PDC_PIPE_STALL : PDC_PIPE_UNSTALL ;

        if(pipe->txrx_idle && pipe->ep_status != PDC_PIPE_STALL) {
            tx_data(pdc, urb_req->pipe, 1);    /* Kick the transfer */
        }

    }

    spin_unlock_irqrestore(&pdc_rdwr_lock, flags);

    return 0;
} /* End of pdc_submit_urb */

/*
 * Cancel an urb for data transfer
 * @urb: urb request block
 */
int pdc_cancel_urb(struct pdc_urb *urb_req)
{
    struct pdc_pipe    *pipe;
    struct pdc_urb    *urb, *prev_urb;
    unsigned long flags;
    struct pdc_dev         *pdc=usb_devices;

    func_debug(("pdc_cancel_urb(%p)\n",urb_req))

    /* Check the pipe value */
    if(urb_req->pipe > PDC_MAX_PIPES)    return -EINVAL;

    pipe = pdc->ep_pipes + urb_req->pipe;

    if((urb_req->pipe == 0) && (urb_req->operation == PDC_OPR_WRITE)) {
        /* Special case of control IN pipe */
        pipe++;
    }

    spin_lock_irqsave(&pdc_rdwr_lock, flags);

    prev_urb = NULL;
    urb = pipe->urb;

    /*
     * search for the URB , The pipe could be either IN or OUT so there is
     * only one URB list
     */
    while((urb!=NULL) && (urb_req != urb)) {
        prev_urb = urb;
        urb = urb->next;
    }

    /*
     * found URB, then Un link it from the list
     */
    if(urb) {
        urb->status = PDC_URB_COMPLETE;
        if(prev_urb)    prev_urb->next = urb->next;
        else pipe->urb = urb->next;
    }

    spin_unlock_irqrestore(&pdc_rdwr_lock, flags);

    return 0;
} /* End of pdc_cancel_urb */

/*
 * Pipe operations
 * @pipe_opr:
 */
int pdc_pipe_operation(struct pdc_pipe_opr   *pipe_opr)
{
    struct pdc_pipe    *pipe;
    __u16    status;
    struct pdc_dev         *pdc=usb_devices;
    pdc_pipe_handle_t    handle = pipe_opr->handle;


    func_debug(("pdc_pipe_control(pipe_opr=%p)\n",pipe_opr))

    if(handle < PDC_MAX_PIPES) {

        pipe = pdc->ep_pipes + handle;

        switch(pipe_opr->opr) {

            case PDC_PIPE_STALL:

                pdc_command(CMD_STALLEP+handle);

                if(handle == EP0OUT) pdc_command(CMD_STALLEP+handle+1);

                break;

            case PDC_PIPE_UNSTALL:

                pdc_command(CMD_UNSTALLEP+handle);
                if(handle == EP0OUT) pdc_command(CMD_UNSTALLEP+handle+1);

                 if(handle > EP0IN) {
                    if(pipe->ep_desc->ep_dir & PDC_EP_DIR_IN)
                        tx_data(pdc, handle, 2);
                    else
                        rx_data(pdc, handle);
                }
                break;

            case PDC_GET_PIPE_STATUS:

                status = pdc_read16(CMD_READEPSTATUS+handle);
                pipe_opr->pipe_status =
                (status&STATUS_EPSTAL) ? PDC_PIPE_STALL :PDC_PIPE_UNSTALL;

                break;
        }

        return 0;
    }

    return -1;
} /* End of pdc_pipe_operation() */

/*--------------------------------------------------------------*
 *          Device Controller intialization functions
 *--------------------------------------------------------------*/
/* Initialisation of the Peripheral Controller */
void pdc_init(struct isp1362_dev    *dev)
{
    func_debug(("pdc_init(dev=%p)\n",dev))

    /* set the ISP1362 Device controller Hardware Configuration */
    isp1362_set_hw_config(dev);

    /* Set DMA mode (no dma) */
    pdc_write16(CMD_WRITEDMACONFIG,0);

    /*bledia Disable all interrupts */
    pdc_write32(CMD_WRITEIRQENABLE,0);

    /* Set default address */
    pdc_write16(CMD_WRITEADDRESS,0);

    return;
} /* End of pdc_init */




/* Actually connect to the bus */
void pdc_connect(void)
{
    /* Set device address & enable */
    pdc_write16(CMD_WRITEADDRESS,ADDRESS_DEVEN|0);

    /* Enable interrupts */
    pdc_write32(CMD_WRITEIRQENABLE,IE_EP0OUT|IE_EP0IN|IE_RST|IE_SUSP);

    /* Connect to the bus */
    pdc_write16(CMD_WRITEMODE,MODE_SOFTCT|MODE_INTENA);

    return;

} /* End of pdc_connect */

int pdc_read_fn(void)
{
    // Dirty HACK
    if(DC_SLEEPING())
    {
        return 0;
    }
    else
    {
        return pdc_read16(DC_FN_REG);
    }
}

/*---------------------------------------------------------------------*
 *                   ISP1362 HOSAL interface functions                 *
 *---------------------------------------------------------------------*/
struct isp1362_driver    pdc_driver = {
    name:        "usb-pdc",
    index:        ISP1362_DC,
    probe:        pdc_probe,
    remove:        pdc_remove,
};

/* Device initialisation */
int pdc_probe (struct isp1362_dev    *dev)
{
    struct pdc_dev *pdc=usb_devices;
    int result, i, data;
    struct pdc_pipe    *pipe;

    func_debug(("pdc_probe(dev=%p)\n",dev))

    /* Grab the IO resources */
    result = isp1362_check_io_region(dev);

    if(result < 0) {
        detail_debug(("%s IO resources are not free\n","isp1362-pdc"))
        return result;
    }

    isp1362_request_io_region(dev);

    pdc->dev = dev;
    isp1362_dc_dev = dev;


    for(i=0; i< PDC_MAX_PIPES;i++) {
        pipe = pdc->ep_pipes + i;

        pipe->urb = NULL;
        pipe->txrx_idle = 1;
        pipe->ep_state = PDC_PIPE_UNCONFIG;
        pipe->ep_desc = NULL;
    }



    /* Configure the default end points */
    /* Configure the control OUT pipe */
    pipe = pdc->ep_pipes;
    pipe->ep_desc = &pdc_ctrl_ep_desc[0];

    pipe->ep_desc->ep_num = 0;
    pipe->ep_desc->ep_dir = PDC_EP_DIR_OUT;
    pipe->ep_desc->attributes = PDC_EP_CONTROL;
    pipe->ep_desc->max_pkt_size = 64;
    pipe->ep_state = PDC_PIPE_CONFIG;

    /* configure the control IN pipe */
    pipe = pdc->ep_pipes + 1;
    pipe->ep_desc = &pdc_ctrl_ep_desc[1];

    pipe->ep_desc->ep_num = 0;
    pipe->ep_desc->ep_dir = PDC_EP_DIR_IN;
    pipe->ep_desc->attributes = PDC_EP_CONTROL;
    pipe->ep_desc->max_pkt_size = 64;
    pipe->ep_state = PDC_PIPE_CONFIG;

    pdc_bus_init(pdc);

    /* Do chip setup */
    pdc_init(dev);

    /* Claim USB IRQ */
    result=isp1362_request_irq(pdc_isr,dev,pdc);

    /* Got it ok? */
    if (result < 0) {
        detail_debug((KERN_ERR "usb-pdc:Can't get USB device IRQ %d.\n",
         dev->irq))
        return result;
    }

    detail_debug(("usb-pdc:device IRQ is initialised, Irq is  %04x\n",dev->irq))

    /* If there are pending IRQs, process them as we can only
       detect edges */

#ifndef CONFIG_USB_OTG
    /* All ready! */
    pdc_connect();
#endif /* CONFIG_USB_OTG */

    dev->driver_data = pdc;

    detail_debug(("pdc:Philips USB device driver in INIT.\n"))

    /* Put it to sleep (Zzzz...Zzzz...) */
    isp1362_hal_suspend_dc();

    return 0;

} /* End of pdc_probe */


void pdc_remove (struct isp1362_dev    *dev)
{
    struct pdc_dev *pdc=(struct pdc_dev*)dev->driver_data;

    func_debug(("pdc_remove(dev=%p)\n",dev))

    pdc_bus_deinit();

    /* Go off bus */
    pdc_write16(CMD_WRITEADDRESS,0);

    /* Turn off IRQs */
    pdc_write32(CMD_WRITEIRQENABLE,0);

    /* Global IRQ disable & turn off softconnect */
    pdc_write16(CMD_WRITEMODE,0);

    /* Free IRQ */
    isp1362_free_irq(dev,pdc);

    isp1362_release_io_region(dev);    /* release IO space */

    pdc->dev = NULL;
    isp1362_dc_dev = NULL;
    pdc->ep_pipes = pdc_eps;

} /* End of pdc_remove */

/*
 * This is module init function
 * This function registers the PDC driver to ISP1362 HOSAL driver
 * which in turn calls the probe function when the dev is found
 */
static int __init pdc_module_init (void)
{
    struct pdc_dev  *pdc = usb_devices;
    int    result;

    func_debug(("pdc_module_init(void)\n"))

    pdc->dev = NULL;
    pdc->ep_pipes = pdc_eps;
    isp1362_dc_dev = NULL;

    result = isp1362_register_driver(&pdc_driver);

    if(result == 0) {
        isp1362_printk(KERN_INFO __FILE__
        ": %s Initialization Success \n",pdc_driver.name);
    } else {
        isp1362_printk(KERN_INFO __FILE__
        ": %s Iinitialization Failed (error = %d)\n",pdc_driver.name,result);
    }

    return  result;
} /* End of pdc_module_init */

/*
 * This is module cleanup function
 * Unregister from isp1362 HOSAL layer which in turn calls
 * Close function
 */
static void __exit pdc_module_cleanup (void)
{

    func_debug(("pdc_module_cleanup (void)\n"))

    return isp1362_unregister_driver(&pdc_driver);

} /* End of pdc_module_cleanup */

module_init (pdc_module_init);
module_exit (pdc_module_cleanup);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR(DRIVER_AUTHOR);
