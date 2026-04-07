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

static char TAG[] = "GD20";

// Định nghĩa cứng các chân theo sơ đồ của bạn
#define TXD_PIN                (4)
#define RXD_PIN                (3)
#define RTS_DE_PIN             (UART_PIN_NO_CHANGE) // Chân DE
#define RE_PIN                 (UART_PIN_NO_CHANGE) // Chân RE riêng
#define CTS_PIN                (UART_PIN_NO_CHANGE)
#define RS485_EN_PIN           5

#define UART_PORT              (UART_NUM_1)
#define BAUD_RATE              (9600) 
#define BUF_SIZE               (127)

// Thông số Modbus của biến tần
#define GD20_SLAVE_ID        0x01
#define FUNC_WRITE_REG       0x06
#define FUNC_READ_REG        0x03
#define GD20_REG_CONTROL_CMD      0x2000  // Thanh ghi lệnh chạy/dừng
#define GD20_REG_SET_FREQ         0x2001  // Thanh ghi cài tần số
#define GD20_REG_ID               0x2103  // GD20 ID code

// Các mã lệnh điều khiển (Value cho thanh ghi 0x2000)
#define CMD_FWD     0x0001
#define CMD_REV     0x0002
#define CMD_STOP    0x0005

// 1. Mã chạy động cơ chiều thuận: 01 06 20 00 00 01 43 CA
// const uint8_t CMD_RUN_FWD[]  = {0x01, 0x06, 0x20, 0x00, 0x00, 0x01, 0x43, 0xCA};

// // 2. Mã dừng động cơ: 01 06 20 00 00 05 42 09
// const uint8_t CMD_STOP[]     = {0x01, 0x06, 0x20, 0x00, 0x00, 0x05, 0x42, 0x09};

// // 3. Mã ghi tần số 50Hz: 01 06 20 01 13 88 DE 9C
// const uint8_t CMD_SET_50HZ[] = {0x01, 0x06, 0x20, 0x01, 0x13, 0x88, 0xDE, 0x9C};

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
void invt_write_register(uint16_t reg_addr, uint16_t value) {
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

void gd20_read_register(uint8_t slave_id, uint16_t reg_addr, uint8_t count) {
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

static void invt_task(void *arg) {
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


    ESP_LOGI(TAG, "Khởi tạo thành công. Bắt đầu điều khiển biến tần...");
    gd20_read_register(GD20_SLAVE_ID, GD20_REG_ID, 1);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void) {
    // Tạo Task điều khiển
    xTaskCreate(invt_task, "invt_task", 4096, NULL, 10, NULL);
}
