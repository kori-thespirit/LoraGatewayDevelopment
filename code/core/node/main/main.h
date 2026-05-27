/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _MAIN_H_
#define _MAIN_H_

/* Dimensions of the buffer that the task being created will use as its stack.
   NOTE: This is the number of words the stack will hold, not the number of
   bytes. For example, if each stack item is 32-bits, and this is set to 200,
   then 800 bytes (200 * 32-bits) will be allocated. */
#define LORA_STACK_SIZE 3072
#define MODBUS_STACK_SIZE 3072 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t *get_modbus_task_handle();
TaskHandle_t *get_lora_task_handle();
void lora_task(void* pvParameters);
void modbus_task(void* pvParameters);
#endif // _MAIN_H_
