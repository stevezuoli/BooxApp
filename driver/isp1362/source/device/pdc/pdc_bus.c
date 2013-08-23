/*************************************************************
 * Philips USB device protocol and Configuration driver
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
 * File Name:    pdc_bus.c
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *     1.1        11/22/02    SYARRA        Clear feature on unknown wIndex
 *                                          is stalled CF_WINDEX_FIX
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

#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>

#include "pdc_intf.h"
#include "usb_pdc.h"
#include "pdc_protocol.h"
#include "pdc_bus.h"




/*------------------------------------------------------------------*
 *                   Local variables                                *
 *------------------------------------------------------------------*/
struct pdc_bus            pdc_bus_device;
struct pdc_bus            *bus_dev = &pdc_bus_device;
struct pdc_pipe_opr        ctrl_pipe_opr;
LIST_HEAD(pdc_class_drv_list);


/* Device descriptor */
unsigned char device_desc[] = {
  PDC_DEV_DESC_LEN,  /* length of this desc. */
  PDC_DEV_DESC_TYPE, /* DEVICE descriptor */
  0x10,0x01,         /* spec rev level (BCD) */
  0x00,              /* device class */
  0x00,              /* device subclass */
  0x00,              /* device protocol */
  PDC_CTRL_MPS,      /* max packet size */
  (isp1362_vendor_id & 0xFF),
  (isp1362_vendor_id >> 8),
  (isp1362_product_id & 0xFF),
  (isp1362_product_id >> 8),
  0x00,0x01,         /* 1362's revision ID */
  1,                 /* index of manuf. string */
  2,                 /* index of prod.  string */
  3,                 /* index of ser. # string */
  PDC_BUS_NUM_CONFIG /* number of configs. */
};

unsigned char conf_desc[] = {
  PDC_CONFIG_DESC_LEN,        /* length of this desc. */
  PDC_CONFIG_DESC_TYPE,        /* CONFIGURATION descriptor */
#ifdef CONFIG_USB_OTG
  0x3A,0x00,                 /* total length returned */
#else
  0x37,0x00,                 /* total length returned */
#endif /* CONFIG_USB_OTG  */
  PDC_BUS_NUM_INTF,         /* number of interfaces */
  0x01,                        /* number of this config */
  0x00,                      /* index of config. string */
  0xC0,                      /* attr.: self powered */
  0x01,                      /* we take no bus power */

  PDC_INTF_DESC_LEN,        /* length of this desc. */
  PDC_INTF_DESC_TYPE,        /* INTERFACE descriptor */
  0x00,                      /* interface number */
  0x00,                      /* alternate setting */
  0x02,                      /* # of (non 0) endpoints */
  0x08,                      /* interface class (Mass Storage)*/
  0x06,                      /* interface subclass (SCSI Transparent) */
  0x50,                      /* interface protocol (BOT Protocol) */
  0x00,                      /* index of intf. string */

  /* Pipe 0 */
  PDC_EP_DESC_LEN,           /* length of this desc. */
  PDC_EP_DESC_TYPE,          /* ENDPOINT descriptor */
  0x82,                      /* address (IN) */
  0x02,                      /* attributes  (BULK) */
  (PDC_CONFIG_PIPE_MPS&0xff),/* max packet size */
  (PDC_CONFIG_PIPE_MPS>>8),
  0,                         /* interval (ms) */

  /* Pipe 1 */
  PDC_EP_DESC_LEN,           /* length of this desc. */
  PDC_EP_DESC_TYPE,          /* ENDPOINT descriptor*/
  0x01,                      /* address (OUT) */
  0x02,                      /* attributes  (BULK) */
  (PDC_CONFIG_PIPE_MPS&0xff),/* max packet size */
  (PDC_CONFIG_PIPE_MPS>>8),
  0,                         /* interval (ms) */

  PDC_INTF_DESC_LEN,        /* length of this desc. */
  PDC_INTF_DESC_TYPE,        /* INTERFACE descriptor */
  0x01,                      /* interface number */
  0x00,                      /* alternate setting */
  0x02,                      /* # of (non 0) endpoints */
  0xEF,                      /* interface class */
  0x01,                      /* interface subclass */
  0x01,                      /* interface protocol */
  0x00,                      /* index of intf. string */

  /* Pipe 0 */
  PDC_EP_DESC_LEN,           /* length of this desc. */
  PDC_EP_DESC_TYPE,          /* ENDPOINT descriptor */
  0x84,                      /* address (IN) */
  0x02,                      /* attributes  (BULK) */
  (PDC_CONFIG_PIPE_MPS&0xff),/* max packet size */
  (PDC_CONFIG_PIPE_MPS>>8),
  0,                         /* interval (ms) */

  /* Pipe 1 */
  PDC_EP_DESC_LEN,           /* length of this desc. */
  PDC_EP_DESC_TYPE,          /* ENDPOINT descriptor*/
  0x03,                      /* address (OUT) */
  0x02,                      /* attributes  (BULK) */
  (PDC_CONFIG_PIPE_MPS&0xff),/* max packet size */
  (PDC_CONFIG_PIPE_MPS>>8),
  0                           /* interval (ms) */
#ifdef CONFIG_USB_OTG

  ,PDC_OTG_DESC_LEN          /* length of this desc. */
  ,PDC_OTG_DESC_TYPE         /* OTG descriptor */
  ,0x03                         /* D1: HNP Support D0:SRP Support */
#endif /* CONFIG_USB_OTG */
};

