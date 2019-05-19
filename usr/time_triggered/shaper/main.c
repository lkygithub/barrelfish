/**
 * \file
 * \brief Time-Triggered shaper application
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

#define SHAPER_DEBUG
#include "debug.h"

unsigned char buffer[TTMSG_PAYLOAD_SIZE];
uint16_t size;

int main(int argc, char **argv)
{
    uint64_t peroid_start_time;
    bool started = false;
    uint64_t start_time = 0xFFFFFFFFFFFFu;
    uint64_t overhead = 14000u; // printf overhead ~= 5ms
    uint64_t gap = 5; //us

    /* init */
    if (argc != 10) {
        PRINT_ERR("Param Number Error\n");
        return 0;
    }
    uint8_t my_task_id = atoi(argv[3]);
    uint8_t src_core_id = atoi(argv[4]);
    uint8_t src_task_id = atoi(argv[5]);
    uint8_t dst_core_id = atoi(argv[6]);
    uint8_t dst_task_id = atoi(argv[7]);
    uint64_t deadline = atoi(argv[8]);
    uint64_t peroid = atoi(argv[9]);

    disp_set_ttask_id(my_task_id);
    tt_msg_init(my_task_id);
    //printf("deadline is %d\n", deadline);
    /* wait for sync*/
    do {
        sys_get_tt_start_flag(&started);
    } while(!started);
    sys_get_tt_start_time(&start_time);
    while(debug_get_syscounter() < start_time)
        ;
    //printf("4\n");
my_start:
    /* get peroid start time */
    sys_get_current_period_start_ts(&peroid_start_time);
    /* Receive msg from producer */
    tt_msg_receive(src_core_id, src_task_id, buffer, &size);
    uint64_t now = debug_get_syscounter();
    PRINT_DEBUG("接收消息，         周期开始时刻：0x%llx，当前时刻（周期内）：%d微秒，消息：%02x\n",
        peroid_start_time, ticks_to_us(now-peroid_start_time), buffer[0]);
    /* reserve low 8bit and high 8bit */
    uint8_t low = buffer[0] & 0x0F;
    uint8_t high = (buffer[0] & 0xF0) >> 4;
    buffer[0] = (low << 4) | high;
    assert(debug_get_syscounter() < peroid_start_time + us_to_ticks(deadline));
    /* wait the code tail to sehd msg */
    while (debug_get_syscounter() < peroid_start_time + us_to_ticks(deadline-overhead))
        ;
    /* send msg to consumer */
    now = debug_get_syscounter();
    tt_msg_send(dst_core_id, dst_task_id, buffer, size);
    PRINT_DEBUG("发送消息，         周期开始时刻：0x%llx，当前时刻（周期内）：%d微秒，消息：%02x\n", 
            peroid_start_time, ticks_to_us(now-peroid_start_time), buffer[0]);
    /* wait for next peroid */
    while (debug_get_syscounter() < peroid_start_time + us_to_ticks(peroid+gap))
        ;
    goto my_start;
}