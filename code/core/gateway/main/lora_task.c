/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"
#include "lora_protocol.h"
#include "main.h"

#define RF_FREQUENCY 433000000LL  // Tần số: 433MHz (hoặc 866000000LL, 915000000LL)
#define LORA_BW 7                 // Bandwidth: 7 là 125kHz
#define LORA_SF 7                 // Spreading Factor: 7
#define LORA_CR 1                 // Coding Rate: 1 là 4/5
#define LORA_CRC 1                // 1: Bật CRC, 0: Tắt CRC
 
static const char *TAG = "lora_task";
void pack_complete(void *pvParameters);
void parse_complete(void *pvParameters, st_lora_protocol_header_t header);

static esp_err_t handle_intertask_request();
static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata);
static void mock_intertask_on_processing_complete_in_ms(uint32_t delay);
static e_task_handle_id_t reply_task_handle_id = (e_task_handle_id_t)0;
static e_task_handle_id_t task_id_name = TASK_ID_LORA;
static uint8_t intertask_on_processing = 0; // To check whether new request can be processed

static uint8_t src_addr = 0;
static uint8_t dest_addr = 0;
static uint8_t dev_addr = 1;
static e_lora_function_t lorafunc = LORA_FUNC_ACTIVE_TRANSMIT;
static uint8_t buf[40] = {0};

void lora_task(void* pvParameters) {

    e_lora_protocol_err_t protocol_err = m_lora_protocol_register_callback(&pack_complete, &parse_complete);
    if(LORA_PROTOCOL_OK != protocol_err)
        ESP_LOGE(TAG, "register callback failed,err:%d",protocol_err);

    if (lora_init() == 0) {
        ESP_LOGE(TAG, "Does not recognize the module");
        while (1) {
            vTaskDelay(1);
        }
    }

    lora_set_frequency(RF_FREQUENCY);
    lora_enable_crc();
    lora_set_coding_rate(LORA_CR);
    ESP_LOGI(TAG, "coding_rate=%d", LORA_CR);

    lora_set_bandwidth(LORA_BW);
    ESP_LOGI(TAG, "bandwidth=%d", LORA_BW);

    lora_set_spreading_factor(LORA_SF);
    ESP_LOGI(TAG, "Start");

    while (1) {
        handle_intertask_request();
        lora_receive();  // put into receive mode
        if (lora_received()) {
            int rxLen = lora_receive_packet(buf, sizeof(buf));
            m_lora_protocol_frame_parse(buf, sizeof(buf));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }  // end while

}

void pack_complete(void *pvParameters)
{
    uint8_t *len = (uint8_t*)pvParameters;

    ESP_LOGI(TAG, "Sending Callback complete with %u byte", *len);
}

void parse_complete(void *pvParameters, st_lora_protocol_header_t header)
{
    if(header.dest_addr == dev_addr) {
       ESP_LOGI(TAG, "This message is for me");
       src_addr =  dev_addr;
       dest_addr = header.src_addr; // The reply address is the received src_addr
       if(reply_task_handle_id) {
           ESP_LOGI(TAG, "Found reply_task_handle_id:%d, relay to this id");
           st_core_data_t *coredata = (st_core_data_t*)pvParameters;
           st_intertask_data_t idata = {
               .src_task_handle_id = task_id_name,
               .coredata = *coredata,
           };
           relay_intertask(reply_task_handle_id, idata);
       }
    }
    else {
        /* TODO: Relay to adjacent lora node */
        return;
    }
    intertask_on_processing = 0;
}

// ------------------- INTER_TASK ------------------- [

static void lora_intertask_core_function(st_core_data_t coredata)
{
    e_lora_protocol_err_t protocol_err = m_lora_protocol_frame_pack((void*)buf, sizeof(buf), (void*)&coredata, sizeof(st_core_data_t), dev_addr, dest_addr, 0);
    ESP_LOGI(TAG, "Sending coredata to address: %u", dest_addr);
    if(LORA_PROTOCOL_OK != protocol_err) {
        ESP_LOGE(TAG, "%s:Pack frame data failed, err:%d", __func__, protocol_err);
    }
    // for(uint8_t i = 0; i < sizeof(buf); i++) {
    //     printf("%x ", buf[i]);
    // }
    // printf("\n");
    lora_send_packet(buf, sizeof(buf));
    bzero(buf, sizeof(buf));
}

// static esp_err_t notify_intertask(e_task_handle_id_t taskid, u_intertask_noti_t notifydata)
// {
//     TaskHandle_t *task_handle = NULL;
//     switch(taskid) {
//         case TASK_ID_NETWORK:
//             task_handle = get_network_handle();
//             break;
//         case TASK_ID_HMI:
//             task_handle = get_hmi_handle();
//             break;
//         case TASK_ID_SDCARD:
//             task_handle = get_common_handle();
//             break;
//         default:
//             ESP_LOGE(TAG, "This TaskHandle doesn't supported");
//             return ESP_ERR_NOT_SUPPORTED;
//             break;
//
//     }
//     if(!task_handle){
//         ESP_LOGE(TAG, "Not found required TaskHandle");
//         return ESP_ERR_NOT_FOUND;
//     }
//
//     if(xTaskNotify(*task_handle, notifydata.value, eSetValueWithoutOverwrite) == pdPASS) {
//         ESP_LOGI(TAG, "Notify to :%d", taskid);
//     }
//     else {
//         ESP_LOGE(TAG, "Fail to notify task");
//     }
//     return ESP_OK;
// }

static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata)
{
    // uint8_t qidx = get_available_queue_common();
    QueueHandle_t *p_queue = (get_queue_common_addr() + taskid);
    if(xQueueSend(*p_queue, (void*)&idata, pdMS_TO_TICKS(200)) == pdPASS){}
    else {
        ESP_LOGE(TAG, "Fail to send queue to task handle id:%d", taskid);
    }
    // u_intertask_noti_t notifydata ;
    // notifydata.notivalue.intertask_err = INTERTASK_ERR_NOT_USE;
    // notifydata.notivalue.qidx = qidx; // queue index
    // notify_intertask(taskid, notifydata);
    return ESP_OK;
}