#define PDC_BUS_CONFIG_DESC_LEN sizeof(conf_desc[0])

/* Unicode descriptors for our device description */
unsigned char unicode_string[]= {
    0x04,0x03,
    0x09,0x04         /* We offer only one language: 0409, US English */
};

unsigned char mfg_string[]=  isp1362_mfg_string;

unsigned char product_string[]= isp1362_product_string;

unsigned char serial_string[]= {
    0x22, 0x03, '0', 0x00, '0',0x00, '1', 0x00, '6', 0x00,
    '7', 0x00, 'C',0x00, 'F', 0x00, 'F', 0x00,
    'F', 0x00, 'F',0x00, '0', 0x00, '0', 0x00,
    '0', 0x00, '0',0x00, '0', 0x00, '0', 0x00
};



/*-------------------------------------------------------------------*
 *                     Local Function Declerations                   *
 *-------------------------------------------------------------------*/
static    void pdc_bus_stall_control_pipe(void);
static    void pdc_bus_dev_notif(unsigned long    context, unsigned long notif);
static  int pdc_bus_ctrl_pipe_notificaion(unsigned long notif_type, void *priv,
unsigned char    *cmd);
static    void pdc_bus_get_descriptor(__u8 *command);
static    int    pdc_nofif_write_control_pipe(__u8    *buff, __u16    len);
static    void pdc_bus_usb_feature(__u8    *command);
static    void pdc_bus_get_status(__u8    *command);
static    void pdc_bus_set_configuration(unsigned char *command);
static    void pdc_bus_set_interface(unsigned char *command);

/*----------------------------------------------------------------*
 *           iRex Technologies B.V. Added Functions               *
 *----------------------------------------------------------------*/

/*
 * Generate a unique iSerialNumber for this USB device, by using the contents
 * of /proc/sysset/euid, which is the device EUID (MAC address) and uniquely
 * identifies an iLiad
 */
static void pdc_get_unique_serial(void)
{
    struct file  *filp;
    mm_segment_t oldfs;
    int          bytes_read;
    char         buffer[18];
    int          i;

    filp = filp_open("/proc/sysset/euid", 0, O_RDONLY);
    if (IS_ERR(filp)||(filp==NULL))
        return;

    if (filp->f_op->read==NULL)
        return;

    /* Now read file into buffer */
    filp->f_pos = 0;
    oldfs = get_fs();
    set_fs(KERNEL_DS);
    bytes_read = filp->f_op->read(filp, buffer, sizeof(buffer), &filp->f_pos);
    set_fs(oldfs);
    /* Close the file */
    fput(filp);

    /*
     * Do magic: rework serial_string into the actual serial.
     * I'm sure there is a better way to do this, but this is
     * Quick'n'Dirty and it works like a charm.
     */
    for (i = 0; i <= (bytes_read - 2); i++)
    {
        if ( (2*i+2) > sizeof(serial_string) )
            return;

        serial_string[2*i+2] = buffer[i];
    }
}

