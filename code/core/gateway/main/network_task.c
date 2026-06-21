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

EventGroupHandle_t network_event_group;
static const char *TAG = "network";

#define TOTAL_TOPIC(_X_) TOTAL_INDEX(_X_, const char*)
const char *sub_topic_list[] = {
    "/topic/project/lora/gateway/#",
    "/topic/project/lora/node/2/gd20/control",
};

const char *pub_topic_list[] = {
    "/topic/project/lora/node/2/gd20/data",
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
        sprintf(buffer, mqtt_gd20_data_json_format, 1.2, 50.0, 198.12, 2000.0, 39.42);
        ESP_ERROR_CHECK(mqtts_app_publish(*(pub_topic_list + 0), buffer));
    }
}

static void data_receive_handle(esp_mqtt_event_handle_t event)
{
    ESP_LOGI(TAG, "MQTT get data:%s, %d",__func__, __LINE__);

}

static void error_handle(esp_mqtt_error_codes_t *error)
{
    ESP_LOGE(TAG, "Func:%s, line:%d",__func__, __LINE__);

}
