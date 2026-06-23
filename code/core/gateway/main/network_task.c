/* -------------------------- LICENSE placeholder -------------------------- */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "wifi_app.h"
#include "mqtts_app.h"
#include <ctype.h>
#include <cJSON.h>

EventGroupHandle_t network_event_group;
static const char *TAG = "network";

#define TOTAL_TOPIC(_X_) TOTAL_INDEX(_X_, const char*)
const char *sub_topic_list[] = {
    "/topic/project/lora/gateway/#",
    "/topic/project/lora/node/+/devices/+/control",
};

const char *pub_topic_list[] = {
    "/topic/project/lora/node/2/devices/gd20/data",
};

const char *example_topic_list[] = {
    "/topic/example/#",
};

/*
char *mqtt_gd20_data_json_format = "\
{                                   \
    \"Ouput Current\":%.2f A   ,    \
    \"Frequency\":%.2f     Hz  ,    \
    \"Out Voltage\":%.2f   V   ,    \
    \"Speed\":%.2f         rpm ,    \
    \"Temperature\":%.2f   °C       \
}";
*/

char *mqtt_gd20_data_json_format =\
" { \"Ouput Current\":%.2f, \"Frequency\":%.2f, \"Out Voltage\":%.2f, \"Speed\":%.2f, \"Temperature\":%.2f }";

static void data_receive_handle(esp_mqtt_event_handle_t event);
static void error_handle(esp_mqtt_error_codes_t *error);

void network_task(void* pvParameters) {
    ESP_LOGI(TAG, "task created");
    network_event_group = xEventGroupCreate();
    printf("LOG START\r\n");
    uint32_t bit_to_wait = 
        BIT_TO_VALUE(NET_WIFI_IS_CONNECTED) |
        BIT_TO_VALUE(NET_ETH_IS_CONNECTED);
#if CONFIG_NETWORK_WIFI
    wifi_station_init(&network_event_group);
#elif CONFIG_NETWORK_ETH 
    w5500_init(&network_event_group);
#endif
    EventBits_t bits = xEventGroupWaitBits(network_event_group,
            bit_to_wait,
            pdFALSE,
            pdFALSE,
            10000 / portTICK_PERIOD_MS);
    if(CHECK_BIT(bits, bit_to_wait)){
        ESP_LOGI(TAG, "Connecting to %s ... broker", MQTT_URI);
        ESP_ERROR_CHECK(mqtts_app_start(&network_event_group));
        ESP_ERROR_CHECK(mqtts_app_register_callback(data_receive_handle, error_handle));
        ESP_ERROR_CHECK(mqtts_app_use_subscribe_list(sub_topic_list, TOTAL_TOPIC(sub_topic_list)));
        ESP_LOGI(TAG, "MQTT connected");
    }
    else {
        ESP_LOGE(TAG, "Failed to connect to network");
    }

    ESP_LOGI(TAG, "size of mqtt_gd20_data_json_format:%d", sizeof(mqtt_gd20_data_json_format));
    char buffer[sizeof(mqtt_gd20_data_json_format) + 100] = {0};
    for(;;){
        vTaskDelay(pdMS_TO_TICKS(5000));
        // sprintf(buffer, mqtt_gd20_data_json_format, 1.2, 50.0, 198.12, 2000.0, 39.42);
        // ESP_ERROR_CHECK(mqtts_app_publish(*(pub_topic_list + 0), buffer));
    }
}

static void handle_topic_request(esp_mqtt_event_handle_t event)
{
    char *topic = (char*)calloc(event->topic_len + 1, sizeof(char));
    char *json_str = (char*)calloc(event->data_len + 1, sizeof(char));
    memcpy(topic, event->topic, event->topic_len);
    memcpy(json_str, event->data, event->data_len);
    // ESP_LOGI(TAG, "topic:%s", topic);
    // ESP_LOGI(TAG, "json_str:%s", json_str);

    char project_topic[] = "/topic/project/lora/";
    size_t project_topic_length = strlen(project_topic);

    /* Get the device topic, e.g: node/2/gd20/control 
     * Extracted from topic /topic/project/lora/node/2/gd20/control */
    char *extracted_device_topic = topic + project_topic_length;
    // ESP_LOGI(TAG, "extracted_device_topic:%s", extracted_device_topic);

    char *token = strtok(extracted_device_topic, "/");
    char token_storage[5][10] = {0};
    uint8_t i = 0;
    while (token != NULL) {
        strcpy(token_storage[i++], token);
        // ESP_LOGI(TAG, "token_storage[%u]:%s", i - 1, token_storage[i - 1]);
        token = strtok(NULL, "/");
    }

    for(uint8_t i = 0; i < strlen(token_storage[1]); i++) {
        char test = token_storage[1][i];
        if(!isdigit(test)) {
            ESP_LOGE(TAG, "%s:Lora address is not digit, refuse to process",__func__);
            return;
        }
    }
    uint8_t lora_address = atoi(token_storage[1]);
    // ESP_LOGI(TAG, "lora_address:%u", lora_address);

    // parse the JSON data
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error at: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        return;
    }

    bool motor_run = false;
    bool forward = false;
    uint16_t frequency = 0;
    uint8_t modbus_device_addr = 0;
    cJSON *temp = cJSON_GetObjectItemCaseSensitive(json, "run");
    if(cJSON_IsBool(temp))
        if(cJSON_IsTrue(temp))
            motor_run = true;

    temp = cJSON_GetObjectItemCaseSensitive(json, "forward");
    if(cJSON_IsBool(temp))
        if(cJSON_IsTrue(temp))
            forward = true;

    temp = cJSON_GetObjectItemCaseSensitive(json, "frequency");
    if(cJSON_IsNumber(temp))
        frequency = (uint16_t)cJSON_GetNumberValue(temp);

    temp = cJSON_GetObjectItemCaseSensitive(json, "device address");
    if(cJSON_IsNumber(temp))
        modbus_device_addr = (uint8_t)cJSON_GetNumberValue(temp);

    ESP_LOGI(TAG, "motor_run:%u", motor_run);
    ESP_LOGI(TAG, "forward:%u", forward);
    ESP_LOGI(TAG, "frequency:%u", frequency);
    ESP_LOGI(TAG, "modbus_device_addr:%u", modbus_device_addr);

    // delete the JSON object
    cJSON_Delete(json);
    free(topic);
    free(json_str);
}

static void data_receive_handle(esp_mqtt_event_handle_t event)
{
    ESP_LOGI(TAG, "MQTT get data:%s, %d",__func__, __LINE__);
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    handle_topic_request(event);

}

static void error_handle(esp_mqtt_error_codes_t *error)
{
    ESP_LOGE(TAG, "Func:%s, line:%d",__func__, __LINE__);

}
