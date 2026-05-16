/* -------------------------- LICENSE placeholder -------------------------- */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "main.h"

// StaticTask_t xLoraTaskBuffer;
// StackType_t xLoraStack[LORA_STACK_SIZE];

// StaticTask_t xModbusTaskBuffer;
// StackType_t xModbusStack[MODBUS_STACK_SIZE];

static const char *TAG = "main";

void app_main() {

    // TaskHandle_t lora_task_handle = xTaskCreateStatic(
    // lora_task,
    // "lora_task",
    // LORA_STACK_SIZE,
    // NULL,
    // 2,
    // xLoraStack,
    // &xLoraTaskBuffer
    // );
    // if (lora_task_handle == NULL) 
    //   ESP_LOGE(TAG,"Fail to create lora task");
    
    // TaskHandle_t modbus_task_handle = xTaskCreateStatic(
    // modbus_task,
    // "modbus_task",
    // MODBUS_STACK_SIZE,
    // NULL,
    // 1,
    // xModbusStack,
    // &xModbusTaskBuffer
    // );
    // if (modbus_task_handle == NULL) 
    //   ESP_LOGE(TAG,"Fail to create modbus task");

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

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

uint8_t get_available_queue_common()
{
    /* TODO: Change queue index base on free mutex */
    uint8_t idx = 0;
    return idx;
}
// QueueHandle_t * get_queue_common_addr() {return q_common;}
QueueHandle_t * get_queue_common_addr() {return &q_common;}
TaskHandle_t  * get_modbus_task_handle() { return &modbus_task_handle; }
TaskHandle_t  * get_lora_task_handle() { return &lora_task_handle; }
TaskHandle_t  * get_network_handle() { return &network_task_handle; }

