#include <stdio.h>
#include <string.h> // Thêm thư viện này để dùng hàm sprintf hoặc strlen
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"
#include "driver/gpio.h"

#define Lora_EN 2
#define LED_ACT 13 // Chân LED mặc định trên nhiều board ESP32
void app_main(void)
{
   gpio_reset_pin(LED_ACT);
   gpio_set_direction(LED_ACT, GPIO_MODE_OUTPUT);
   uint8_t buf[255];
   int counter = 0;
   gpio_reset_pin(Lora_EN);
   gpio_set_direction(Lora_EN, GPIO_MODE_OUTPUT);
   gpio_set_level(Lora_EN , 1);
   // 1. Khởi tạo LoRa
   //lora_init();
   ESP_LOGI("CONFIG", "--- Thong so cau hinh LoRa ---");
    
    // In các chân Pin (Thay tên define tương ứng với thư viện bạn dùng)
    // Thông thường là: CONFIG_CS_GPIO, CONFIG_RST_GPIO, v.v.
#ifdef CONFIG_CS_GPIO
    ESP_LOGI("PINS", "CS Pin: %d", CONFIG_CS_GPIO);
    ESP_LOGI("PINS", "RST Pin: %d", CONFIG_RST_GPIO);
    ESP_LOGI("PINS", "MISO Pin: %d", CONFIG_MISO_GPIO);
    ESP_LOGI("PINS", "MOSI Pin: %d", CONFIG_MOSI_GPIO);
    ESP_LOGI("PINS", "SCK Pin: %d", CONFIG_SCK_GPIO);
#else
    ESP_LOGW("PINS", "Khong tim thay dinh nghia chan trong menuconfig!");
#endif

    if (lora_init() == 0) {
        ESP_LOGE("LORA", "Khoi tao THAT BAI! Kiem tra lai day noi.");
    } else {
        ESP_LOGI("LORA", "Khoi tao THANH CONG!");
    }
    lora_set_frequency(433e6);
    lora_enable_crc();
   for(;;) {
      // 2. Chuẩn bị dữ liệu vào mảng buf
      // Hàm sprintf giúp chèn số thứ tự vào chuỗi để bạn dễ theo dõi ở bên nhận
      int len = sprintf((char *)buf, "Data serial: %d", counter);

      gpio_set_level(LED_ACT, 1);

      // 3. Gửi gói tin
      // len là độ dài thực tế của chuỗi, không nhất thiết phải gửi đủ 255 byte
      lora_send_packet(buf, len);

      gpio_set_level(LED_ACT, 0);

      // 4. In log ra màn hình console để kiểm tra
      ESP_LOGI("LoRa_TX", "Sent: %s", buf);

      counter++; // Tăng số đếm cho lần gửi sau
      
      // Đợi 2 giây trước khi gửi gói tiếp theo
      vTaskDelay(2000 / portTICK_PERIOD_MS);
   }
}