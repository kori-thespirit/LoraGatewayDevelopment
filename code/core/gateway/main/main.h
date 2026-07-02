/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _MAIN_H_
#define _MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../intertask_typedef.h"
#include "../data_typedef.h"
#include "gd20_inverter.h"
#include "sht20_sensor.h"
#include "modbus_application_driver.h"

/* Dimensions of the buffer that the task being created will use as its stack.
   NOTE: This is the number of words the stack will hold, not the number of
   bytes. For example, if each stack item is 32-bits, and this is set to 200,
   then 800 bytes (200 * 32-bits) will be allocated. */
#define LORA_STACK_SIZE 3072
#define COMMON_STACK_SIZE 3072

#define QUEUE_COMMON_ITEMS 5

#define TOTAL_IDX(_ARRAY_) (sizeof(_ARRAY_)/sizeof(_ARRAY_[0]))

/* NOTE: Can try the idea of using OR bit for supported task ID to handle instead of using switch case*/
typedef enum task_handle_id{
    TASK_ID_NETWORK,
    TASK_ID_LORA,
    TASK_ID_HMI,
    TASK_ID_COMMON,
    TASK_ID_MAX,
} e_task_handle_id_t;


#ifdef __cplusplus
extern "C" {
#endif

uint8_t lora_get_dest_addr();
esp_err_t lora_set_dest_addr(uint8_t addr);
uint8_t get_available_queue_common();
uint8_t network_get_mqtt_status();
QueueHandle_t * get_queue_common_addr();
TaskHandle_t  * get_lora_task_handle();
TaskHandle_t  * get_network_handle();
TaskHandle_t  * get_hmi_handle();
TaskHandle_t  * get_common_handle();
void lora_task(void* pvParameters);
void network_task(void* pvParameters);
void daemon_task(void* pvParameters);
void common_task(void* pvParameters);
void hmi_task(void* pvParameters);
void other_task(void* pvParameters);
#endif // _MAIN_H_

#ifdef __cplusplus
}
#endif
