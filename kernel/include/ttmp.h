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

STATIC_ASSERT_SIZEOF(struct ttmp_msg_buff_slot, TTMP_MSG_SLOT_SIZE);
STATIC_ASSERT_SIZEOF(union ttmp_sch_table_slot, TTMP_SCH_SLOT_SIZE);

#endif
