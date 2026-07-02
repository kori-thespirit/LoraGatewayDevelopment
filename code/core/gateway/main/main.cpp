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

TaskHandle_t hmi_task_handle;
TaskHandle_t lora_task_handle;
TaskHandle_t common_task_handle;
TaskHandle_t network_task_handle;
// static QueueHandle_t q_common;
static QueueHandle_t q_common[TASK_ID_MAX];

typedef enum task_priority
{
    TASK_PRIORITY_DAEMON = 0,
    TASK_PRIORITY_OTHER,
    TASK_PRIORITY_COMMON,
    TASK_PRIORITY_LORA,
    TASK_PRIORITY_NETWORK,
} task_priority;

#ifdef __cplusplus
extern "C" {
#endif

void other_task(void* pvParameters) ;

void app_main(void) {

    for(uint8_t i = 0; i < TOTAL_IDX(q_common); i++) {
        q_common[i] = xQueueCreate(QUEUE_COMMON_ITEMS, sizeof(st_core_data_t));
        if(NULL == q_common[i]) {
            ESP_LOGE(TAG,"Fail to create queue: %u", i);
        }
    }
    // q_common = xQueueCreate(QUEUE_COMMON_ITEMS, sizeof(st_intertask_data_t));
    lora_task_handle = xTaskCreateStatic(
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

    
    BaseType_t err = xTaskCreate(
    hmi_task,
    "hmi_task",
    4096,
    NULL,
    TASK_PRIORITY_COMMON,
    &hmi_task_handle
    );
    if (err != pdPASS) 
      ESP_LOGE(TAG,"Fail to create HMI task");
    

    
    common_task_handle = xTaskCreateStatic(
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
            network_task,           // Task function
            "network_task",         // Name for debugging
            4096,                   // Stack size in words
            NULL,                   // Task input parameter
            TASK_PRIORITY_NETWORK,  // Priority (higher number = higher priority)
            &network_task_handle,    // Task handle
            1                       // Core ID (0 or 1)
    );
    if (ret != pdPASS) 
      ESP_LOGE(TAG,"Fail to create network task");
    
    

    // xTaskCreate(other_task, "other_task", 1024 * 2, NULL, TASK_PRIORITY_OTHER, NULL);

    for(;;){
        daemon_task(NULL);
    }
}

void other_task(void* pvParameters) {

    while (1) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

uint8_t get_available_queue_common()
{
    /* TODO: Change queue index base on free mutex */
    uint8_t idx = 0;
    return idx;
}
// QueueHandle_t * get_queue_common_addr() {return &q_common;}
QueueHandle_t * get_queue_common_addr() {return q_common;}
TaskHandle_t  * get_lora_task_handle() { return &lora_task_handle; }
TaskHandle_t  * get_network_handle() { return &network_task_handle; }
TaskHandle_t  * get_hmi_handle() { return &hmi_task_handle; }
TaskHandle_t  * get_common_handle() { return &common_task_handle; }

#ifdef __cplusplus
}
#endif
