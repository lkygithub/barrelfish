/*
 * Copyright (c) 2019, BUAA LES.
 * All rights reserved.
 * 
 * Author: mwaicx.
 */

#include <stdio.h>
#include <string.h>

#include <tt_message_passing/msg.h>
#include <barrelfish/barrelfish.h>
#include "ttmp_debug.h"

# define TTMP_DEBUG

typedef struct {
    uint16_t my_task_id;
    uint16_t my_core_id;
    uint16_t dst_core_id;
    uint16_t dst_task_id;
} tt_msg_info_t;

static tt_msg_info_t tt_msg_info;

errval_t tt_msg_init(uint16_t dst_core_id, uint16_t dst_task_id)
{
    //assert(sizeof(tt_msg_info) == 32);
    PRINT_DEBUG("tt_msg_info size is %d\n", sizeof(tt_msg_info));
    /* setup dst info */
    tt_msg_info.dst_core_id = dst_core_id;
    tt_msg_info.dst_task_id = dst_task_id;
    /* setup myself info by disp interface */
    /* interface impl...ed in domain.c */
    tt_msg_info.my_core_id = disp_get_core_id();
    tt_msg_info.my_task_id = disp_get_ttask_id();

    return SYS_ERR_OK;
}

errval_t tt_msg_send(const unsigned char *buffer, uint16_t size)
{
    errval_t err = SYS_ERR_OK;

    /* Check params if it's invalid? */
    if (buffer == NULL || size > MAX_PAYLOAD_SIZE) {
        PRINT_ERR("invalid params err in func %s, line %s\n", __func__, __line__);
        err = TTMP_ERR_INVALID_PARAMETER;
        return err;
    }
    /* get disp tt-msg buffer */
    (tt_msg_head_t *) head = disp_get_ttmsg_buffer();
    (unsigned char *) payload = (unsigned char *) head + TTMSG_HEAD_SIZE;
    /* setup tt message */
    head->src = (tt_msg_info.my_core_id & 0xFFFF) << 16 
                | (tt_msg_info.my_task_id & 0xFFFF);
    head->dst = (tt_msg_info.dst_core_id & 0xFFFF) << 16 
                | (tt_msg_info.dst_task_id & 0xFFFF);
    head->valid = 1u;
    head->size = size;
    head->id = tt_msg_info.my_task_id;
    /* copy msg payload */
    memcpy(payload, buffer, size);
    /* TODO: call send syscall */

    return err;
}

errval_t tt_msg_receive(unsigned char *buffer, uint16_t *size_p,
                        uint16_t src_core_id, uint16_t src_task_id)
{
    errval_t err = SYS_ERR_OK;

    /* TODO: call receive syscall */
    
    /* get tt message from disp buffer */
    /* get disp tt-msg buffer */
    (tt_msg_head_t *) head = disp_get_ttmsg_buffer();
    (unsigned char *) payload = (unsigned char *) head + TTMSG_HEAD_SIZE;
    /* Check tt message header */
    if ((head->src != (src_core_id & 0xFFFF) << 16 | (src_task_id & 0xFFFF))
        || head-> valid != 1) {
            PRINT_ERR("wrong header err in func %s, line %s\n", __func__, __line__);
            err = TTMP_ERR_WRONG_HEADER;
            return err;
    }
    /* Copy buffer and setup size */
    *size_p = head->size;
    memcpy(buffer, payload, head->size);

    return err;
}