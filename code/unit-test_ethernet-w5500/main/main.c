#include "ethernet_w5500.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdint.h>

void app_main(void) {
  w5500_init();
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
