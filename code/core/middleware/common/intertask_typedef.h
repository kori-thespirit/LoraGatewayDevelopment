/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _INTERTASK_TYPEDEF_H_
#define _INTERTASK_TYPEDEF_H_
#include <stdint.h>
#include "data_typedef.h"

typedef enum intertask_err{
    INTERTASK_OK,
    INTERTASK_ERR_TASKHANDLE,
    INTERTASK_ERR_TIMEOUT,
    INTERTASK_ERR_ABORT,
    INTERTASK_ERR_NOT_USE,
} e_intertask_err_t;

typedef union intertask_noti {
    struct {
        uint32_t intertask_err: 8;
        uint32_t qidx:8; // queue index
        uint32_t is_request:1; // Avoid replying to false intertask
        uint32_t reserve:15; // reserve for future use
    }notivalue;
    uint32_t value;
} u_intertask_noti_t;

typedef struct intertask_data{
    int src_task_handle_id; // TODO: move this to u_intertask_noti_t in the future and remove this st_intertask_data_t
    st_core_data_t coredata;
} st_intertask_data_t;

#endif // _INTERTASK_TYPEDEF_H_
