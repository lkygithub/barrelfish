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
#include <stdlib.h>

#include <barrelfish/barrelfish.h>

int main(int argc, char **argv)
{
    if (argc != 2){
        printf("Error Param Number in tt_starter\n");
        return -1;
    }
    char *endptr;
    uint64_t t = strtoull(argv[0], &endptr, 10);
    uint64_t super_peroid = strtoull(argv[1], &endptr, 10);
    printf("Core %d start in %llu(%s), and super peroid is %d\n", disp_get_core_id(), t, argv[0], super_peroid);
    sys_setoff_tt(t, super_peroid);
}
