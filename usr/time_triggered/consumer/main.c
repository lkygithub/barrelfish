/**
 * \file
 * \brief Time-Triggered consumer application
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

#define CONSUMER_DEBUG
#include "debug.h"

unsigned char buffer[TTMSG_PAYLOAD_SIZE];
uint16_t size;

int main(int argc, char **argv)
{
    uint64_t peroid_start_time;
    bool started = false;
    uint64_t start_time = 0xFFFFFFFFFFFFu;
    uint64_t gap = 5; //us

    /* init */
    if (argc != 8) {
        PRINT_ERR("Param Number Error\n");
        return 0;
    }
    uint8_t my_task_id = atoi(argv[3]);
    uint8_t src_core_id = atoi(argv[4]);
    uint8_t src_task_id = atoi(argv[5]);
    //uint64_t deadline = atoi(argv[6]);
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

    sys_get_current_period_start_ts(&peroid_start_time);
    /* Receive msg */
    tt_msg_receive(src_core_id, src_task_id, buffer, &size);
    uint64_t now = debug_get_syscounter();
    PRINT_DEBUG("Receive Msg,       Peroid Start Time: 0x%llx, Current Time(relative to peroid): %8d us, Msg: 0x%02x\n",
        peroid_start_time, ticks_to_us(now-peroid_start_time), buffer[0]);

    /* wait for next peroid */
    while (debug_get_syscounter() < peroid_start_time + us_to_ticks(peroid+gap))
        ;
    goto my_start;
}
