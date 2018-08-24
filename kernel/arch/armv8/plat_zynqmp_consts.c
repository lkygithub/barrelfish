/**
 * \file plat_zynqmp_const.c
 * \brief
 */


/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetsstrasse 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <kernel.h>
#include <maps/zynqmp_map.h>
/* RAM starts at 0, provided by the MMAP */
lpaddr_t phys_memory_start= ZYNQMP_PSDDR_MEM_BASEADDR;

/*
 * ----------------------------------------------------------------------------
 * GIC
 * ----------------------------------------------------------------------------
 */

// These values are from linux devicetree file
lpaddr_t platform_gic_cpu_base  = ZYNQMP_APU_GIC_CPU_BASE;
lpaddr_t platform_gic_dist_base = ZYNQMP_APU_GIC_D1ST_BASE;

/*
 * ----------------------------------------------------------------------------
 * UART
 * ----------------------------------------------------------------------------
 */

/* the maximum number of UARTS supported */
#define MAX_NUM_UARTS 2

/* the serial console port */
unsigned int serial_console_port = 0;

/* the debug console port */
unsigned int serial_debug_port = 0;

/* the number of physical ports */
unsigned serial_num_physical_ports = 2;

/* uart bases */
const lpaddr_t
uart_base[MAX_NUM_UARTS]= {
        ZYNQMP_UART0_BASEADDR, ZYNQMP_UART1_BASEADDR
};

/* uart sizes, for one page each*/
const size_t
uart_size[MAX_NUM_UARTS]= {
    4096, 4096
};