/*----------------------------------------------------------------*
 *                   External Interface Functions                 *
 *----------------------------------------------------------------*/

/*
 * Class Driver registration/Deregistration functions
 */
int pdc_register_class_drv( struct pdc_class_drv *drv)
{
    func_debug(("pdc_register_class_drv(%p)\n",drv))

    pdc_get_unique_serial();

    /*
     * Put it in the class drivers list
     */
    list_add(&drv->driver_list, &pdc_class_drv_list);

    isp1362_printk(KERN_INFO __FILE__ ": Registered Driver %s\n",drv->name);

    /* TODO if the device is active, call the probe function */


    return 0;    /* Success */
} /* End of pdc_register_class_drv() */

void pdc_deregister_class_drv( struct pdc_class_drv *drv)
{
    func_debug(("pdc_deregister_class_drv(%p)\n",drv))

    list_del(&drv->driver_list);

    if(drv && drv->set_config) {
        drv->set_config(drv->priv_data,0);
    }

    isp1362_printk(KERN_INFO __FILE__ ": De-registered Driver %s\n",drv->name);


    return;
} /* End of pdc_deregister_class_drv() */



/* Initialize all the end point data structures */
int    pdc_bus_init(struct pdc_dev    *pdc)
{
    /* Enable only control end points */
    __u16    size;
    struct pdc_ep_desc    *ep_desc;
    int        result;

    func_debug(("pdc_bus_init(void)\n"))

    bus_dev->ctrl_pipe = PDC_INV_PIPE_HANDLE;
    INIT_LIST_HEAD(&pdc_class_drv_list);

    bus_dev->configuration = 0;
    bus_dev->interface = 0;

    bus_dev->state = PDC_BUS_UNKNOWN;

    size = sizeof(struct pdc_config_dev) +
    (sizeof(struct pdc_ep_desc) * PDC_MAX_PIPES);
    bus_dev->pdc_config = (struct pdc_config_dev*) kmalloc(size, GFP_KERNEL);

    if(!(bus_dev->pdc_config)) {
        return -1;
    }

    bus_dev->pdc_config->num_eps = 3;    /* Control+Bulk In + Bulk OUT */

    ep_desc = bus_dev->pdc_config->ep_desc;

    ep_desc->ep_num = 0;                        /* Control End point*/
    ep_desc->ep_dir = PDC_EP_DIR_OUT;            /* In endpoint */
    ep_desc->attributes = PDC_EP_CONTROL;        /* Control type*/
    ep_desc->max_pkt_size = 64;                    /* Maximum packet size */

    ep_desc++;
    ep_desc->ep_num = 1;                        /* Bulk out end point */
    ep_desc->ep_dir = PDC_EP_DIR_OUT;            /* Out endpoint */
    ep_desc->attributes = PDC_EP_BULK;            /* Transfer type*/
    ep_desc->max_pkt_size = 64;                    /* Maximum packet size */

    ep_desc++;
    ep_desc->ep_num = 2;                        /* Bulk In end point */
    ep_desc->ep_dir = PDC_EP_DIR_IN;            /* In endpoint */
    ep_desc->attributes = PDC_EP_BULK;            /* Transfer type*/
    ep_desc->max_pkt_size = 64;                    /* Maximum packet size */

    ep_desc++;
    ep_desc->ep_num = 3;                        /* Bulk out end point */
    ep_desc->ep_dir = PDC_EP_DIR_OUT;            /* Out endpoint */
    ep_desc->attributes = PDC_EP_BULK;            /* Transfer type*/
    ep_desc->max_pkt_size = 64;                    /* Maximum packet size */

    ep_desc++;
    ep_desc->ep_num = 4;                        /* Bulk In end point */
    ep_desc->ep_dir = PDC_EP_DIR_IN;            /* In endpoint */
    ep_desc->attributes = PDC_EP_BULK;            /* Transfer type*/
    ep_desc->max_pkt_size = 64;                    /* Maximum packet size */

    bus_dev->pdc_config->num_eps = 1;    /* Control in and out */
    bus_dev->pdc_config->context = (unsigned long)bus_dev->pdc_config;
    bus_dev->pdc_config->notif = pdc_bus_dev_notif;

    result = pdc_configure_device(bus_dev->pdc_config);

    if(result==0) {
        ep_desc = bus_dev->pdc_config->ep_desc;

        bus_dev->pipe_desc.ep = ep_desc->ep_num;
        bus_dev->pipe_desc.ep_dir = ep_desc->ep_dir;
        bus_dev->pipe_desc.context = (unsigned long)ep_desc;
        bus_dev->pipe_desc.priv = (void*)bus_dev->pdc_config;
        bus_dev->pipe_desc.notify = pdc_bus_ctrl_pipe_notificaion;
        bus_dev->ctrl_pipe = pdc_open_pipe(&bus_dev->pipe_desc);

        if(bus_dev->ctrl_pipe == PDC_INV_PIPE_HANDLE) {
            result = -1;
        } else {
            result = 0;
            bus_dev->state = PDC_BUS_INIT;
            bus_dev->pdc_priv = pdc;
        }
    }

    return result;
} /* End of pdc_bus_init(void) */

