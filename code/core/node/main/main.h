/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _MAIN_H_
#define _MAIN_H_

/* Dimensions of the buffer that the task being created will use as its stack.
   NOTE: This is the number of words the stack will hold, not the number of
   bytes. For example, if each stack item is 32-bits, and this is set to 200,
   then 800 bytes (200 * 32-bits) will be allocated. */
#define LORA_STACK_SIZE 3072
#define MODBUS_STACK_SIZE 3072 

#include "gd20_inverter.h"
#include "../intertask_typedef.h"
#include "../data_typedef.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "modbus_application_driver.h"

#define QUEUE_COMMON_ITEMS 3

typedef enum task_handle_id{
    TASK_ID_MODBUS = 1,
    TASK_ID_NETWORK,
    TASK_ID_LORA,
    TASK_ID_HMI,
    TASK_ID_SDCARD,
} e_task_handle_id_t;

uint8_t get_available_queue_common();
QueueHandle_t * get_queue_common_addr();
TaskHandle_t  * get_modbus_task_handle();
TaskHandle_t  * get_lora_task_handle();
TaskHandle_t  * get_network_handle();
void lora_task(void* pvParameters);
void modbus_task(void* pvParameters);
uint8_t lora_get_dest_addr();
esp_err_t lora_set_dest_addr(uint8_t addr);
#endif // _MAIN_H_
