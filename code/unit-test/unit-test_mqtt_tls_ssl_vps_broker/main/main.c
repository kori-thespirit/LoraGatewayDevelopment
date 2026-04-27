#include "esp_err.h"
#include "esp_log.h"
#include "wifi_app.h"
#include "mqtts_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <stdint.h>

EventGroupHandle_t network_event_group;
static const char *TAG = "main";

void app_main(void) {
  network_event_group = xEventGroupCreate();
  wifi_station_init(&network_event_group);
  EventBits_t bits = xEventGroupWaitBits(network_event_group,
      NET_EG_BIT_GET_VALUE(NET_WIFI_IS_CONNECTED), 
      pdFALSE,
      pdFALSE,
      10000 / portTICK_PERIOD_MS);
  if(NET_EG_BIT_CHECK_BIT(bits, NET_EG_BIT_GET_VALUE(NET_WIFI_IS_CONNECTED))){
    ESP_LOGI(TAG, "Connecting %s ...", MQTT_URI);
    ESP_ERROR_CHECK(mqtts_app_start(&network_event_group));
    ESP_LOGI(TAG, "MQTT connected");
  }
  else {
    ESP_LOGE(TAG, "WiFi failed to connect");
  }
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
