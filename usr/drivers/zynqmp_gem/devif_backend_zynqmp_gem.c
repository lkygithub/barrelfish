/*
 * Copyright (c) 2017 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <barrelfish/barrelfish.h>
#include <barrelfish/deferred.h>
#include <barrelfish/nameservice_client.h>
#include <barrelfish/inthandler.h>
#include <devif/queue_interface_backend.h>
#include <net_interfaces/flags.h>
#include <debug_log/debug_log.h>
#include <if/zynqmp_gem_devif_defs.h>
#include <int_route/int_route_client.h>


#include "zynqmp_gem.h"
#include "zynqmp_gem_desc.h"
#include "zynqmp_gem_devq.h"



/*****************************************************************
 * allocate a single frame, mapping it into our vspace with given
 * attributes
 ****************************************************************/
static void *alloc_map_frame(vregion_flags_t attr, size_t size, struct capref *retcap)
{
    struct capref frame;
    errval_t err;
    void *va;

    err = frame_alloc(&frame, size, NULL);
    if (err_is_fail(err)) {
        ZYNQMP_GEM_DEBUG("ERROR: frame_alloc failed.\n");
        return NULL;
    }

    err = vspace_map_one_frame_attr(&va, size, frame, attr, NULL, NULL);

    if (err_is_fail(err)) {
        ZYNQMP_GEM_DEBUG("Error: vspace_map_one_frame failed\n");
        return NULL;
    }

    if (retcap != NULL) {
        *retcap = frame;
    }

    return va;
}

static errval_t zynqmp_gem_register(struct devq* q, struct capref cap,
                                  regionid_t rid)
{
    zynqmp_gem_queue_t *q_ext = (zynqmp_gem_queue_t *)q;
    struct frame_identity id;
    errval_t err;
    
    err = invoke_frame_identify(cap, &id);
    assert(err_is_ok(err));
    assert(!q_ext->region_id);
    q_ext->region_id = rid;
    q_ext->region_base = id.base;
    q_ext->region_size = id.bytes;
    ZYNQMP_GEM_DEBUG("queue register: rid:%d, base:%lx, size:%lx\n", rid, id.base, id.bytes);
    return SYS_ERR_OK;
}

static errval_t zynqmp_gem_deregister(struct devq* q, regionid_t rid)
{
    return SYS_ERR_OK;
}


static errval_t zynqmp_gem_control(struct devq* q, uint64_t cmd, uint64_t value,
                                 uint64_t *result)
{
    zynqmp_gem_queue_t *q_ext = (zynqmp_gem_queue_t *)q;
    *result = q_ext->mac_address;
    return SYS_ERR_OK;
}

static errval_t zynqmp_gem_enqueue_rx(zynqmp_gem_queue_t *q, regionid_t rid,
                               genoffset_t offset, genoffset_t length,
                               genoffset_t valid_data, genoffset_t valid_length,
                               uint64_t flags)
{

    if (zynqmp_gem_queue_free_rxslots(q) == 0) {
        ZYNQMP_GEM_DEBUG("Not enough space in RX ring, not transmitting buffer \n" );
        return DEVQ_ERR_QUEUE_FULL;
    }

    rx_desc_t rxdesc;

/*
    rxdesc.raw[0] = rxdesc.raw[1] = 0;
    rxdesc.rx_read_format.buffer_address = q->region_base + offset;
 */
    q->rx_ring[q->rx_tail] = rxdesc;
    q->rx_tail = (q->rx_tail + 1) % ZYNQMP_GEM_N_RX_BUFS;
    

/*
    zynqmp_gem_dqval_t dqval = 0;
    dqval = zynqmp_gem_dqval_val_insert(dqval, q->rx_tail);
    zynqmp_gem_rdt_wr(&q->hw_device, 0, dqval);
 */

    return SYS_ERR_OK;
}

static errval_t zynqmp_gem_dequeue_rx(zynqmp_gem_queue_t *q, regionid_t* rid, 
                                 genoffset_t* offset,
                                 genoffset_t* length, genoffset_t* valid_data,
                                 genoffset_t* valid_length, uint64_t* flags)
{
    if (q->rx_head == q->rx_tail) {
        return DEVQ_ERR_QUEUE_EMPTY;
    }

    volatile rx_desc_t *rxdesc;
    rxdesc = &q->rx_ring[q->rx_head];
    /*
    if (!rxdesc->rx_read_format.info.status.dd ||
            !rxdesc->rx_read_format.info.status.eop) {
        return DEVQ_ERR_QUEUE_EMPTY;
    }
     */
    
    q->rx_head = (q->rx_head + 1) % ZYNQMP_GEM_N_RX_BUFS;
    /*
    *rid = q->region_id;
    *offset = rxdesc->rx_read_format.buffer_address - q->region_base;
    *length = 2048;
    *valid_data = 0;
    *valid_length = rxdesc->rx_read_format.info.length;
    *flags = NETIF_RXFLAG;
     */
    return SYS_ERR_OK;
}

