#include "driver/gpio.h"
#include "esp_err.h"
#include "ethernet_w5500.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdint.h>

#define ESP32C3_SUPERMINI_BUILTIN_BLUE_LED 8
uint8_t data[1500] = {0};

void app_main(void) {
  for (uint16_t i = 0; i < sizeof(data); i++) {
    data[i] = i % 256;
  }
  const gpio_config_t blueled = {.pin_bit_mask =
                                     1 << ESP32C3_SUPERMINI_BUILTIN_BLUE_LED,
                                 .mode = GPIO_MODE_OUTPUT,
                                 .pull_up_en = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                 .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&blueled));
  w5500_init();
  while (1) {
    gpio_set_level(ESP32C3_SUPERMINI_BUILTIN_BLUE_LED, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(ESP32C3_SUPERMINI_BUILTIN_BLUE_LED, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
