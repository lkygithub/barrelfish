/*
 * Copyright (c) 2016, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <kernel.h>
#include <sysreg.h>
#include <dev/armv8_dev.h>
#include <dev/gic_v3_dev.h>
#include <dev/gic_v2_cpu_dev.h>
#include <platform.h>
#include <paging_kernel_arch.h>
#include <arch/armv8/gic_v3.h>

#include <dev/zynqmp/zynqmp_uart_dev.h>
#include <maps/zynqmp_map.h>

static gic_v3_t gic_v3_dev;
static gic_v2_cpu_t gic_v2_cpu_dev;

#define MSG(format, ...) printk( LOG_NOTE, "GICv2: "format, ## __VA_ARGS__ )

/*
 * This should return 1<<my_core_id
 */
static uint8_t gic_get_cpumask(void){
    uint32_t mask = gic_v3_GICD_ITARGETSR_rd(&gic_v3_dev , 0);
    mask |= mask >> 16;
    mask |= mask >> 8;
    return mask;
}

/*
 * Reads th STATUSR register, prints error on error condition
 */
static void check_cpu_if_statusr(void){
    gic_v2_cpu_STATUSR_t raw =  gic_v2_cpu_STATUSR_rawrd(&gic_v2_cpu_dev);
    if(raw) {
        char buf[512];
        gic_v2_cpu_STATUSR_pr(buf,sizeof(buf),&gic_v2_cpu_dev);
        printk(LOG_NOTE, "gic_v2: Error condition! Status: %s\n", buf); 
    }
}

/*
 * Initialize the global interrupt controller
 *
 * There are three types of interrupts
 * 1) Software generated Interrupts (SGI) - IDs 0-15
 * 2) Private Peripheral Interrupts (PPI) - IDs 16-31
 * 3) Shared Peripheral Interrups (SPI) - IDs 32...
 */
errval_t gicv3_init(void)
{
    lvaddr_t gic_dist = local_phys_to_mem(platform_get_distributor_address());
    gic_v3_initialize(&gic_v3_dev, (char *)gic_dist);

    lvaddr_t gic_cpu = local_phys_to_mem(platform_get_gic_cpu_address());
    gic_v2_cpu_initialize(&gic_v2_cpu_dev, (char *)gic_cpu);

    if(gic_v3_GICD_TYPER_SecurityExtn_rdf(&gic_v3_dev)){
        printk(LOG_NOTE, "gic_v2: In init. GIC supports secure mode\n"); 
    } else {
        printk(LOG_NOTE, "gic_v2: In init. GIC does not support secure mode\n"); 
    }

    MSG("gic_init done\n");

    return SYS_ERR_OK;
}

/*
 * Returns active interrupt of group 1 
 */
uint32_t gicv3_get_active_irq(void)
{
    uint32_t res = gic_v2_cpu_IAR_intid_rdf(&gic_v2_cpu_dev);
    check_cpu_if_statusr();
    return res;
}

struct zynqmp_uart_t test_uart = 
{
    (mackerel_addr_t)(ZYNQMP_UART0_BASEADDR + KERNEL_OFFSET),
    0,
    0
};

/*
 * ACKs group 1 interrupt
 */
void gicv3_ack_irq(uint32_t irq)
{
    gic_v2_cpu_EOIR_t reg = 0;
    reg = gic_v2_cpu_EOIR_intid_insert(reg, irq);
    gic_v2_cpu_EOIR_rawwr(&gic_v2_cpu_dev, irq);
    check_cpu_if_statusr();
    
    if(irq==53){
        zynqmp_uart_ISR_rtrig_wrf(&test_uart, 1);
        //printf("irq acked\n");
    }       
}

/*
 * Raise an SGI on a core. 
 */
void gicv3_raise_softirq(coreid_t cpuid, uint8_t irq)
{
    // assuming affinity routing DISABLED
    assert(irq <= 15);
    gic_v3_GICD_SGIR_t reg = 0;
    reg = gic_v3_GICD_SGIR_INTID_insert(reg, irq);
    reg = gic_v3_GICD_SGIR_CPUTargetList_insert(reg, 1<<(cpuid));
    gic_v3_GICD_SGIR_wr(&gic_v3_dev, reg);
    check_cpu_if_statusr();
}

