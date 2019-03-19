#ifndef __ZYNQMP_GEM_DESC_H__
#define __ZYNQMP_GEM_DESC_H__

#define ZYNQMP_GEM_RXBUF_EOF_MASK	0x00008000 /* End of frame. */
#define ZYNQMP_GEM_RXBUF_LEN_MASK	0x00001FFF /* Mask for length field */
#define ZYNQMP_GEM_RXBUF_OWN_MASK   0x00000001
#define ZYNQMP_GEM_RXBUF_WRAP_MASK	0x00000002 
#define ZYNQMP_GEM_RXBUF_ADDR_MASK	0xFFFFFFFC 

#define ZYNQMP_GEM_TXBUF_WRAP_MASK  0x40000000
#define ZYNQMP_GEM_TXBUF_LAST_MASK  0x00008000 /* Last buffer */
#define ZYNQMP_GEM_TXBUF_USED_MASK  0x80000000 
#define ZYNQMP_GEM_TXBUF_LEN_MASK	0x00003FFF
#define ZYNQMP_GEM_TXBUF_EXHAUSTED  0x08000000

typedef struct tx_desc {
    uint32_t addr;
    uint32_t info;
} tx_desc_t;

typedef tx_desc_t rx_desc_t;

#endif