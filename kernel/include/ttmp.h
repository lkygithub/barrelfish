#ifndef __TTMP_H
#define __TTMP_H

#include <kernel.h>
/**
 * time triggered message passing
 */
#define TTMP_MAX_CORE_NUM       8
#define TTMP_MSG_SLOT_SIZE      32 //bytes
#define TTMP_SCH_SLOT_SIZE      8  //bytes

#define TTMP_SCHD_BUFF_SIZE     0x2000 // 8K
#define TTMP_CORE_BUFF_SIZE     0x1000 // 4K

#define TTMP_TX_SLOT_NUM        64
#define TTMP_RX_SLOT_NUM        64
#define TTMP_SET_SLOT_NUM       4
#define TTMP_SCHD_ENTRY_NUM     1024

typedef struct
{
    uint32_t src : 16;
    uint32_t dst : 16;
    uint32_t valid : 1;
    uint32_t size : 15;
    uint32_t id : 16;
} tt_msg_head_t;

typedef struct
{
    unsigned char value[24];
} tt_msg_payload_t;

struct ttmp_msg_buff_slot
{
    tt_msg_head_t head;
    tt_msg_payload_t payload;
};

union ttmp_sch_table_slot {
    uint64_t raw;
    struct {
        uint64_t timestamp : 48;
        uint64_t msg_id : 16;
    } named;
};

struct ttmp_msg_buff_each_core {
    struct ttmp_msg_buff_slot tx_slots[TTMP_TX_SLOT_NUM];
    struct ttmp_msg_buff_slot rx_slots[TTMP_RX_SLOT_NUM];
};

struct ttmp_buff {
    union ttmp_sch_table_slot sch_table[TTMP_SCHD_ENTRY_NUM];
    struct ttmp_msg_buff_each_core cores[0]; // variable length
};

STATIC_ASSERT_SIZEOF(struct ttmp_msg_buff_slot, TTMP_MSG_SLOT_SIZE);
STATIC_ASSERT_SIZEOF(union ttmp_sch_table_slot, TTMP_SCH_SLOT_SIZE);

#endif
