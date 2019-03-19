/**
 * \file
 * \brief ZYNQMP GEM driver.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <barrelfish/barrelfish.h>
#include <barrelfish/inthandler.h>
#include <barrelfish/nameservice_client.h>

#include <if/zynqmp_gem_devif_defs.h>

#include <dev/zynqmp_gem_dev.h>

#include "zynqmp_gem_debug.h"
#include "zynqmp_gem_desc.h"
#include "zynqmp_gem.h"

static zynqmp_gem_t device;

static uint8_t zynqmp_gem_mac[6];

/* 
static void add_txslot(uint32_t paddr, uint32_t len, void *opaque, bool islast) {
    memset((void*)&txring[txtail], 0, sizeof(txbd));
    tx_opaque[txtail] = opaque;
    txring[txtail].addr = paddr;
    txring[txtail].info = (len & ZYNQMP_GEM_TXBUF_LEN_MASK) | 
        (islast ? ZYNQMP_GEM_TXBUF_LAST_MASK : 0) |
        ((txtail + 1) % ZYNQMP_GEM_TX_RING_LEN ? 0 :ZYNQMP_GEM_TXBUF_WRAP_MASK);
    txtail = (txtail + 1) % ZYNQMP_GEM_TX_RING_LEN;
}

static void get_txslot(void **opaque) {
    *opaque = &tx_opaque[txhead];
    // FIXME: Do something with info if needed
    if (txring[txhead].info & ZYNQMP_GEM_TXBUF_EXHAUSTED) {
#ifdef ZYNQMP_GEM_DEBUG
        ZYNQMP_GEM_DEBUG("TX buffers exhausted in mid frame\n");
#endif
    }
    txhead = (txhead + 1) % ZYNQMP_GEM_TX_RING_LEN;
}

static void add_rxslot(uint32_t paddr, void *opaque) {
    memset((void*)&rxring[rxtail], 0, sizeof(rxbd));
    rx_opaque[rxtail] = opaque;
    rxring[rxtail].addr = (paddr & ZYNQMP_GEM_RXBUF_ADDR_MASK) |
        ((rxtail + 1) % ZYNQMP_GEM_RX_RING_LEN ? 0 :ZYNQMP_GEM_RXBUF_WRAP_MASK);
    rxring[rxtail].info = 0;
    rxtail = (rxtail + 1) % ZYNQMP_GEM_RX_RING_LEN;
}

static void get_rxslot(void** opaque, size_t *len, bool *last) {
    *opaque = &rx_opaque[rxhead];
    //FIXME: Do someting with info if needed
    *len = rxring[rxhead].info & ZYNQMP_GEM_RXBUF_LEN_MASK;
    *last = rxring[rxhead].info & ZYNQMP_GEM_RXBUF_EOF_MASK;
    rxhead = (rxhead + 1) % ZYNQMP_GEM_RX_RING_LEN;
}


static void get_mac_address_fn(uint8_t *mac)
{
    memcpy(mac, zynqmp_gem_mac, 6);
}
 */

/**
 * \brief Send Ethernet packet.
 *
 * The packet should be a complete Ethernet frame. Nothing is added
 * by the card or the driver.
 *
 */
/*
static errval_t transmit_pbuf_list_fn(struct driver_buffer *buffers, size_t count)
{
    size_t i;
    size_t totallen = 0;

#ifdef ZYNQMP_GEM_DEBUG
    ZYNQMP_GEM_DEBUG("Add buffer callback %d:\n", count);
#endif

    for (i = 0; i < count; i++) {
        totallen += buffers[i].len;
    }

    //FIXME: Copied code. Not sure if I should use txhead here. Remember to invoke cap to get phyaddr to fill into the desc
    for (i = 0; i < count; i++) {
        add_txslot(buffers[i].pa, buffers[i].len, buffers[i].opaque, i == count - 1);
    }

    return SYS_ERR_OK;
}

static uint64_t tx_get_free_slots_fn(void) {
    if (txtail >= txhead) {
        return ZYNQMP_GEM_TX_RING_LEN - (txtail - txhead) - 1; // TODO: could this be off by 1?
    } else {
        return ZYNQMP_GEM_TX_RING_LEN - (txtail + ZYNQMP_GEM_TX_RING_LEN - txhead) - 1; // TODO: off by 1?
    }
}

static bool handle_free_tx_slot_fn(void)
{
    void *opaque;
    get_txslot(&opaque);
    handle_tx_done(opaque);
    return true;
}
 */
