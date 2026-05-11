/* -------------------------- LICENSE placeholder -------------------------- */
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdint.h>
#include "esp_log.h"
#include "main.h"

static const char *TAG = "main";

StaticTask_t xLoraTaskBuffer;
StackType_t xLoraStack[LORA_STACK_SIZE];

StaticTask_t xCommonTaskBuffer;
StackType_t xCommonStack[COMMON_STACK_SIZE];

typedef enum task_priority
{
  TASK_PRIORITY_DAEMON = 0,
  TASK_PRIORITY_OTHER,
  TASK_PRIORITY_COMMON,
  TASK_PRIORITY_LORA,
  TASK_PRIORITY_NETWORK,
} task_priority;

void other_task(void* pvParameters) ;

void app_main(void) {

    TaskHandle_t lora_task_handle = xTaskCreateStatic(
    lora_task,
    "lora_task",
    LORA_STACK_SIZE,
    NULL,
    TASK_PRIORITY_LORA,
    xLoraStack,
    &xLoraTaskBuffer
    );
    if (lora_task_handle == NULL) 
      ESP_LOGE(TAG,"Fail to create lora task");

    TaskHandle_t common_task_handle = xTaskCreateStatic(
    common_task,
    "common_task",
    COMMON_STACK_SIZE,
    NULL,
    TASK_PRIORITY_COMMON,
    xCommonStack,
    &xCommonTaskBuffer
    );
    if (common_task_handle == NULL) 
      ESP_LOGE(TAG,"Fail to create common task");

    /* Highspeed CPU core to handle network task */
    BaseType_t ret = xTaskCreatePinnedToCore(
            network_task,          // Task function
            "network_task",        // Name for debugging
            4096,               // Stack size in words
            NULL,               // Task input parameter
            TASK_PRIORITY_NETWORK,                  // Priority (higher number = higher priority)
            NULL,               // Task handle
            1                   // Core ID (0 or 1)
    );
    if (ret != pdPASS) 
      ESP_LOGE(TAG,"Fail to create network task");

    xTaskCreate(other_task, "other_task", 1024 * 2, NULL, TASK_PRIORITY_OTHER, NULL);

    for(;;){
        daemon_task(NULL);
    }
}

void other_task(void* pvParameters) {

    while (1) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

