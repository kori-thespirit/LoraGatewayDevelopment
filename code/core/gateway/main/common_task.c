/* -------------------------- LICENSE placeholder -------------------------- */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtc_ds3231.h"
#include "sdcard.h"
#include "main.h"

static const char *TAG = "common";
TimerHandle_t stimer_common;

void stimer_cb( TimerHandle_t xTimer )
{

}

void common_task(void* pvParameters) {
    // ds3231_init();
    // sdcard_init();
    stimer_common = xTimerCreate("common timer", pdMS_TO_TICKS(100), true, NULL, stimer_cb);
    ESP_LOGI(TAG, "task created");
    for(;;){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
