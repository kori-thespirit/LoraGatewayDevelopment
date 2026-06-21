/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef MQTTS_APP_H
#define MQTTS_APP_H
#include "network_event_group_define.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"

#define MQTT_URI CONFIG_MQTT_URI
// #define MQTT_USERNAME CONFIG_MQTT_USERNAME
// #define MQTT_PASSWORD CONFIG_MQTT_PASSWORD

// #define MQTT_URI "mqtts://kolabori.com.vn"
// #define MQTT_USERNAME "example"
// #define MQTT_PASSWORD "12345"

typedef void (* p_mqtt_data_cb)(esp_mqtt_event_handle_t event_data);
typedef void (* p_mqtt_error_cb)(esp_mqtt_error_codes_t *errhandle);
esp_err_t mqtts_app_start(EventGroupHandle_t *net_eg);

esp_err_t mqtts_app_register_callback(
        void (* p_mqtt_data_cb)     (esp_mqtt_event_handle_t),
        void (* p_mqtt_error_cb)    (esp_mqtt_error_codes_t *));
esp_err_t mqtts_app_use_subscribe_list(const char **sublist, uint8_t total);
esp_err_t mqtts_app_publish(const char* topic, char* data);
#endif // MQTTS_APP_H
