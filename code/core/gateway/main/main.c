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

void app_main(void) {

    TaskHandle_t lora_task_handle = xTaskCreateStatic(
    lora_task,      
    "lora_task",    
    LORA_STACK_SIZE,
    NULL,           
    2,
    xLoraStack,    
    &xLoraTaskBuffer      
    );
    if (lora_task_handle == NULL) 
      ESP_LOGE(TAG,"Fail to create lora task");

    /* Highspeed CPU core to handle network task */
    BaseType_t ret = xTaskCreatePinnedToCore(
            network_task,          // Task function
            "network_task",        // Name for debugging
            4096,               // Stack size in words
            NULL,               // Task input parameter
            3,                  // Priority (higher number = higher priority)
            NULL,               // Task handle
            1                   // Core ID (0 or 1)
    );
    if (ret != pdPASS) 
      ESP_LOGE(TAG,"Fail to create network task");

    for(;;){
        daemon_task(NULL);
    }
}
