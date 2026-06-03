/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "main.h"
#include "modbus_application_driver.h"
#include "modbus_payload_handle.h"

static void tx_complete(void *pvParameter);
static void rx_complete(void *pvParameter, const st_modbus_params_descriptor_t *desc);
static void modbus_payload_err(void *pvParameter);
static const char *TAG = "modbus_task";
static esp_err_t intertask_handle();
static esp_err_t send_to_intertask(e_task_handle_id_t taskid, void *pvParameter, size_t param_size);
static e_task_handle_id_t reply_task_id = 0;

void modbus_task(void* pvParameters)
{
    e_modbus_payload_err_t err = ESP_OK;
    modbus_init();
    err = m_modbus_register_callback(tx_complete, rx_complete, modbus_payload_err);
    if(MB_PAYLOAD_OK != err)
        ESP_LOGE(TAG, "register callback failed: %d", err);

    // ESP_ERROR_CHECK(modbus_send(MB_FUNC_R, 1, GD20_REG_ID, 1));
    // ESP_ERROR_CHECK(modbus_send(MB_FUNC_W, 1, GD20_REG_CONTROL_CMD, 1));
    // ESP_ERROR_CHECK(modbus_send(MB_FUNC_W, 1, GD20_REG_CONTROL_CMD, 5));
    for(;;){
        ESP_ERROR_CHECK(intertask_handle());
        ESP_ERROR_CHECK(modbus_uart_event_handle());
    }
}
 
static void tx_complete(void *pvParameter)
{
    ESP_LOGI(TAG, "Write data successfully");
}

static void rx_complete(void *pvParameter, const st_modbus_params_descriptor_t *desc)
{
    ESP_LOGI(TAG, "Read data successfully");
    if(reply_task_id) {
        uint16_t *value = (uint16_t*)pvParameter;
        st_modbus_intertask_t data = {
            .payload = *value,
            .src_task_handle_id = TASK_ID_MODBUS,
        };
        send_to_intertask(reply_task_id, (void*)&data, sizeof(data));
        reply_task_id = 0;
    }
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



static esp_err_t send_to_intertask(e_task_handle_id_t taskid, void *pvParameter, size_t param_size)
{
    TaskHandle_t *task_handle = NULL;
    uint8_t qidx = get_available_queue_common();
    QueueHandle_t *p_queue = (get_queue_common_addr() + qidx);
    if(param_size != sizeof(st_modbus_intertask_t)){
        ESP_LOGE(TAG, "Invalid modbus intertask size");
        return ESP_ERR_INVALID_SIZE;
    }
    // memcpy(temp, (uint8_t*)pvParameter, param_size);
    switch(taskid) {
        case TASK_ID_NETWORK:
            task_handle = get_network_handle();
            break;
        case TASK_ID_LORA:
            task_handle = get_lora_task_handle();
            break;
        case TASK_ID_MODBUS: 
        case TASK_ID_HMI:
        default:
            ESP_LOGE(TAG, "This TaskHandle doesn't supported");
            return ESP_ERR_NOT_SUPPORTED;
            break;

    }
    if(!task_handle){
        ESP_LOGE(TAG, "Not found required TaskHandle");
        return ESP_ERR_NOT_FOUND;
    }
    
    if(xQueueSend(*p_queue, (void*)pvParameter, pdMS_TO_TICKS(200)) == pdPASS){}
    else {
        ESP_LOGE(TAG, "Fail to send queue");
    }
   

    if(xTaskNotify(*task_handle, (uint32_t)qidx, eSetValueWithoutOverwrite) == pdPASS) {}
    else {
        ESP_LOGE(TAG, "Fail to notify task");
    }

    return ESP_OK;
}

static esp_err_t handle_intertask_request()
{
    uint8_t qidx;
    if(xTaskNotifyWait(0x00, 0x00, (uint32_t*)&qidx, pdMS_TO_TICKS(100)) == pdFALSE)
        return ESP_OK;
    ESP_LOGI(TAG, "Received notify, Get qidx:%u",qidx);
    QueueHandle_t *p_queue = (get_queue_common_addr() + qidx);
    st_modbus_intertask_t data;

    if(xQueueReceive(*p_queue, (void*)&data, pdMS_TO_TICKS(100)) == pdPASS){
        e_modbus_function_t modbus_func;
        /* Sender want to request data */
        if(data.is_request){
            modbus_func = MB_FUNC_R;
        }
        else { /* Sender want to write data */
            modbus_func = MB_FUNC_W;
        }
        ESP_LOGI(TAG,"reply_task_id:%d",data.src_task_handle_id);
        ESP_LOGI(TAG,"func:%d, addr:%u, reg:%u, payload:%u",modbus_func, data.addr, data.reg, data.payload);
        /* Save the sender task_handle_id for replying back */
        reply_task_id = data.src_task_handle_id;
        ESP_ERROR_CHECK(modbus_send(modbus_func, data.addr, data.reg, data.payload));
    }
    return ESP_OK;

}

static esp_err_t intertask_handle()
{
    ESP_ERROR_CHECK(handle_intertask_request());
    return ESP_OK;

}
