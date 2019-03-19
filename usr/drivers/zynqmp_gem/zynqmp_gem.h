#ifndef __ZYNQMP_GEM_H__
#define __ZYNQMP_GEM_H__

#define ZYNQMP_GEM_IOBASE 0xFF0E0000
#define ZYNQMP_GEM_MAC_PREFIX 0x000A35
#define ZYNQMP_GEM_RX_BUFSIZE 1536
#define ZYNQMP_GEM_PKTSZ_ALIGN ZYNQMP_GEM_RX_BUFSIZE
#define ZYNQMP_GEM_N_RX_BUFS 256
#define ZYNQMP_GEM_N_TX_BUFS 256
#define ZYNQMP_GEM_IRQ  95

struct zynqmp_gem_state {
    bool initialized;
    bool queue_init_done;

    /* For use with the net_queue_manager */
    char *service_name;
};

#endif //ZYNQ_GEM_H