void pdc_bus_deinit(void)
{
    func_debug(("pdc_bus_deinit(void)\n"))

    /* Close the control pipe */
    if(bus_dev->ctrl_pipe!= PDC_INV_PIPE_HANDLE){
        pdc_close_pipe(bus_dev->ctrl_pipe);
        bus_dev->ctrl_pipe= PDC_INV_PIPE_HANDLE;
    }

    /* Free the configuration data */
    if(bus_dev->pdc_config) kfree(bus_dev->pdc_config);
    bus_dev->pdc_config = NULL;
    bus_dev->pdc_priv = NULL;

    bus_dev->state = PDC_BUS_UNKNOWN;

    return;
} /* End of pdc_bus_deinit() */



void pdc_bus_dev_notif(unsigned long    context, unsigned long notif)
{
    struct list_head *tmp;

    func_debug(("pdc_bus_dev_notif(context=%x,notif=%x)\n",context, notif))


    switch(notif) {
        case PDC_RESET:


            if(bus_dev->configuration) {

                bus_dev->configuration = 0;
                tmp = pdc_class_drv_list.next;
                /* Find the class driver that supports this request */
                while (tmp != &pdc_class_drv_list) {
                    struct pdc_class_drv *cd = list_entry(tmp,
                    struct pdc_class_drv, driver_list);

                    tmp = tmp->next;
                    if(cd->set_config)    cd->set_config(cd->priv_data,
                        bus_dev->configuration);
                }
                bus_dev->pdc_config->num_eps = 1;    /* Control */
                pdc_configure_device(bus_dev->pdc_config);
            }

            bus_dev->configuration = 0;
            bus_dev->state = PDC_BUS_DEFAULT;
            break;

        case PDC_RESUME:
            bus_dev->state = bus_dev->prev_state;
            break;

        case PDC_BUSTATUS:
            break;
        case PDC_SUSPEND:
            bus_dev->prev_state = bus_dev->state;
            bus_dev->state = PDC_BUS_SUSPENDED;
            break;

    }

#ifdef CONFIG_USB_OTG
    /* all kind of device notifications are passed to the OTG controller
     * (reset, suspend, resume)
         */
    if( bus_dev->otg && bus_dev->otg->otg_notif) {
        bus_dev->otg->otg_notif(bus_dev->otg->priv_data, notif,0);
    }
#endif /* CONFIG_USB_OTG */

    return;
} /* End of pdc_bus_dev_notif */


/*--------------------------------------------------------------*
 *           USB Class/Vendor specific functions
 *--------------------------------------------------------------*/

