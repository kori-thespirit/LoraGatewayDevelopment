/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _INTERTASK_TYPEDEF_H_
#define _INTERTASK_TYPEDEF_H_
#include <stdint.h>

typedef enum intertask_err{
    INTERTASK_OK,
    INTERTASK_INVALID,
    INTERTASK_TIMEOUT,
} e_intertask_err_t;

typedef struct intertask_noti {

} st_intertask_noti_t;

typedef struct intertask_data{
    int src_task_handle_id;
    void *payload;
} st_intertask_data_t;

#endif // _INTERTASK_TYPEDEF_H_
