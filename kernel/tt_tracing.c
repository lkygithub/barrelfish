/**
 * \file
 * \brief Function for time-triggered tracing
 * \author mwiacx
 * \date 2019-4-24
 */

#include <kernel.h>
#include <global.h>
#include <arch/armv8/tt_tracing_zynqmp.h>

// Record current log position for each cores
static int pos[TT_TRACING_CORE_NUM];

void tt_tracing_init(void *tt_tracing_buff)
{
    /* clean position array */
    for (int i = 0; i < TT_TRACING_CORE_NUM; ++i)
    {
        pos[i] = 0;
    }
    /* clean buffer */
    memset(tt_tracing_buff, 0, TT_TRACING_BUFF_SIZE);
}

/* static function */
static inline void tt_tracing_add_event(uint64_t timestamp, uint8_t subsys, uint8_t event, uint64_t extra_info)
{
    // check positon
    if (pos[my_core_id] == TT_TRACING_SLOT_NUM)
        return;
    // get core buffer
    struct tt_tracing_buff *buff = (struct tt_tracing_buff *) global->ttmp_ctrl_info.tt_tracing_buff;
    struct tt_tracing_buff_each_core core_buff = buff->cores[my_core_id];
    // create log entry
    tt_tracing_slot_t slot = core_buff.slots[pos[my_core_id]];
    slot.named.timestamp = timestamp;
    slot.named.subsys = subsys;
    slot.named.event = event;
    slot.named.extra_info = extra_info & 0xFFFFFFFFFFFF;
    // move position
    pos[my_core_id] += 1;
}

void tt_tracing_add_kernel_event(uint64_t timestamp, uint8_t subsys, uint8_t event)
{
    // call gerneric function
    // set extra_info with zero
    tt_tracing_add_event(timestamp, subsys, event, 0);
}

void tt_tracing_add_sche_event(uint64_t timestamp, uint8_t subsy, uint8_t event,
                                uint16_t task_id, uint16_t core_id)
{
    uint64_t extra_info = ((core_id & 0xFFF) << 12) |
                          (task_id & 0xFFF);
    // call gerneric function
    tt_tracing_add_event(timestamp, subsys, event, extra_info);
}

void tt_tracing_add_ttmp_event(uint64_t timestamp, uint8_t subsys, uint8_t event,
                                uint16_t src_task_id, uint16_t src_core_id,
                                uint16_t dst_task_id, uint16_t dst_core_id)
{
    uint64_t extra_info = ((dst_core_id & 0xFFF) << 36) |
                          ((dst_task_id & 0xFFF) << 24) |
                          ((src_core_id & 0xFFF) << 12) |
                          ((src_task_id & 0xFFF));
    // call gerneric function
    tt_tracing_add_event(timestamp, subsys, evnet, extra_info);
}

void tt_tracing_dump_log(void)
{
    // get core buffer
    struct tt_tracing_buff *buff = (struct tt_tracing_buff *) global->ttmp_ctrl_info.tt_tracing_buff;
    struct tt_tracing_buff_each_core core_buff = buff->cores[my_core_id];
    int logs_num = pos[my_core_id];
    for (int i = 0; i < logs_num; i++) {
        tt_tracing_slot_t slot = core_buff.slots[i];
        printf("%llx %llx", slot.raw[1], slot.raw[0]);
    }
}