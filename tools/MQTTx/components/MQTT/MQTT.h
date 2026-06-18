#ifndef _MQTT_H

#define _MQTT_H
#include "esp_log.h"
#include "mqtt_client.h"
#include <string.h>

typedef struct Authentication {
  char *User;
  char *Password;
} Authentication;

// typedef void (*mqtt_rx_callback_t)(const char *topic, const char *data);

typedef void (*mqtt_connect_callback_t)(void);

typedef void (*mqtt_disconnect_callback_t)(void);

typedef void (*mqtt_publish_callback_t)(int msg_id);

typedef void (*mqtt_error_callback_t)(void);

typedef void (*mqtt_rx_callback_t)(const char *topic, const char *data,
                                   int topic_len, int data_len);

typedef struct MQTT_context {
  Authentication Auth;
  esp_mqtt_client_handle_t client;
  esp_mqtt_client_config_t mqt_cfg;
  struct {
    // mqtt_rx_callback_t rx_cb;
    mqtt_connect_callback_t connect_cb;
    mqtt_disconnect_callback_t disconnect_cb;
    mqtt_publish_callback_t publish_cb;
    mqtt_error_callback_t error_cb;
    mqtt_rx_callback_t rx_cb;
  } Register_Callback;
} MQTT_Handle_t;

Authentication Auth_Enable(MQTT_Handle_t *pHandle, char *user, char *password);

void MQTT_App_Start(MQTT_Handle_t *pHandle, char *MQTT_URL);
void MQTT_Publish(MQTT_Handle_t *pHandle, const char *topic, const char *data,
                  int qos, int retain);

void MQTT_Subscribe(MQTT_Handle_t *pHandle, const char *topic, int qos);

// void MQTT_Register_Rx_Callback(MQTT_Handle_t *pHandle, mqtt_rx_callback_t
// cb);

void MQTT_Register_Connect_Callback(MQTT_Handle_t *pHandle,
                                    mqtt_connect_callback_t cb);

void MQTT_Register_Disconnect_Callback(MQTT_Handle_t *pHandle,
                                       mqtt_disconnect_callback_t cb);

void MQTT_Register_Publish_Callback(MQTT_Handle_t *pHandle,
                                    mqtt_publish_callback_t cb);

void MQTT_Register_Error_Callback(MQTT_Handle_t *pHandle,
                                  mqtt_error_callback_t cb);

void MQTT_Register_Rx_Callback(MQTT_Handle_t *pHandle, mqtt_rx_callback_t cb);
#endif
