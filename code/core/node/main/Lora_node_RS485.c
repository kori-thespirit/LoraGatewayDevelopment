/* The example of ESP-IDF
 *
 * This sample code is in the public domain.
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"

#define RF_FREQUENCY 433000000LL  // Tần số: 433MHz (hoặc 866000000LL, 915000000LL)
#define LORA_BW 7                 // Bandwidth: 7 là 125kHz
#define LORA_SF 7                 // Spreading Factor: 7
#define LORA_CR 1                 // Coding Rate: 1 là 4/5
#define LORA_CRC 1                // 1: Bật CRC, 0: Tắt CRC

#define Lora_EN 2
#define LED_ACT 13  // Chân LED mặc định trên nhiều board ESP32

void task_tx(void* pvParameters) {
    ESP_LOGI(pcTaskGetName(NULL), "Start");
    uint8_t buf[255];  // Maximum Payload size of SX1276/77/78/79 is 255
    while (1) {
        TickType_t nowTick = xTaskGetTickCount();
        int send_len = sprintf((char*)buf, "Hello World!! %" PRIu32, nowTick);
        // --- DÒNG LOG THÊM VÀO ĐÂY ---
        ESP_LOGI(pcTaskGetName(NULL), "Sending packet: [%s] (length: %d)", buf, send_len);
        // -----------------------------
        lora_send_packet(buf, send_len);
        ESP_LOGI(pcTaskGetName(NULL), "%d byte packet sent...", send_len);
        int lost = lora_packet_lost();
        if (lost != 0) {
            ESP_LOGW(pcTaskGetName(NULL), "%d packets lost", lost);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }  // end while

    // never reach here
    vTaskDelete(NULL);
}

void app_main() {
    gpio_reset_pin(LED_ACT);
    gpio_set_direction(LED_ACT, GPIO_MODE_OUTPUT);

    gpio_reset_pin(Lora_EN);
    gpio_set_direction(Lora_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(Lora_EN, 1);
    // Initialize LoRa
    if (lora_init() == 0) {
        ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the module");
        while (1) {
            vTaskDelay(1);
        }
    }
    lora_set_frequency(RF_FREQUENCY);

    lora_enable_crc();

    lora_set_coding_rate(LORA_CR);
    // lora_set_coding_rate(CONFIG_CODING_RATE);
    // cr = lora_get_coding_rate();
    ESP_LOGI(pcTaskGetName(NULL), "coding_rate=%d", LORA_CR);

    lora_set_bandwidth(LORA_BW);
    // lora_set_bandwidth(CONFIG_BANDWIDTH);
    // int bw = lora_get_bandwidth();
    ESP_LOGI(pcTaskGetName(NULL), "bandwidth=%d", LORA_BW);

    lora_set_spreading_factor(LORA_SF);
    // lora_set_spreading_factor(CONFIG_SF_RATE);
    // int sf = lora_get_spreading_factor();
    ESP_LOGI(pcTaskGetName(NULL), "spreading_factor=%d", LORA_SF);

    xTaskCreate(&task_tx, "TX", 1024 * 3, NULL, 5, NULL);
}
