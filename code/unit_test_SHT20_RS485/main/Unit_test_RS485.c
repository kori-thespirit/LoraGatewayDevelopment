#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static char TAG[] = "SHT20_RS485";

// Định nghĩa cứng các chân theo sơ đồ của bạn
#define TXD_PIN (17)
#define RXD_PIN (14)
#define RTS_DE_PIN (16)  // Chân DE
#define RE_PIN (15)      // Chân RE riêng
#define CTS_PIN (UART_PIN_NO_CHANGE)
#define RS485_EN_PIN (12)

#define UART_PORT (UART_NUM_2)
#define BAUD_RATE (9600)
#define BUF_SIZE (127)

static void sht20_task(void* arg) {
    // In log thông tin cấu hình chân ra màn hình monitor
    ESP_LOGI(TAG, "--- ĐANG KHỞI TẠO CẤU HÌNH RS485 ---");
    ESP_LOGI(TAG, "UART Port: %d", UART_PORT);
    ESP_LOGI(TAG, "TXD Pin  : GPIO %d (Nối vào DI của module)", TXD_PIN);
    ESP_LOGI(TAG, "RXD Pin  : GPIO %d (Nối vào RO của module)", RXD_PIN);
    ESP_LOGI(TAG, "DE Pin   : GPIO %d (Nối vào DE của module - Điều khiển gửi)", RTS_DE_PIN);
    ESP_LOGI(TAG, "RE Pin   : GPIO %d (Nối vào RE của module - Điều khiển nhận)", RE_PIN);
    ESP_LOGI(TAG, "Baudrate : %d", BAUD_RATE);
    ESP_LOGI(TAG, "------------------------------------");

    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 1. Cài đặt Driver
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

    // 2. Gán chân GPIO (5 tham số chuẩn ESP-IDF v5.x)
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, RTS_DE_PIN, CTS_PIN));

    // 3. Cấu hình chân RE (IO15)
    gpio_reset_pin(RE_PIN);
    gpio_set_direction(RE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RE_PIN, 0);  // RE=0 để luôn cho phép module nhận dữ liệu từ SHT20

    // 4. Chế độ RS485 Half Duplex (Tự động kéo DE lên 1 khi gửi)
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));

    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);
    // Khung lệnh đọc liên tục 0x01 04 00 01 00 02 (CRC 20 0B)
    const uint8_t read_cmd[] = {0x01, 0x04, 0x00, 0x01, 0x00, 0x02, 0x20, 0x0B};

    while (1) {
        // Gửi yêu cầu
        uart_write_bytes(UART_PORT, (const char*)read_cmd, sizeof(read_cmd));

        // Đợi 500ms để nhận đủ 9 byte phản hồi
        int len = uart_read_bytes(UART_PORT, data, BUF_SIZE, pdMS_TO_TICKS(500));

        if (len >= 9 && data[1] == 0x04) {
            int16_t raw_temp = (data[3] << 8) | data[4];
            float temp = raw_temp / 10.0;

            uint16_t raw_humid = (data[5] << 8) | data[6];
            float humid = raw_humid / 10.0;

            ESP_LOGI(TAG, "Dữ liệu: %.1f°C - %.1f%%", temp, humid);
        } else {
            ESP_LOGW(TAG, "Không có phản hồi (len=%d). Kiểm tra dây A/B!", len);
            if (len > 0) esp_log_buffer_hex("DATA_RAW", data, len);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    free(data);
}

void app_main(void) {
    gpio_reset_pin(RS485_EN_PIN);
    gpio_set_direction(RS485_EN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(RS485_EN_PIN, 1);
    xTaskCreate(sht20_task, "sht20_task", 4096, NULL, 10, NULL);
}