#ifndef MODBUS_PAYLOAD_HANDLE_H
#define MODBUS_PAYLOAD_HANDLE_H

#include "modbus_application_driver.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum modbus_permission {
    PERM_READ,
    PERM_WRITE,
    PERM_READWRITE,
}e_modbus_permission_t;

typedef enum modbus_param_type {
    MB_PARAM_HOLDING = 0x00,            /*!< Modbus Holding register. */
    MB_PARAM_INPUT,                     /*!< Modbus Input register. */
    MB_PARAM_COIL,                      /*!< Modbus Coils. */
    MB_PARAM_DISCRETE,                  /*!< Modbus Discrete bits. */
    MB_PARAM_COUNT,
    MB_PARAM_CUSTOM,                    /*!< Modbus custom commands (is not counted in area descriptors). */
    MB_PARAM_UNKNOWN = 0xFF
} e_modbus_param_type_t;

typedef enum modbus_payload_err {
    MB_PAYLOAD_OK,
    MB_PAYLOAD_ERR_CALLBACK_IS_NULL,
    MB_PAYLOAD_ERR_RANGE_INVALID,
    MB_PAYLOAD_ERR_DEVICE_NOT_FOUND,
    MB_PAYLOAD_ERR_MODBUS_TYPE,
    MB_PAYLOAD_ERR_DESCRIPTOR_NOT_FOUND,
    MB_PAYLOAD_ERR_DATA_SIZE_MISMACTH,
    MB_PAYLOAD_ERR_ADDRESS_MISMACTH,
    MB_PAYLOAD_ERR_DATA_INVALID,
    MB_PAYLOAD_ERR_PERMISSON,
} e_modbus_payload_err_t ;

typedef struct modbus_params_range{
    int min;
    int max;
}st_modbus_params_range_t;

typedef struct modbus_params_descriptor {
    uint8_t size; // In word
    uint16_t reg; // register address
    const char *unit;
    st_modbus_params_range_t range;
    e_modbus_param_type_t type;
    e_modbus_permission_t perm;
} st_modbus_params_descriptor_t;

typedef struct modbus_device_info {
    uint8_t address;
    char *name;
    uint8_t total_idx;
    const st_modbus_params_descriptor_t *desc; 
}st_modbus_device_info_t;

typedef void (* p_modbus_tx_complete_cb)(void *pvParameters);
typedef void (* p_modbus_rx_complete_cb)(void *pvParameters, const st_modbus_device_info_t *devinfo );
typedef void (* p_modbus_error_cb      )(void *pvParameters);

void m_modbus_payload_handle(uint8_t *modbus_payload, bool is_send);
e_modbus_payload_err_t m_modbus_register_callback(
            void (* p_modbus_tx_complete_cb)(void *),
            void (* p_modbus_rx_complete_cb)(void *, const st_modbus_device_info_t *devinfo),
            void (* p_modbus_error_cb)      (void *));
#endif // MODBUS_PAYLOAD_HANDLE_H
