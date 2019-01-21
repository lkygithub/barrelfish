/*
 * Copyright (c) 2019, BUAA LES.
 * All rights reserved.
 * 
 * Author: mwaicx.
 */

#include <stdio.h>
#include <string.h>

#include <tt_message_passing/tt_msg.h>
#include <barrelfish/barrelfish.h>
#include "ttmp_debug.h"

#define TTMP_DEBUG

typedef struct {
    uint8_t my_task_id;
    uint8_t my_core_id;
    tt_msg_t *tt_msg;
} tt_msg_info_t;

static tt_msg_info_t tt_msg_info;

/**
 * \brief init basic info and return payload buffer
 */

void tt_msg_init(void)
{
    //assert(sizeof(tt_msg_info) == 32);
    PRINT_DEBUG("tt_msg_head size is %d\n", sizeof(tt_msg_head_t));
    PRINT_DEBUG("tt_msg_payload size is %d\n", sizeof(tt_msg_payload_t));
    /* setup myself info by disp interface */
    /* interface impl...ed in domain.c */
    tt_msg_info.my_core_id = disp_get_core_id();
    tt_msg_info.my_task_id = disp_get_ttask_id();
    /* return tt_msg_payload buffer */
    tt_msg_info.tt_msg = (tt_msg_t *)disp_get_ttmsg_buffer();

    return;
}

/**
 * \brief send tt msg, buffer should be prepare before calling.
 */

errval_t tt_msg_send(uint8_t dst_core_id, uint8_t dst_task_id,
                        unsigned char *buffer, uint16_t buff_size)
{
    errval_t err = SYS_ERR_OK;

    /* Check params if it's invalid? */
    if (buffer == NULL || buff_size > MAX_PAYLOAD_SIZE) {
        PRINT_ERR("invalid params err in func %s, line %s\n", __func__, __line__);
        err = TTMP_ERR_INVALID_PARAMETER;
        goto out;
    }
    /* get disp tt-msg buffer */
    tt_msg_head_t *head = (tt_msg_info.tt_msg)->head;
    unsigned char *payload = (tt_msg_info.tt_msg)->payload;
    /* setup tt message */
    head->src = (tt_msg_info.my_core_id & 0xFF) << 8 
                | (tt_msg_info.my_task_id & 0xFF);
    head->dst = (dst_core_id & 0xFF) << 8 
                | (dst_task_id & 0xFF);
    head->valid = 1u;
    head->size = buff_size;
    head->id = (tt_msg_info.my_core_id & 0xFF) << 8 
                | (tt_msg_info.my_task_id & 0xFF);
    /* copy msg payload */
    memcpy(payload, buffer, buff_size);
    /* TODO: call send syscall */
    sys_ttmp_send();

out:

    return err;
}

/**
 * \brief receive a tt msg.
 */

errval_t tt_msg_receive(uint16_t src_core_id, uint16_t src_task_id, 
                            unsigned char *buffer, uint16_t *buff_size)
{
    errval_t err = SYS_ERR_OK;

    /* TODO: call receive syscall */
    sys_ttmp_receive();
    /* get tt message from disp buffer */
    /* get disp tt-msg buffer */
    tt_msg_head_t *head = (tt_msg_info.tt_msg)->head;
    unsigned char *payload = (tt_msg_info.tt_msg)->payload;
    /* Check tt message header */
    if (head->src != ((src_core_id & 0xFF) << 8 | (src_task_id & 0xFF))
        || head-> valid != 1) {
            PRINT_ERR("wrong header err in func %s, line %s\n", __func__, __line__);
            err = TTMP_ERR_WRONG_HEADER;
            goto out;
    }
    /* Setup size */
    *buff_size = head->size;
    /* Copy */
    memcpy(buffer, payload, head->size);

out:
    return err;
}