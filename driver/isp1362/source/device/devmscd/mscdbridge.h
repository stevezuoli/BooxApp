/*************************************************************
 * Philips USB Mass Storage Bridge Interface
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
 * File Name:    mscdbridge.h
 *
 * History:
 *
 *    Version    Date        Author        Comments
 * -------------------------------------------------
 *     1.0        09/23/02    SYARRA        Initial Creation
 *
 * Note: use tab space 4
 *************************************************************/

#ifndef __MSCDBRIDGE_H__
#define    __MSCDBRIDGE_H__



/* Mass Storage Class Driver Request Block */
typedef struct mscd_req {
    struct mscd_req    *next;                 // next request in the queue
    unsigned char    req_type;                // request type
    unsigned char    status;                  // status of the request
    unsigned char    *data_buff;              // data buffer pointer
    unsigned long    req_data_len;            // requested data length
    unsigned long    res_data_len;            // response data length
    void (*complete)(struct mscd_req *req);   // pointer to completion routine
    void *priv_data;                          // sender private data
} mscd_req_t;

/* Mass storage class driver bridge request is similar to mass storage class
   driver req */
typedef mscd_req_t    mscdbridge_req_t;



#define        mscd_fill_req(req, type, buff, req_len, callback, priv)    \
                            (req)->next = NULL;                    \
                            (req)->req_type = type;                \
                            (req)->status = 0xFF;                \
                            (req)->data_buff = buff;            \
                            (req)->req_data_len = req_len;        \
                            (req)->res_data_len = 0x00;            \
                            (req)->complete = callback;            \
                            (req)->priv_data = priv


/* mscd_req req_type field values */
#define        MSCDBRIDGE_INIT         0x00      // mass storage brigde init
#define        MSCDBRIDGE_DEINIT       0x01      // mass storage brigde de-init
#define        MSCDBRIDGE_RESET        0x02      // Reset mass storage device
#define        MSCD_COMMAND            0x03      // receive ms command
#define        MSCD_COMMAND_RES        0x04      // send command response
#define        MSCD_READ               0x05      // Read from USB host
#define        MSCD_WRITE              0x06      // write to USB host


/* mscd_req status field values */
#define        MSCD_SUCCESS            0x00
#define        MSCD_FAILED             0x01
#define        MSCD_ERROR              0x02
#define        MSCD_PENDING            0x04


/*
 * functional Interface  mass storage bridge -> mass storage class driver
 *
 * In linux the following requests are processed by the mass storage class
   driver
 * MSCD_READ
 * MSCD_WRITE
 *
 * The response to the request is sent in the call back functions.
 */
extern    int devmscd_submit_req(mscd_req_t *req);
extern    int devmscd_cancel_req(mscd_req_t *req);

/*
 * functional Interface  mass storage class driver -> mass storage bridge
 * In linux implementation it is assumed that these functions return
 * immediately (because they are being executed in ISR context. Might be
 * bridge bridge should run under seperate thread.) If a call back function is
 * provided, the result of the request will be sent in the call back function
 * or else the return value is treated as the result of the request
 *
 *  The following requests should be processed by the bridge
 * MSBRIDGE_INIT
 * MSBRIDGE_DEINIT
 * MSBRIDGE_OPEN
 * MSBRIDGE_CLOSE
 * MSBRIDGE_RESET
 * MSCD_COMMAND
 */
extern    int mscdbridge_submit_req(mscdbridge_req_t *req);
extern    int mscdbridge_cancel_req(mscdbridge_req_t *req);

#endif    /* __MSCDBRIDGE_H__ */
