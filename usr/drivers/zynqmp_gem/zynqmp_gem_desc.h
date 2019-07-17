#ifndef __ZYNQMP_GEM_DESC_H__
#define __ZYNQMP_GEM_DESC_H__

#define ZYNQMP_GEM_RX_SOF_MASK	0x00004000
#define ZYNQMP_GEM_RX_EOF_MASK	0x00008000 /* End of frame. */
#define ZYNQMP_GEM_RX_LEN_MASK	0x00001FFF /* Mask for length field */
#define ZYNQMP_GEM_RX_USED_MASK 0x00000001
#define ZYNQMP_GEM_RX_WRAP_MASK	0x00000002 
#define ZYNQMP_GEM_RX_ADDR_MASK	0xFFFFFFFC 

#define ZYNQMP_GEM_TX_LEN_MASK	0x00003FFF
#define ZYNQMP_GEM_TX_WRAP_MASK 0x40000000
#define ZYNQMP_GEM_TX_LAST_MASK 0x00008000 /* Last buffer */
//#define ZYNQMP_GEM_TX_LAST_MASK 0x00002000
#define ZYNQMP_GEM_TX_USED_MASK 0x80000000

typedef struct rx_desc {
    uint32_t addr;
    uint32_t info;
} rx_desc_t;

typedef rx_desc_t tx_desc_t;

#endif