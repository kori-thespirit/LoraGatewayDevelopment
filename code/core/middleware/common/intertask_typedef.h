/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _INTERTASK_TYPEDEF_H_
#define _INTERTASK_TYPEDEF_H_
#include <stdint.h>

typedef struct modbus_intertask{
    uint8_t addr;
    uint8_t is_request;
    uint16_t reg;
    uint8_t payload[50];
} st_modbus_intertask_t;

#endif // _INTERTASK_TYPEDEF_H_
