/*
 *  Auther : Ta Thuan
 *  2026-05-31 08:32
 *  Project: MQTT test
 */
/*Private library*/
#include "MQTT.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
/*Private Marco*/
#define WIFI_SSID "Thuan"
#define WIFI_PASS "chi12345"

#define User "Thuan"
#define Password "nihongo"

#define Mosquitto_URI "mqtt://192.168.1.10:1884" // ip + Port number
/*Private Variables*/
MQTT_Handle_t MQTT_Ctrl;
/*Private Functions*/
void MQTT_Conected(void) {
  ESP_LOGI("APP", "MQTT Connected");
  MQTT_Publish(&MQTT_Ctrl, "test1", "Conect success", 1, 0);
  // vTaskDelay(pdMS_TO_TICKS(3000));
}

void MQTT_Published(int msg_id) {
  // ESP_LOGI("APP",
  //          "Publish success msg_id=%d",
  //          msg_id);
  MQTT_Publish(&MQTT_Ctrl, "test1", "Hello", 0, 0);
  vTaskDelay(pdMS_TO_TICKS(3000));
}

void MQTT_Disconnected(void) { ESP_LOGW("APP", "MQTT Disconnected"); }

void MQTT_Error(void) { ESP_LOGE("APP", "MQTT Error"); }
void khoitao(void) {
  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = WIFI_SSID,
              .password = WIFI_PASS,
          },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI("WIFI ", "Connecting WiFi...");

  ESP_ERROR_CHECK(esp_wifi_connect());
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

  esp_netif_ip_info_t ip;

  while (1) {
    esp_netif_get_ip_info(netif, &ip);

    if (ip.ip.addr != 0) {
      ESP_LOGI("NET", "IP=" IPSTR, IP2STR(&ip.ip));
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  Auth_Enable(&MQTT_Ctrl, User, Password);
  MQTT_Register_Connect_Callback(&MQTT_Ctrl, MQTT_Conected);
  // MQTT_Register_Publish_Callback(&MQTT_Ctrl, MQTT_Published);
  MQTT_Register_Disconnect_Callback(&MQTT_Ctrl, MQTT_Disconnected);
  MQTT_Register_Error_Callback(&MQTT_Ctrl, MQTT_Error);
  MQTT_App_Start(&MQTT_Ctrl, Mosquitto_URI);
}
void main_loops() {
  static uint32_t count = 0;
  char buff[64];
  sprintf(buff, "Hello %lu", count++);
  MQTT_Publish(&MQTT_Ctrl, "test1", buff, 0, 0);
  ESP_LOGI("mess", "%s", buff);
  vTaskDelay(pdMS_TO_TICKS(3000));
}
void app_main(void) {
  khoitao();
  while (1) {
    main_loops();
  }
}
