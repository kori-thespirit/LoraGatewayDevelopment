#ifndef WIFI_APP_H
#define WIFI_APP_H
#include "network_event_group_define.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define MAXIMUM_RETRY  5
// #define WIFI_SSID      CONFIG_ESP_WIFI_SSID
// #define WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD

#define WIFI_SSID      "KEESOORI"
#define WIFI_PASS      "248715121a"

void wifi_station_init(EventGroupHandle_t *net_eg);

#endif // WIFI_APP_H
