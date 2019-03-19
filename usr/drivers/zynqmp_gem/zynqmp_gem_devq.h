/*
 * Copyright (c) 2008, ETH Zurich. All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef __ZYNQMP_GEM_DEVQ_H__
#define __ZYNQMP_GEM_DEVQ_H__

typedef struct zynqmp_gem_queue {
    struct devq q;

    zynqmp_gem_t device;
    uint64_t mac_address;

    volatile struct rx_desc *rx_ring;
    volatile struct rx_desc *dummy_rx_ring;
    volatile struct tx_desc *tx_ring;
    volatile struct tx_desc *dummy_tx_ring;
    struct capref rx;
    struct capref dummy_rx;
    struct capref tx;
    struct capref dummy_tx;

    int n_rx_buffers;
    int n_tx_buffers;

    regionid_t region_id;
    genpaddr_t region_base;
    gensize_t  region_size;

    unsigned rx_head, rx_tail;
    unsigned tx_head, tx_tail;

    // binding
    bool bound;
    struct zynqmp_gem_devif_binding* b;
} zynqmp_gem_queue_t;

static inline size_t zynqmp_gem_queue_free_rxslots(zynqmp_gem_queue_t* q)
{
    size_t head = q->rx_head;
    size_t tail = q->rx_tail;
    size_t size = q->n_rx_buffers;

    if (tail >= head) {
        return size - (tail - head) -1; 
    } else {
        return size - (tail + size - head) -1; 
    }
}

static inline size_t zynqmp_gem_queue_free_txslots(zynqmp_gem_queue_t* q)
{

    size_t head = q->tx_head;
    size_t tail = q->tx_tail;
    size_t size = q->n_tx_buffers;

    if (tail >= head) {
        return size - (tail - head) - 1; 
    } else {
        return size - (tail + size - head) - 1; 
    }

}

#endif
