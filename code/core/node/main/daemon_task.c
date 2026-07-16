/* -------------------------- LICENSE placeholder -------------------------- */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
void daemon_task(void* pvParameters) {
      vTaskDelay(pdMS_TO_TICKS(1000));
}
