/* -------------------------- LICENSE placeholder -------------------------- */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "DWIN.h"
#include "dwin_app_driver.h"
#ifdef __cplusplus
extern "C" {
#endif

static const char *TAG = "hmi_task";
static e_task_handle_id_t reply_task_handle_id = (e_task_handle_id_t)0;

static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata);
static esp_err_t intertask_handle();

static st_intertask_data_t get_intertask_modbus(
    uint8_t modbus_address, 
    uint8_t modbus_function,
    uint16_t reg,
    uint16_t value)
{
    st_core_data_t coredata;
    st_modbus_data_t mdata = {
        .addr = modbus_address,
        .modbus_function = modbus_function,
        .reg = reg,
        .value = value,
    };
    coredata.cdataid = COREDATA_ID_MB_DATA ;
    bzero(coredata.cdata, sizeof(coredata.cdata));
    memcpy((void*)coredata.cdata, (void*)&mdata, sizeof(coredata.cdata));

    st_intertask_data_t idata = { .src_task_handle_id = TASK_ID_HMI, .coredata = coredata, };
    return idata;
}

void hmi_parse_complete(st_hmi_frame_t hmiframe, void *pvParameter)
{
    st_intertask_data_t idata;
    uint16_t *value = (uint16_t*)pvParameter;
    switch(hmiframe.lastbyte) {
        case 0x0001:
            ESP_LOGI(TAG, "[BUTTON] SET FREQUENCY");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_SET_FREQ, *value);
            break;
        case 0x0002: 
            ESP_LOGI(TAG, "[BUTTON] RUN");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 1);
            break;
        case 0x0003: 
            ESP_LOGI(TAG, "[BUTTON] STOP");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 5);
            break;
        case 0x0004: 
            ESP_LOGI(TAG, "[BUTTON] BACK TO KEYBOARD");
            break;
        case 0x0005: 
            ESP_LOGI(TAG, "[BUTTON] FORWARD MOTOR");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 1);
            break;
        case 0x0006: 
            ESP_LOGI(TAG, "[BUTTON] REVERSE MOTOR");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 2);
            break;
        default:
            ESP_LOGE(TAG, "No command is found");
            break;
    }
    ESP_LOGI(TAG, "\n");
    ESP_ERROR_CHECK(relay_intertask(TASK_ID_LORA, idata));
}

void test_send()
{
    st_intertask_data_t idata;
    // idata = get_intertask_modbus(SHT20_SLAVE_ID, (uint8_t)MB_FUNC_R_INPUT, SHT20_REG_TEMP, 1);
    // idata = get_intertask_modbus(SHT20_SLAVE_ID, (uint8_t)MB_FUNC_R_INPUT, SHT20_REG_HUMID, 1);
    // idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, GD20_SET_FREQ      , 1);
    // idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, GD20_REG_ID   , 1);
    // idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_VOLTAGE, 1);
    // idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_CURRENT, 1);
    // idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_SPEED  , 1);
    // idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_POWER  , 1);
    // idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_TORQUE , 1);
    idata = get_intertask_modbus(GD20_SLAVE_ID, (uint8_t)MB_FUNC_R_HOLDING, 0x070C, 1);
    ESP_ERROR_CHECK(relay_intertask(TASK_ID_LORA, idata));
}

void hmi_task(void* pvParameters) {
    ESP_LOGI(TAG, "task created");
    hmi_start();
    hmi_register_callback(hmi_parse_complete);
    ESP_ERROR_CHECK(lora_set_dest_addr(2));
    test_send();
    for(;;){
        hmi_listen();
        intertask_handle();
    }
}

// ------------------- INTER_TASK ------------------- [

