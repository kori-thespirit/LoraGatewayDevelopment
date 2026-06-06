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
static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata);
static e_task_handle_id_t reply_task_handle_id = 0;
static uint8_t reply_dev_addr = 0;
static uint8_t reply_modbus_function = 0;

void modbus_task(void* pvParameters)
{
    e_modbus_payload_err_t err = ESP_OK;
    modbus_init();
    err = m_modbus_register_callback(tx_complete, rx_complete, modbus_payload_err);
    if(MB_PAYLOAD_OK != err)
        ESP_LOGE(TAG, "register callback failed: %d", err);
    for(;;){
        ESP_ERROR_CHECK(intertask_handle());
        ESP_ERROR_CHECK(modbus_uart_event_handle());
    }
}
 

// ------------------- INTER_TASK ------------------- [

static esp_err_t notify_intertask(e_task_handle_id_t taskid, u_intertask_noti_t notifydata)
{
    TaskHandle_t *task_handle = NULL;
    switch(taskid) {
        case TASK_ID_NETWORK:
            task_handle = get_network_handle();
            break;
        case TASK_ID_LORA:
            task_handle = get_lora_task_handle();
            break;
        default:
            ESP_LOGE(TAG, "This TaskHandle doesn't supported");
            return ESP_ERR_NOT_SUPPORTED;
            break;

    }
    if(!task_handle){
        ESP_LOGE(TAG, "Not found required TaskHandle");
        return ESP_ERR_NOT_FOUND;
    }

    if(xTaskNotify(*task_handle, notifydata.value, eSetValueWithoutOverwrite) == pdPASS) {}
    else {
        ESP_LOGE(TAG, "Fail to notify task");
    }
    return ESP_OK;
}

static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata)
{
    uint8_t qidx = get_available_queue_common();
    QueueHandle_t *p_queue = (get_queue_common_addr() + qidx);
    if(xQueueSend(*p_queue, (void*)&idata, pdMS_TO_TICKS(200)) == pdPASS){}
    else {
        ESP_LOGE(TAG, "Fail to send queue");
    }
    u_intertask_noti_t notifydata ;
    notifydata.notivalue.intertask_err = INTERTASK_ERR_NOT_USE;
    notifydata.notivalue.qidx = qidx; // queue index
    notify_intertask(taskid, notifydata);
    return ESP_OK;
}

static esp_err_t handle_intertask_request()
{
    u_intertask_noti_t notifydata ;
    if(xTaskNotifyWait(0x00, 0x00, (uint32_t*)&notifydata.value, pdMS_TO_TICKS(100)) == pdFALSE)
        return ESP_OK;
    ESP_LOGI(TAG, "Received notify, qidx:%u, intertask_err:%u", 
            notifydata.notivalue.qidx,
            notifydata.notivalue.intertask_err
            );
    uint8_t qidx = notifydata.notivalue.qidx;
    QueueHandle_t *p_queue = (get_queue_common_addr() + qidx);
    st_intertask_data_t idata;
    e_intertask_err_t ierr = notifydata.notivalue.intertask_err;
    /* Bypass xQueueReceive if intertask reply with status */
    if(INTERTASK_ERR_NOT_USE != ierr) {
        ESP_LOGW(TAG, "Relay intertask has status:%d", ierr);
        return ESP_OK;
    }
    if(xQueueReceive(*p_queue, (void*)&idata, pdMS_TO_TICKS(100)) == pdPASS){
        st_core_data_t coredata =   idata.coredata;
        st_modbus_data_t mdata;
        switch(coredata.cdataid) {
            case COREDATA_ID_MB_DATA:
                memcpy((void*)&mdata, (void*)coredata.cdata,sizeof(mdata));
                reply_dev_addr = mdata.addr;
                reply_modbus_function = mdata.modbus_function;
                break;
            case COREDATA_ID_MB_CFG:
                break;
            default:
                ESP_LOGE(TAG, "%s:Core data id not supported",__func__);
                return ESP_ERR_NOT_SUPPORTED;
                break;

        }
        /* Save the sender task_handle_id for replying back */
        reply_task_handle_id = idata.src_task_handle_id;

        ESP_LOGI(TAG,"src_task_handle_id:%d, func:%d, addr:%u, reg:%x, value:%u",
                idata.src_task_handle_id, 
                mdata.modbus_function, 
                mdata.addr, 
                mdata.reg, 
                mdata.value);
        ESP_ERROR_CHECK(modbus_send(
                    mdata.modbus_function, 
                    mdata.addr, 
                    mdata.reg, 
                    mdata.value));
    }
    else {
        ESP_LOGE(TAG, "%s:Fail to receive queue",__func__);
    }
    return ESP_OK;

}


static esp_err_t intertask_handle()
{
    ESP_ERROR_CHECK(handle_intertask_request());
    return ESP_OK;

}

// ------------------- INTER TASK ------------------- ]

// ------------------- MODBUS PAYLOAD HANDLE ------------------- [
static void tx_complete(void *pvParameter)
{
    ESP_LOGI(TAG, "Write data successfully, reply_task_handle_id:%d",reply_task_handle_id);
    u_intertask_noti_t notifydata;
    notifydata.notivalue.intertask_err = INTERTASK_OK;
    ESP_ERROR_CHECK(notify_intertask(reply_task_handle_id, notifydata));
    reply_task_handle_id = 0;
}

static void rx_complete(void *pvParameter, const st_modbus_params_descriptor_t *desc)
{
    ESP_LOGI(TAG, "Read data successfully");
    if(reply_task_handle_id) {
        uint16_t *value = (uint16_t*)pvParameter;
        st_modbus_data_t mdata = {
            .addr = reply_dev_addr,
            .reg = desc->reg,
            .modbus_function = reply_modbus_function,
            .value = *value,
        };
        st_core_data_t coredata = {
            .cdataid = COREDATA_ID_MB_DATA,
        };
        /* Cleanup data before copy */
        bzero(coredata.cdata,sizeof(coredata.cdata));
        memcpy((void*)coredata.cdata, (void*)&mdata, sizeof(mdata));
        st_intertask_data_t idata = {
            .src_task_handle_id = TASK_ID_MODBUS,
            .coredata = coredata
        };
        if(MB_FUNC_R == reply_modbus_function)
            ESP_ERROR_CHECK(relay_intertask(reply_task_handle_id, idata));
        reply_task_handle_id = 0;
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
// ------------------- MODBUS PAYLOAD HANDLE ------------------- ]
