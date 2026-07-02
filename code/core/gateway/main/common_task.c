/* -------------------------- LICENSE placeholder -------------------------- */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtc_ds3231.h"
#include "sdcard.h"
#include "main.h"

static esp_err_t handle_intertask_request();
static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata);
static e_task_handle_id_t reply_task_handle_id = (e_task_handle_id_t)0;
static e_task_handle_id_t task_id_name = TASK_ID_COMMON;
static uint8_t intertask_on_processing = 0; // To check whether new request can be processed
static uint8_t is_on_modbus_request = 0;
static uint8_t retry_flag = 0;

static const char *TAG = "common";
TimerHandle_t stimer_common;

static st_modbus_data_t intetask_request_data[] = {
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_SPEED  , 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OPERATION_FREQ, 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_VOLTAGE, 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_CURRENT, 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_CONVERTER_TEMP, 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_REG_STATUS    , 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_SET_FREQ      , 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_REG_ID        , 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_POWER  , 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_TORQUE , 1},
    // {SHT20_SLAVE_ID, (uint8_t)MB_FUNC_R_INPUT  , SHT20_REG_TEMP     , 1},
    // {SHT20_SLAVE_ID, (uint8_t)MB_FUNC_R_INPUT  , SHT20_REG_HUMID    , 1},
};

static st_intertask_data_t get_intertask_modbus_data(st_modbus_data_t mdata)
{
    st_core_data_t coredata;
    coredata.cdataid = (uint8_t)COREDATA_ID_MB_DATA ;
    bzero(coredata.cdata, sizeof(coredata.cdata));
    memcpy((void*)coredata.cdata, (void*)&mdata, sizeof(st_modbus_data_t));

    st_intertask_data_t idata = { .src_task_handle_id = task_id_name, .coredata = coredata, };
    return idata;
}

static void request_data_from_modbus()
{
    // static uint32_t last_request_tick = 0;
    static uint8_t request_idx = 0;
    uint32_t current_tick = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if(retry_flag) {
        is_on_modbus_request = 0;
        retry_flag = 0;
        if(request_idx > 0)
            request_idx--;
    }

    if(is_on_modbus_request)
        return;

    is_on_modbus_request = 1;
    if(request_idx > TOTAL_IDX(intetask_request_data) - 1) {
        request_idx = 0;
        is_on_modbus_request = 0;
    }
    st_intertask_data_t idata = get_intertask_modbus_data(intetask_request_data[request_idx++]);
    ESP_LOGI(TAG, "Send request to TASK_ID_LORA with core ID:%u, request_idx:%u", idata.coredata.cdataid, request_idx);
    ESP_ERROR_CHECK(lora_set_dest_addr(20));
    ESP_ERROR_CHECK(relay_intertask(TASK_ID_LORA, idata));
    // last_request_tick = current_tick;
}

void common_task(void* pvParameters) {
    // ds3231_init();
    // sdcard_init();
    // stimer_common = xTimerCreate("common timer", pdMS_TO_TICKS(100), true, NULL, stimer_cb);
    ESP_LOGI(TAG, "task created");
    // while(!network_get_mqtt_status()) {
    //     vTaskDelay(pdMS_TO_TICKS(2000));
    // }
    for(;;){
        request_data_from_modbus();
        handle_intertask_request();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ------------------- INTER_TASK ------------------- [

static void common_intertask_core_function(st_core_data_t coredata)
{
    ESP_LOGI(TAG, "Inside %s, get core ID:%u",__func__, coredata.cdataid);

    st_intertask_data_t idata = { .src_task_handle_id = task_id_name, .coredata = coredata, };
    relay_intertask(TASK_ID_HMI, idata);
    if(network_get_mqtt_status())
        relay_intertask(TASK_ID_NETWORK, idata);
    intertask_on_processing = 0;
    is_on_modbus_request = 0;
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
    if(intertask_on_processing)
        return ESP_OK;

    QueueHandle_t *p_queue = (get_queue_common_addr() + task_id_name);
    st_intertask_data_t idata;
    if((xQueueReceive(*p_queue, (void*)&idata, pdMS_TO_TICKS(100)) == pdPASS)){
        intertask_on_processing = 1;
        reply_task_handle_id = (e_task_handle_id_t) idata.src_task_handle_id;
        ESP_LOGI(TAG, "Receive queue from :%d", reply_task_handle_id);
        common_intertask_core_function(idata.coredata);
    }
    // else {
    //     ESP_LOGE(TAG, "Line %d - Fail to handle QueueReceive", __LINE__);
    // }
    return ESP_OK;
}

// ------------------- INTER TASK ------------------- ]

void common_task_retry() { retry_flag = 1; }
