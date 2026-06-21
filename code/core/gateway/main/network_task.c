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
const char *loragateway_topic_list[] = {
    "/topic/project/lora/gateway/#",
    "/topic/project/lora/node/#",
};

const char *example_topic_list[] = {
    "/topic/example/#",
};

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
        ESP_ERROR_CHECK(mqtts_app_use_subscribe_list(loragateway_topic_list, TOTAL_TOPIC(loragateway_topic_list)));
        ESP_LOGI(TAG, "MQTT connected");
    }
    else {
        ESP_LOGE(TAG, "Failed to connect to network");
    }

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void data_receive_handle(esp_mqtt_event_handle_t event)
{
    ESP_LOGI(TAG, "MQTT get data:%s, %d",__func__, __LINE__);

}

static void error_handle(esp_mqtt_error_codes_t *error)
{
    ESP_LOGE(TAG, "MQTT error:%s, %d",__func__, __LINE__);

}
