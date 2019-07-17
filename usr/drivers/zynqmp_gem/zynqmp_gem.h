#ifndef __ZYNQMP_GEM_H__
#define __ZYNQMP_GEM_H__

#define ZYNQMP_GEM_MAC_PREFIX 0x000A35
#define ZYNQMP_GEM_TX_BUFSIZE 2048 //consisitent with BUFFER_SIZE in interface_raw.c
#define ZYNQMP_GEM_RX_BUFSIZE ZYNQMP_GEM_TX_BUFSIZE //consisitent with BUFFER_SIZE in interface_raw.c
#define ZYNQMP_GEM_INT_TC_MASK 0x00000080
#define ZYNQMP_GEM_INT_RC_MASK 0x00000002
#define ZYNQMP_GEM_INT_TXUSED_MASK 0x00000008
#define ZYNQMP_GEM_INT_RXUSED_MASK 0x00000004
#define ZYNQMP_GEM_N_RX_BUFS 256
#define ZYNQMP_GEM_N_TX_BUFS 256
#define ZYNQMP_GEM0_IRQ 89
#define ZYNQMP_GEM1_IRQ 91
#define ZYNQMP_GEM2_IRQ 93
#define ZYNQMP_GEM3_IRQ 95
#define ZYNQMP_GEM_IRQ ZYNQMP_GEM3_IRQ

struct zynqmp_gem_state {
    bool initialized;

    /* For use with the net_queue_manager */
    char *service_name;
};

#endif //ZYNQ_GEM_H
