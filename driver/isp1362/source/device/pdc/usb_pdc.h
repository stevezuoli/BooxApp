/*************************************************************
 * Philips ISP1181 device controller
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
 * File Name:    usb_pdc.h
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SBANSAL        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/
#ifndef __USB_PDC_H__
#define __USB_PDC_H__

#include "pdc_intf.h"

/* EPs */
#define EP0OUT                    0
#define EP0IN                    1
#define EP1                        2
#define EP2                        3
#define EP3                        4
#define EP4                        5
#define EP5                        6
#define EP6                        7
#define EP7                        8
#define EP8                        9
#define EP9                        10
#define EP10                     11
#define EP11                    12
#define EP12                    13
#define EP13                    14
#define EP14                    15

#define MAX_EP0_SIZE             0x40

/* Commands & bitsets */
#define CMD_WRITEEP                0x00
#define CMD_VALIDATEEP            0x60
#define CMD_CLEAREP                0x70
#define CMD_READEP                0x10

#define CMD_READEPSTATUS        0x50
#define CMD_CHECKEPSTATUS        0xd0
#define STATUS_CPUBUF            0x02
#define STATUS_SETUPT            0x04
#define STATUS_OVERWRITE        0x08
#define STATUS_EPFULL0            0x20
#define STATUS_EPFULL1            0x40
#define STATUS_EPSTAL            0x80
#define STATUS_EPFULL            (STATUS_EPFULL0    |STATUS_EPFULL1)

#define CMD_ACKSETUP            0xf4 /* to both IN and OUT control EPs */

#define CMD_READEPERROR            0xa0
#define ERROR_RTOK                0x01

#define ERROR_DATA01            0x40
#define ERROR_UNREAD            0x80

#define CMD_STALLEP                0x40
#define CMD_UNSTALLEP            0x80

#define CMD_WRITEEPCONFIG        0x20
#define CMD_READEPCONFIG        0x30
#define EPCONFIG_FIFO8            0x00
#define EPCONFIG_FIFO16            0x01
#define EPCONFIG_FIFO32            0x02
#define EPCONFIG_FIFO64            0x03
#define EPCONFIG_FFOISO            0x10
#define EPCONFIG_ISO_FIFO16        0x00
#define EPCONFIG_ISO_FIFO32     0x01
#define EPCONFIG_ISO_FIFO48        0x02
#define EPCONFIG_ISO_FIFO64        0x03
#define EPCONFIG_ISO_FIFO96        0x04
#define EPCONFIG_ISO_FIFO128    0x05
#define EPCONFIG_ISO_FIFO160    0x06
#define EPCONFIG_ISO_FIFO192    0x07
#define EPCONFIG_ISO_FIFO256    0x08
#define EPCONFIG_ISO_FIFO320    0x09
#define EPCONFIG_ISO_FIFO384    0x0A
#define EPCONFIG_ISO_FIFO512    0x0B
#define EPCONFIG_ISO_FIFO640    0x0C
#define EPCONFIG_ISO_FIFO768    0x0D
#define EPCONFIG_ISO_FIFO896    0x0E
#define EPCONFIG_ISO_FIFO1023    0x0F
#define EPCONFIG_DBLBUF            0x20
#define EPCONFIG_EPDIR            0x40
#define EPCONFIG_FIFOEN            0x80

#define CMD_WRITEADDRESS        0xb6
#define CMD_READADDRESS            0xb7
#define ADDRESS_DEVEN            0x80

#define CMD_WRITEMODE            0xb8
#define CMD_READMODE            0xb9
#define MODE_SOFTCT                0x01
#define MODE_DISGLBL            0x02
#define MODE_DBGMOD                0x04
#define MODE_INTENA                0x08
#define MODE_GOSUSP                0x20
#define MODE_SNDRSU                0x40
#define MODE_DMAWD                0x80

#define CMD_WRITECONFIG            0xba
#define CMD_READCONFIG            0xbb
#define CONFIG_INTPOL_LOW          0x0000
#define CONFIG_INTPOL_HIGH         0x0001
#define CONFIG_INTLVL_LEVEL        0x0000
#define CONFIG_INTLVL_PULSE        0x0002
#define CONFIG_PWROFF            0x0004
#define CONFIG_WKUPCS            0x0008
#define CONFIG_EOTPOL            0x0010
#define CONFIG_DAKPOL            0x0020
#define CONFIG_DRQPOL            0x0040
#define CONFIG_DAKOLY            0x0080
#define CONFIG_CLKDIV15            0x0f00
#define CONFIG_CLKRUN            0x1000
#define CONFIG_NOLAZY            0x2000
#define CONFIG_EXTPUL            0x4000

