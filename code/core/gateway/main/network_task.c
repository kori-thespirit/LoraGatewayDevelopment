/* -------------------------- LICENSE placeholder -------------------------- */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"

static const char *TAG = "network";

void network_task(void* pvParameters) {
  ESP_LOGI(TAG, "task created");
  for(;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
