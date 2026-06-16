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
void parse_complete(void *pvParameters, st_lora_protocol_header_t header);
static void lora_send();
static esp_err_t intertask_handle();
static esp_err_t relay_intertask(e_task_handle_id_t taskid, st_intertask_data_t idata);

static e_task_handle_id_t reply_task_handle_id = 0;
static uint8_t src_addr = 0; 
static uint8_t dest_addr = 0;
static uint8_t dev_addr = 2;
static e_lora_function_t lorafunc = LORA_FUNC_LISTEN_ONLY;
static uint8_t buf[40] = {0};

void lora_task(void* pvParameters) {
    e_lora_protocol_err_t protocol_err = m_lora_protocol_register_callback(&pack_complete, &parse_complete); 
    if(LORA_PROTOCOL_OK != protocol_err){
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

    while (1) {
        // test_send();
        ESP_ERROR_CHECK(intertask_handle());
        lora_receive();  // put into receive mode
        if (lora_received()) {
            int rxLen = lora_receive_packet(buf, sizeof(buf));
            for(uint8_t i = 0; i < sizeof(buf); i++) {
                printf("0x%x ", buf[i]);
            }
            printf("\n");
            m_lora_protocol_frame_parse(buf, sizeof(buf));
        }
        int lost = lora_packet_lost();
        if (lost != 0) {
            ESP_LOGW(TAG, "%d packets lost", lost);
        }
    }
    vTaskDelete(NULL);
}

void pack_complete(void *pvParameters)
{
    uint8_t *frame_length = (uint8_t*)pvParameters;
    ESP_LOGI(TAG, "Sending message via lora complete with %u bytes", *frame_length);
}

void parse_complete(void *pvParameters, st_lora_protocol_header_t header)
{
    ESP_LOGI(TAG, "payload_length: %u", header.payload_length);
    ESP_LOGI(TAG, "src_addr: %u", header.src_addr);
    ESP_LOGI(TAG, "dest_addr: %u", header.dest_addr);
    ESP_LOGI(TAG, "header.dest_addr:%u, dev_addr:%u", header.dest_addr, dev_addr);
    if(header.dest_addr == dev_addr) {
       ESP_LOGI(TAG, "This message is for me");
       src_addr =  dev_addr;
       dest_addr = header.src_addr; // The reply address is the received src_addr
    }
    else {
        /* TODO: Relay to adjacent lora node */
        return;
    }
    st_core_data_t *coredata = (st_core_data_t*)pvParameters;
    e_core_data_id_t cdataid = coredata->cdataid;
    st_intertask_data_t idata = {
        .src_task_handle_id = TASK_ID_LORA,
        .coredata = *coredata,
    };
    ESP_LOGI(TAG, "src_addr:%u, dest_addr:%u, cdataid:%d", src_addr, dest_addr, cdataid);
    switch(cdataid){
        /* Relay to network task */
        case COREDATA_ID_NET:
            relay_intertask(TASK_ID_NETWORK,idata);
            break;
        /* Relay to modbus task */
        case COREDATA_ID_MB_DATA:
        case COREDATA_ID_MB_CFG:
            ESP_LOGI(TAG, "Relay to Modbus task");
            relay_intertask(TASK_ID_MODBUS,idata);
            break;
        /* Handle Lora data itself */
        case COREDATA_ID_LORA_CFG_REG:
            st_lora_cfg_reg_t loracfg;
            memcpy((void*)&loracfg, (void*)coredata->cdata, sizeof(st_lora_cfg_reg_t));
            ESP_LOGI(TAG, "Get lora config, sf:%u, cr:%u, bw:%u, freq:%lu", loracfg.cr, loracfg.bw, loracfg.freq);
            break;
        default:
            ESP_LOGE(TAG, "Core data ID not supported");
            break;
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
        case TASK_ID_MODBUS:
            task_handle = get_modbus_task_handle();
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
    e_intertask_err_t ierr = notifydata.notivalue.intertask_err;

    /* Bypass xQueueReceive if intertask reply with status */
    if(INTERTASK_ERR_NOT_USE != ierr) {
        ESP_LOGW(TAG, "Relay intertask has status:%d", ierr);
        return ESP_OK;
    }
    if(xQueueReceive(*p_queue, (void*)&idata, pdMS_TO_TICKS(100)) == pdPASS){

        st_core_data_t coredata =   idata.coredata;
        reply_task_handle_id = idata.src_task_handle_id;
        if(COREDATA_ID_LORA_CFG_REG == coredata.cdataid){
            /* TODO: Handle this case */
            return ESP_OK;
        }

        /* Perform core function */
        e_lora_protocol_err_t protocol_err = m_lora_protocol_frame_pack((void*)buf, sizeof(buf), (void*)&coredata, sizeof(st_core_data_t), src_addr, dest_addr, 0);
        if(LORA_PROTOCOL_OK != protocol_err) {
            ESP_LOGE(TAG, "%s:Pack frame data failed, err:%d", __func__, protocol_err);
        }
        lora_send_packet(buf, sizeof(buf)); // TODO: Sending all buffer is not neccessary, need to reduce this
        bzero(buf, sizeof(buf));
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
