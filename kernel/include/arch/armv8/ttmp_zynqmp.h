#ifndef __TTMP_ZYNQMP_H
#define __TTMP_ZYNQMP_H

#include <ttmp.h>

#define TTMP_TASK_CORE_NUM          3
#define TTMP_TRAN_CORE_NUM          1

#define TTMP_BUFF_SIZE (TTMP_SCHD_BUFF_SIZE + \
                        (TTMP_TASK_CORE_NUM * TTMP_CORE_BUFF_SIZE))
/*
#define TTMP_BUFF_BASE 0xFFFF0000FF000000LLU

#define TTMP_SCHD_BUFF_BASE TTMP_BUFF_BASE
#define TTMP_BUFF_BASE_N(core_id) \
    (TTMP_BUFF_BASE + TTMP_SCHD_BUFF_SIZE + TTMP_CORE_BUFF_SIZE * (core_id))
#define TTMP_BUFF_BASE_N_TX(core_id) TTMP_BUFF_BASE_N(core_id)
#define TTMP_BUFF_BASE_N_RX(core_id) \
    (TTMP_BUFF_BASE_N(core_id) + TTMP_CORE_BUFF_SIZE / 2)
*/
struct ttmp_buff_fixed {
    struct ttmp_buff var;
    struct ttmp_msg_buff_each_core cores[TTMP_TASK_CORE_NUM];
};

STATIC_ASSERT_SIZEOF(struct ttmp_buff_fixed, TTMP_BUFF_SIZE);

#endif //_TTMP_ZYNQMP_H
