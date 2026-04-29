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

/*static void invt_task(void *arg) {
  modbus_init();
  ESP_LOGI(TAG, "Khởi tạo thành công. Bắt đầu điều khiển biến tần...");

  // Chờ UART ổn định
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "Starting inverter control sequence...");

  while (1) {
    // --- TRƯỜNG HỢP 1: KIỂM TRA TRẠNG THÁI HIỆN TẠI ---
        ESP_LOGW(TAG, ">>> TEST 1: Kiểm tra trạng thái và thông số trước khi chạy");
        modbud_read_single_register(GD20_SLAVE_ID,GD20_REG_STATUS, 1);      // Đọc trạng thái (2100H)
        vTaskDelay(pdMS_TO_TICKS(500));
        modbud_read_single_register(GD20_SLAVE_ID,GD20_OPERATION_FREQ, 1); // Đọc tần số thực (3000H)
        vTaskDelay(pdMS_TO_TICKS(500));
        modbud_read_single_register(GD20_SLAVE_ID, GD20_OUTPUT_CURRENT, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        // --- TRƯỜNG HỢP 2: CÀI ĐẶT TẦN SỐ 50HZ ---
        ESP_LOGW(TAG, ">>> TEST 2: Cài đặt tần số mục tiêu 50Hz");
        modbud_write_register_with_fb(GD20_SLAVE_ID,GD20_REG_SET_FREQ, 50000);
        vTaskDelay(pdMS_TO_TICKS(500));

        ESP_LOGW(TAG, ">>> TEST 3: Cài đặt tần số mục tiêu 50Hz");
        modbud_write_register_with_fb(GD20_SLAVE_ID,GD20_REG_SET_FREQ, 5000);
        vTaskDelay(pdMS_TO_TICKS(500));

        // --- TRƯỜNG HỢP 3: LỆNH CHẠY THUẬN ---
        ESP_LOGW(TAG, ">>> TEST 4: Lệnh chạy thuận (FWD)");
        modbud_write_register(GD20_SLAVE_ID,GD20_REG_CONTROL_CMD, GD20_CONTROL_CMD_FWD); // Gửi lệnh chạy thuận
        vTaskDelay(pdMS_TO_TICKS(10000));
        modbud_read_single_register(GD20_SLAVE_ID,GD20_OPERATION_FREQ, 1); // Xem tần số đang lên chưa
        vTaskDelay(pdMS_TO_TICKS(500));
        modbud_read_single_register(GD20_SLAVE_ID,GD20_OUTPUT_CURRENT, 1); // Xem dòng điện
        vTaskDelay(pdMS_TO_TICKS(500));

        // --- TRƯỜNG HỢP 4: THAY ĐỔI TẦN SỐ KHI ĐANG CHẠY (25HZ) ---
        ESP_LOGW(TAG, ">>> TEST 5: Giảm tốc xuống 25Hz");
        modbud_write_register(GD20_SLAVE_ID,GD20_REG_SET_FREQ, GD20_SET_FREQ_25HZ); // Gửi lệnh thay đổi tần số
        vTaskDelay(pdMS_TO_TICKS(500));
        modbud_read_single_register(GD20_SLAVE_ID,GD20_OPERATION_FREQ, 1); // Xem tần số đã giảm chưa
        vTaskDelay(pdMS_TO_TICKS(500));

        // --- TRƯỜNG HỢP 5: DỪNG ĐỘNG CƠ ---
        ESP_LOGW(TAG, ">>> TEST 6: Lệnh dừng động cơ (STOP)");
        modbud_write_register(GD20_SLAVE_ID,GD20_REG_CONTROL_CMD, GD20_CONTROL_CMD_STOP);
        vTaskDelay(pdMS_TO_TICKS(10000));
      
        ESP_LOGW(TAG, "--- KẾT THÚC 1 CHU KỲ TEST - NGHỈ 10 GIÂY ---");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
  
 //vTaskDelete(NULL);
  
}*/
void invt_task(void* arg) {
    
    vTaskDelay(pdMS_TO_TICKS(500));

    // --- GIAI ĐOẠN 1: LỆNH ĐIỀU KHIỂN BAN ĐẦU ---
    ESP_LOGI(TAG, "Cài đặt tần số 40Hz và Chạy thuận...");
    modbud_write_register(GD20_SLAVE_ID, GD20_REG_SET_FREQ, 4000);  // 40.00 Hz
    vTaskDelay(pdMS_TO_TICKS(100));                                 // Nghỉ ngắn giữa các lệnh Modbus

    modbud_write_register(GD20_SLAVE_ID, GD20_REG_CONTROL_CMD, GD20_CONTROL_CMD_FWD);
    vTaskDelay(pdMS_TO_TICKS(100));
    int count = 0;
    // --- GIAI ĐOẠN 2: VÒNG LẶP GIÁM SÁT (Polling Loop) ---
    // Trong khi biến tần đang quay, ta liên tục hỏi các thông số
    while (1) {
        // Hỏi Tần số thực tế
        modbud_read_single_register(GD20_SLAVE_ID, GD20_OPERATION_FREQ, 1);
        vTaskDelay(pdMS_TO_TICKS(200));  // Đợi biến tần phản hồi và giãn cách gói tin
        count++;
        // // Hỏi Dòng điện
        // modbud_read_single_register(GD20_SLAVE_ID, GD20_OUTPUT_CURRENT, 1);
        // vTaskDelay(pdMS_TO_TICKS(200));

        // // Hỏi Điện áp Bus
        // modbud_read_single_register(GD20_SLAVE_ID, GD20_OUTPUT_VOLTAGE, 1);
        // vTaskDelay(pdMS_TO_TICKS(200));
        if (count == 20) {
            ESP_LOGW(TAG, "!!! Đã đủ thời gian, ra lệnh STOP động cơ !!!");
            modbud_write_register(GD20_SLAVE_ID, GD20_REG_CONTROL_CMD, GD20_CONTROL_CMD_STOP);
        }

        if (count > 40) {
            ESP_LOGE(TAG, "Kết thúc chương trình test.");
            break;  // Thoát khỏi vòng lặp while
        }
        // Bạn có thể thêm logic thay đổi tốc độ tại đây nếu cần
        // Ví dụ: if (button_pressed) modbud_write_register(...);
    }
    vTaskDelete(NULL);
}
void app_main(void) {
    modbus_init();
    vTaskDelay(pdMS_TO_TICKS(100));  // Nghỉ một chút cho ổn định điện áp/UART
    xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL,12, NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
    // Tạo Task điều khiển
    xTaskCreate(invt_task, "invt_task", 4096, NULL, 10, NULL);
}