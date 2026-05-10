/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "modbus_application_driver.h"
#include "gd20_inverter.h"

void modbus_task(void* pvParameters)
{
  for(;;){
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
