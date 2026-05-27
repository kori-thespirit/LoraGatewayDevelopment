/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef MODBUS_APP_DRIVER_H
#define MODBUS_APP_DRIVER_H

#include "gd20_inverter.h"
#include <stdint.h>
#include "esp_err.h"

#define TXD_PIN                (5)
#define RXD_PIN                (6)
#define RTS_DE_PIN             (UART_PIN_NO_CHANGE) 
#define RE_PIN                 (UART_PIN_NO_CHANGE) 
#define CTS_PIN                (UART_PIN_NO_CHANGE)
#define RS485_EN_PIN           12

#define UART_PORT              (UART_NUM_1)
#define BAUD_RATE              (9600) 
#define BUF_SIZE               (127)

typedef void (* p_modbus_tx_complete_cb)(void *pvParameters);
typedef void (* p_modbus_rx_complete_cb)(void *pvParameters);
typedef void (* p_modbus_error_cb      )(void *pvParameters);

typedef enum {
    MODBUS_FUNC_W = 0x06,
    MODBUS_FUNC_R = 0x03,
} e_modbus_function_t;

void modbus_init();
esp_err_t modbus_register_callback(
            void (* p_modbus_tx_complete_cb)(void *),
            void (* p_modbus_rx_complete_cb)(void *),
            void (* p_modbus_error_cb)      (void *));
esp_err_t modbud_write_register_with_fb(uint8_t slave_id,uint16_t reg_addr, uint16_t value);
esp_err_t modbus_send(e_modbus_function_t modbus_func, uint8_t slave_id, uint16_t reg_addr, uint8_t count);
esp_err_t uart_event_handle();
#endif // !MODBUS_APP_DRIVER_H
