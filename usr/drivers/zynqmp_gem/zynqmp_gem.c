/**
 * \file
 * \brief ZYNQMP GEM driver.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <net_queue_manager/net_queue_manager.h>
#include <barrelfish/inthandler.h>
#include "zynqmp_gem.h"
#include <dev/zynqmp_gem_dev.h>

static zynqmp_gem_t device;
static uint64_t assumed_queue_id = 0; // queue_id that will be initialized

/** Capability for TX ring */
static struct capref txring_frame;
static struct capref dummy_txring_frame;

/** Capability for RX ring */
static struct capref rxring_frame;
static struct capref dummy_rxring_frame;

volatile static txbd *txring;
volatile static rxbd *rxring;
volatile static txbd *dummy_txring;
volatile static rxbd *dummy_rxring;
volatile static void **tx_opaque;
volatile static void **rx_opaque;
static uint8_t txhead, txtail;
static uint8_t rxhead, rxtail;


static uint8_t zynqmp_gem_mac[6];

//this functions polls all the client's channels as well as the transmit and
//receive descriptor rings
static void polling_loop(void)
{
    errval_t err;
    struct waitset *ws = get_default_waitset();
    while (1) {
        err = event_dispatch(ws);
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "in event_dispatch\n");
            break;
        }
#if ZYNQMP_GEM_DEBUG
        //ZYNQMP_GEM_DEBUG("inside event dispatch\n");
#endif
/*        notify_client_next_free_tx();
*/
    }
}


static errval_t alloc_map_frame(vregion_flags_t attr, size_t size, lvaddr_t *va, struct capref *frame) {
    errval_t err;
    err = frame_alloc(frame, size, NULL);
    if (err_is_fail(err)) {
        return err;
    }
    err = vspace_map_one_frame_attr((void**)&va, size, *frame, attr, NULL, NULL);
    if (err_is_fail(err)) {
        return err;
    }
    return SYS_ERR_OK;
}

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

/** Tell card driver to stop this queue. */
static void terminate_queue(void)
{
    errval_t err;
    err = vspace_unmap((void*)txring);
    assert(err_is_ok(err));
    err = vspace_unmap((void*)dummy_txring);
    assert(err_is_ok(err));
    err = vspace_unmap((void*)rxring);
    assert(err_is_ok(err));
    err = vspace_unmap((void*)dummy_rxring);
    assert(err_is_ok(err));
    err = cap_delete(txring_frame);
    assert(err_is_ok(err));
    err = cap_delete(dummy_txring_frame);
    assert(err_is_ok(err));
    err = cap_delete(rxring_frame);
    assert(err_is_ok(err));
    err = cap_delete(dummy_rxring_frame);
    assert(err_is_ok(err));
    exit(0);
}
/**
 * \brief Send Ethernet packet.
 *
 * The packet should be a complete Ethernet frame. Nothing is added
 * by the card or the driver.
 *
 */
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

/** Callback for net_queue_mgr library. */
static errval_t rx_register_buffer_fn(uintptr_t paddr, void *vaddr, void *opaque) {
    add_rxslot(paddr, opaque);
    return SYS_ERR_OK;
}

/**
 * Callback for net_queue_mgr library. Since we do PIO anyways, we only ever use
 * one buffer.
 */
static uint64_t rx_get_free_slots_fn(void) {
    if (rxtail >= rxhead) {
        return ZYNQMP_GEM_RX_RING_LEN - (rxtail - rxhead) - 1; // TODO: could this be off by 1?
    } else {
        return ZYNQMP_GEM_RX_RING_LEN - (rxtail + ZYNQMP_GEM_RX_RING_LEN - rxhead) - 1; // TODO: off by 1?
    }
}

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

