/**
 * \file
 * \brief Time-Triggered producer application
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

int main(int argc, char **argv)
{

    tt_msg_payload_t content;
    bool started = false;
    uint64_t start_time = 0xFFFFFFFFFFFFu;
    uint64_t peroid_start_time;
    uint64_t overhead = 5000u; // printf overhead ~= 5ms
    uint64_t gap = 1; //us
    /* init */
    if (argc != 8){
        PRINT_ERR("Param Number Error\n");
        return 0;
    }
    uint8_t my_task_id = atoi(argv[3]);
    uint8_t dst_core_id = atoi(argv[4]);
    uint8_t dst_task_id = atoi(argv[5]);
    uint64_t deadline = atoi(argv[6]);
    uint64_t peroid = atoi(argv[7]);
    
    disp_set_ttask_id(my_task_id);
    tt_msg_init(my_task_id);
    
    /* wait for sync*/
    do {
        sys_get_tt_start_flag(&started);
    } while(!started);
    sys_get_tt_start_time(&start_time);
    while(debug_get_syscounter() < start_time)
        ;
    //printf("4\n");
my_start:
    /* get start timestamp in period */
    sys_get_current_period_start_ts(&peroid_start_time);
    /* Write msg content */
    content.value[0] = (int) debug_get_syscounter();
    /* wait to send */
    while ((debug_get_syscounter() - peroid_start_time) < us_to_ticks(deadline - overhead))
        ;
    /* Send msg */
    tt_msg_send(dst_core_id, dst_task_id, content.value, 1);
    uint64_t now = debug_get_syscounter();
    PRINT_DEBUG("Sending message, this peroid start at 0x%llx, now is %dus,  msg is %02x\n", 
            peroid_start_time, (now-peroid_start_time)/100, content.value[0]);
    /* wait for next period */
    while((debug_get_syscounter() - peroid_start_time) < us_to_ticks(peroid+gap))
        ;
    goto my_start;

    return 0;
}
