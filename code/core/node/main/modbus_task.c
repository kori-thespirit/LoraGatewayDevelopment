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
#include "lora_protocol.h"

static void tx_complete(void *pvParameter);
static void rx_complete(void *pvParameter);

static const char *TAG = "modbus";

void modbus_task(void* pvParameters)
{
    esp_err_t err = ESP_OK;
    modbus_init();
    err = modbus_register_callback(tx_complete, rx_complete, NULL);
    modbus_send(MODBUS_FUNC_R, 1, GD20_REG_ID, 1);
    for(;;){
        uart_event_handle();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
 
static void tx_complete(void *pvParameter)
{

}
static void rx_complete(void *pvParameter)
{

}
