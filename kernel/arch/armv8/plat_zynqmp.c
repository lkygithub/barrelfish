/**
 * \file plat_zynqmp.c
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
#include <offsets.h>
#include <platform.h>
#include <serial.h>
#include <arch/arm/gic.h>
#include <sysreg.h>
#include <dev/armv8_dev.h>
#include <barrelfish_kpi/arm_core_data.h>
#include <psci.h>
#include <arch/armv8/global.h>
#include <arch/armv8/gic_v3.h>
#include <paging_kernel_arch.h>
#include <arch/arm/zynqmp_uart.h>
#include <maps/zynqmp_map.h>

/* the maximum number of UARTS supported */
#define MAX_NUM_UARTS ZYNQMP_UART_MAX_PORTS

errval_t serial_init(unsigned port, bool initialize_hw)
{
    if (port >= MAX_NUM_UARTS) {
        return SYS_ERR_SERIAL_PORT_INVALID;
    }

    if (!initialize_hw) {
        // hw initialized, this is for non-bsp cores, where hw has been
        // initialized by bsp core and we come through here just to setup our
        // local zynqmp_uart struct for the port.
        zynqmp_uart_init(port, (lvaddr_t)(uart_base[port] + KERNEL_OFFSET),0);
        return SYS_ERR_OK;
    }
     /* initialize_hw is 1 means that the hardware need initializing */
    zynqmp_uart_init(port, (lvaddr_t)(uart_base[port] + KERNEL_OFFSET),1);

    panic("device init NYI");
    return SYS_ERR_OK;
}

errval_t serial_early_init(unsigned port)
{
    if (port >= MAX_NUM_UARTS) {
        return SYS_ERR_SERIAL_PORT_INVALID;
    }

    zynqmp_uart_early_init(port, (lvaddr_t)uart_base[port]);
    return SYS_ERR_OK;
}
/* what the fuck */
errval_t serial_early_init_mmu_enabled(unsigned port)
{
    return serial_early_init(port);
}


/**
 * \brief Prints a single character to a serial port.
 */
void serial_putchar(unsigned port, char c)
{
    assert(port < MAX_NUM_UARTS);
    zynqmp_uart_putchar(port,c);
}

/**
 * \brief Reads a single character from the default serial port.
 * This function spins waiting for a character to arrive.
 */
char serial_getchar(unsigned port)
{
    assert(port < MAX_NUM_UARTS);
    return zynqmp_uart_getchar(port);
}


void platform_get_info(struct platform_info *pi)
{
    pi->arch = PI_ARCH_ARMV8A;
    pi->platform = PI_PLATFORM_ZYNQMP;
}

/**
 * TODO
 */
void armv8_get_info(struct arch_info_armv8 *ai)
{

}

size_t platform_get_ram_size(void)
{
    return (ZYNQMP_PSDDR_MEM_HIGHADDR - ZYNQMP_PSDDR_MEM_BASEADDR + 1);
}

/*
 * Return the address of the UART device.
 */
lpaddr_t platform_get_uart_address(unsigned port)
{
    // is this nessacery ??
    return uart_base[port];
}
/*
void platform_set_uart_address(unsigned port, lpaddr_t uart_base)
{
    // ?????
}

lpaddr_t platform_get_private_region(void)
{
    // todo 
}
*/
/*
 * Do any extra initialisation for this particular CPU (e.g. A9/A15).
 */
void platform_revision_init(void)
{
    // todo
}

/*
 * Return the core count
 */
size_t platform_get_core_count(void)
{
    return 4;
}

/*
 * Print system identification. MMU is NOT yet enabled.
 */
void platform_print_id(void)
{
    printf("This is zynqMP on ZCU104\n");
}


/* GIC */

errval_t platform_gic_init(void) {
    /* GIC v3 ?????? */
    //# warning "GIC mismatches"
    gicv3_init();
    return SYS_ERR_OK;
}

errval_t platform_gic_cpu_interface_enable(void) {
    gicv3_cpu_interface_enable();
    return SYS_ERR_OK;
}

/**
 * @brief obtain the address of the GIC CPU interface
 *
 * @return physical address of the CBAR region
 */
lpaddr_t platform_get_gic_cpu_address(void) {
    assert(paging_mmu_enabled());
    return platform_gic_cpu_base;
}

/**
 * @brief obtain the address of the GIC distributor interface
 *
 * @return physical address of the CBAR region
 */
lpaddr_t platform_get_distributor_address(void) {
    assert(paging_mmu_enabled());
    return platform_gic_dist_base;
}
/*
lpaddr_t platform_get_distributor_size(void)
{
    // todo
}
lpaddr_t platform_get_gic_cpu_size(void)
{
    // todo
}
*/
void platform_set_gic_cpu_address(lpaddr_t gic_cpu_addr){}
void platform_set_distributor_address(lpaddr_t gic_dist_addr){}


errval_t platform_boot_core(hwid_t target, genpaddr_t gen_entry, genpaddr_t context)
{
    printf("Invoking PSCI on: cpu=0x%lx, entry=0x%lx, context=0x%lx\n", target, gen_entry, context);
    struct armv8_core_data *cd = (struct armv8_core_data *)local_phys_to_mem(context);
    cd->page_table_root = armv8_TTBR1_EL1_rd(NULL);
    cd->cpu_driver_globals_pointer = (uintptr_t)global;
    __asm volatile("dsb   sy\n"
                   "dmb   sy\n"
                   "isb     \n");

    /*
     * An IRQ interrupt, even if the PSTATE I-bit is set.
An FIQ interrupt, even if the PSTATE F-bit is set.
     *
     *
     */

    gicv3_raise_softirq(target, 1);
     
    return SYS_ERR_OK;
}

