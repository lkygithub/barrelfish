/**
 * \file
 * \brief Kernel serial driver for the Xilinx ZynqMP-series UART
 */

/*
 * Copyright (c) 2016, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <kernel.h>
#include <arch/arm/arm.h>
#include <dev/zynqmp/zynqmp_uart_dev.h>
#include <paging_kernel_arch.h>
#include <arch/arm/zynqmp_uart.h>

/* Serial console and debugger interfaces. */
static zynqmp_uart_t ports[ZYNQMP_UART_MAX_PORTS];

/*
 * Initialise Zynq UART
 */
static void
zynqmp_uart_hw_init(zynqmp_uart_t *uart) {
    /* Disable all interrupts. */
    zynqmp_uart_IDR_rawwr(uart, 0);

    /* Clear all interrupts. */
    zynqmp_uart_ISR_rawwr(uart, 0xffffffff);

    /* Trigger an interrupt on a single byte. */
    zynqmp_uart_RXWM_RTRIG_wrf(uart, 1);

    /* Enable RX trigger interrupt. */
    zynqmp_uart_IER_rtrig_wrf(uart, 1);

    /* Enable receiver. */
    zynqmp_uart_CR_rx_dis_wrf(uart, 0);
    zynqmp_uart_CR_rx_en_wrf(uart, 1);
}


void
zynqmp_uart_init(unsigned port, lvaddr_t base, bool initialize_hw) {
    assert(port < ZYNQMP_UART_MAX_PORTS);
    zynqmp_uart_initialize(&ports[port], (mackerel_addr_t) base);
    if(initialize_hw) zynqmp_uart_hw_init(&ports[port]);
}

void
zynqmp_uart_early_init(unsigned port, lpaddr_t addr)
{
    assert(port < ZYNQMP_UART_MAX_PORTS);
    zynqmp_uart_initialize(&ports[port], (mackerel_addr_t) addr);
    zynqmp_uart_hw_init(&ports[port]);
}

/**
 * \brief Prints a single character to a serial port. 
 */
void
zynqmp_uart_putchar(unsigned port, char c) {
    assert(port <= ZYNQMP_UART_MAX_PORTS);
    assert(ports[port].base != 0);
    zynqmp_uart_t *uart = &ports[port];

    /* Wait until FIFO can hold more characters. */
    while(zynqmp_uart_SR_TXFULL_rdf(uart));

    /* Write character. */
    zynqmp_uart_FIFO_FIFO_wrf(uart, c);
}

/** 
 * \brief Reads a single character from the default serial port.
 */
char
zynqmp_uart_getchar(unsigned port) {
    assert(port <= ZYNQMP_UART_MAX_PORTS);
    assert(ports[port].base != 0);
    zynqmp_uart_t *uart = &ports[port];

    /* Drain the FIFO. */
    char c= zynqmp_uart_FIFO_FIFO_rdf(uart);
    while(!zynqmp_uart_SR_RXEMPTY_rdf(uart)) {
        c= zynqmp_uart_FIFO_FIFO_rdf(uart);
    }
    

   //while(zynqmp_uart_SR_RXEMPTY_rdf(uart));
   //char c= zynqmp_uart_FIFO_FIFO_rdf(uart);

    /* Clear the RXTRIG interrupt. */
    zynqmp_uart_ISR_rtrig_wrf(uart, 1);

    /* Return the character. */
    return c;
}
