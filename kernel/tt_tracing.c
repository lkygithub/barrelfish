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
// Each cores has its own pos, because multi-kernel do not share data
static int pos = 0;

/* static function */
static inline void tt_tracing_add_event(uint64_t timestamp, uint8_t subsys, uint8_t event, uint64_t extra_info)
{
    // check positon
    if (pos == TT_TRACING_SLOT_NUM) {
        printf("##### Error: TT_TRACING slots is full #####\n");
        return;
    }
    // get core buffer
    struct tt_tracing_buff *buff = (struct tt_tracing_buff *) global->ttmp_ctrl_info.tt_tracing_buff;
    struct tt_tracing_buff_each_core *core_buff = &(buff->cores[my_core_id]);
    // create log entry
    tt_tracing_slot_t *slot = &(core_buff->slots[pos]);
    slot->named.timestamp = timestamp;
    slot->named.subsys = subsys;
    slot->named.event = event;
    slot->named.extra_info = extra_info & 0xFFFFFFFFFFFFu;
    // move position
    pos += 1;
}

void tt_tracing_add_kernel_event(uint64_t timestamp, uint8_t subsys, uint8_t event)
{
    // call gerneric function
    // set extra_info with zero
    tt_tracing_add_event(timestamp, subsys, event, 0);
}

void tt_tracing_add_sche_event(uint64_t timestamp, uint8_t subsys, uint8_t event,
                                uint16_t task_id, uint16_t core_id)
{
    uint64_t extra_info = ((core_id & 0xFFFu) << 12) |
                          (task_id & 0xFFFu);
    // call gerneric function
    tt_tracing_add_event(timestamp, subsys, event, extra_info);
}

void tt_tracing_add_ttmp_event(uint64_t timestamp, uint8_t subsys, uint8_t event,
                                uint16_t src_task_id, uint16_t src_core_id,
                                uint16_t dst_task_id, uint16_t dst_core_id)
{
    //fix 'left shift count >= width of type' warning
    uint64_t dst_core_id_long = dst_core_id & 0xFFFu;
    //
    uint64_t extra_info = ((dst_core_id_long) << 36)     |
                          ((dst_task_id & 0xFFFu) << 24) |
                          ((src_core_id & 0xFFFu) << 12) |
                          ((src_task_id & 0xFFFu));
    // call gerneric function
    tt_tracing_add_event(timestamp, subsys, event, extra_info);
}

void tt_tracing_dump_log(void)
{
    // get core buffer
    struct tt_tracing_buff *buff = (struct tt_tracing_buff *) global->ttmp_ctrl_info.tt_tracing_buff;
    struct tt_tracing_buff_each_core *core_buff = &(buff->cores[my_core_id]);
    tt_tracing_slot_t *core_slot_base = (tt_tracing_slot_t *) core_buff;

    //printf("Core %d, Total %d Events.\n", my_core_id, pos);
    for (int i = 0; i < pos; i++) {
        tt_tracing_slot_t *slot = core_slot_base + i;
        printf("%016llx %016llx\n", slot->raw[0], slot->raw[1]);
    }
}
