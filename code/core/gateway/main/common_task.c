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
static uint8_t on_processing = 0; // To check whether new request can be processed

static const char *TAG = "common";
TimerHandle_t stimer_common;

static st_modbus_data_t intetask_request_data[] = {
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_SET_FREQ      , 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OPERATION_FREQ, 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_VOLTAGE, 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_CURRENT, 1},
    {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_CONVERTER_TEMP, 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_REG_ID        , 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_SPEED  , 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_POWER  , 1},
    // {GD20_SLAVE_ID , (uint8_t)MB_FUNC_R_HOLDING, GD20_OUTPUT_TORQUE , 1},
    // {SHT20_SLAVE_ID, (uint8_t)MB_FUNC_R_INPUT  , SHT20_REG_TEMP     , 1},
    // {SHT20_SLAVE_ID, (uint8_t)MB_FUNC_R_INPUT  , SHT20_REG_HUMID    , 1},
};

static st_intertask_data_t get_intertask_modbus_data(st_modbus_data_t mdata)
{
    st_core_data_t coredata;
    coredata.cdataid = COREDATA_ID_MB_DATA ;
    bzero(coredata.cdata, sizeof(coredata.cdata));
    memcpy((void*)coredata.cdata, (void*)&mdata, sizeof(st_modbus_data_t));

    st_intertask_data_t idata = { .src_task_handle_id = task_id_name, .coredata = coredata, };
    return idata;
}

static void request_data_from_modbus()
{
    static uint32_t last_request_tick = 0;
    static uint8_t request_idx = 0;
    uint32_t current_tick = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if(on_processing) {
        if ((current_tick - last_request_tick) > 2000) {
            ESP_LOGW(TAG, "Timeout! Reset on_processing.");
            on_processing = 0;
        } else {
            return;
        }
    }

    if(request_idx >= TOTAL_IDX(intetask_request_data))
        request_idx = 0;
    st_intertask_data_t idata = get_intertask_modbus_data(intetask_request_data[request_idx++]);
    ESP_ERROR_CHECK(relay_intertask(TASK_ID_LORA, idata));
    last_request_tick = current_tick;
}

void common_task(void* pvParameters) {
    // ds3231_init();
    // sdcard_init();
    // stimer_common = xTimerCreate("common timer", pdMS_TO_TICKS(100), true, NULL, stimer_cb);
    ESP_LOGI(TAG, "task created");
    for(;;){
        request_data_from_modbus();
        handle_intertask_request();
        vTaskDelay(100);
    }
}

// ------------------- INTER_TASK ------------------- [

static void common_intertask_core_function(st_core_data_t coredata)
{
    ESP_LOGI(TAG, "Inside %s",__func__);

    st_intertask_data_t idata = { .src_task_handle_id = task_id_name, .coredata = coredata, };
    relay_intertask(TASK_ID_HMI, idata);
    relay_intertask(TASK_ID_NETWORK, idata);
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
        common_intertask_core_function(idata.coredata);
    }
    // else {
    //     ESP_LOGE(TAG, "Line %d - Fail to handle QueueReceive", __LINE__);
    // }
    return ESP_OK;
}

// ------------------- INTER TASK ------------------- ]
