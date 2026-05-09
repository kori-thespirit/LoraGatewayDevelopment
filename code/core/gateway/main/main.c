#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdint.h>
#include "esp_log.h"
#include "main.h"

static const char *TAG = "main";

void app_main(void) {
    xTaskCreate(lora_task, "lora_task", 4096, NULL, 10, NULL);
}
