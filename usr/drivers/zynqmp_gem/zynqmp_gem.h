#ifndef __ZYNQMP_GEM_H__
#define __ZYNQMP_GEM_H__

#define ZYNQMP_GEM_MAC_PREFIX 0x000A35
#define ZYNQMP_GEM_TX_BUFSIZE 2048 //consisitent with BUFFER_SIZE in interface_raw.c
#define ZYNQMP_GEM_RX_BUFSIZE ZYNQMP_GEM_TX_BUFSIZE //consisitent with BUFFER_SIZE in interface_raw.c
#define ZYNQMP_GEM_INT_TC_MASK 0x00000080
#define ZYNQMP_GEM_INT_RC_MASK 0x00000002
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
