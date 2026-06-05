/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "driver/gpio.h"
#include "lora.h"
#include "lora_protocol.h"
#include "main.h"

#define RF_FREQUENCY 433000000LL  // Tần số: 433MHz (hoặc 866000000LL, 915000000LL)
#define LORA_BW 7                 // Bandwidth: 7 là 125kHz
#define LORA_SF 7                 // Spreading Factor: 7
#define LORA_CR 1                 // Coding Rate: 1 là 4/5
#define LORA_CRC 1                // 1: Bật CRC, 0: Tắt CRC
                                  //
#define Lora_EN 2
#define LED_ACT 13

static const char *TAG = "lora_task";
void pack_complete(void *pvParameters);
void parse_complete(void *pvParameters);

static void lora_send();
static esp_err_t intertask_handle();
static e_task_handle_id_t reply_task_id = 0;
static esp_err_t send_to_intertask(e_task_handle_id_t taskid, void *pvParameter, size_t param_size);

void lora_task(void* pvParameters) {
    /*đăng ký callback để nhận thông báo khi đóng gói hoặc phân tích khung hoàn tất*/
    e_lora_protocol_err_t protocol_err = m_lora_protocol_register_callback(&pack_complete, &parse_complete); 
    if(LORA_PROTOCOL_ERR_OK != protocol_err){
        ESP_LOGE(TAG, "%s:Register callback failed, err:%d",__func__, protocol_err);
    }
    gpio_reset_pin(LED_ACT);
    gpio_set_direction(LED_ACT, GPIO_MODE_OUTPUT);

    gpio_reset_pin(Lora_EN);
    gpio_set_direction(Lora_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(Lora_EN, 1);
    // Initialize LoRa
    if (lora_init() == 0) {
        ESP_LOGE(TAG, "Does not recognize the module");
        while (1) {
            vTaskDelay(1);
        }
    }
    lora_set_frequency(RF_FREQUENCY);

    lora_enable_crc();

    lora_set_coding_rate(LORA_CR);
    ESP_LOGI( TAG, "coding_rate=%d", LORA_CR);

    lora_set_bandwidth(LORA_BW);
    ESP_LOGI(TAG, "bandwidth=%d", LORA_BW);

    lora_set_spreading_factor(LORA_SF);
    ESP_LOGI(TAG, "spreading_factor=%d", LORA_SF);

    st_modbus_data_t mdata = {
        .addr = 1,
        .reg = GD20_REG_ID,
        .modbus_function = (uint8_t)MB_FUNC_R,
        .value = 1,
    };
    st_core_data_t coredata = { .cdataid = COREDATA_ID_MB_DATA, .cdata = (void*)&mdata, };
    st_intertask_data_t idata = { .src_task_handle_id = TASK_ID_LORA, .payload = (void*)&coredata, };
    ESP_ERROR_CHECK(send_to_intertask(TASK_ID_MODBUS, (void*)&idata, sizeof(idata)));
    while (1) {
        // test_send();
        ESP_ERROR_CHECK(intertask_handle());
        int lost = lora_packet_lost();
        if (lost != 0) {
            ESP_LOGW(TAG, "%d packets lost", lost);
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
    vTaskDelete(NULL);
}

void pack_complete(void *pvParameters)
{
    uint8_t *frame_length = (uint8_t*)pvParameters;
    ESP_LOGI(TAG, "Sending message via lora complete with %u bytes", *frame_length);
}

void parse_complete(void *pvParameters)
{
    ESP_LOGI(TAG, "Parse Callback");
}

static esp_err_t send_to_intertask(e_task_handle_id_t taskid, void *pvParameter, size_t param_size)
{
    TaskHandle_t *task_handle = NULL;
    uint8_t qidx = get_available_queue_common();
    QueueHandle_t *p_queue = (get_queue_common_addr() + qidx);
    switch(taskid) {
        case TASK_ID_NETWORK:
            task_handle = get_network_handle();
            break;
        case TASK_ID_MODBUS:
            task_handle = get_modbus_task_handle();
            break;
        case TASK_ID_LORA:
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
    st_intertask_data_t idata;

    if(xQueueReceive(*p_queue, (void*)&idata, pdMS_TO_TICKS(100)) == pdPASS){
        uint8_t buffer[20] = {0};
        st_modbus_data_t *mdata = (st_modbus_data_t*)idata.payload;

        e_lora_protocol_err_t protocol_err = m_lora_protocol_frame_pack((void*)buffer, sizeof(buffer), (void*)mdata, sizeof(st_modbus_data_t), 1, 0);
        if(LORA_PROTOCOL_ERR_OK != protocol_err) {
            ESP_LOGE(TAG, "%s:Pack frame data failed, err:%d",__func__, protocol_err);
        }
        lora_send_packet(buffer, sizeof(buffer));
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
