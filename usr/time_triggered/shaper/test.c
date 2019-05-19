/**
 * \file
 * \brief Time-Triggered producer test application
 * \author mwiacx.
 */

/*
 * Copyright (c) 2018, BUAA LES.
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <barrelfish/barrelfish.h>
#include <tt_message_passing/tt_msg.h>

#define PRODUCER_DEBUG
#include "debug.h"

#define MAXCOUNT 5000

int main(int argc, char **argv)
{

    tt_msg_payload_t content;

    /* init */
    disp_set_ttask_id(0);
    tt_msg_init();
    /* Write msg content */
    for (int i = 0; i < TTMSG_PAYLOAD_SIZE; i++) {
        content.value[i] = TTMSG_PAYLOAD_SIZE - i;
    }

    systime_t time_0, time_1, sum, count;
    sum = count = 0;
    /* Send msg */
    //PRINT_DEBUG("Sending message\n");
    /* dst_core_id = 1, dst_task_id = 0 */
    time_0 = debug_get_syscounter();
    tt_msg_send(1, 0, content.value, TTMSG_PAYLOAD_SIZE);
    time_1 = debug_get_syscounter();
    PRINT_DEBUG("send delay is %f\n", (time_1-time_0) / 100);
    //PRINT_DEBUG("Sending done\n");

    return 0;
}