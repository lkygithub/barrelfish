/**
 * \file
 * \brief Time-Triggered free application
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

#define FREE_DEBUG
#include "debug.h"

int main(int argc, char **argv)
{
    
    uint64_t peroid_start_time;
    bool started = false;
    uint64_t start_time = 0xFFFFFFFFFFFFu;
    uint64_t overhead = 5000u; //printf overhead ~5ms
    uint64_t gap = 5; //us
    
    if (argc != 6)
        PRINT_ERR("Param Number Error\n");
    uint8_t my_task_id = atoi(argv[3]);
    uint64_t deadline = atoi(argv[4]);
    uint64_t peroid = atoi(argv[5]);
    uint8_t my_core_id = disp_get_core_id();
    disp_set_ttask_id(my_task_id);
    /* wait for sync*/
    do {
        sys_get_tt_start_flag(&started);
    } while(!started);
    sys_get_tt_start_time(&start_time);
    while(debug_get_syscounter() < start_time)
        ;

my_start:

    sys_get_current_period_start_ts(&peroid_start_time);
    uint64_t now = debug_get_syscounter();
    PRINT_DEBUG("Core %d FreeTask %d，周期开始时刻：0x%llx，当前时刻（周期内）：%d微秒\n",
        my_core_id, my_task_id, peroid_start_time, ticks_to_us(now-peroid_start_time));

    while (debug_get_syscounter() < peroid_start_time + us_to_ticks(deadline - overhead))
        ;
    now = debug_get_syscounter();
    PRINT_DEBUG("Core %d FreeTask %d，周期开始时刻：0x%llx，当前时刻（周期内）：%d微秒\n",
        my_core_id, my_task_id, peroid_start_time, ticks_to_us(now-peroid_start_time));
    /* wait for next peroid */
    while (debug_get_syscounter() < peroid_start_time + us_to_ticks(peroid + gap))
        ;

    goto my_start;
}
