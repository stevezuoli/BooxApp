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
 * File Name:    pdc_protocol.h
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SBANSAL        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/
#ifndef __PDC_PROTOCOL_H__
#define __PDC_PROTOCOL_H__

#define PDC_DEFAULT                 0x00
#define PDC_DESCRIPTOR_READ         0x01
#define PDC_ADDRESS_ASSIGNED        0x02
#define PDC_CONFIGURED              0x03

/* GLOBAL STATUS VALUES */
#define STD_COMMAND                 0x00
#define SETUP_COMMAND_PHASE         0x40
#define FUNCTION_ERROR              0x7F
                         function EP0 */
#define HUB_ERROR                   0xFF
                         HUB EP0 */

/*
 * bmRequestType Values
*/

#define STANDARD_REQUEST            0x00
#define CLASS_REQUEST                0x20
#define VENDOR_REQUEST                0x40

/*
 * bmRequestType Recipient
 */
#define    RECIP_DEVICE                0x00
#define    RECIP_INTERFACE                0x01
#define    RECIP_ENDPOINT                0x02
#define    RECIP_OTHER                    0x03

/*
 * bmRequestType Dir mask
 */
#define    PDC_REQTYPE_DIR_MASK        0x80

 /*
  * bRequest Values
  */
#define GET_STATUS                  0x00
#define CLEAR_FEATURE               0x01
#define SET_FEATURE                 0x03
#define SET_ADDRESS                 0x05
#define GET_DESCRIPTOR              0x06
#define SET_DESCRIPTOR              0x07
#define GET_CONFIGURATION           0x08
#define SET_CONFIGURATION           0x09
#define GET_INTERFACE               0x0A
#define SET_INTERFACE               0x0B
#define SYNCH_FRAME                 0x0C
#define REQ_DONE                    0xFF    /*private code: request done*/


/*
 * Set Feature values
 */
#define PDC_FEATURE_B_HNP_ENABLE    0x0003
#define PDC_FEATURE_A_HNP_SUPPORT   0x0004
#define PDC_FEATURE_A_ALTHNP_SUPPORT    0x0005


/*
 * Recipient Selectors/Masks
 */

#define PDC_RECIPIENT_MASK            0x1F
#define PDC_DEVICE_RECIPIENT        0x00
#define PDC_INTERFACE_RECIPIENT     0x01
#define PDC_ENDPOINT_RECIPIENT      0x02
#define PDC_OTHER_RECIPIENT         0x03

/*
 * Feature Selectors
 */

#define DEVICE_REMOTE_WAKEUP           0x01
#define ENDPOINT_STALL                 0x00

#define PDC_MAX_STRING_DESC_LEN        255

// values for the bits returned by the USB GET_STATUS command
#define PDC_GETSTATUS_SELF_POWERED                0x01
#define PDC_GETSTATUS_REMOTE_WAKEUP_ENABLED       0x02

#define PDC_DESC_MAKE_TYPE_AND_INDEX(d, i) ((USHORT)((USHORT)d<<8 | i))

/*
 * Descriptor Types
 */
#define    PDC_DEV_DESC_TYPE            0x01
#define    PDC_CONFIG_DESC_TYPE        0x02
#define    PDC_STRING_DESC_TYPE        0x03
#define    PDC_INTF_DESC_TYPE            0x04
#define    PDC_EP_DESC_TYPE            0x05
#define    PDC_OTG_DESC_TYPE            0x09

#define    PDC_DEV_DESC_LEN            0x12
#define    PDC_CONFIG_DESC_LEN            0x09
#define    PDC_INTF_DESC_LEN            0x09
#define    PDC_EP_DESC_LEN                0x07
#define    PDC_OTG_DESC_LEN            0x03

/*
 * Values for bmAttributes field of an
 * endpoint descriptor
 */

#define PDC_EP_TYPE_MASK            0x03

#define PDC_EP_TYPE_CONTROL         0x00
#define PDC_EP_TYPE_ISOCHRONOUS     0x01
#define PDC_EP_TYPE_BULK            0x02
#define PDC_EP_TYPE_INTERRUPT       0x03


/*
 * definitions for bits in the bmAttributes field of a
 * configuration descriptor.
 */
#define PDC_CONFIG_POWERED_MASK     0xc0
#define PDC_CONFIG_BUS_POWERED      0x80
#define PDC_CONFIG_SELF_POWERED     0x40
#define PDC_CONFIG_REMOTE_WAKEUP    0x20

/*
 * Endpoint direction bit, stored in address
 */

#define PDC_EP_DIR_MASK             0x80


/*
 * USB defined request codes
 * see chapter 9 of the USB 1.1 specifcation for
 * more information.
 */

/*
 * These are the correct values based on the USB 1.1
 * specification
 */

#define PDC_REQ_GET_STATUS          0x00
#define PDC_REQ_CLEAR_FEATURE       0x01
#define PDC_REQ_SET_FEATURE         0x03
#define PDC_REQ_SET_ADDRESS         0x05
#define PDC_REQ_GET_DESCRIPTOR      0x06
#define PDC_REQ_SET_DESCRIPTOR      0x07
#define PDC_REQ_GET_CONFIGURATION   0x08
#define PDC_REQ_SET_CONFIGURATION   0x09
#define PDC_REQ_GET_INTERFACE         0x0A
#define PDC_REQ_SET_INTERFACE        0x0B
#define PDC_REQ_SYNC_FRAME            0x0C


/*
 * defined USB device classes
 */


#define PDC_DC_RESERVED               0x00
#define PDC_DC_AUDIO                  0x01
#define PDC_DC_COMMUNICATIONS         0x02
#define PDC_DC_HUMAN_INTERFACE        0x03
#define PDC_DC_MONITOR                0x04
#define PDC_DC_PHYSICAL_INTERFACE     0x05
#define PDC_DC_POWER                  0x06
#define PDC_DC_PRINTER                0x07
#define PDC_DC_STORAGE                0x08
#define PDC_DC_HUB                    0x09
#define PDC_DC_VENDOR_SPECIFIC        0xFF

/*
 * USB defined Feature selectors
 */

#define PDC_FEATURE_ENDPOINT_STALL  0x0000
#define PDC_FEATURE_REMOTE_WAKEUP   0x0001
#define PDC_FEATURE_POWER_D0        0x0002
#define PDC_FEATURE_POWER_D1        0x0003
#define PDC_FEATURE_POWER_D2        0x0004
#define PDC_FEATURE_POWER_D3        0x0005

#define BUS_POWERED                 0x80
#define SELF_POWERED                0x40
#define REMOTE_WAKEUP               0x20

/*
 * USB power descriptor added to core specification
 */

#define PDC_SUPPORT_D0_COMMAND        0x01
#define PDC_SUPPORT_D1_COMMAND      0x02
#define PDC_SUPPORT_D2_COMMAND         0x04
#define PDC_SUPPORT_D3_COMMAND      0x08

#define PDC_SUPPORT_D1_WAKEUP       0x10
#define PDC_SUPPORT_D2_WAKEUP       0x20


#endif /* __PDC_PROTOCOL_H__ */