static errval_t zynqmp_gem_enqueue_tx(zynqmp_gem_queue_t *q, regionid_t rid,
                               genoffset_t offset, genoffset_t length,
                               genoffset_t valid_data, genoffset_t valid_length,
                               uint64_t flags)
{
    tx_desc_t txdesc;

    if (zynqmp_gem_queue_free_txslots(q) == 0) {
        ZYNQMP_GEM_DEBUG("Not enough space in TX ring, not transmitting buffer \n" );
        return DEVQ_ERR_QUEUE_FULL;
    }
    

/*
    if (q->advanced_descriptors == 3) {
        txdesc.buffer_address = q->region_base + offset + valid_data;
        txdesc.ctrl.raw = 0;
        txdesc.ctrl.advanced_data.dtalen = valid_length;
        txdesc.ctrl.advanced_data.dtyp.d.dtyp = 3; // advanced data descriptor
        txdesc.ctrl.advanced_data.dcmd.d.eop = 1;
        txdesc.ctrl.advanced_data.dcmd.d.ifcs = 1;
        txdesc.ctrl.advanced_data.dcmd.d.rs = 1;
        txdesc.ctrl.advanced_data.dcmd.d.dext = 1;
        txdesc.ctrl.advanced_data.popts_paylen.d.paylen = valid_length;
    } else if (q->advanced_descriptors == 1) {
        txdesc.buffer_address = q->region_base + offset + valid_data;
        txdesc.ctrl.raw = 0;
        txdesc.ctrl.extended_data.data_len = valid_length;
        txdesc.ctrl.extended_data.dtyp = 1; // extended data descriptor
        txdesc.ctrl.extended_data.dcmd.d.rs = 1;
        txdesc.ctrl.extended_data.dcmd.d.ifcs = 1;
        txdesc.ctrl.extended_data.dcmd.d.eop = 1;
        txdesc.ctrl.extended_data.dcmd.d.dext = 1;
    } else {
        txdesc.buffer_address = q->region_base + offset + valid_data;
        txdesc.ctrl.raw = 0;
        txdesc.ctrl.legacy.data_len = valid_length;
        txdesc.ctrl.legacy.cmd.d.eop = 1;
        txdesc.ctrl.legacy.cmd.d.ifcs = 1;
        txdesc.ctrl.legacy.cmd.d.rs = 1;
    }
 */
    q->tx_ring[q->tx_tail] = txdesc;
    q->tx_tail = (q->tx_tail + 1) % ZYNQMP_GEM_N_TX_BUFS;
/*
    zynqmp_gem_dqval_t dqval = 0;
    dqval = zynqmp_gem_dqval_val_insert(dqval, q->tx_tail);
    zynqmp_gem_tdt_wr(&q->hw_device, 0, dqval);
 */
    return SYS_ERR_OK;
}

static errval_t zynqmp_gem_dequeue_tx(zynqmp_gem_queue_t *q, regionid_t* rid, genoffset_t* offset,
                                 genoffset_t* length, genoffset_t* valid_data,
                                 genoffset_t* valid_length, uint64_t* flags)
{
    if (q->tx_head == q->tx_tail) {
        return DEVQ_ERR_QUEUE_EMPTY;
    }

    volatile tx_desc_t *txdesc;
    txdesc = &q->tx_ring[q->tx_head];
    /*
    if (txdesc->ctrl.legacy.stat_rsv.d.dd != 1) {
        return DEVQ_ERR_QUEUE_EMPTY;
    }
     */
    q->tx_head = (q->tx_head + 1) % ZYNQMP_GEM_N_TX_BUFS;
    /*
    *rid = q->region_id;
    *offset = txdesc->buffer_address - q->region_base;
    *length = 2048;
    *valid_data = *offset & 2047;
    *offset &= ~2047;
    *valid_length = txdesc->ctrl.legacy.data_len;
    *flags = NETIF_TXFLAG | NETIF_TXFLAG_LAST;
     */
    return SYS_ERR_OK;
}


static errval_t zynqmp_gem_enqueue(struct devq* q, regionid_t rid,
                                 genoffset_t offset, genoffset_t length,
                                 genoffset_t valid_data, genoffset_t valid_length,
                                 uint64_t flags)
{
    zynqmp_gem_queue_t *q_ext = (zynqmp_gem_queue_t *)q;
    errval_t err;

    if (flags & NETIF_RXFLAG) {
        /* can not enqueue receive buffer larger than 2048 bytes */
        assert(length <= 2048);

        err = zynqmp_gem_enqueue_rx(q_ext, rid, offset, length, valid_data, valid_length,
                             flags);
        if (err_is_fail(err)) {
            return err;
        }
    } else if (flags & NETIF_TXFLAG) {
        assert(length <= BASE_PAGE_SIZE);

        err = zynqmp_gem_enqueue_tx(q_ext, rid, offset, length, valid_data, valid_length,
                             flags);
        if (err_is_fail(err)) {
            return err;
        }
    } else {
        printf("Unknown buffer flags \n");
        return NIC_ERR_ENQUEUE;
    }

    return SYS_ERR_OK;
}

