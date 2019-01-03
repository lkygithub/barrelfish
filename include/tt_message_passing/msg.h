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

errval_t tt_msg_init(uint16_t dst_core_id, uint16_t dst_task_id);

errval_t tt_msg_send(char *buffer, uint16_t size);

errval_t tt_msg_recieve(char *buffer, uint16_t *size_p);

#endif //LIBTT_MESSAGE_PASSING_H