/* Make a configuration active */
void pdc_bus_set_configuration(unsigned char *command)
{
    int result;
    struct list_head *tmp;

    func_debug(("set_configuration((command=%p)\n",command))

    if((bus_dev->state < PDC_BUS_ADDRESSED) ||
        (command[0]&PDC_REQTYPE_DIR_MASK)) {
        /* You are not supposed to receive this in these states */
        pdc_bus_stall_control_pipe();
        return;
    }


    if(command[2] > 1) {

        /* Invalid configuration */
        /* Stall control in and out pipes */
        pdc_bus_stall_control_pipe();

    } else {
        if((bus_dev->configuration) || (command[2] == 0)) {
            /* Unconfigure the class drivers */

            tmp = pdc_class_drv_list.next;
            /* Find the class driver that supports this request */
            while (tmp != &pdc_class_drv_list) {
                struct pdc_class_drv *cd = list_entry(tmp,struct pdc_class_drv,
                 driver_list);

                tmp = tmp->next;
                if(cd->set_config)    cd->set_config(cd->priv_data,0);
            }

            bus_dev->pdc_config->num_eps = 1;    /* Control */
            result = pdc_configure_device(bus_dev->pdc_config);
            bus_dev->state = PDC_BUS_ADDRESSED;
        }

        bus_dev->configuration = command[2];
        bus_dev->interface = bus_dev->alternate = 0;

        if(bus_dev->configuration) {
            /* Configure the class drivers */
            bus_dev->pdc_config->num_eps = 5;    /* Control + bulk */
            result = pdc_configure_device(bus_dev->pdc_config);
            bus_dev->state = PDC_BUS_CONFIGURED;

            tmp = pdc_class_drv_list.next;
            /* Find the class driver that supports this request */
            while (tmp != &pdc_class_drv_list) {
                struct pdc_class_drv *cd = list_entry(tmp,struct pdc_class_drv,
                 driver_list);

                tmp = tmp->next;
                if(cd->set_config)    cd->set_config(cd->priv_data,
                    bus_dev->configuration);
            }
        }

        pdc_nofif_write_control_pipe(NULL, 0);

    }

    return;

} /* End of pdc_bus_set_configuration() */

void pdc_bus_set_interface(unsigned char *command)
{
    struct list_head *tmp;

    if((bus_dev->state > PDC_BUS_DEFAULT) && (command[2] < PDC_BUS_NUM_INTF) &&
        !(command[0]&PDC_REQTYPE_DIR_MASK)) {
        bus_dev->interface = command[2];
        bus_dev->alternate = command[4];

        tmp = pdc_class_drv_list.next;
        /* Find the class driver that supports this request */
        while (tmp != &pdc_class_drv_list) {
            struct pdc_class_drv *cd = list_entry(tmp,struct pdc_class_drv,
             driver_list);

            if(cd->set_intf)    cd->set_intf(cd->priv_data,bus_dev->interface,
                                             bus_dev->alternate);
            tmp = tmp->next;
        }
        pdc_nofif_write_control_pipe(NULL, 0);
    } else {
        pdc_bus_stall_control_pipe();
    }

} /* End of pdc_bus_set_interface() */

void pdc_bus_get_status(__u8 *command)
{
    unsigned char reply[4];
    __u8    bmRequestType = command[0];
    pdc_pipe_handle_t        handle;
    __u8    reply_len = 2;

    if((bus_dev->state < PDC_BUS_ADDRESSED) ||
         !(command[0]&PDC_REQTYPE_DIR_MASK)) {
        /* You are not supposed to receive this in these states */
        pdc_bus_stall_control_pipe();
        return;
    }

    /* Find request target */
    switch (bmRequestType&0x03) {
        case RECIP_DEVICE:          /* DEVICE */
            reply[0] = PDC_GETSTATUS_SELF_POWERED;
            reply[1]=0;
            break;

        case RECIP_INTERFACE:       /* INTERFACE */

            reply[0]=0;
            reply[1]=0;
            break;

        case RECIP_ENDPOINT:
            /* reply[0] needs to be 1 if the endpoint
            referred to in command[3] is stalled,
            otherwise 0 */

            /* ep->pipe handle and context conversion */
            handle = pdc_usb_to_epreg((command[4]&0x0F),(command[4]&0x80));

            /* Check EP status */
            ctrl_pipe_opr.handle = handle;
            ctrl_pipe_opr.context = (unsigned long) bus_dev;
            ctrl_pipe_opr.opr = PDC_GET_PIPE_STATUS;
            pdc_pipe_operation(&ctrl_pipe_opr);

            reply[0] = (ctrl_pipe_opr.pipe_status & PDC_PIPE_STALL) ? 1 : 0;
            reply[1]=0;

            break;

        default:                        /* UNDEFINED */
            /* Stall endpoints 0 & 1 */
            reply_len = 0;

            return;
    }

    /* Write this packet */
    pdc_nofif_write_control_pipe(reply, reply_len);

    return;

} /* End of pdc_bus_get_status() */

