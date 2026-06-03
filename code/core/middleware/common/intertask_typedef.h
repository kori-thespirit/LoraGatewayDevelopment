/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _INTERTASK_TYPEDEF_H_
#define _INTERTASK_TYPEDEF_H_
#include <stdint.h>

typedef enum intertask_err{
    INTERTASK_OK,
    INTERTASK_INVALID,

} e_intertask_err_t;

typedef struct modbus_intertask{
    uint8_t addr;
    uint8_t is_request;
    uint16_t reg;
    int src_task_handle_id;
    uint16_t payload;
} st_modbus_intertask_t;

#endif // _INTERTASK_TYPEDEF_H_
