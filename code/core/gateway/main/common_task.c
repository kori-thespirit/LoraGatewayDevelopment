/* -------------------------- LICENSE placeholder -------------------------- */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtc_ds3231.h"
#include "sdcard.h"
#include "main.h"

static const char *TAG = "common";

void common_task(void* pvParameters) {
  ds3231_init();
  sdcard_init();
  ESP_LOGI(TAG, "task created");
  for(;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
