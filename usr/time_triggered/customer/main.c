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

unsigned char buffer[TTMSG_PAYLOAD_SIZE];
uint16_t size;

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

    /* Send msg */
    PRINT_DEBUG("buffer addr 0x%p, size addr 0x%p\n", buffer, &size);
    PRINT_DEBUG("Receiving message\n");

    /* src_core_id = 0, src_task_id = 0 */
    tt_msg_receive(0, 0, buffer, &size);
    PRINT_DEBUG("Receiving done\n");

    printf("CUSTOMER receive data:\n");
    for (int i = 0; i < size; i++) {
        printf("%u ", buffer[i]);
    }
    printf("\n");

    return 0;
}
