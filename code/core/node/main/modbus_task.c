/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "main.h"
#include "modbus_application_driver.h"
#include "gd20_inverter.h"

static void tx_complete(void *pvParameter);
static void rx_complete(void *pvParameter);

static const char *TAG = "modbus";

static void transfer_payload_to_other_task()
{

}

void modbus_task(void* pvParameters)
{
    esp_err_t err = ESP_OK;
    modbus_init();
    err = modbus_register_callback(tx_complete, rx_complete, NULL);
    if(ESP_OK != err)
        ESP_LOGE(TAG, "register callback failed: %d", err);

    ESP_ERROR_CHECK(modbus_send(MB_FUNC_R, 1, GD20_REG_ID, 1));
    for(;;){
        modbus_uart_event_handle();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
 
static void tx_complete(void *pvParameter)
{

}
static void rx_complete(void *pvParameter)
{
}
