/**
 * \file
 * \brief A struct for all shared data between the kernels
 */

/*
 * Copyright (c) 2008, 2010 ETH Zurich
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */


#ifndef KERNEL_ARCH_AARCH64_GLOBAL_H
#define KERNEL_ARCH_AARCH64_GLOBAL_H

#include <barrelfish_kpi/spinlocks_arch.h>
#include <barrelfish_kpi/types.h>

/**
 * \brief Struct passed to app_cores during boot.
 * Contains information that the bsp_kernel wants to pass to the app_kernels.
 */
struct global {
    /// Shared locks between the kernels
    struct {
        spinlock_t print;       ///< Lock for printing
        spinlock_t ttracing;       ///< Lock for time triggerd logging
    } locks;

    uint32_t tickspersec;

    genpaddr_t notify[MAX_COREID];

    struct {
        bool sync_flag;            ///< flag of syncED
        systime_t sys_launch_time; ///< the launch time of all cores after sync
        uint16_t cores;            ///< the number of cores
        void *ttmp_buff;           ///< address of ttmp buffer
    } ttmp_ctrl_info;
};

extern struct global *global;

// XXX: check this
#define GLOBAL_VBASE	0x21000

#endif
