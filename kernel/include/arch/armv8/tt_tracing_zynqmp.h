#ifndef __TT_TRACING_ZYNQMP_H
#define __TT_TRACING_ZYNQMP_H
/**
 * \brief time triggered tracing for zynqmp
 * \author mwiacx
 * \date 2019-4-24
 */
#include <tt_tracing.h>

#define TT_TRACING_CORE_NUM             (3+1) // 3 Task core and 1 transfer core

#define TT_TRACING_BUFF_SIZE (TT_TRACING_CORE_NUM * TT_TRACING_CORE_BUFF_SIZE)

struct tt_tracing_buff_fixed {
    struct tt_tracing_buff var;
    struct tt_tracing_buff_each_core cores[TT_TRACING_CORE_NUM];
}

STATIC_ASSERT_SIZEOF(struct tt_tracing_buff_fixed, TT_TRACING_BUFF_SIZE);

#endif //__TT_TRACING_ZYNQMP_H