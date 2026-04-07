#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "gd20_modbus_driver.h"
#include <stdint.h>

void app_main(void) {
  ESP_ERROR_CHECK(master_init());
  vTaskDelay(10 / portTICK_PERIOD_MS);
  master_operation_func(NULL);
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
