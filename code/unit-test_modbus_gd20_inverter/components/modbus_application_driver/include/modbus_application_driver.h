#ifndef MODBUS_APP_DRIVER_H
#define MODBUS_APP_DRIVER_H

#include "gd20_inverter.h"

#define TXD_PIN                (17)
#define RXD_PIN                (14)
#define RTS_DE_PIN             (16) // Chân DE
#define RE_PIN                 (15) // Chân RE riêng
#define CTS_PIN                (UART_PIN_NO_CHANGE)
#define RS485_EN_PIN           12

#define UART_PORT              (UART_NUM_2)
#define BAUD_RATE              (9600) 
#define BUF_SIZE               (127)

#define FUNC_WRITE_REG       0x06
#define FUNC_READ_REG        0x03

void modbus_init();
void modbud_read_single_register( uint8_t slave_id,uint16_t reg_addr, uint8_t count);
void modbud_write_register(uint8_t slave_id,uint16_t reg_addr, uint16_t value);
esp_err_t modbud_write_register_with_fb(uint8_t slave_id,uint16_t reg_addr, uint16_t value);
void process_inverter_data(uint16_t reg_addr, uint16_t value);
void uart_event_task(void* pvParameters);
void gd20_check_identity();
#endif // !MODBUS_APP_DRIVER_H