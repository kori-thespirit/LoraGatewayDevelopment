/* -------------------------- LICENSE placeholder -------------------------- */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"

StaticTask_t xLoraTaskBuffer;
StackType_t xLoraStack[LORA_STACK_SIZE];

StaticTask_t xModbusTaskBuffer;
StackType_t xModbusStack[MODBUS_STACK_SIZE];

static const char *TAG = "main";

void app_main() {
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

    TaskHandle_t modbus_task_handle = xTaskCreateStatic(
    modbus_task,
    "modbus_task",
    MODBUS_STACK_SIZE,
    NULL,
    1,
    xModbusStack,
    &xModbusTaskBuffer
    );
    if (modbus_task_handle == NULL) 
      ESP_LOGE(TAG,"Fail to create modbus task");

}