static errval_t zynqmp_gem_queue_init(void) {
    size_t tx_size, rx_size;
    errval_t err;
    struct frame_identity fid;

    tx_size = sizeof(txbd) * ZYNQMP_GEM_TX_RING_LEN;
    alloc_map_frame(VREGION_FLAGS_READ_WRITE_NOCACHE, tx_size, (lvaddr_t*)txring, &txring_frame);
    memset((void*)txring, 0, tx_size);
    txhead = txtail = 0;
    err = invoke_frame_identify(txring_frame, &fid);
    if (err_is_fail(err)) {
        return err;
    }
    zynqmp_gem_txqptr_wr(&device, fid.base);

    alloc_map_frame(VREGION_FLAGS_READ_WRITE_NOCACHE, sizeof(txbd), (lvaddr_t*)dummy_txring, &dummy_txring_frame);
    memset((void*)dummy_txring, 0, sizeof(txbd));
    dummy_txring[0].info = ZYNQMP_GEM_TXBUF_WRAP_MASK |
        ZYNQMP_GEM_TXBUF_LAST_MASK |
        ZYNQMP_GEM_TXBUF_USED_MASK;
    err = invoke_frame_identify(txring_frame, &fid);
    if (err_is_fail(err)) {
        return err;
    }
    zynqmp_gem_txq1ptr_wr(&device, fid.base);


    rx_size = sizeof(rxbd) * ZYNQMP_GEM_RX_RING_LEN;
    alloc_map_frame(VREGION_FLAGS_READ_WRITE, rx_size, (lvaddr_t*)rxring, &rxring_frame);
    memset((void*)rxring, 0, rx_size);
    rxhead = rxtail = 0;
    err = invoke_frame_identify(rxring_frame, &fid);
    if (err_is_fail(err)) {
        return err;
    }
    zynqmp_gem_rxqptr_wr(&device, fid.base);

    alloc_map_frame(VREGION_FLAGS_READ_WRITE, sizeof(rxbd), (lvaddr_t*)dummy_rxring, &dummy_rxring_frame);
    memset((void*)dummy_rxring, 0, sizeof(rxbd));
    dummy_rxring[0].addr = ZYNQMP_GEM_RXBUF_WRAP_MASK |
        ZYNQMP_GEM_RXBUF_OWN_MASK;
    err = invoke_frame_identify(rxring_frame, &fid);
    if (err_is_fail(err)) {
        return err;
    }
    zynqmp_gem_rxq1ptr_wr(&device, fid.base);
    return SYS_ERR_OK;
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

static void zynqmp_gem_init(void) {
    char *service_name = "zynqmp_gem";
    errval_t err;

    zynqmp_gem_queue_init();
    zynqmp_gem_hardware_init();
    err = inthandler_setup_arm(zynqmp_gem_interrupt_handler, NULL, ZYNQMP_GEM_IRQ);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "interrupt setup failed.");
    }
	ethersrv_init(service_name, assumed_queue_id, get_mac_address_fn, terminate_queue,
            transmit_pbuf_list_fn, tx_get_free_slots_fn, handle_free_tx_slot_fn,
            ZYNQMP_GEM_PKTSZ_ALIGN, rx_register_buffer_fn, rx_get_free_slots_fn);

#if ZYNQMP_GEM_DEBUG
    ZYNQMP_GEM_DEBUG("after ethersrv init\n");
#endif  

    //Enable interrupt
    zynqmp_gem_inten_wr(&device, 0xFFFFFFFF);

    //Enable controller
    zynqmp_gem_netctl_mpe_wrf(&device, 0x1);
    zynqmp_gem_netctl_er_wrf(&device, 0x1);
    zynqmp_gem_netctl_et_wrf(&device, 0x1);

}

int main(int argc, char *argv[])
{
    int i;
  
#if ZYNQMP_GEM_DEBUG
    ZYNQMP_GEM_DEBUG("Starting xilinx gem driver.....\n");
#endif

    // Process commandline arguments
    for (i = 1; i < argc; i++) {
        ethersrv_argument(argv[i]);
    }
    
    printf("Starting GEM for hardware\n");
    
    while(1);
    // Initialize driver
    zynqmp_gem_init();

#if ZYNQMP_GEM_DEBUG
    ZYNQMP_GEM_DEBUG("registered driver\n");
#endif

    polling_loop(); //loop myself
}
