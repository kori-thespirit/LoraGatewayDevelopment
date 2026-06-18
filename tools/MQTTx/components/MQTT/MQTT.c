#include "MQTT.h"
#include "esp_event_base.h"
#include "mqtt_client.h"

static const char *TAG = "mqtts_handle";
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  MQTT_Handle_t *pHandle = (MQTT_Handle_t *)handler_args;

  esp_mqtt_event_handle_t event = event_data;

  switch (event->event_id) {
  case MQTT_EVENT_CONNECTED:

    ESP_LOGI(TAG, "MQTT Connected");

    if (pHandle->Register_Callback.connect_cb)
      pHandle->Register_Callback.connect_cb();

    break;

  case MQTT_EVENT_DISCONNECTED:

    ESP_LOGW(TAG, "MQTT Disconnected");

    if (pHandle->Register_Callback.disconnect_cb)
      pHandle->Register_Callback.disconnect_cb();

    break;

  case MQTT_EVENT_PUBLISHED:

    // ESP_LOGI(TAG, "Publish Success msg_id=%d", event->msg_id);

    if (pHandle->Register_Callback.publish_cb)
      pHandle->Register_Callback.publish_cb(event->msg_id);

    break;

  case MQTT_EVENT_DATA:
    if (pHandle->Register_Callback.rx_cb) {
      pHandle->Register_Callback.rx_cb(event->topic, event->data,
                                       event->topic_len, event->data_len);
    }

    break;

  case MQTT_EVENT_ERROR:

    ESP_LOGE(TAG, "MQTT Error");

    if (pHandle->Register_Callback.error_cb)
      pHandle->Register_Callback.error_cb();

    break;

  default:
    break;
  }
}
Authentication Auth_Enable(MQTT_Handle_t *pHandle, char *user, char *password) {
  size_t len_user = strlen(user); // Ví dụ string là "Thuan" sẽ có chuỗi là 5
  size_t len_password = strlen(password);

  pHandle->Auth.User =
      malloc(len_user + 1); // + 1 biến User là một mảng char + 1 là \0
  pHandle->Auth.Password = malloc(len_password + 1);
  if (pHandle->Auth.User == NULL || pHandle->Auth.Password == NULL) {
    ESP_LOGE(TAG, "Malloc failed");

    Authentication empty = {0};
    return empty;
  }
  strcpy(pHandle->Auth.User, user);
  strcpy(pHandle->Auth.Password, password);

  ESP_LOGI(TAG, "User          : %s", pHandle->Auth.User);
  ESP_LOGI(TAG, "User password : %s", pHandle->Auth.Password);
  pHandle->mqt_cfg.credentials.username = pHandle->Auth.User;

  pHandle->mqt_cfg.credentials.authentication.password = pHandle->Auth.Password;

  return pHandle->Auth;
}

void MQTT_App_Start(MQTT_Handle_t *pHandle, char *MQTT_URL) {
  if (pHandle == NULL || MQTT_URL == NULL) {
    return;
  }

  pHandle->mqt_cfg.broker.address.uri = MQTT_URL;

  pHandle->client = esp_mqtt_client_init(&pHandle->mqt_cfg);
  if (pHandle->client == NULL) {
    ESP_LOGE(TAG, "MQTT init failed");
    return;
  }
  esp_mqtt_client_register_event(pHandle->client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, pHandle);
  esp_mqtt_client_start(pHandle->client);
  ESP_LOGI(TAG, "MQTT Started");
}
void MQTT_Publish(MQTT_Handle_t *pHandle, const char *topic, const char *data,
                  int qos, int retain) {
  if (pHandle == NULL || pHandle->client == NULL) {
    return;
  }

  esp_mqtt_client_publish(pHandle->client, topic, data, 0, qos, retain);
}

void MQTT_Subscribe(MQTT_Handle_t *pHandle, const char *topic, int qos) {
  if (pHandle == NULL || pHandle->client == NULL) {
    return;
  }
  esp_mqtt_client_subscribe(pHandle->client, topic, qos);
}
//
// void MQTT_Register_Rx_Callback(MQTT_Handle_t *pHandle, mqtt_rx_callback_t cb)
// {
//   pHandle->rx_cb = cb;
// }

void MQTT_Register_Connect_Callback(MQTT_Handle_t *pHandle,
                                    mqtt_connect_callback_t cb) {
  pHandle->Register_Callback.connect_cb = cb;
}

void MQTT_Register_Disconnect_Callback(MQTT_Handle_t *pHandle,
                                       mqtt_disconnect_callback_t cb) {
  pHandle->Register_Callback.disconnect_cb = cb;
}

void MQTT_Register_Publish_Callback(MQTT_Handle_t *pHandle,
                                    mqtt_publish_callback_t cb) {
  pHandle->Register_Callback.publish_cb = cb;
}

void MQTT_Register_Error_Callback(MQTT_Handle_t *pHandle,
                                  mqtt_error_callback_t cb) {
  pHandle->Register_Callback.error_cb = cb;
}
void MQTT_Register_Rx_Callback(MQTT_Handle_t *pHandle, mqtt_rx_callback_t cb) {
  pHandle->Register_Callback.rx_cb = cb;
}
