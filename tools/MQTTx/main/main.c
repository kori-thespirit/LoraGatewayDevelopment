/*
 *  Auther : Ta Thuan
 *  2026-05-31 08:32
 *  Project: MQTT test
 */
/*Private library*/
#include "MQTT.h"
#include "driver/uart.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

#define UART_PORT UART_NUM_2
#define TXD_PIN 17
#define RXD_PIN 16

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
  MQTT_Subscribe(&MQTT_Ctrl, "test1", 0);
  // vTaskDelay(pdMS_TO_TICKS(3000));
}

// void MQTT_Published(int msg_id) {
//   // ESP_LOGI("APP",
//   //          "Publish success msg_id=%d",
//   //          msg_id);
//   MQTT_Publish(&MQTT_Ctrl, "test1", "Hello", 0, 0);
//   vTaskDelay(pdMS_TO_TICKS(3000));
// }

void MQTT_Subcribe(const char *topic, const char *data, int topic_len,
                   int data_len) {
  if (data_len < 1)
    return;
  uint8_t frame[64];

  int frame_len = data_len / 2;

  for (int i = 0; i < frame_len; i++) {
    char tmp[3];

    tmp[0] = data[i * 2];
    tmp[1] = data[i * 2 + 1];
    tmp[2] = '\0';

    frame[i] = (uint8_t)strtol(tmp, NULL, 16);
  }
  uart_write_bytes(UART_PORT, (const char *)frame, frame_len);
  printf("%02X%02X%02X%02X%02X%02X%02X%02X", frame[0], frame[1], frame[2],
         frame[3], frame[4], frame[5], frame[6], frame[7]);

  printf("\n");
}
void MQTT_Disconnected(void) { ESP_LOGW("APP", "MQTT Disconnected"); }

void MQTT_Error(void) { ESP_LOGE("APP", "MQTT Error"); }
void uart_init_app(void) {
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };

  uart_driver_install(UART_PORT, 1024, 0, 0, NULL, 0);
  uart_param_config(UART_PORT, &uart_config);
  uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);
}
void uart_task(void *arg) {
  uint8_t rxbuf[128];

  while (1) {
    int len =
        uart_read_bytes(UART_PORT, rxbuf, sizeof(rxbuf), pdMS_TO_TICKS(100));
    if (len > 0) {
      for (int i = 0; i < len; i++) {
        printf("%02X ", rxbuf[i]);
      }
      printf("\n");
    }
  }
}
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
  MQTT_Register_Rx_Callback(&MQTT_Ctrl, MQTT_Subcribe);
  MQTT_App_Start(&MQTT_Ctrl, Mosquitto_URI);

  uart_init_app();
  xTaskCreate(uart_task, "uart_task", 4096, NULL, 5, NULL);
}
void main_loops() {
  // char buff[64];

  // MQTT_Publish(&MQTT_Ctrl, "test1", buff, 0, 0);
  // ESP_LOGI("mess", "%s", buff);
  vTaskDelay(pdMS_TO_TICKS(3000));
}
void app_main(void) {
  khoitao();
  while (1) {
    main_loops();
  }
}
