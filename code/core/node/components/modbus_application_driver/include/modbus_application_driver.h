/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef MODBUS_APP_DRIVER_H
#define MODBUS_APP_DRIVER_H

#include "gd20_inverter.h"
#include <stdint.h>
#include "esp_err.h"

#define TXD_PIN                (17)
#define RXD_PIN                (14)
#define RTS_DE_PIN             (16) // Ch├ón DE
#define RE_PIN                 (15) // Ch├ón RE ri├¬ng
#define CTS_PIN                (UART_PIN_NO_CHANGE)
#define RS485_EN_PIN           12

#define UART_PORT              (UART_NUM_1)
#define BAUD_RATE              (9600) 
#define BUF_SIZE               (127)


typedef enum {
    MB_FUNC_W = 0x06,
    MB_FUNC_R = 0x03,
} e_modbus_function_t;

void modbus_init();
esp_err_t modbus_send(e_modbus_function_t modbus_func, uint8_t slave_id, uint16_t reg_addr, uint16_t payload);
esp_err_t modbus_uart_event_handle();
#endif // !MODBUS_APP_DRIVER_H