/*
 * Enable GIC CPU-IF and local distributor
 */
errval_t gicv3_cpu_interface_enable(void)
{
    printk(LOG_NOTE, "gic_v2: GICC IIDR "
            "implementer=0x%x, revision=0x%x, variant=0x%x, prodid=0x%x, raw=0x%x\n",
            gic_v2_cpu_IIDR_Implementer_rdf(&gic_v2_cpu_dev),
            gic_v2_cpu_IIDR_Revision_rdf(&gic_v2_cpu_dev),
            gic_v2_cpu_IIDR_Variant_rdf(&gic_v2_cpu_dev),
            gic_v2_cpu_IIDR_ProductID_rdf(&gic_v2_cpu_dev),
            gic_v2_cpu_IIDR_rawrd(&gic_v2_cpu_dev)
            );

    // Do as Linux does: 
    // set priority mode: PMR to 0xf0
    gic_v2_cpu_PMR_wr(&gic_v2_cpu_dev, 0xf0);
    check_cpu_if_statusr();
    // Set binary point to 1: 6 group priority bits, 2 subpriority bits
    gic_v2_cpu_BPR_wr(&gic_v2_cpu_dev, 1);
    check_cpu_if_statusr();

    //We enable both group 0 and 1, but let both trigger IRQs (and not FIQs)
    gic_v2_cpu_CTLR_NS_rawwr(&gic_v2_cpu_dev, 3);  // code for non-secure
    gic_v2_cpu_CTLR_FIQEn_wrf(&gic_v2_cpu_dev, 0); // route both groups to IRQ

    // Disable all GIC bypassing (no wake-up interrupts). This does not
    // seem to have any effect?
    gic_v2_cpu_CTLR_FIQBypDisGrp1_wrf(&gic_v2_cpu_dev, 1);
    gic_v2_cpu_CTLR_FIQBypDisGrp0_wrf(&gic_v2_cpu_dev, 1);
    gic_v2_cpu_CTLR_IRQBypDisGrp1_wrf(&gic_v2_cpu_dev, 1);
    gic_v2_cpu_CTLR_IRQBypDisGrp0_wrf(&gic_v2_cpu_dev, 1);
    check_cpu_if_statusr();

    gic_v3_GICD_CTLR_secure_t ctrl = 0;
    // Set affinity routing (redundant on CN88xx)
    ctrl = gic_v3_GICD_CTLR_secure_ARE_NS_insert(ctrl, 1);
    // Enable group 1 interrupts
    ctrl = gic_v3_GICD_CTLR_secure_EnableGrp1NS_insert(ctrl, 1);
    gic_v3_GICD_CTLR_secure_wr(&gic_v3_dev, ctrl);

    check_cpu_if_statusr();

    printk(LOG_NOTE, "gic_v2: GICD IIDR "
            "implementer=0x%x, revision=0x%x, variant=0x%x, prodid=0x%x, raw=0x%x\n",
            gic_v3_GICD_IIDR_Implementer_rdf(&gic_v3_dev),
            gic_v3_GICD_IIDR_Revision_rdf(&gic_v3_dev),
            gic_v3_GICD_IIDR_Variant_rdf(&gic_v3_dev),
            gic_v3_GICD_IIDR_ProductID_rdf(&gic_v3_dev),
            gic_v3_GICD_IIDR_rawrd(&gic_v3_dev)
            );


    uint32_t itlines = gic_v3_GICD_TYPER_ITLinesNumber_rdf(&gic_v3_dev);
    itlines = (itlines+1)*32;
    if(itlines > 1020)
        itlines = 1020;
    printk(LOG_NOTE, "gic_v2: #INTIDs supported: %" PRIu32 "\n", itlines);

    uint32_t lspi = gic_v3_GICD_TYPER_LSPI_rdf(&gic_v3_dev);
    printk(LOG_NOTE, "gic_v2: #LSPIs supported: %" PRIu32 "\n", lspi);


    // Setup distributor so it forwards all interrupts to this CPU.
    uint8_t my_cpumask = gic_get_cpumask();
    uint32_t dest_cpumask = my_cpumask;
    dest_cpumask = dest_cpumask | dest_cpumask << 8 | dest_cpumask << 16 | dest_cpumask << 24;
    for(int i=8; i*4 < itlines; i++)
        gic_v3_GICD_ITARGETSR_wr(&gic_v3_dev, i, dest_cpumask);

    // Put all interrupts into Group 0 and enable them
    #define MASK_32     0xffffffff
    for(int i=0; i*32 < itlines; i++){
        // Clear
        gic_v3_GICD_ICACTIVER_wr(&gic_v3_dev, i, MASK_32);
        // Enable
        gic_v3_GICD_ISENABLER_wr(&gic_v3_dev, i, MASK_32);
        // And put in group 0
        gic_v3_GICD_IGROUPR_rawwr(&gic_v3_dev, i, 0);
    }

    // Disable interrupt FIQ Bypass interrupt 28
    gic_v3_GICD_ICENABLER_wr(&gic_v3_dev, 0, (1<<28));


    gic_v3_GICD_CTLR_rawwr(&gic_v3_dev, 0x1); // Enable Distributor
    check_cpu_if_statusr();

    return SYS_ERR_OK;
}

