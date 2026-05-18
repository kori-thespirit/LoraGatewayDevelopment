/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"
#include "lora_protocol.h"
#include "main.h"

#define RF_FREQUENCY 433000000LL  // Tần số: 433MHz (hoặc 866000000LL, 915000000LL)
#define LORA_BW 7                 // Bandwidth: 7 là 125kHz
#define LORA_SF 7                 // Spreading Factor: 7
#define LORA_CR 1                 // Coding Rate: 1 là 4/5
#define LORA_CRC 1                // 1: Bật CRC, 0: Tắt CRC
                          
static const char *TAG = "lora_task";
void pack_complete(void *pvParameters);
void parse_complete(void *pvParameters);
static void test_receive();

void lora_task(void* pvParameters) {

    e_lora_protocol_err_t protocol_err = m_lora_protocol_register_callback(&pack_complete, &parse_complete);
    if(LORA_PROTOCOL_ERR_OK != protocol_err)
      ESP_LOGE(TAG, "register callback failed");

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
    while (1) {
        test_receive();
        vTaskDelay(10);  // Avoid WatchDog alerts
    }  // end while

}

static void test_receive()
{
    uint8_t buffer[20] = {0};
    lora_receive();  // put into receive mode
    if (lora_received()) {
        uint8_t rx_len = lora_receive_packet(buffer, sizeof(buffer));
        m_lora_protocol_frame_parse(buffer, rx_len);
    }
}


void pack_complete(void *pvParameters)
{
    ESP_LOGI(TAG, "Pack Callback");
}

void parse_complete(void *pvParameters)
{
    // st_sensor_sht20_t *sht20 = (st_sensor_sht20_t*)pvParameters;
    // ESP_LOGI(TAG, "Receive SHT20 complete: temperature:%.2f, humidity:%.2f", sht20->temperature, sht20->humidity);
    st_inverter_gd20_t *gd20 = (st_inverter_gd20_t*)pvParameters;
    ESP_LOGI(TAG, "Receive GD20 complete: frequency:%.2f, speed:%.2f, out_i:%.2f, out_v:%.2f", gd20->frequency, gd20->speed, gd20->out_i, gd20->out_v);
}
