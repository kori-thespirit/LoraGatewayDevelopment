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
static const char *TAG = "modbus_task";

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
        ESP_ERROR_CHECK(modbus_uart_event_handle());
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
    switch(*err){
        case MB_PAYLOAD_OK:
        case MB_PAYLOAD_ERR_CALLBACK_IS_NULL:
        case MB_PAYLOAD_ERR_RANGE_INVALID:
            ESP_LOGE(TAG, "Value is out of range");
            break;
        case MB_PAYLOAD_ERR_DEVICE_NOT_FOUND:
            ESP_LOGE(TAG, "Device not found");
            break;
        case MB_PAYLOAD_ERR_DESCRIPTOR_NOT_FOUND:
            ESP_LOGE(TAG, "Descriptor not found");
            break;
        case MB_PAYLOAD_ERR_DATA_SIZE_MISMACTH:
            ESP_LOGE(TAG, "Data size mismatch");
            break;
        case MB_PAYLOAD_ERR_ADDRESS_MISMACTH:
            ESP_LOGE(TAG, "Address mismatch");
            break;
        case MB_PAYLOAD_ERR_DATA_INVALID: 
            ESP_LOGE(TAG, "Invalid data");
            break;
        case MB_PAYLOAD_ERR_PERMISSON: 
            ESP_LOGE(TAG, "Invalid permission to read or write");
            break;

    }
    ESP_ERROR_CHECK(1);
}
