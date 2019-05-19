/*
 * Copyright (c) 2019, BUAA LES.
 * All rights reserved.
 * 
 * Author: mwaicx.
 */

#ifndef LIBTT_MESSAGE_PASSING_H
#define LIBTT_MESSAGE_PASSING_H

#include <barrelfish/caddr.h>
#include <barrelfish/types.h>
#include <errors/errno.h>

#define MAX_PAYLOAD_SIZE 24 // byte
#define TTMSG_HEAD_SIZE 8 // byte
#define TTMSG_PAYLOAD_SIZE 24 // byte

typedef struct {
    uint32_t src : 16;
    uint32_t dst : 16;
    uint32_t valid : 1;
    uint32_t size : 15;
    uint32_t id : 16;
} tt_msg_head_t;

typedef struct {
    unsigned char value[24];
} tt_msg_payload_t;

typedef struct {
    tt_msg_head_t head;
    tt_msg_payload_t payload;
} tt_msg_t;

void tt_msg_init(uint8_t my_task_id);

errval_t tt_msg_send(uint8_t dst_core_id, uint8_t dst_task_id,
                        unsigned char *buffer, uint16_t buff_size);

errval_t tt_msg_receive(uint8_t src_core_id, uint8_t src_task_id, 
                            unsigned char *buffer, uint16_t *buff_size);

#endif //LIBTT_MESSAGE_PASSING_H