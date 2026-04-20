#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "modbus_application_driver.h"

static char TAG[] = "GD20";

// --- HÀM TÍNH TOÁN CRC16 MODBUS ---
uint16_t crc16_modbus(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (int j = 8; j != 0; j--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// --- HÀM GỬI LỆNH GHI THANH GHI (FUNCTION 06) ---
void modbud_write_register(uint16_t reg_addr, uint16_t value) {
    uint8_t frame[8];
    frame[0] = GD20_SLAVE_ID;
    frame[1] = FUNC_WRITE_REG;
    frame[2] = (reg_addr >> 8) & 0xFF;
    frame[3] = reg_addr & 0xFF;
    frame[4] = (value >> 8) & 0xFF;
    frame[5] = value & 0xFF;

    uint16_t crc = crc16_modbus(frame, 6);
    frame[6] = crc & 0xFF;          // Byte thấp CRC
    frame[7] = (crc >> 8) & 0xFF;   // Byte cao CRC

    uart_write_bytes(UART_PORT, (const char*)frame, 8);
    ESP_LOGI(TAG, "Gửi lệnh: Reg 0x%04X = 0x%04X (CRC: 0x%04X)", reg_addr, value, crc);
}

void modbud_read_register(uint8_t slave_id, uint16_t reg_addr, uint8_t count) {
  uint8_t frame[10];
  frame[0] = slave_id;
  frame[1] = FUNC_READ_REG;
  frame[2] = (reg_addr >> 8) & 0xFF;
  frame[3] = reg_addr & 0xFF;
  frame[4] = (count >> 8) & 0xFF;
  frame[5] = count & 0xFF;

  uint16_t crc = crc16_modbus(frame, 6);
  frame[6] = crc & 0xFF;          // Byte thấp CRC
  frame[7] = (crc >> 8) & 0xFF;   // Byte cao CRC

  int len = uart_write_bytes(UART_PORT, (const char*)frame, 8);
  ESP_LOGI("TAG","total write byte: %d", len);

  
  memset(frame, 0, sizeof(frame));
  len = uart_read_bytes(UART_PORT, (void*)frame, sizeof(frame), pdMS_TO_TICKS(500));
  if(len)
    for (uint8_t i = 0; i < sizeof(frame); i++) {
      ESP_LOGI("TAG","frame[%u]: 0x%x", i, frame[i]);
    }
}

void modbus_init() {
    // 1. Cấu hình UART
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    
    // 2. Gán chân và chế độ RS485
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, RTS_DE_PIN, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));
}
