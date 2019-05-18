/**
 * \file
 * \brief User-side system call wrappers
 */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef LIBBARRELFISH_SYSCALL_H
#define LIBBARRELFISH_SYSCALL_H

/* Proper Barrelfish system calls */

#include <sys/cdefs.h>    /* for __BEGIN_DECLS, __END_DECLS */
#include <errors/errno.h> /* for errval_t */
#include <barrelfish_kpi/types.h>

__BEGIN_DECLS

/**
 * \brief Yield the CPU.
 *
 * Yields the remainder of the time-slice for this dispatcher to the next
 * runnable dispatcher.
 *
 * \param target Dispatcher to yield to, or CPTR_NULL for an undirected yield
 *
 * \return Syscall error code (#SYS_ERR_OK on success).
 */
errval_t sys_yield(capaddr_t target);

/** Suspend the current cpu */
errval_t sys_suspend(bool halt);

/* Debug/Benchmarking system calls */
errval_t sys_nop(void);
errval_t sys_reboot(void);

/**
 * \brief Print a string through the kernel.
 *
 * This calls #SYSCALL_PRINT to print 'string' of length 'length' through
 * the kernel. Whether and where 'string' is printed is determined by
 * the kernel.
 *
 * \param string        Pointer to string to print.
 * \param length        Length of string.
 *
 * \return Syscall error code (#SYS_ERR_OK on success).
 */
errval_t sys_print(const char *string, size_t length);

errval_t sys_getchar(char *c);

errval_t sys_setoff_tt(uint64_t tt_start_timestamp, uint64_t super_peroid);
/**
 * \brief get time elapsed (in milliseconds) since system boot.
 */

uint64_t sys_get_absolute_time(void);

/**
 * \brief send/receive tt msg from kernel buffer.
 */
errval_t sys_ttmp_send(void);

errval_t sys_ttmp_receive(void);

errval_t sys_get_current_period_start_ts(uint64_t *current);
errval_t sys_get_tt_start_flag(bool *started);
errval_t sys_get_tt_start_time(uint64_t *ts);
__END_DECLS

#endif //LIBBARRELFISH_SYSCALL_H
