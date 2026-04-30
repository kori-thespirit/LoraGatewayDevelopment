#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "modbus_application_driver.h"

#define PATTERN_CHR_NUM (3)
static char GD20[] = "GD20";
static char MASTER[] = "MASTER";
static QueueHandle_t uart0_queue;
static int uart_queue_size = 20;
static uint16_t current_reading_reg = 0; //Cờ thanh ghi đang đọc để xử lý dữ liệu đúng cách trong hàm uart_event_task
// --- BẢNG TRA CRC16 MODBUS (256 phần tử) ---
static const uint16_t crc_table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

// Hàm tính nhanh bằng bảng tra cứu
// --- HÀM TÍNH TOÁN CRC16 MODBUS SỬ DỤNG BẢNG TRA ---
uint16_t crc16_modbus(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        crc = (crc >> 8) ^ crc_table[(crc ^ byte) & 0xFF];
    }
    
    return crc;
}
void process_inverter_data(uint16_t reg_addr, uint16_t value) {
    switch (reg_addr) {
        case GD20_REG_STATUS:
            switch (value) {
                    case GD20_STATUS_RUN:   ESP_LOGW("STATUS", "Biến tần: ĐANG CHẠY THUẬN");break;
                    case GD20_STATUS_REV:   ESP_LOGW("STATUS", "Biến tần: ĐANG CHẠY NGHỊCH");break;
                    case GD20_STATUS_STOP:  ESP_LOGW("STATUS", "Biến tần: ĐANG DỪNG");break;
                    default:                ESP_LOGE("STATUS", "Biến tần: LỖI/KHÔNG XÁC ĐỊNH (%d)", value);break;

            }
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
void modbud_write_register(uint8_t slave_id,uint16_t reg_addr, uint16_t value) {
    uint8_t frame[8];
    frame[0] = slave_id;
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
esp_err_t modbud_write_register_with_fb(uint8_t slave_id,uint16_t reg_addr, uint16_t value) {
    uint8_t frame[8];
    frame[0] = slave_id;
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
    ESP_LOGI(MASTER, "Gửi lệnh: %02X %02X %04X %04X %02X%02X", slave_id, FUNC_WRITE_REG, reg_addr, value, frame[6], frame[7]);
    uint8_t response[10];
    // Đợi phản hồi (Timeout thường khoảng 100-500ms)
    int len = uart_read_bytes(UART_PORT, response, 8, pdMS_TO_TICKS(500));
     if (len > 0) {
        ESP_LOG_BUFFER_HEX("Giá trị phản hồi:", response, len);
        if (response[1] & GD20_RESP_CODE_FAULT) {
        uint8_t exception_code = response[2];
        switch (exception_code) {
            case GD20_EXC_ILLEGAL_CMD:          ESP_LOGE(GD20, "Lỗi: Lệnh không hợp lệ!"); break;
            case GD20_EXC_ILLEGAL_DATA_ADDR:    ESP_LOGE(GD20, "Lỗi: Địa chỉ data không hợp lệ!"); break;
            case GD20_EXC_ILLEGAL_VALUE:        ESP_LOGE(GD20, "Lỗi: Giá trị dữ liệu gửi đi không hợp lệ!"); break;
            case GD20_EXC_OPERATION_FAILED:     ESP_LOGE(GD20, "Lỗi: Thông số không hợp lệ!"); break;
            case GD20_EXC_PASSWORD_ERROR:       ESP_LOGE(GD20, "Lỗi: Password không đúng!"); break;
            case GD20_EXC_DATA_FRAME_ERROR:     ESP_LOGW(GD20, "Lỗi: Lỗi khung dữ liệu!"); break;
            default:                            ESP_LOGE(GD20, "Lỗi Modbus chưa xác định: 0x%02X", exception_code); break;
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
void modbud_read_single_register(uint8_t slave_id, uint16_t reg_addr, uint8_t count) {
    current_reading_reg = reg_addr;
    uint8_t frame[10];
    frame[0] = slave_id;
    frame[1] = FUNC_READ_REG;
    frame[2] = (reg_addr >> 8) & 0xFF;
    frame[3] = reg_addr & 0xFF;
    frame[4] = (count >> 8) & 0xFF;
    frame[5] = count & 0xFF;

    uint16_t crc = crc16_modbus(frame, 6);
    frame[6] = crc & 0xFF;         // Byte thấp CRC
    frame[7] = (crc >> 8) & 0xFF;  // Byte cao CRC

    int len = uart_write_bytes(UART_PORT, (const char*)frame, 8);
    //   ESP_LOGI(MASTER,"Gửi lệnh đọc: %02X %02X %04X %04X %02X%02X", slave_id, FUNC_READ_REG, reg_addr, count,
    //   frame[6], frame[7]); ESP_LOGI(MASTER,"total write byte: %d", len);

    //   // THÊM DÒNG NÀY: Xóa rác còn sót lại trong bộ đệm UART
    //   uart_flush_input(UART_PORT);

    //   uint8_t response[10];
    //   memset(response, 0, sizeof(response));
    //   len = uart_read_bytes(UART_PORT, (void*)response, sizeof(response), pdMS_TO_TICKS(500));

    //     if (len > 0) {
    //         // 2. Phân tích giá trị
    //         if (response[1] == FUNC_READ_REG) { // Nếu là hàm đọc thành công

    //             uint8_t bytes = response[2];
    //             ESP_LOGI(MASTER, "Số byte dữ liệu nhận được: %d", bytes);
    //             ESP_LOG_BUFFER_HEX("Phản hồi nhận được:", response, len);

    //             uint16_t val = (response[3 + 0] << 8) | response[4 + 0];
    //             process_inverter_data(reg_addr, val);
    //         }
    //         if (response[1] == GD20_RESP_CODE_FAULT) { // Nếu là lỗi (hàm trả về mã lỗi)
    //             uint8_t error_code = response[2];
    //             ESP_LOGE(MASTER, "Biến tần trả về lỗi: Mã lỗi 0x%02X", error_code);
    //         }
    //     } else {
    //         ESP_LOGE(MASTER, "Không nhận được phản hồi từ biến tần!");
    //     }
}
// --- HÀM NGẮT XỬ LÝ SỰ KIỆN (Trái tim của thư viện) ---
void uart_event_task(void* pvParameters) {
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*)malloc(BUF_SIZE);

    for (;;) {
        // Đợi tín hiệu từ hàng đợi (không tốn CPU)
        if (xQueueReceive(uart0_queue, (void*)&event, portMAX_DELAY)) {
            bzero(dtmp, BUF_SIZE);
            switch (event.type) {
                case UART_DATA:
                    // Đọc dữ liệu từ Ring Buffer
                    int len = uart_read_bytes(UART_PORT, dtmp, event.size, pdMS_TO_TICKS(10));
                    if (len >= 5) {
                        // 1. Kiểm tra CRC
                        uint16_t res_crc = (dtmp[len - 1] << 8) | dtmp[len - 2];
                        if (res_crc == crc16_modbus(dtmp, len - 2)) {
                            // 2. Phân tích gói tin dựa trên Function Code
                            if (dtmp[1] == FUNC_READ_REG) {
                                uint16_t val = (dtmp[3] << 8) | dtmp[4];
                                // Gọi hàm xử lý logic bạn đã viết
                                process_inverter_data(current_reading_reg, val);
                            } else if (dtmp[1] == FUNC_WRITE_REG) {
                                ESP_LOGI(GD20, "Ghi thành công thanh ghi 0x%04X", (dtmp[2] << 8) | dtmp[3]);
                            }
                        } else {
                            ESP_LOGE(MASTER, "Sai mã CRC!");
                        }
                    }
                    break;

                case UART_FIFO_OVF:
                    uart_flush_input(UART_PORT);
                    xQueueReset(uart0_queue);
                    break;

                case UART_PARITY_ERR:
                    ESP_LOGE(MASTER, "Lỗi Parity - Kiểm tra nhiễu!");
                    break;

                default:
                    break;
            }
        }
    }
    free(dtmp);
    vTaskDelete(NULL);
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

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, uart_queue_size, &uart0_queue, 0));
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

    // Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(UART_PORT, '+', PATTERN_CHR_NUM, 9, 0, 0);
    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(UART_PORT, 20);
    
    // 5. Bật nguồn module RS485 
    gpio_reset_pin(RS485_EN_PIN);
    gpio_set_direction(RS485_EN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RS485_EN_PIN, 1);
}