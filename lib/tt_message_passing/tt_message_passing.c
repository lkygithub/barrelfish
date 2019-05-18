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
#define TTMP_DEBUG
#include "ttmp_debug.h"

typedef struct {
    uint8_t my_task_id;
    uint8_t my_core_id;
    tt_msg_t *tt_msg;
} tt_msg_info_t;

static tt_msg_info_t tt_msg_info;

/**
 * \brief init basic info and return payload buffer
 */

void tt_msg_init(uint8_t my_task_id)
{
    //assert(sizeof(tt_msg_info) == 32);
    //PRINT_DEBUG("tt_msg_head size is %d\n", sizeof(tt_msg_head_t));
    //PRINT_DEBUG("tt_msg_payload size is %d\n", sizeof(tt_msg_payload_t));
    /* setup myself info by disp interface */
    /* interface impl...ed in domain.c */
    tt_msg_info.my_core_id = disp_get_core_id();
    tt_msg_info.my_task_id = my_task_id;
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
    //bool was_enabled;
    //dispatcher_handle_t disp = disp_try_disable(&was_enabled);
    /* Check params if it's invalid? */
    if (buffer == NULL || buff_size > MAX_PAYLOAD_SIZE) {
        PRINT_ERR("Invalid params err in %s line %d\n", __FILE__, __LINE__);
        err = TTMP_ERR_INVALID_PARAMETER;
        goto out;
    }

    /* get disp tt-msg buffer */
    tt_msg_head_t *head = &((tt_msg_info.tt_msg)->head);
    unsigned char *payload = ((tt_msg_info.tt_msg)->payload).value;

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
    err = sys_ttmp_send();

    if (err_is_fail(err)) {
        PRINT_ERR("syscall sys_ttmp_send err %d in %s line %d\n", err, __FILE__, __LINE__);
    }

out:
    //if (!was_enabled)
    //    disp_enable(disp);
    return err;
}

/**
 * \brief receive a tt msg.
 */

errval_t tt_msg_receive(uint8_t src_core_id, uint8_t src_task_id,
                        unsigned char *buffer, uint16_t *buff_size)
{
    errval_t err = SYS_ERR_OK;
    //bool was_enabled;
    //dispatcher_handle_t disp = disp_try_disable(&was_enabled);
    //PRINT_DEBUG("Start receive syscall\n");
    err = sys_ttmp_receive();
    //PRINT_DEBUG("Receive syscall done\n");
    if (err_is_fail(err)) {
        PRINT_ERR("syscall sys_ttmp_receive err %d in %s line %d\n", err, __FILE__, __LINE__);
        goto out;
    }
    /* get tt message from disp buffer */
    /* get disp tt-msg buffer */
    tt_msg_head_t *head = &((tt_msg_info.tt_msg)->head);
    //PRINT_DEBUG("Head addr is 0x%p\n", head);
    unsigned char *payload = ((tt_msg_info.tt_msg)->payload).value;
    //PRINT_DEBUG("Payload addr is 0x%p\n", payload);
    /* Check tt message header */
#if 1
    if (head->src != ((src_core_id & 0xFF) << 8 | (src_task_id & 0xFF))
        || head-> valid != 1) {
            PRINT_ERR("Wrong header err in %s, line %d\n", __FILE__, __LINE__);
            err = TTMP_ERR_WRONG_HEADER;
            goto out;
    }
#endif
    /* Setup size */
    *buff_size = head->size;
    /* Copy */
    memcpy(buffer, payload, head->size);

out:
    //if(!was_enabled)
    //    disp_enable(disp);
    return err;
}
