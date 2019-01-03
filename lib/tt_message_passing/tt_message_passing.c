/*
 * Copyright (c) 2019, BUAA LES.
 * All rights reserved.
 * 
 * Author: mwaicx.
 */

#include <stdio.h>

#include <tt_message_passing/msg.h>
#include <barrelfish/barrelfish.h>

errval_t tt_msg_init(uint16_t dst_core_id, uint16_t dst_task_id)
{
    errval_t err = SYS_ERR_OK;
    
    return err;
}

errval_t tt_msg_send(char *buffer, uint16_t size)
{
    errval_t err = SYS_ERR_OK;
    
    return err;
}

errval_t tt_msg_recieve(char *buffer, uint16_t *size_p)
{
    errval_t err = SYS_ERR_OK;
    
    return err;
}