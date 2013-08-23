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
 * File Name:    pdc_bus.h
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/

#ifndef __PDC_BUS_H__
#define    __PDC_BUS_H__

#define    PDC_CONFIG_PIPE_MPS        PDC_CTRL_MPS


#define    PDC_BUS_NUM_CONFIG        0x01
#define    PDC_BUS_NUM_INTF        0x02
#define    PDC_BUS_NUM_STRINGS        0x03


/*
 * Values of bus states
 */
#define    PDC_BUS_INIT        0x00
#define    PDC_BUS_ATTACHED    0x01
#define    PDC_BUS_DEFAULT        0x02
#define    PDC_BUS_ADDRESSED    0x03
#define    PDC_BUS_CONFIGURED    0x04
#define    PDC_BUS_SUSPENDED    0x05
#define    PDC_BUS_UNKNOWN        0xFF


typedef struct pdc_bus {

    __u8    state;
    __u8    prev_state;
    __u8    configuration;
    __u8    interface;
    __u8    alternate;

    struct pdc_config_dev    *pdc_config;

    pdc_pipe_handle_t         ctrl_pipe;

    struct pdc_pipe_desc     pipe_desc;

    struct pdc_urb            ctrl_write_urb;

    void                    *pdc_priv;
#ifdef CONFIG_USB_OTG
    pdc_otg_data_t *otg;
#endif /* CONFIG_USB_OTG */
} pdc_bus_t;

struct pdc_bus_ctrlrequest {
    __u8 bRequestType;
    __u8 bRequest;
    __u16 wValue;
    __u16 wIndex;
    __u16 wLength;
} __attribute__ ((packed));



#endif /* __PDC_BUS_H__ */