static errval_t zynqmp_gem_dequeue(struct devq* q, regionid_t* rid, genoffset_t* offset,
                                 genoffset_t* length, genoffset_t* valid_data,
                                 genoffset_t* valid_length, uint64_t* flags)
{
    zynqmp_gem_queue_t *q_ext = (zynqmp_gem_queue_t *)q;
    
    if (zynqmp_gem_dequeue_tx(q_ext, rid, offset, length, valid_data, valid_length,  flags) == SYS_ERR_OK) 
        return SYS_ERR_OK;
    if (zynqmp_gem_dequeue_rx(q_ext, rid, offset, length, valid_data, valid_length, flags) == SYS_ERR_OK) {
        return SYS_ERR_OK;
    }
    return DEVQ_ERR_QUEUE_EMPTY;
}

static errval_t zynqmp_gem_notify(struct devq* q)
{
    assert(0);
    return SYS_ERR_OK;
}

static errval_t zynqmp_gem_destroy(struct devq * q)
{
    zynqmp_gem_queue_t* q_ext = (zynqmp_gem_queue_t *) q;
    errval_t err;
    err = vspace_unmap((void*)q_ext->rx_ring);
    assert(err_is_ok(err));
    err = vspace_unmap((void*)q_ext->dummy_rx_ring);
    assert(err_is_ok(err));
    err = vspace_unmap((void*)q_ext->tx_ring);
    assert(err_is_ok(err));
    err = vspace_unmap((void*)q_ext->dummy_tx_ring);
    assert(err_is_ok(err));
    err = cap_delete(q_ext->rx);
    assert(err_is_ok(err));
    err = cap_delete(q_ext->dummy_rx);
    assert(err_is_ok(err));
    err = cap_delete(q_ext->tx);
    assert(err_is_ok(err));
    err = cap_delete(q_ext->dummy_tx);
    assert(err_is_ok(err));
    free(q_ext);
    return SYS_ERR_OK;
}

static void bind_cb(void *st, errval_t err, struct zynqmp_gem_devif_binding *b)
{
    zynqmp_gem_queue_t* q = (zynqmp_gem_queue_t*) st;
    b->st = st;
    
    q->b = b;
    zynqmp_gem_devif_rpc_client_init(q->b);
    q->bound = true;
}

errval_t zynqmp_gem_queue_create(zynqmp_gem_queue_t ** pq, void (*int_handler)(void *))
{
    errval_t err;
    zynqmp_gem_queue_t *q;
    struct frame_identity id;

    q = malloc(sizeof(zynqmp_gem_queue_t));
    assert(q);

    q->rx_head = 0;
    q->rx_tail = 0;
    q->tx_head = 0;
    q->tx_tail = 0;
    q->region_id = 0;
    q->bound = false;

    q->n_rx_buffers = ZYNQMP_GEM_N_RX_BUFS;
    q->n_tx_buffers = ZYNQMP_GEM_N_TX_BUFS;

    char service[128] = "zynqmp_gem_devif";

    iref_t iref;
    err = nameservice_blocking_lookup(service, &iref);
    if (err_is_fail(err)) {
        return err;
    }

    err = zynqmp_gem_devif_bind(iref, bind_cb, q, get_default_waitset(), 1);
    if (err_is_fail(err)) {
        return err;
    }
    
    // wait until bound
    while(!q->bound) {
        event_dispatch(get_default_waitset());
    }

    q->rx_ring = alloc_map_frame(VREGION_FLAGS_READ_WRITE_NOCACHE, 
            ZYNQMP_GEM_N_RX_BUFS * sizeof(rx_desc_t), &q->rx);
    if (q->rx_ring == NULL) {
        return DEVQ_ERR_INIT_QUEUE;
    }

    q->tx_ring = alloc_map_frame(VREGION_FLAGS_READ_WRITE_NOCACHE, 
            ZYNQMP_GEM_N_TX_BUFS * sizeof(tx_desc_t), &q->tx);
    if (q->tx_ring == NULL) {
        return DEVQ_ERR_INIT_QUEUE;
    }

    errval_t err2;
    err = q->b->rpc_tx_vtbl.create_queue(q->b, q->rx, q->tx, &q->mac_address);
    if (err_is_fail(err) || err_is_fail(err2)) {
        err = err_is_fail(err) ? err: err2;
        return err;
    }

    err = devq_init(&q->q, false);
    assert(err_is_ok(err));
    
    q->q.f.enq = zynqmp_gem_enqueue;
    q->q.f.deq = zynqmp_gem_dequeue;
    q->q.f.reg = zynqmp_gem_register;
    q->q.f.dereg = zynqmp_gem_deregister;
    q->q.f.ctrl = zynqmp_gem_control;
    q->q.f.notify = zynqmp_gem_notify;
    q->q.f.destroy = zynqmp_gem_destroy;
    
    *pq = q;

    return SYS_ERR_OK;
}
