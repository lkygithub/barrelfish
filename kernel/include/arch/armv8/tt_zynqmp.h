#ifndef __TT_H
#define __TT_H

#define TT_TASK_CORE_NUM            3
#define TT_TASK_ENTRY_NUM           512
#define TT_TASK_ENTRY_SIZE          8 // byte

#define TT_TASK_SCHD_CORE_SIZE      ((TT_TASK_ENTRY_SIZE)*(TT_TASK_ENTRY_NUM))
#define TT_TASK_SCHD_SIZE           (((TT_TASK_ENTRY_SIZE)*(TT_TASK_ENTRY_NUM))*(TT_TASK_CORE_NUM))

#endif //_TT_H