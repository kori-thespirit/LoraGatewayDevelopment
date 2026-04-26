#ifndef MQTTS_APP_H
#define MQTTS_APP_H
#include "network_event_group_define.h"
#include "esp_err.h"

#define MQTT_URI CONFIG_MQTT_URI

esp_err_t mqtts_app_start(void);
#endif // MQTTS_APP_H
