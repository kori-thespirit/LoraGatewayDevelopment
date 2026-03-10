#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"
#include <stdint.h>
uint8_t buf[255] = {0};

void app_main(void) {
  lora_init();
  lora_send_packet(buf, sizeof(buf));
  for (;;) {
    ESP_LOGI("sample", "Hello world");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