/** Callback for net_queue_mgr library. */
/*
static errval_t rx_register_buffer_fn(uintptr_t paddr, void *vaddr, void *opaque) {
    add_rxslot(paddr, opaque);
    return SYS_ERR_OK;
}
 */
/**
 * Callback for net_queue_mgr library. Since we do PIO anyways, we only ever use
 * one buffer.
 */
/*
static uint64_t rx_get_free_slots_fn(void) {
    if (rxtail >= rxhead) {
        return ZYNQMP_GEM_RX_RING_LEN - (rxtail - rxhead) - 1; // TODO: could this be off by 1?
    } else {
        return ZYNQMP_GEM_RX_RING_LEN - (rxtail + ZYNQMP_GEM_RX_RING_LEN - rxhead) - 1; // TODO: off by 1?
    }
}
 */
/**
 * \brief Zynqmp gem IRQ handler
 *
 * This handler assumes the card is at page 0.
 *
 * The order of actions in this function is important. An interrupt
 * needs to be acknowledged to the card first, before reading packets
 * from the card. Otherwise a race between reading packets and newly
 * received packets arises and packet reception can be delayed
 * arbitrarily.
 */
/*
static void zynqmp_gem_interrupt_handler(void* placeholder)
{
    size_t len, pkt_cnt = 0;
    void *opaque;
    bool last;
    struct driver_rx_buffer buf[ZYNQMP_GEM_RX_RING_LEN];

    //FIXME: Read int_status register to see the interrupt type(transmit completed or receive completed)
    get_rxslot(&opaque, &len, &last);
    for (; last;) {
        buf[pkt_cnt].opaque = opaque;
        buf[pkt_cnt].len = len;
        pkt_cnt++;
        get_rxslot(&opaque, &len, &last);
    }
    process_received_packet(buf, pkt_cnt, 0);
}
 */

static void polling_loop(void)
{
    errval_t err;
    struct waitset *ws = get_default_waitset();
    while (1) {
        err = event_dispatch(ws);
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "in event_dispatch");
            break;
        }
    }
}

static void zynqmp_gem_hardware_init(void) {
    int i;
    uint64_t mac_addr;

    zynqmp_gem_initialize(&device, (void *)ZYNQMP_GEM_IOBASE);

    //Disable all interrupts(This reg is reset as disabled, so no need to do this I guess)
    //zynqmp_gem_intdis_wr(&device, 0x7FFFEFF);

    //Clear the network control register
    zynqmp_gem_netctl_wr(&device, 0);

    //Clear the statistics registers
    zynqmp_gem_netctl_casr_wrf(&device, 0x1);

    //Clear the status registers
    zynqmp_gem_rxstat_wr(&device, 0x0F);
    zynqmp_gem_txstat_wr(&device, 0xFF);
    //Clear the buffer queues
    zynqmp_gem_rxqptr_wr(&device, 0);
    zynqmp_gem_txqptr_wr(&device, 0);
    zynqmp_gem_rxq1ptr_wr(&device, 0);
    zynqmp_gem_txq1ptr_wr(&device, 0);

    //Clear mac registers
	for (i = 0; i < 4; i++) {
	    zynqmp_gem_spcaddbt_wr(&device, i, 0);
	    zynqmp_gem_spcaddtp_wr(&device, i, 0);
		// Do not use MATCHx register
	    zynqmp_gem_spctype_wr(&device, i, 0);
	}

    zynqmp_gem_phymng_wr(&device, 0);

    zynqmp_gem_hashbt_wr(&device, 0); 
    zynqmp_gem_hashtp_wr(&device, 0);

    zynqmp_gem_netcfg_fd_wrf(&device, 0x1);
    zynqmp_gem_netcfg_fr_wrf(&device, 0x1);
    zynqmp_gem_netcfg_mcd_wrf(&device, 0x3);
    zynqmp_gem_netcfg_gme_wrf(&device, 0x1);
    zynqmp_gem_netcfg_spd_wrf(&device, 0x1);
    
    //FIXME: use burned-in MAC address instead of random one if possible
    srand((int)time(0));
    mac_addr = ((uint64_t)ZYNQMP_GEM_MAC_PREFIX << 24) | ((uint64_t)rand() % 0x1000000);
    for (i = 0; i < 6; i++) zynqmp_gem_mac[i] = (mac_addr & (0xff << (8 * (5 - i)))) >> (8 * (5 - i));
    zynqmp_gem_spcaddbt_wr(&device, 0, mac_addr & 0xffffffff);
    zynqmp_gem_spcaddtp_addr_wrf(&device, 0, (mac_addr & ((uint64_t)0xffff << 32)) >> 32);

    zynqmp_gem_dmacfg_rbs_wrf(&device, 0x18);
    zynqmp_gem_dmacfg_rps_wrf(&device, 0x3);
    zynqmp_gem_dmacfg_tps_wrf(&device, 0x1);
    zynqmp_gem_dmacfg_abl_wrf(&device, 0x4);

    //FIXME: PHY configuration to be implemented if needed.
}

