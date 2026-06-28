/* -------------------------- LICENSE placeholder -------------------------- */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "DWIN.h"
#include "dwin_app_driver.h"
#include "rtc_ds3231.h"
#ifdef __cplusplus
extern "C" {
#endif


static esp_err_t handle_intertask_request();
static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata);
static e_task_handle_id_t reply_task_handle_id = (e_task_handle_id_t)0;
static e_task_handle_id_t task_id_name = TASK_ID_HMI;
static uint8_t on_processing = 0; // To check whether new request can be processed

static const char *TAG = "hmi_task";

static st_intertask_data_t get_intertask_modbus(
    uint8_t modbus_address,
    uint8_t modbus_function,
    uint16_t reg,
    uint16_t value)
{
    st_core_data_t coredata;
    coredata.cdataid = COREDATA_ID_MB_DATA ;
    st_modbus_data_t mdata = {
        .addr = modbus_address,
        .modbus_function = modbus_function,
        .reg = reg,
        .value = value,
    };
    bzero(coredata.cdata, sizeof(coredata.cdata));
    memcpy((void*)coredata.cdata, (void*)&mdata, sizeof(st_modbus_data_t));

    st_intertask_data_t idata = { .src_task_handle_id = TASK_ID_HMI, .coredata = coredata, };
    return idata;
}

void hmi_parse_complete(st_hmi_frame_t hmiframe, void *pvParameter)
{
    st_intertask_data_t idata;
    uint16_t *value = (uint16_t*)pvParameter;
    e_task_handle_id_t taskid;
    switch(hmiframe.lastbyte) {
        case 0x0001:
            ESP_LOGI(TAG, "[BUTTON] SET FREQUENCY");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_SET_FREQ, (*value)*100);
            taskid = TASK_ID_LORA;
            break;
        case 0x0002: 
            ESP_LOGI(TAG, "[BUTTON] RUN");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 1);
            taskid = TASK_ID_LORA;
            break;
        case 0x0003: 
            ESP_LOGI(TAG, "[BUTTON] STOP");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 5);
            taskid = TASK_ID_LORA;
            break;
        case 0x0004: 
            ESP_LOGI(TAG, "[BUTTON] BACK TO KEYBOARD");
            return;
            break;
        case 0x0005: 
            ESP_LOGI(TAG, "[BUTTON] FORWARD MOTOR");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 1);
            taskid = TASK_ID_LORA;
            break;
        case 0x0006: 
            ESP_LOGI(TAG, "[BUTTON] REVERSE MOTOR");
            idata = get_intertask_modbus(1, (uint8_t)MB_FUNC_W_HOLDING, GD20_REG_CONTROL_CMD, 2);
            taskid = TASK_ID_LORA;
            break;
        default:
            ESP_LOGE(TAG, "No command is found");
            break;
    }
    relay_intertask(taskid, idata);
}

void hmi_task(void* pvParameters) {
    ESP_LOGI(TAG, "task created");
    hmi_start();
    hmi_register_callback(hmi_parse_complete);
    ESP_ERROR_CHECK(lora_set_dest_addr(2));

    // Khởi tạo RTC
    ds3231_init();
    TickType_t last_rtc_update = 0;

    for(;;){
        hmi_listen();
        handle_intertask_request();

        // Cập nhật RTC lên DWIN mỗi 1 giây
        if ((xTaskGetTickCount() * portTICK_PERIOD_MS) - last_rtc_update > 1000) {
            st_rtc_time_t current_time;
            if (ds3231_get_time(&current_time) == ESP_OK) {
                hmi_send_data_to_display(VP_TIME_HOUR,   current_time.hour);
                hmi_send_data_to_display(VP_TIME_MINUTE, current_time.minute);
                hmi_send_data_to_display(VP_TIME_SECOND, current_time.second);
                hmi_send_data_to_display(VP_DATE_DAY,    current_time.date);
                hmi_send_data_to_display(VP_DATE_MONTH,  current_time.month);
                hmi_send_data_to_display(VP_DATE_YEAR,   2000 + current_time.year);
            }
            last_rtc_update = xTaskGetTickCount() * portTICK_PERIOD_MS;
        }
        vTaskDelay(10);
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
    on_processing = 0;
}

static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata)
{
    QueueHandle_t *p_queue = (get_queue_common_addr() + taskid);
    if(xQueueSend(*p_queue, (void*)&idata, pdMS_TO_TICKS(200)) == pdPASS){}
    else {
        ESP_LOGE(TAG, "Fail to send queue to task handle id:%d", taskid);
    }
    return ESP_OK;
}


static esp_err_t handle_intertask_request()
{
    if(on_processing)
        return ESP_OK;

    QueueHandle_t *p_queue = (get_queue_common_addr() + task_id_name);
    st_intertask_data_t idata;
    if((xQueueReceive(*p_queue, (void*)&idata, pdMS_TO_TICKS(100)) == pdPASS)){
        on_processing = 1;
        reply_task_handle_id = (e_task_handle_id_t) idata.src_task_handle_id;
        ESP_LOGI(TAG, "Receive queue from :%d", reply_task_handle_id);
        hmi_intertask_core_function(idata.coredata);
    }
    return ESP_OK;

}

// ------------------- INTER TASK ------------------- ]

#ifdef __cplusplus
}
#endif
