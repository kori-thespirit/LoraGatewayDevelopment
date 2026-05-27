/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "driver/gpio.h"
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
                                  //
#define Lora_EN 2
#define LED_ACT 13

static const char *TAG = "lora_task";
static void test_send();
void pack_complete(void *pvParameters);
void parse_complete(void *pvParameters);

void lora_task(void* pvParameters) {
    /*đăng ký callback để nhận thông báo khi đóng gói hoặc phân tích khung hoàn tất*/
  e_lora_protocol_err_t protocol_err = m_lora_protocol_register_callback(&pack_complete, &parse_complete); 
  if(LORA_PROTOCOL_ERR_OK != protocol_err)
    ESP_LOGE(TAG, "register callback failed");
  gpio_reset_pin(LED_ACT);
  gpio_set_direction(LED_ACT, GPIO_MODE_OUTPUT);

  gpio_reset_pin(Lora_EN);
  gpio_set_direction(Lora_EN, GPIO_MODE_OUTPUT);
  gpio_set_level(Lora_EN, 1);
  // Initialize LoRa
  if (lora_init() == 0) {
      ESP_LOGE(TAG, "Does not recognize the module");
      while (1) {
          vTaskDelay(1);
      }
  }
  lora_set_frequency(RF_FREQUENCY);

  lora_enable_crc();

  lora_set_coding_rate(LORA_CR);
  ESP_LOGI( TAG, "coding_rate=%d", LORA_CR);

  lora_set_bandwidth(LORA_BW);
  ESP_LOGI(TAG, "bandwidth=%d", LORA_BW);

  lora_set_spreading_factor(LORA_SF);
  ESP_LOGI(TAG, "spreading_factor=%d", LORA_SF);
  while (1) {
      test_send();
      int lost = lora_packet_lost();
      if (lost != 0) {
          ESP_LOGW(TAG, "%d packets lost", lost);
      }
      vTaskDelay(pdMS_TO_TICKS(3000));
  }  // end while

  // never reach here
  vTaskDelete(NULL);
}

static void test_send()
{
    uint8_t buffer[100] = {0};
    uint8_t payload[128] = {0};
    for (uint8_t i = 0; i < sizeof(payload); i++) {
        payload[i] = i;
    }
    e_lora_protocol_err_t protocol_err = m_lora_protocol_frame_pack((void*)buffer, sizeof(buffer), (void*)&payload, sizeof(payload), 1, 0);
    if(LORA_PROTOCOL_ERR_OK != protocol_err) {
        ESP_LOGE(TAG, "Pack frame data failed, refuse to send");
        return;
    }
    lora_send_packet(buffer, sizeof(buffer));
}

void pack_complete(void *pvParameters)
{
    uint8_t *frame_length = (uint8_t*)pvParameters;
    ESP_LOGI(TAG, "Sending message via lora complete with %u bytes", *frame_length);
}

void parse_complete(void *pvParameters)
{
    ESP_LOGI(TAG, "Parse Callback");
}