static errval_t on_create_queue(struct zynqmp_gem_devif_binding *b, struct capref rx, struct capref dummy_rx, struct capref tx, struct capref dummy_tx, uint64_t *mac) {
    errval_t err;
    struct frame_identity frameid = { .base = 0, .bytes = 0 };

    err = invoke_frame_identify(rx, &frameid);
    assert(err_is_ok(err));
    zynqmp_gem_rxqptr_wr(&device, frameid.base);

    err = invoke_frame_identify(dummy_rx, &frameid);
    assert(err_is_ok(err));
    zynqmp_gem_rxq1ptr_wr(&device, frameid.base);

    err = invoke_frame_identify(tx, &frameid);
    assert(err_is_ok(err));
    zynqmp_gem_txqptr_wr(&device, frameid.base);

    err = invoke_frame_identify(dummy_tx, &frameid);
    assert(err_is_ok(err));
    zynqmp_gem_txq1ptr_wr(&device, frameid.base);

    memcpy(mac, zynqmp_gem_mac, sizeof(zynqmp_gem_mac));

    return SYS_ERR_OK;
}

static void export_devif_cb(void *st, errval_t err, iref_t iref)
{
    struct zynqmp_gem_state* s = (struct zynqmp_gem_state*) st;
    const char *suffix = "devif";
    char name[256];

    assert(err_is_ok(err));

    // Build label for interal management service
    sprintf(name, "%s_%s", s->service_name, suffix);

    err = nameservice_register(name, iref);
    assert(err_is_ok(err));
    s->initialized = true;
}

static errval_t connect_devif_cb(void *st, struct zynqmp_gem_devif_binding *b)
{

    b->rpc_rx_vtbl.create_queue_call = on_create_queue;
    b->st = st;
    return SYS_ERR_OK;
}

static void zynqmp_gem_init_mngif(struct zynqmp_gem_state *st)
{
    errval_t err;

    err = zynqmp_gem_devif_export(st, export_devif_cb, connect_devif_cb,
                           get_default_waitset(), 1);
    assert(err_is_ok(err));
}

static void zynqmp_gem_init(void) {
    //errval_t err;

    zynqmp_gem_hardware_init();
    /*err = inthandler_setup_arm(zynqmp_gem_interrupt_handler, NULL, ZYNQMP_GEM_IRQ);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "interrupt setup failed.");
    }*/

    ZYNQMP_GEM_DEBUG("after ethersrv init\n");

    //Enable interrupt
    zynqmp_gem_inten_wr(&device, 0xFFFFFFFF);

    //Enable controller
    zynqmp_gem_netctl_mpe_wrf(&device, 0x1);
    zynqmp_gem_netctl_er_wrf(&device, 0x1);
    zynqmp_gem_netctl_et_wrf(&device, 0x1);

}

int main(int argc, char *argv[])
{
    struct zynqmp_gem_state *st;
  
    ZYNQMP_GEM_DEBUG("Starting xilinx gem driver.....\n");
    
    // Initialize driver
    zynqmp_gem_init();
    st = (struct zynqmp_gem_state*)malloc(sizeof(struct zynqmp_gem_state));
    st->initialized = false;
    st->queue_init_done = false;
    st->service_name = "zynqmp_gem";
    /* For use with the net_queue_manager */

    zynqmp_gem_init_mngif(st);

    ZYNQMP_GEM_DEBUG("Driver started.\n");

    polling_loop();
}
