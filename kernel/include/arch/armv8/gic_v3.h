/*
 * Copyright (c) 2016, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr 6, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef __GIC_V3_H__
#define __GIC_V3_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <barrelfish_kpi/types.h>

// Helpers for enabling interrupts
#define GIC_IRQ_PRIO_LOWEST       (0xF)
#define GIC_IRQ_CPU_TRG_ALL       (0x3) // For two cores on the PandaBoard
#define GIC_IRQ_CPU_TRG_BSP       (0x1)
#define GIC_IRQ_EDGE_TRIGGERED    (0x1)
#define GIC_IRQ_LEVEL_SENSITIVE   (0x0)
#define GIC_IRQ_1_TO_N            (0x1)
#define GIC_IRQ_N_TO_N            (0x0)

/*
 * generic interrupt controller functionality
 */
errval_t gicv3_init(void);

errval_t gicv3_cpu_interface_enable(void);

uint32_t gicv3_get_active_irq(void);

void gicv3_ack_irq(uint32_t irq);

void gicv3_raise_softirq(coreid_t cpuid, uint8_t irq);

void gicv3_enable_interrupt(uint32_t int_id, uint8_t cpu_targets, uint16_t prio,
                          bool edge_triggered, bool one_to_n);
#endif // __GIC_V3_H__