void pdc_bus_usb_feature(__u8    *command)
{
    __u8    bmRequestType = command[0];
    __u8    inv_state = PDC_BUS_ADDRESSED;

    /*
     * For USB 2.0 Suppliment OTG 1.0 Specification,
     * Set feature will be accepted in default state also
     */

    if(command[1]==SET_FEATURE) inv_state = PDC_BUS_DEFAULT;

    if((bus_dev->state < inv_state) || (command[0]&PDC_REQTYPE_DIR_MASK)) {
        /* You are not supposed to receive this in these states */
        pdc_bus_stall_control_pipe();
        return;
    }

    switch (bmRequestType & 0x03) {

        case RECIP_DEVICE:

        if (command[2]==PDC_FEATURE_REMOTE_WAKEUP) {

            pdc_nofif_write_control_pipe(NULL, 0);
        }
#ifdef CONFIG_USB_OTG
        else if ((command[2]==PDC_FEATURE_B_HNP_ENABLE)||
                (command[2]==PDC_FEATURE_A_HNP_SUPPORT)||
                (command[2]==PDC_FEATURE_A_ALTHNP_SUPPORT))
        {
            pdc_nofif_write_control_pipe(NULL, 0);

            //inform the OTG module of this
            if( bus_dev->otg && bus_dev->otg->otg_notif) {
                bus_dev->otg->otg_notif(bus_dev->otg->priv_data,
                 PDC_SET_HNP,command[2]);
            }
        }
#endif /* CONFIG_USB_OTG */
        else {
            /* CF_WINDEX_FIX */
            /* Stall endpoints 0 & 1 */
            pdc_bus_stall_control_pipe();
        }

        break;

        /* INTERFACE */
        case RECIP_INTERFACE:
            pdc_nofif_write_control_pipe(NULL, 0);
            break;

        /* ENDPOINT */
        case RECIP_ENDPOINT:
        {
            /* Find endpoint */
            pdc_pipe_handle_t handle;
            int stall=(command[1]==SET_FEATURE);

            handle = pdc_usb_to_epreg((command[4]&0x0F),(command[4]&0x80));
            detail_debug(("pdc: endpoint stall(%d)\n",handle))

            /* Set/clear endpoint stall flag */
            ctrl_pipe_opr.handle = handle;
            ctrl_pipe_opr.context = (unsigned long) bus_dev;
            if (stall) {
                ctrl_pipe_opr.opr = PDC_PIPE_STALL;
            } else if(handle > 0x01) {
                /* For Control In & Out already unstalled by
                    HW */
                ctrl_pipe_opr.opr = PDC_PIPE_UNSTALL;
            }
            pdc_pipe_operation(&ctrl_pipe_opr);

            /* 0-byte ACK */
            pdc_nofif_write_control_pipe(NULL, 0);
        }
        break;

        /* UNDEFINED */
        default:
            /* Stall endpoints 0 & 1 */
            pdc_bus_stall_control_pipe();
            break;
    }

}


