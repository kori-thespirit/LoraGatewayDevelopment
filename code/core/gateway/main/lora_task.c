/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"
#include "main.h"

#define RF_FREQUENCY 433000000LL  // Tần số: 433MHz (hoặc 866000000LL, 915000000LL)
#define LORA_BW 7                 // Bandwidth: 7 là 125kHz
#define LORA_SF 7                 // Spreading Factor: 7
#define LORA_CR 1                 // Coding Rate: 1 là 4/5
#define LORA_CRC 1                // 1: Bật CRC, 0: Tắt CRC
                          
static const char *TAG = "lora";

void lora_task(void* pvParameters) {

    if (lora_init() == 0) {
        ESP_LOGE(TAG, "Does not recognize the module");
        while (1) {
            vTaskDelay(1);
        }
    }

    lora_set_frequency(RF_FREQUENCY);
    lora_enable_crc();
    lora_set_coding_rate(LORA_CR);
    ESP_LOGI(TAG, "coding_rate=%d", LORA_CR);

    lora_set_bandwidth(LORA_BW);
    ESP_LOGI(TAG, "bandwidth=%d", LORA_BW);

    lora_set_spreading_factor(LORA_SF);
    ESP_LOGI(TAG, "Start");
    uint8_t buf[255];  // Maximum Payload size of SX1276/77/78/79 is 255
    while (1) {
        lora_receive();  // put into receive mode
        if (lora_received()) {
            int rx_len = lora_receive_packet(buf, sizeof(buf));
            ESP_LOGI(TAG, "%d byte packet received:[%.*s]", rx_len, rx_len, buf);
        }
        vTaskDelay(10);  // Avoid WatchDog alerts
    }  // end while

}
