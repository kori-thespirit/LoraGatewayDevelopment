/* The example of ESP-IDF
 *
 * This sample code is in the public domain.
 */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"

#define RF_FREQUENCY 433000000LL  // Tần số: 433MHz (hoặc 866000000LL, 915000000LL)
#define LORA_BW 7                 // Bandwidth: 7 là 125kHz
#define LORA_SF 7                 // Spreading Factor: 7
#define LORA_CR 1                 // Coding Rate: 1 là 4/5
#define LORA_CRC 1                // 1: Bật CRC, 0: Tắt CRC

void task_rx(void* pvParameters) {
    ESP_LOGI(pcTaskGetName(NULL), "Start");
    uint8_t buf[255];  // Maximum Payload size of SX1276/77/78/79 is 255
    while (1) {
        lora_receive();  // put into receive mode
        if (lora_received()) {
            int rxLen = lora_receive_packet(buf, sizeof(buf));
            ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received:[%.*s]", rxLen, rxLen, buf);
        }
        vTaskDelay(1);  // Avoid WatchDog alerts
    }  // end while

    // never reach here
    vTaskDelete(NULL);
}

void app_main() {
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
    xTaskCreate(&task_rx, "RX", 1024 * 3, NULL, 5, NULL);
}
