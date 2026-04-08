#ifndef MODBUS_APP_DRIVER_H
#define MODBUS_APP_DRIVER_H

#include "gd20_inverter.h"

#define TXD_PIN                (4)
#define RXD_PIN                (3)
#define RTS_DE_PIN             (UART_PIN_NO_CHANGE) // Chân DE
#define RE_PIN                 (UART_PIN_NO_CHANGE) // Chân RE riêng
#define CTS_PIN                (UART_PIN_NO_CHANGE)
#define RS485_EN_PIN           5

#define UART_PORT              (UART_NUM_1)
#define BAUD_RATE              (9600) 
#define BUF_SIZE               (127)

#define FUNC_WRITE_REG       0x06
#define FUNC_READ_REG        0x03

void modbus_init();
void modbud_read_register(uint8_t slave_id, uint16_t reg_addr, uint8_t count);
void modbud_write_register(uint16_t reg_addr, uint16_t value);
#endif // !MODBUS_APP_DRIVER_H