static esp_err_t handle_intertask_request()
{
    /* Common step of handle intertask request */
    // u_intertask_noti_t notifydata ;
    // if(xTaskNotifyWait(0x00, 0x00, (uint32_t*)&notifydata.value, pdMS_TO_TICKS(10)) == pdTRUE)
    // {
    //     ESP_LOGI(TAG, "Received notify, qidx:%u, intertask_err:%u", 
    //             notifydata.notivalue.qidx,
    //             notifydata.notivalue.intertask_err
    //             );
    //     uint8_t qidx = notifydata.notivalue.qidx;
    //     e_intertask_err_t ierr = notifydata.notivalue.intertask_err;
    // }

    /* Bypass xQueueReceive if intertask reply with status */
    // if(INTERTASK_ERR_NOT_USE != ierr) {
    //     ESP_LOGW(TAG, "Relay intertask has status:%d", ierr);
    //     return ESP_OK;
    // }
    static uint32_t last_request_tick = 0;
    uint32_t current_tick = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if(intertask_on_processing) {
        if ((current_tick - last_request_tick) > 4000) {
            ESP_LOGW(TAG, "Timeout! Reset intertask_on_processing.");
            intertask_on_processing = 0;
        }
        else 
            return ESP_OK;
    }

    st_intertask_data_t idata;
    QueueHandle_t *p_queue = (get_queue_common_addr() + task_id_name);
    if((xQueueReceive(*p_queue, (void*)&idata, pdMS_TO_TICKS(100)) == pdPASS)){
        intertask_on_processing = 1;
        reply_task_handle_id = (e_task_handle_id_t) idata.src_task_handle_id;
        ESP_LOGI(TAG, "Receive queue from :%d", reply_task_handle_id);
        lora_intertask_core_function(idata.coredata);
    }
    return ESP_OK;

}
static void mock_intertask_on_processing_complete_in_ms(uint32_t delay)
{
    if(!intertask_on_processing) return;
    vTaskDelay(pdMS_TO_TICKS(delay));
    st_core_data_t coredata = {0};
    st_intertask_data_t idata = {
        .src_task_handle_id = task_id_name,
        .coredata = coredata,
    };
    relay_intertask(reply_task_handle_id, idata);
    intertask_on_processing = 0;
}

// ------------------- INTER TASK ------------------- ]
// ------------------- HELPER FUNCTION ------------------- [
uint8_t lora_get_dest_addr() {return dest_addr;}
esp_err_t lora_set_dest_addr(uint8_t addr) {
    if(addr == src_addr || addr == dev_addr) return ESP_ERR_INVALID_ARG;
    dest_addr = addr;
    ESP_LOGI(TAG, "%s dest_addr:%u, addr:%u", __func__, dest_addr, addr);
    return ESP_OK;

}

// ------------------- HELPER FUNCTION ------------------- ]