enum IrqType {
    IrqType_SGI,
    IrqType_PPI,
    IrqType_SPI
};

/**
 * \brief Returns the IRQ type based on the interrupt ID
 *
 * We have three types of interrupts
 * 1) Software generated Interrupts (SGI): IDs 0-15
 * 2) Private Peripheral Interrupts (PPI): IDs 16-31
 * 3) Shared Peripheral Interrups (SPI): IDs 32-
 *
 * \return The type of the interrupt.
 */
static enum IrqType get_irq_type(uint32_t int_id)
{
    if (int_id < 16) {
        return IrqType_SGI;
    } else if (int_id < 32) {
        return IrqType_PPI;
    } else {
        return IrqType_SPI;
    }
}

/**
 * \brief Enable an interrupt
 *
 * \see ARM Generic Interrupt Controller Architecture Specification v2.0
 *
 * \param int_id
 * \param cpu_targets 8 Bit mask. One bit for each core in the system.
 *    (chapter 4.3.11)
 * \param prio Priority of the interrupt (lower is higher). We allow 0..15.
 *    The number of priority bits is implementation specific, but at least 16
 *    (using bits [7:4] of the priority field, chapter 3.3)
 * \param 0 is level-sensitive, 1 is edge-triggered
 * \param 0 is N-to-N, 1 is 1-N
 */
