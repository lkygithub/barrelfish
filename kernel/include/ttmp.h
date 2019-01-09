#ifndef __TTMP_H
#define __TTMP_H

#include <kernel.h>
/**
 * time triggered message passing
 */
#define TTMP_MAX_CORE_NUM       8
#define TTMP_MSG_SLOT_SIZE      32 //bytes
#define TTMP_SCH_SLOT_SIZE      8  //bytes
/* TODO: address in aarch64 */
#FIXME
#define TTMP_BUFF_BASE          0xFF000000
#define TTMP_SCHD_BUFF_SIZE     0x10000 // 64K
#define TTMP_CORE_BUFF_SIZE     0x1000 // 4K

#define TTMP_SCHD_BUFF_BASE TTMP_BUFF_BASE
#define TTMP_BUFF_BASE_N(core_id) \
        (TTMP_BUFF_BASE + TTMP_SCHD_BUFF_SIZE + TTMP_CORE_BUFF_SIZE * (core_id))
#define TTMP_BUFF_BASE_N_TX(core_id) TTMP_BUFF_BASE_N(core_id)
#define TTMP_BUFF_BASE_N_RX(core_id) \
        (TTMP_BUFF_BASE_N(core_id) + TTMP_CORE_BUFF_SIZE / 2)

union ttmp_sch_table_slot {
    uint64_t raw;
    struct {
        uint64_t msg_id : 16;
        uint64_t timestramp : 48;
    } named;
};

struct ttmp_msg_buff_slot {
    unsigned char bytes[32];
};

struct ttmp_msg_buff_each_core {
    struct ttmp_msg_buff_slot slots[128];
};

struct ttmp_buff {
    union ttmp_sch_table_slot sch_table[1024];
    struct ttmp_msg_buff_each_core cores[0]; // variable length
};

STATIC_ASSERT(sizeof(struct ttmp_msg_buff_slot) == TTMP_MSG_SLOT_SIZE);
STATIC_ASSERT(sizeof(union ttmp_sch_table_slot) == TTMP_SCH_SLOT_SIZE);

extren strcut ttmp_buff *global_ttmp_buff;

#endif