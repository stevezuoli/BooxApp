#ifndef _DEVMSGD_H
#define _DEVMSGD_H

#include <linux/types.h>
#include <linux/wait.h>

#define DRIVER_AUTHOR "Jinlei Li, lijinlei1@hotmail.com"
#define DRIVER_DESC   "USB Message Driver"
#define DRIVER_NAME   "devmsgd"
#define DEVMSGD_MAJOR 18

typedef struct
{
    wait_queue_head_t read_wq;
    __u8              read_lock;
    __u32             read_bytes;

   wait_queue_head_t  write_wq;
   __u8               write_lock;
   __u32              write_bytes;
} devmsgd_intf_info;

typedef struct
{
    __u8 data_in_pipe;
    __u8 data_out_pipe;
    __u8 bConfig;
} MsgDev;

#endif

