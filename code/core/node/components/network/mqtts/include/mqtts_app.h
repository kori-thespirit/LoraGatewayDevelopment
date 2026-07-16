/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef MQTTS_APP_H
#define MQTTS_APP_H
#include "network_event_group_define.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// #define MQTT_URI CONFIG_MQTT_URI
// #define MQTT_USERNAME CONFIG_MQTT_USERNAME
// #define MQTT_PASSWORD CONFIG_MQTT_PASSWORD

#define MQTT_URI "mqtts://kolabori.com.vn"
#define MQTT_USERNAME "example"
#define MQTT_PASSWORD "12345"

esp_err_t mqtts_app_start(EventGroupHandle_t *net_eg);
#endif // MQTTS_APP_H
