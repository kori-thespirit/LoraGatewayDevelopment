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

void network_task(void* pvParameters) {
    ESP_LOGI(TAG, "task created");
    network_event_group = xEventGroupCreate();
    printf("LOG START\r\n");
    uint32_t bit_to_wait = NET_EG_BIT_GET_VALUE(NET_WIFI_IS_CONNECTED) | NET_EG_BIT_GET_VALUE(NET_ETH_IS_CONNECTED);
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
    if(NET_EG_BIT_CHECK_BIT(bits, bit_to_wait)){
        ESP_LOGI(TAG, "Connecting %s ...", MQTT_URI);
        ESP_ERROR_CHECK(mqtts_app_start(&network_event_group));
        ESP_LOGI(TAG, "MQTT connected");
    }
    else {
        ESP_LOGE(TAG, "Failed to connect to network");
    }
    for(;;){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
