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

#define CUSTOMER_DEBUG
#include "debug.h"

#define MAXCOUNT 5000

unsigned char buffer[TTMSG_PAYLOAD_SIZE];
uint16_t size;

static inline uint64_t debug_get_syscounter(void);
static inline uint64_t debug_get_syscounter(void){
    uint64_t cntpct;
    __asm volatile(
        "mrs %[cntpct], cntpct_el0 \n\t"
        : [cntpct] "=r"(cntpct)
        );
    return cntpct;
}

int main(int argc, char **argv)
{

    /* init */
    disp_set_ttask_id(0);
    tt_msg_init();
    PRINT_DEBUG("Wait for a while\n");
    for(int i = 0; i < 400; i++) {
        printf(".");
        if ((i+1)%80 == 0)
            printf("\n");
    }

    /* Receive msg */
    uint64_t time_0, time_1, sum = 0;
    int count = 0;
    PRINT_DEBUG("buffer addr 0x%p, size addr 0x%p\n", buffer, &size);
    PRINT_DEBUG("Receiving message\n");

    /* src_core_id = 0, src_task_id = 0 */
    for (int i = 0; i < MAXCOUNT; i++) {
        time_0 = debug_get_syscounter();
        tt_msg_receive(0, 0, buffer, &size);
        time_1 = debug_get_syscounter();
        sum += time_1 - time_0;
        count++;
    }

    PRINT_DEBUG("Receive delay is %f\n", sum/(float)count/100);
    PRINT_DEBUG("Receiving done\n");

    printf("CUSTOMER receive data:\n");
    for (int i = 0; i < size; i++) {
        printf("%u ", buffer[i]);
    }
    printf("\n");

    return 0;
}
