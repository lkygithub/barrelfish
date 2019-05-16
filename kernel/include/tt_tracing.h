#ifndef __TT_TRACING_H
#define __TT_TRACING_H
/**
 * \brief time triggered tracing
 * \author mwiacx
 * \date 2019-4-24
 */
#include <kernel.h>

#define TT_TRACING_SLOT_NUM           1024
#define TT_TRACING_ENTRY_SIZE         16 // bytes
#define TT_TRACING_CORE_BUFF_SIZE (TT_TRACING_ENTRY_SIZE * TT_TRACING_SLOT_NUM)

typedef union {
    uint64_t raw[2];
    struct {
        uint64_t timestamp;
        uint64_t extra_info : 48;
        uint64_t event : 8;
        uint64_t subsys : 8;
    } named;
} tt_tracing_slot_t;

struct tt_tracing_buff_each_core {
    tt_tracing_slot_t slots[TT_TRACING_SLOT_NUM];
};

struct tt_tracing_buff {
    struct tt_tracing_buff_each_core cores[0]; // variable length
};

STATIC_ASSERT_SIZEOF(struct tt_tracing_buff_each_core, TT_TRACING_CORE_BUFF_SIZE);
STATIC_ASSERT_SIZEOF(tt_tracing_slot_t, TT_TRACING_ENTRY_SIZE);

/**
 * Subsys and Event defination
 */
#define TT_TRACING_SUBSYS_KERNEL                0
#define TT_TRACING_SUBSYS_SCHE                  1
#define TT_TRACING_SUBSYS_TTMP                  2
// Kernel Event
#define TT_TRACING_KERNEL_LOG_BUFF_INIT_ERR     0
#define TT_TRACING_KERNEL_TTMP_BUFF_INIT_ERR    1
#define TT_TRACING_KERNEL_CORE_INIT_DONE        2
#define TT_TRACING_KERNEL_SYS_SYNC_DONE         3
// Scheduler Event
#define TT_TRACING_SCHE_TASK_RELEASE            0
//#define TT_TRACING_SCHE_TASK_RUN                1
#define TT_TRACING_SCHE_TASK_YEILD              2
#define TT_TRACING_SCHE_TASK_FINISH             3
// TTMP Event
#define TT_TRACING_TTMP_SEND                    0
#define TT_TRACING_TTMP_RECEIVE                 1
#define TT_TRACING_TTMP_TRANSFER                2

/**
 * Interface for tracing event
 */
// init tt-tracing
// void tt_tracing_init(void *tt_tracing_buff);
// dump all logs
void tt_tracing_dump_log(void);
/*
void tt_tracing_add_event(uint64_t timestamp, uint8_t subsys, unit8_t event,
                    uint64_t extra_info);
*/
// kernel event
void tt_tracing_add_kernel_event(uint64_t timestamp, uint8_t subsys,
                    uint8_t event);
// sche event: task_id is 12 bit and core_id is 12bit.
void tt_tracing_add_sche_event(uint64_t timestamp, uint8_t subsys, uint8_t event,
                    uint16_t task_id, uint16_t core_id);
// ttmp event: src_task_id, src_core_id, dst_task_id and dst_core_id is all 12 bit.
void tt_tracing_add_ttmp_event(uint64_t timestamp, uint8_t subsys, uint8_t event,
                    uint16_t src_task_id, uint16_t src_core_id,
                    uint16_t dst_task_id, uint16_t dst_core_id);


#endif //__TT_TRACING_H