#define CMD_WRITEIRQENABLE        0xc2
#define CMD_READIRQENABLE        0xc3
#define CMD_READIRQ                0xc0
#define IE_RST                    0x00000001
#define IE_RESM                   0x00000002
#define IE_SUSP                    0x00000004
#define IE_EOT                     0x00000008
#define IE_SOF                     0x00000010
#define IE_SOFN                    0x00000020
#define BUSTATUS                0x00000080
#define IE_EP0OUT                0x00000100
#define IE_EP0IN                0x00000200
#define IE_EP1                    0x00000400
#define IE_EP2                    0x00000800
#define IE_EP3                    0x00001000
#define IE_EP4                    0x00002000 /* EP5-14 much the same */
#define IE_EP5                    0x00004000 /* EP5-14 much the same */
#define IE_EP6                    0x00008000 /* EP5-14 much the same */
#define IE_EP7                    0x00010000 /* EP5-14 much the same */
#define IE_EP8                    0x00020000 /* EP5-14 much the same */
#define IE_EP9                    0x00040000 /* EP5-14 much the same */
#define IE_EP10                    0x00080000 /* EP5-14 much the same */
#define IE_EP11                    0x00100000 /* EP5-14 much the same */
#define IE_EP12                    0x00200000 /* EP5-14 much the same */
#define IE_EP13                    0x00400000 /* EP5-14 much the same */
#define IE_EP14                    0x00800000 /* EP5-14 much the same */

#define    IE_NON_CTRL_EP_MASK  \
      (IE_EP1 | IE_EP2 | IE_EP3 | IE_EP4 | IE_EP5 | IE_EP6 | IE_EP7 | IE_EP8 | \
       IE_EP9 | IE_EP10 | IE_EP11 | IE_EP12 | IE_EP13 | IE_EP14)

#define CMD_WRITEDMACONFIG        0xf0
#define CMD_READDMACONFIG        0xf1
#define DMA_BURSTL1                0x0000
#define DMA_BURSTL4                0x0001
#define DMA_BURSTL8                0x0002
#define DMA_BURSTL16            0x0003
#define DMA_AUTOLD                0x0004
#define DMA_DMAEN                0x0008
#define DMA_SHORTP                0x4000
#define DMA_CNTREN                0x8000

#define CMD_WRITEDMACOUNTER        0xf2
#define CMD_READDMACOUNTER        0xf3

#define CMD_UNLOCKDEVICE        0xb0
#define UNLOCK_DATA             0xaa37

#define CMD_WRITESCRATCH        0xb2
#define CMD_READSCRATCH            0xb3

#define CMD_READFRAME            0xb4

#define CMD_READCHIPID            0xb5

#define CMD_RESETDEVICE            0xf6

#include "hal_intf.h"

/*-----------------------------------------------------------------*
 *       pipe data structure                                       *
 *-----------------------------------------------------------------*/
#define        PDC_MAX_EPS                    0x10

#define        pdc_usb_to_epreg(ep,dir)    (((ep==0) && (dir==0)) ? 0 : (ep+1))
#define        pdc_epreg_to_usb(epreg)        ((epreg<2) ? 0 : (epreg-1))
#define        pdc_pipe_to_epreg(pipe)        (pipe)
#define        pdc_epreg_to_pipe(epreg)    (epreg)


#define        PDC_PIPE_UNCONFIG            0x00
#define        PDC_PIPE_CONFIG                0x01
#define        PDC_PIPE_OPEN                0x02

typedef struct pdc_pipe {
    __u8                    pipe;            /* pipe number 0-15 */
    __u8                    ep_state;        /* State of the endpoint */
    __u8                    txrx_idle;        /* tx, rx idle flag */
    __u16                    ep_status;

    struct pdc_urb            *urb;            /* USB request block list */
    struct pdc_ep_desc        *ep_desc;        /* End point descriptor */
    struct pdc_pipe_desc    *pipe_desc;        /* Pipe descriptor */
} pdc_pipe_t;


#define MAXTXPACKET                     0x40
#define MAXRXPACKET                     0x40

/* USB device: one in, one out, and various stats */
typedef struct pdc_dev
{
    /* Transmit buffer for this endpoint */
    struct isp1362_dev    *dev;


    /* Configuration driver information */
    struct pdc_config_dev *pdc_bus;

    /* pdc pipe data structure */
    struct pdc_pipe     *ep_pipes;

}pdc_dev_t;


#endif /* __USB_PDC_H__ */
