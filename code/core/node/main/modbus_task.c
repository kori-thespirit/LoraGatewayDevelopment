/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "main.h"
#include "modbus_application_driver.h"
#include "modbus_payload_handle.h"
#include "gd20_inverter.h"

static void tx_complete(void *pvParameter);
static void rx_complete(void *pvParameter);
static void modbus_payload_err(void *pvParameter);
static const char *TAG = "modbus";

static void transfer_payload_to_other_task()
{

}

void modbus_task(void* pvParameters)
{
    e_modbus_payload_err_t err = ESP_OK;
    modbus_init();
    err = m_modbus_register_callback(tx_complete, rx_complete, modbus_payload_err);
    if(MB_PAYLOAD_OK != err)
        ESP_LOGE(TAG, "register callback failed: %d", err);

    ESP_ERROR_CHECK(modbus_send(MB_FUNC_R, 1, GD20_REG_ID, 1));
    // ESP_ERROR_CHECK(modbus_send(MB_FUNC_W, 1, GD20_REG_CONTROL_CMD, 5));
    for(;;){
        ESP_ERROR_CHECK( modbus_uart_event_handle());
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
 
static void tx_complete(void *pvParameter)
{
    ESP_LOGI(TAG, "Write data successfully");
}

static void rx_complete(void *pvParameter)
{
    ESP_LOGI(TAG, "Read data successfully");
}

static void modbus_payload_err(void *pvParameter)
{

    e_modbus_payload_err_t *err = (e_modbus_payload_err_t *) pvParameter;
    ESP_LOGE(TAG, "Error in modbus payload handle:%d", *err);
}
