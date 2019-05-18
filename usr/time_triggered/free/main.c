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

#define MAXCOUNT 5000

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
    uint8_t task_id;
    uint64_t period;
    uint64_t overhead = 5000;
    
    if (argc != 2)
        PRINT_ERR("Param Number Error\n");
    period = atoi(argv[1]);

my_start:

    uint64_t sts = disp_get_tt_sts_period();

    while ((debug_get_syscounter - sts) < (peroid - overhead) * 100)
        ;
    PRINT_DEBUG("Free Task (ID:%d) Running\n", );

    goto my_start;
}