/* Deal with get_descriptor */
void pdc_bus_get_descriptor(__u8 *command)
{
    __u8    *desc_ptr = NULL;
    __u16    desc_len = 0, req_len;

    func_debug(("get_descriptor(command=%p)\n",command))

    if((bus_dev->state < PDC_BUS_ADDRESSED) &&
         !(command[0]&PDC_REQTYPE_DIR_MASK)) {
        /* You are not supposed to receive this in these states */
        pdc_bus_stall_control_pipe();
        return;
    }


    switch( command[3]) {

        case PDC_DEV_DESC_TYPE:
            desc_ptr = device_desc;
            desc_len = sizeof(device_desc);
            break;

        case PDC_CONFIG_DESC_TYPE:

            if(command[2] < PDC_BUS_NUM_CONFIG) {
                desc_ptr = conf_desc;
                desc_len = sizeof(conf_desc);
            }
            break;

        case PDC_STRING_DESC_TYPE:

            switch(command[2]) {

                case 0:
                    desc_ptr = unicode_string;
                    desc_len = sizeof(unicode_string);

                    break;

                case 1:
                    desc_ptr = mfg_string;
                    desc_len = sizeof(mfg_string);

                    break;

                case 2:
                    desc_ptr = product_string;
                    desc_len = sizeof(product_string);

                    break;
                case 3:
                    desc_ptr = serial_string;
                    desc_len = sizeof(serial_string);

                default:
                    break;
            }
            break;
#ifdef CONFIG_USB_OTG
        case PDC_OTG_DESC_TYPE:
            desc_ptr = &conf_desc[PDC_CONFIG_DESC_LEN +
            PDC_BUS_NUM_INTF*(PDC_INTF_DESC_LEN+2*PDC_EP_DESC_LEN)];
            desc_len = desc_ptr[0];

            break;
#endif /* CONFIG_USB_OTG */

        default:
            break;
    }

    if(desc_ptr) {
        /* Get max length that remote end wants */
        req_len =command[6]|(command[7]<<8);
        if (desc_len > req_len ) {
            desc_len = req_len ;
        }

        pdc_nofif_write_control_pipe(desc_ptr, desc_len);
    } else {
        pdc_bus_stall_control_pipe();
    }

    return;
} /* End of pdc_bus_get_descriptor() */



int    pdc_nofif_write_control_pipe(__u8    *buff, __u16    len)
{
    struct pdc_urb    *urb  = &bus_dev->ctrl_write_urb;

    if(!buff) len = 0;

    /* Fill the URB and submit it */
    pdc_fill_non_iso_urb(urb, bus_dev->ctrl_pipe, PDC_PIPE_CONTROL,
                        PDC_OPR_WRITE, buff, len, NULL,NULL);
    return pdc_submit_urb(urb);
} /* End of pdc_nofif_write_control_pipe() */

void pdc_bus_stall_control_pipe(void)
{

    /* Stall Control Out Pipe */
    ctrl_pipe_opr.handle = bus_dev->ctrl_pipe;
    ctrl_pipe_opr.context = (unsigned long) bus_dev;
    ctrl_pipe_opr.opr = PDC_PIPE_STALL;
    pdc_pipe_operation(&ctrl_pipe_opr);

} /* End of pdc_bus_stall_control_pipe() */


