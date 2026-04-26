#include "esp_err.h"
#include "wifi_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include "freertos/event_groups.h"

EventGroupHandle_t network_event_group;

void app_main(void) {
  network_event_group = xEventGroupCreate();

  wifi_station_init(&network_event_group);

  // ESP_ERROR_CHECK(mqtt_app_start());
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
