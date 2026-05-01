#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include "gd20_inverter.h"
#include "modbus_application_driver.h"
#include "esp_log.h"

static const char *TAG = "MAIN_APP";

void invt_task(void* arg) {
    
    vTaskDelay(pdMS_TO_TICKS(500));

    // --- GIAI ĐOẠN 1: KHỞI TẠO THÔNG SỐ BIẾN TẦN ---
    ESP_LOGI(TAG, "Cài đặt tần số 40Hz và Chạy thuận...");
    modbud_write_register(GD20_SLAVE_ID, GD20_REG_SET_FREQ, GD20_SET_FREQ_30HZ);  
    vTaskDelay(pdMS_TO_TICKS(100));                                 

    modbud_write_register(GD20_SLAVE_ID, GD20_REG_CONTROL_CMD, GD20_CONTROL_CMD_FWD);
    vTaskDelay(pdMS_TO_TICKS(100));

    int count = 0;
    // --- GIAI ĐOẠN 2: CHẠY TASK BIẾN TẦN ---

    while (1) {
        // Lấy thông tin tần số hoạt động của biến tần
        modbud_read_single_register(GD20_SLAVE_ID, GD20_OPERATION_FREQ, 1);
        vTaskDelay(pdMS_TO_TICKS(5000));  // Đợi biến tần phản hồi và giãn cách gói tin

        // Lấy thông tin dòng điện hoạt động của biến tần
        modbud_read_single_register(GD20_SLAVE_ID, GD20_OUTPUT_CURRENT, 1);
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Lấy thông tin điện áp đầu ra của biến tần
        modbud_read_single_register(GD20_SLAVE_ID, GD20_OUTPUT_VOLTAGE, 1);
        vTaskDelay(pdMS_TO_TICKS(5000));
        count++;
        if (count == 1) {
            ESP_LOGW(TAG, "!!! Đã đủ thời gian, ra lệnh STOP động cơ !!!");
            modbud_write_register(GD20_SLAVE_ID, GD20_REG_CONTROL_CMD, GD20_CONTROL_CMD_STOP);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if (count > 2) {
            ESP_LOGE(TAG, "Kết thúc chương trình test.");
            break;  // Thoát khỏi vòng lặp while
        }
        
    }
    vTaskDelete(NULL);
}
void app_main(void) {
    modbus_init();
    vTaskDelay(pdMS_TO_TICKS(100));  
    xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL,12, NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreate(invt_task, "invt_task", 4096, NULL, 10, NULL);
}