int pdc_bus_ctrl_pipe_notificaion(unsigned long notif_type,
 void *priv,unsigned char    *cmd)
{
    __u8    RequestType;
    __u8    bmRequest ;
    int        result = -1;
    __u8    reply[2];
    struct list_head *tmp;
    __u8    address;



    if(notif_type == PDC_SETUP_COMMAND) {
        /* SETUP Command from the control pipe */
        RequestType = cmd[0] & 0x60;

        switch(RequestType) {

            case STANDARD_REQUEST:

                /* Process USB Standrad request */
                   bmRequest = cmd[1];

                switch (bmRequest) {

                    case GET_DESCRIPTOR:

                        pdc_bus_get_descriptor(cmd);
                        break;

                    case CLEAR_FEATURE:
                    case SET_FEATURE:
                        pdc_bus_usb_feature(cmd);
                        break;

                    case SET_ADDRESS:

                        /*
                         * Set the address in the device controller
                         * and send the status back to the Host
                         */
                        if((bus_dev->state != PDC_BUS_DEFAULT &&
       bus_dev->state != PDC_BUS_ADDRESSED) || (cmd[0]&PDC_REQTYPE_DIR_MASK)) {
                            pdc_bus_stall_control_pipe();
                        } else {

                            address = (cmd[2]&0x7F);

           pdc_set_device_address((unsigned long)bus_dev->pdc_config,address);

                            pdc_nofif_write_control_pipe(NULL, 0);

                            if(address)    bus_dev->state = PDC_BUS_ADDRESSED;
                            else bus_dev->state = PDC_BUS_DEFAULT;
                        }

                        break;

                    case SET_CONFIGURATION:

                        pdc_bus_set_configuration(cmd);

                        break;

                    case SET_INTERFACE:

                        pdc_bus_set_interface(cmd);

                        break;

                    case GET_CONFIGURATION:

           /*
            * Send the current configuration, the status stage is taken care
            * by the device controller
            */
     if((bus_dev->state >= PDC_BUS_DEFAULT) && (cmd[0]&PDC_REQTYPE_DIR_MASK)) {
                            reply[0] = bus_dev->configuration;
                            pdc_nofif_write_control_pipe(reply, 1);
                        } else {
                            pdc_bus_stall_control_pipe();
                        }

                        break;

                    case GET_INTERFACE:

            /*
             * Send the current configuration, the status stage is taken care
             * by the device controller
             */
  if((bus_dev->state >= PDC_BUS_CONFIGURED) && (cmd[0]&PDC_REQTYPE_DIR_MASK)) {
                            reply[0] = bus_dev->interface;
                            pdc_nofif_write_control_pipe(reply, 1);
                        } else {
                     /* You are not supposed to receive this in these states */
                            pdc_bus_stall_control_pipe();
                        }

                        break;

                    case GET_STATUS:

                        pdc_bus_get_status(cmd);

                        break;

                    default:

                        /* SET_DESCRIPTOR, SYNCH_FRAME TODO */
                        /* Other requests are not known to us */
                        pdc_bus_stall_control_pipe();
                        break;
                }


                break;

            case CLASS_REQUEST:
            case VENDOR_REQUEST:

                /* Process Class Vendor request */

                /* Find the class driver that supports this request */
                tmp = pdc_class_drv_list.next;
                while (tmp != &pdc_class_drv_list) {
   struct pdc_class_drv *cd = list_entry(tmp,struct pdc_class_drv, driver_list);

                    tmp = tmp->next;
                    if(cd->class_vendor) {
                        /* Currently done for only one class driver */
                        result = cd->class_vendor(cd->priv_data, cmd);
                        break;
                    }
                }

                if(result == 0){
                    pdc_nofif_write_control_pipe(NULL, 0);
                }

                if(result < 0){
                    /* Stall control in and out pipes */
                    pdc_bus_stall_control_pipe();
                }

                break;


            default:

                /* All other requests need to be stalled */
                pdc_bus_stall_control_pipe();

                break;
        }

    }

    return 0;
} /* End of pdc_bus_ctrl_pipe_notifacaion() */


/*-----------------------------------------------------------------*
 *                     External OTG interface functions            *
 *-----------------------------------------------------------------*/

#ifdef CONFIG_USB_OTG
int        pdc_otg_register(pdc_otg_data_t    *otg_data)
{

    if( bus_dev && otg_data ) {
        otg_data->dc_priv_data = (void*)bus_dev;
        bus_dev->otg = otg_data;
        isp1362_printk(KERN_INFO __FILE__ ": Registered Driver usb-otg\n");
        return 0;
    }
    return -1;
}

void    pdc_otg_unregister(pdc_otg_data_t    *otg_data)
{

    if(bus_dev && otg_data) {
        otg_data->dc_priv_data = NULL;
        bus_dev->otg = NULL;
        isp1362_printk(KERN_INFO __FILE__ ": De-registered Driver usb-otg\n");
    }
}

void    pdc_otg_control(void *priv, unsigned long opr)
{
    struct list_head *tmp;

    pdc_dev_control(opr);

    switch(opr) {
        case PDC_ENABLE:
        bus_dev->state = PDC_BUS_ATTACHED;
        break;

        case PDC_DISABLE:
            bus_dev->state = PDC_BUS_INIT;

            if(bus_dev->configuration) {
                bus_dev->configuration = 0;
                /* Find the class driver that supports this request */
                tmp = pdc_class_drv_list.next;
                while (tmp != &pdc_class_drv_list) {
  struct pdc_class_drv *cd = list_entry(tmp,struct pdc_class_drv, driver_list);

                    /* Currently done for only one class driver */
  if(cd->set_config)    cd->set_config(cd->priv_data,bus_dev->configuration);
                    tmp = tmp->next;
                }
            }
            break;
    }

}

#endif /* CONFIG_USB_OTG */
