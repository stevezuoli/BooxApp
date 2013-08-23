
/*************************************************************
 * Philips USB peripheral controller driver interface
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
 * File Name:    pdc_intf.h
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/

#ifndef __PDC_INTF_H__
#define    __PDC_INTF_H__

#include "linux/list.h"

#define        PDC_MAX_PIPES        0x10  /* Maximum number of logical pipes */

#define        PDC_CTRL_MPS        0x40   /* Control Pipe maximum packet size*/

#define        PDC_PIPE_CONTROL        PDC_EP_CONTROL
#define        PDC_PIPE_ISOCHRONOUS    PDC_EP_ISOCHRONOUS
#define        PDC_PIPE_BULK            PDC_EP_BULK
#define        PDC_PIPE_INTERRUPT        PDC_EP_INTERRUPT

#define        PDC_EP_CONTROL        0x00
#define        PDC_EP_ISOCHRONOUS    0x01
#define        PDC_EP_BULK            0x02
#define        PDC_EP_INTERRUPT    0x03


#define        PDC_EP_DIR_OUT        0x00
#define        PDC_EP_DIR_IN        0x80

#define        PDC_INV_PIPE_HANDLE    0xFF

typedef unsigned char pdc_pipe_handle_t;

typedef struct pdc_ep_desc {
    unsigned char    ep_num;
    unsigned char    ep_dir;
    unsigned char    attributes;
    unsigned short    max_pkt_size;
} pdc_ep_desc_t;

typedef struct pdc_config_dev {
    unsigned long        context;                /* Identification */
    void (*notif)(unsigned long context,
                unsigned long notification);
    unsigned char        num_eps;
    struct pdc_ep_desc    ep_desc[0];
} pdc_config_dev_t;

/*
 * Values of various operations and notifications
 */
#define        PDC_PIPE_STALL        0x0001
#define        PDC_PIPE_UNSTALL    0x0002
#define        PDC_GET_PIPE_STATUS    0x0004
#define        PDC_SETUP_COMMAND    0x0008

#define        PDC_ENABLE            0x0040
#define        PDC_DISABLE            0x0080
#define        PDC_RESET            0x0100
#define        PDC_RESUME            0x0200
#define        PDC_SUSPEND            0x0400
#define        PDC_SET_HNP            0x0800
#define        PDC_CONNECT            0x1000
#define        PDC_DISCONNECT        0x2000
#define        PDC_BUSTATUS        0x4000

typedef struct pdc_pipe_opr {
    pdc_pipe_handle_t    handle;
    unsigned long        context;
    unsigned short        opr;
    unsigned short        pipe_status;
} pdc_pipe_opr_t;

typedef struct pdc_pipe_desc {
    unsigned char    ep;
    unsigned char    ep_dir;
    unsigned long    context;
    void            *priv;
    int (*notify)(unsigned long notif_type,
                void *priv,
                unsigned char *data);        /* pipe notification function */
} pdc_pipe_desc;

typedef struct pdc_class_drv {
    const char *name;
    int (*class_vendor)(void *priv,
                        unsigned char *buff);    /* Class Vendor Request */
    int (*set_config)(void *priv,
                        unsigned char config);    /* Configure pipes */
    int (*set_intf)(void *priv,
                    unsigned char intf,
                    unsigned char alt_set);    /* Configure interface */
    void *priv_data;                                /* Driver private data */
    struct list_head driver_list;
} pdc_class_drv_t;


#define    PDC_URB_MAX_ISOPKTS            8

typedef struct pdc_iso_pkt_info {
    unsigned int offset;
    unsigned int length;
    unsigned int actual_length;
    unsigned int status;
} pdc_iso_pkt_info_t;

typedef struct pdc_urb {
    struct pdc_urb         *next;
    pdc_pipe_handle_t    pipe;
    unsigned char         pipe_type;
    unsigned char operation;
    int status;
    void *transfer_buffer;
    int transfer_buffer_length;
    int actual_length;
    void (*complete)(struct pdc_urb *urb);
    void *req_priv;
    int    number_of_packets;
    struct pdc_iso_pkt_info    iso_pkt[0];
} pdc_urb_t;

#define        PDC_OPR_READ            0x00
#define        PDC_OPR_WRITE            0x01

#define        PDC_URB_COMPLETE        0x01
#define        PDC_URB_PENDING            0x02
#define        PDC_EP_STALLED            0x03
#define        PDC_SHORT_PACKET        0x04
#define        PDC_SETUP_OVERRITE        0x05

#define pdc_fill_non_iso_urb(urb,handle,type,opr,buff,len,callback,priv) \
                urb->next = NULL;                                        \
                urb->pipe = handle;                                        \
                urb->pipe_type = type;                                    \
                urb->operation = opr;                                    \
                urb->transfer_buffer = buff;                            \
                urb->transfer_buffer_length = len;                        \
                urb->actual_length = 0;                                    \
                urb->complete = callback;                                \
                urb->req_priv = priv


/*-----------------------------------------------------------------------*
 *                  External function declerations                       *
 *-----------------------------------------------------------------------*/
extern    pdc_pipe_handle_t    pdc_open_pipe(struct pdc_pipe_desc  *pipe_desc);
extern    void pdc_close_pipe(pdc_pipe_handle_t    pipe_handle);
extern    int pdc_pipe_operation(struct pdc_pipe_opr    *pipe_opr);

extern    int pdc_submit_urb(struct pdc_urb *urb_req);
extern    int pdc_cancel_urb(struct pdc_urb *urb_req);

extern    unsigned long    pdc_get_frame_number(struct pdc_class_drv *cd);
extern int pdc_set_device_address(unsigned long context,unsigned short address);
extern    int pdc_configure_device(struct pdc_config_dev *config);
extern  void pdc_dev_control(unsigned long    opr);


extern    int pdc_register_class_drv( struct pdc_class_drv *drv);
extern    void pdc_deregister_class_drv( struct pdc_class_drv *drv);

#ifdef CONFIG_USB_OTG
typedef struct {
    void    *priv_data;
    void    (*otg_notif)(void *otg_priv,
                        unsigned long notif,
                        unsigned long data);
    void    *dc_priv_data;
} pdc_otg_data_t;

extern    int        pdc_otg_register(pdc_otg_data_t    *otg_data);
extern    void    pdc_otg_unregister(pdc_otg_data_t    *otg_data);
extern    void     pdc_otg_control(void *priv, unsigned long    opr);
#endif /* CONFIG_USB_OTG */

extern int pdc_read_fn(void);


#endif    /* __PDC_INTF_H__ */