static void hmi_intertask_core_function(st_core_data_t coredata)
{
    ESP_LOGI(TAG, "Inside %s",__func__);
    float fdata;
    uint16_t u16data;
    uint8_t u8data;
    switch(coredata.cdataid)
    {
        case COREDATA_ID_SHT20_TEMP:
            memcpy((void*)&fdata, (void*)&coredata.cdata, sizeof(float));
            hmi_send_data_to_display(ADDRESS_TEMP, (uint16_t)(fdata * 100));
            ESP_LOGI(TAG, "SHT20_TEMP - fdata:%.2f",fdata);
            break;
        case COREDATA_ID_SHT20_HUMID:
            memcpy((void*)&fdata, (void*)&coredata.cdata, sizeof(float));
            hmi_send_data_to_display(ADDRESS_HUMID, (uint16_t)(fdata * 100));
            ESP_LOGI(TAG, "SHT20_HUMID - fdata:%.2f",fdata);
            break;
        case COREDATA_ID_GD20_ID:
            memcpy((void*)&u16data, (void*)&coredata.cdata, sizeof(uint16_t));
            ESP_LOGI(TAG, "GD20_ID:0x%x",u16data);
            break;
        case COREDATA_ID_GD20_SPEED:
            memcpy((void*)&fdata, (void*)&coredata.cdata, sizeof(float));
            ESP_LOGI(TAG, "GD20_SPEED:%.2f",fdata);
            break;
        case COREDATA_ID_GD20_POWER:
            break;
        case COREDATA_ID_GD20_TORQUE:
            break;
        case COREDATA_ID_GD20_STATUS:
            memcpy((void*)&u8data, (void*)&coredata.cdata, sizeof(uint8_t));
            ESP_LOGI(TAG, "GD20_STATUS:%u",u8data);
            break;
        case COREDATA_ID_GD20_FREQ:
            memcpy((void*)&fdata, (void*)&coredata.cdata, sizeof(float));
            ESP_LOGI(TAG, "GD20_FREQ:%.2f",fdata);
            hmi_send_data_to_display(ADDRESS_GD20_FREQ, (uint16_t)(fdata * 100));
            break;
        case COREDATA_ID_GD20_CURRENT:
            memcpy((void*)&fdata, (void*)&coredata.cdata, sizeof(float));
            ESP_LOGI(TAG, "GD20_CURRENT:%.2f",fdata);
            hmi_send_data_to_display(ADDRESS_GD20_AMP, (uint16_t)(fdata * 100));
            break;
        case COREDATA_ID_GD20_VOLTAGE:
            memcpy((void*)&fdata, (void*)&coredata.cdata, sizeof(float));
            ESP_LOGI(TAG, "GD20_VOLTAGE:%.2f",fdata);
            hmi_send_data_to_display(ADDRESS_GD20_VOLT, (uint16_t)(fdata * 100));
            break;
        case COREDATA_ID_GD20_CONVETER_TEMP:
            memcpy((void*)&fdata, (void*)&coredata.cdata, sizeof(float));
            ESP_LOGI(TAG, "GD20_CONVETER_TEMP:%.2f",fdata);
            hmi_send_data_to_display(ADDRESS_GD20_TEMP, (uint16_t)(fdata * 100));

            break;
        default:
            break;
    }
}

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
        case TASK_ID_SDCARD:
            task_handle = get_common_handle();
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

    if(xTaskNotify(*task_handle, notifydata.value, eSetValueWithoutOverwrite) == pdPASS) {
        ESP_LOGI(TAG, "Notify to :%d", taskid);
    }
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
    /* Common step of handle intertask request */
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
    e_intertask_err_t ierr = (e_intertask_err_t)notifydata.notivalue.intertask_err;

    /* Bypass xQueueReceive if intertask reply with status */
    if(INTERTASK_ERR_NOT_USE != ierr) {
        ESP_LOGW(TAG, "Relay intertask has status:%d", ierr);
        return ESP_OK;
    }
    if(xQueueReceive(*p_queue, (void*)&idata, pdMS_TO_TICKS(100)) == pdPASS){

        reply_task_handle_id = (e_task_handle_id_t) idata.src_task_handle_id;
        hmi_intertask_core_function(idata.coredata);
    }
    else {
        ESP_LOGE(TAG, "%s:Fail to handle QueueReceive", __func__);
    }
    return ESP_OK;

}

static esp_err_t intertask_handle()
{
    ESP_ERROR_CHECK(handle_intertask_request());
    return ESP_OK;

}
// ------------------- INTER TASK ------------------- ]

#ifdef __cplusplus
}
#endif
