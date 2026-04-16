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

static char GD20[] = "GD20";
static char MASTER[] = "MASTER";

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
void process_inverter_data(uint16_t reg_addr, uint16_t value) {
    switch (reg_addr) {
        case GD20_REG_STATUS:
            if (value == 1) ESP_LOGW("STATUS", "Biến tần: ĐANG CHẠY THUẬN");
            else if (value == 2) ESP_LOGW("STATUS", "Biến tần: ĐANG CHẠY NGHỊCH");
            else if (value == 3) ESP_LOGW("STATUS", "Biến tần: ĐANG DỪNG");
            else ESP_LOGE("STATUS", "Biến tần: LỖI/KHÔNG XÁC ĐỊNH (%d)", value);
            break;

        case GD20_OPERATION_FREQ:
            // Tần số thường có tỉ lệ 1/100
            ESP_LOGW("DATA", "Tần số thực tế: %.2f Hz", (float)value / 100.0);
            break;

        case GD20_OUTPUT_CURRENT:
            // Dòng điện thường có tỉ lệ 1/10
            ESP_LOGW("DATA", "Dòng điện: %.1f A", (float)value / 10.0);
            break;

        case GD20_OUTPUT_VOLTAGE:
            ESP_LOGW("DATA", "Điện áp Bus: %d V", value);
            break;

        default:
            ESP_LOGI("DATA", "Thanh ghi 0x%04X: Giá trị = %d", reg_addr, value);
            break;
    }
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
    //ESP_LOGI(TAG, "Gửi lệnh: Reg 0x%04X = 0x%04X (CRC: 0x%04X)", reg_addr, value, crc);
    ESP_LOGI(MASTER, "Gửi lệnh: %02X %02X %04X %04X %02X%02X", GD20_SLAVE_ID, FUNC_WRITE_REG, reg_addr, value, frame[6], frame[7]);
}
esp_err_t modbud_write_register_with_fb(uint16_t reg_addr, uint16_t value) {
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
    //ESP_LOGI(TAG, "Gửi lệnh: Reg 0x%04X = 0x%04X (CRC: 0x%04X)", reg_addr, value, crc);
    ESP_LOGI(MASTER, "Gửi lệnh: %02X %02X %04X %04X %02X%02X", GD20_SLAVE_ID, FUNC_WRITE_REG, reg_addr, value, frame[6], frame[7]);
    uint8_t response[10];
    // Đợi phản hồi (Timeout thường khoảng 100-500ms)
    int len = uart_read_bytes(UART_PORT, response, 8, pdMS_TO_TICKS(500));
     if (len > 0) {
        ESP_LOG_BUFFER_HEX("Giá trị phản hồi:", response, len);
        if (response[1] & 0x80) {
        uint8_t exception_code = response[2];
        switch (exception_code) {
            case 0x01: ESP_LOGE(GD20, "Lỗi: Lệnh không hợp lệ!"); break;
            case 0x02: ESP_LOGE(GD20, "Lỗi: Địa chỉ data không hợp lệ!"); break;
            case 0x03: ESP_LOGE(GD20, "Lỗi: Giá trị dữ liệu gửi đi không hợp lệ!"); break;
            case 0x04: ESP_LOGE(GD20, "Lỗi: Thông số không hợp lệ!"); break;
            case 0x05: ESP_LOGE(GD20, "Lỗi: Password không đúng!"); break;
            case 0x06: ESP_LOGW(GD20, "Lỗi: Lỗi khung dữ liệu!"); break;
            default:   ESP_LOGE(GD20, "Lỗi Modbus chưa xác định: 0x%02X", exception_code); break;
            }
        return ESP_FAIL;
        }
        ESP_LOGI(GD20, "Ghi thành công: Reg 0x%04X = 0x%04X", reg_addr, value);
    return ESP_OK;
    }
    if (len <= 0) {
        ESP_LOGE(GD20, "Lỗi: Không có phản hồi từ Slave %d (Timeout)", GD20_SLAVE_ID);
        return ESP_ERR_TIMEOUT;
    }

    // Kiểm tra CRC của phản hồi
    uint16_t expected_crc = crc16_modbus(response, len - 2);
    uint16_t received_crc = (response[len - 1] << 8) | response[len - 2];

    if (expected_crc != received_crc) {
        ESP_LOGE(GD20, "Lỗi: Sai mã CRC phản hồi!");
        return ESP_ERR_INVALID_CRC;
    }
    return ESP_OK;
}
void modbud_read_single_register( uint16_t reg_addr, uint8_t count) {
  uint8_t frame[10];
  frame[0] = GD20_SLAVE_ID;
  frame[1] = FUNC_READ_REG;
  frame[2] = (reg_addr >> 8) & 0xFF;
  frame[3] = reg_addr & 0xFF;
  frame[4] = (count >> 8) & 0xFF;
  frame[5] = count & 0xFF;

  uint16_t crc = crc16_modbus(frame, 6);
  frame[6] = crc & 0xFF;          // Byte thấp CRC
  frame[7] = (crc >> 8) & 0xFF;   // Byte cao CRC

  int len = uart_write_bytes(UART_PORT, (const char*)frame, 8);
  ESP_LOGI(MASTER,"Gửi lệnh đọc: %02X %02X %04X %04X %02X%02X", GD20_SLAVE_ID, FUNC_READ_REG, reg_addr, count, frame[6], frame[7]);
  ESP_LOGI(MASTER,"total write byte: %d", len);

  // THÊM DÒNG NÀY: Xóa rác còn sót lại trong bộ đệm UART
  uart_flush_input(UART_PORT);
  
  uint8_t response[10];
  memset(response, 0, sizeof(response));
  len = uart_read_bytes(UART_PORT, (void*)response, sizeof(response), pdMS_TO_TICKS(500));

    if (len > 0) {
        // 2. Phân tích giá trị
        if (response[1] == 0x03) { // Nếu là hàm đọc thành công

            uint8_t bytes = response[2];
            ESP_LOGI(MASTER, "Số byte dữ liệu nhận được: %d", bytes);
            ESP_LOG_BUFFER_HEX("Phản hồi nhận được:", response, len);
            
            uint16_t val = (response[3 + 0] << 8) | response[4 + 0];
            process_inverter_data(reg_addr, val);
        }
        if (response[1] == 0x86) { // Nếu là lỗi (hàm trả về mã lỗi)
            uint8_t error_code = response[2];
            ESP_LOGE(MASTER, "Biến tần trả về lỗi: Mã lỗi 0x%02X", error_code);
        }
    } else {
        ESP_LOGE(MASTER, "Không nhận được phản hồi từ biến tần!");
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
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, RTS_DE_PIN, CTS_PIN));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));

    // 3. Cấu hình chân RE (IO15)
    gpio_reset_pin(RE_PIN);
    gpio_set_direction(RE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RE_PIN, 0); // RE=0 để luôn cho phép module nhận dữ liệu từ biến tần

    // 4. Chế độ RS485 Half Duplex (Tự động kéo DE lên 1 khi gửi)
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));

    // 5. Bật nguồn module RS485 
    gpio_reset_pin(RS485_EN_PIN);
    gpio_set_direction(RS485_EN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RS485_EN_PIN, 1);
}