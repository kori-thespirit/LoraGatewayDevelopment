#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define W5500_SCK 4
#define W5500_MISO 5
#define W5500_MOSI 6
#define W5500_CS 7

const gpio_config_t w5500_pin_cfg[4] = {{.pin_bit_mask = 1ULL << W5500_SCK,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE},
                                        {.pin_bit_mask = 1ULL << W5500_MISO,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE},
                                        {.pin_bit_mask = 1ULL << W5500_MOSI,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE},
                                        {.pin_bit_mask = 1ULL << W5500_CS,
                                         .mode = GPIO_MODE_OUTPUT,
                                         .pull_up_en = GPIO_PULLUP_DISABLE,
                                         .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                         .intr_type = GPIO_INTR_DISABLE}};

void app_main(void) {

  for (int i = 0; i < sizeof(w5500_pin_cfg) / sizeof(gpio_config_t); i++) {
    gpio_config(w5500_pin_cfg + i);
  }
  while (1) {
    gpio_set_level(W5500_SCK, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(W5500_SCK, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    gpio_set_level(W5500_MOSI, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(W5500_MOSI, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    gpio_set_level(W5500_MISO, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(W5500_MISO, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    gpio_set_level(W5500_CS, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(W5500_CS, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