void gicv3_enable_interrupt(uint32_t int_id, uint8_t cpu_targets, uint16_t prio,
                          bool edge_triggered, bool one_to_n)
{
    // Set Interrupt Set-Enable Register
    uint32_t ind = int_id / 32;
    uint32_t bit_mask = (1U << (int_id % 32));
    uint32_t regval;

    MSG("gic_enable_interrupt for id=0x%"PRIx32", "
           "offset=0x%"PRIx32", index=0x%"PRIx32"\n",
           int_id, bit_mask, ind);

    enum IrqType irq_type = get_irq_type(int_id);
    MSG("TYPE %d\n",irq_type);

    // Set the Interrupt Set Enable register to enable the interupt
    // See ARM GIC TRM
    if (irq_type == IrqType_SGI) {
        MSG("unhandled SGI IRQ %d\n", int_id);
        return;    // Do nothing for SGI interrupts
    }

    // XXX: check what we need to do if int_id > it_num_lines
    //  -SG, 2012/12/13
    // assert(int_id <= it_num_lines);

    // Enable
    // 1 Bit per interrupt
    regval = gic_v3_GICD_ISENABLER_rd(&gic_v3_dev, ind);
    regval |= bit_mask;
    gic_v3_GICD_ISENABLER_wr(&gic_v3_dev, ind, regval);

    // TODO: cleanup pl130 mackerel file so that we don't need bit magic
    // here.  -SG, 2012/12/13

    // Priority
    // 8 Bit per interrupt
    // chp 4.3.10
    //ind = int_id/4;
    // XXX: check that priorities work properly, -SG, 2012/12/13
    prio = (prio & 0xF)<<4;
    gic_v3_GICD_IPRIORITYR_wr(&gic_v3_dev, int_id, prio);
    //switch(int_id % 4) {
    //case 0:
    //    gic_v3_ICDIPR_prio_off0_wrf(&gic, ind, prio);
    //    break;
    //case 1:
    //    gic_v3_ICDIPR_prio_off1_wrf(&gic, ind, prio);
    //    break;
    //case 2:
    //    gic_v3_ICDIPR_prio_off2_wrf(&gic, ind, prio);
    //    break;
    //case 3:
    //    gic_v3_ICDIPR_prio_off3_wrf(&gic, ind, prio);
    //    break;
    //}

    // Target processors (only SPIs)
    // 8 Bit per interrupt
    ind = int_id/4;
    if (irq_type == IrqType_SPI) { // rest is ro
        switch (int_id % 4) {
        case 0:
            gic_v3_GICD_ITARGETSR_targets_off0_wrf(&gic_v3_dev, ind, cpu_targets);
            break;
        case 1:
            gic_v3_GICD_ITARGETSR_targets_off1_wrf(&gic_v3_dev, ind, cpu_targets);
            break;
        case 2:
            gic_v3_GICD_ITARGETSR_targets_off2_wrf(&gic_v3_dev, ind, cpu_targets);
            break;
        case 3:
            gic_v3_GICD_ITARGETSR_targets_off3_wrf(&gic_v3_dev, ind, cpu_targets);
            break;
        }
    }

    // Configuration registers
    // 2 Bit per IRQ
    ind = int_id/16;
    uint8_t val = ((edge_triggered&0x1) << 1) | (one_to_n&0x1);
    switch (int_id % 16) {
    case 0:
        gic_v3_GICD_ICFGR_conf0_wrf(&gic_v3_dev, ind, val);
        break;
    case 1:
        gic_v3_GICD_ICFGR_conf1_wrf(&gic_v3_dev, ind, val);
        break;
    case 2:
        gic_v3_GICD_ICFGR_conf2_wrf(&gic_v3_dev, ind, val);
        break;
    case 3:
        gic_v3_GICD_ICFGR_conf3_wrf(&gic_v3_dev, ind, val);
        break;
    case 4:
        gic_v3_GICD_ICFGR_conf4_wrf(&gic_v3_dev, ind, val);
        break;
    case 5:
        gic_v3_GICD_ICFGR_conf5_wrf(&gic_v3_dev, ind, val);
        break;
    case 6:
        gic_v3_GICD_ICFGR_conf6_wrf(&gic_v3_dev, ind, val);
        break;
    case 7:
        gic_v3_GICD_ICFGR_conf7_wrf(&gic_v3_dev, ind, val);
        break;
    case 8:
        gic_v3_GICD_ICFGR_conf8_wrf(&gic_v3_dev, ind, val);
        break;
    case 9:
        gic_v3_GICD_ICFGR_conf9_wrf(&gic_v3_dev, ind, val);
        break;
    case 10:
        gic_v3_GICD_ICFGR_conf10_wrf(&gic_v3_dev, ind, val);
        break;
    case 11:
        gic_v3_GICD_ICFGR_conf11_wrf(&gic_v3_dev, ind, val);
        break;
    case 12:
        gic_v3_GICD_ICFGR_conf12_wrf(&gic_v3_dev, ind, val);
        break;
    case 13:
        gic_v3_GICD_ICFGR_conf13_wrf(&gic_v3_dev, ind, val);
        break;
    case 14:
        gic_v3_GICD_ICFGR_conf14_wrf(&gic_v3_dev, ind, val);
        break;
    case 15:
        gic_v3_GICD_ICFGR_conf15_wrf(&gic_v3_dev, ind, val);
        break;
    }
}