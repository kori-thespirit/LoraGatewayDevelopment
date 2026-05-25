#ifndef MODBUS_PAYLOAD_HANDLE_H
#define MODBUS_PAYLOAD_HANDLE_H

#include "modbus_application_driver.h"
#include "node_device_typedef.h"
#include "gd20_inverter.h"

typedef enum {
    MODBUS_DEVICE_INVERTER_GD20 = 1,
    MODBUS_DEVICE_SHT20,
} e_modbus_device_t;

typedef struct modbus_device_info {
    uint8_t id;
    e_modbus_device_t device_type;
    void *reg;

}st_modbus_device_info_t;

void m_modbusph_payload_handle(void *payload, size_t payload_size);
#endif // MODBUS_PAYLOAD_HANDLE_H
