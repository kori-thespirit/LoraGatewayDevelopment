/*
 *  Auther : Ta Thuan
 *  2026-05-31 08:32
 *  Project: MQTT test
 */
/*Private library*/
#include "MQTT.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

#define CMD_Write 0x06
#define CMD_Read 0x03

#define com_control_cmd 0x2000
#define com_setting_freq 0x2001

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
static uint16_t modbus_crc16(uint8_t *buffer, uint16_t length) {
  uint16_t crc = 0xFFFF;

  for (uint16_t i = 0; i < length; i++) {
    crc ^= buffer[i];

    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }

  return crc;
}
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
static void Send_Modbus_RTU(uint8_t slave, uint8_t CMD, uint16_t reg,
                            uint16_t value) {
  uint8_t frame[8];

  frame[0] = slave;
  frame[1] = CMD;

  frame[2] = reg >> 8;
  frame[3] = reg & 0xFF;

  frame[4] = value >> 8;
  frame[5] = value & 0xFF;

  uint16_t crc = modbus_crc16(frame, 6);

  frame[6] = crc & 0xFF;
  frame[7] = crc >> 8;

  uart_write_bytes(UART_PORT, (const char *)frame, sizeof(frame));

  printf("RTU TX : ");

  for (int i = 0; i < 8; i++)
    printf("%02X ", frame[i]);

  printf("\n");
}
void MQTT_Subcribe(const char *topic, const char *data, int topic_len,
                   int data_len) {
  char buff[256];
  if (data_len >= sizeof(buff)) {
    data_len = sizeof(buff) - 1;
  }
  memcpy(buff, data, data_len);
  buff[data_len] = '\0';
  cJSON *root = cJSON_Parse(buff);

  if (root == NULL) {
    cJSON_Delete(root);
    return;
  }
  cJSON *run = cJSON_GetObjectItem(root, "run");
  cJSON *frequency = cJSON_GetObjectItem(root, "frequency");
  cJSON *device_address = cJSON_GetObjectItem(root, "device address");

  if (run == NULL || frequency == NULL || device_address == NULL) {
    cJSON_Delete(root);
    return;
  }
  uint8_t slave = (uint8_t)cJSON_GetNumberValue(device_address);

  uint16_t freq = (uint16_t)cJSON_GetNumberValue(frequency);

  uint16_t run_cmd = (uint16_t)cJSON_GetNumberValue(run);

  printf("Slave=%u Freq=%u Run=%u\n", slave, freq, run_cmd);

  /* Set Frequency */
  Send_Modbus_RTU(slave, CMD_Write, com_setting_freq, freq * 100);

  vTaskDelay(pdMS_TO_TICKS(100));

  /* Run / Stop */
  Send_Modbus_RTU(slave, CMD_Write, com_control_cmd, run_cmd);
  cJSON_Delete(root);
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
  uint8_t rxbuf[256];

  static uint16_t status = 0;
  static uint16_t frequency = 0;

  while (1) {
    int len =
        uart_read_bytes(UART_PORT, rxbuf, sizeof(rxbuf), pdMS_TO_TICKS(10));

    if (len < 6) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    uint8_t address = rxbuf[0];
    uint16_t reg = ((uint16_t)rxbuf[3] << 8) | rxbuf[2];
    uint16_t value = ((uint16_t)rxbuf[5] << 8) | rxbuf[4];

    printf("RX: ADDR=%02X REG=%04X VAL=%04X\n", address, reg, value);

    if (reg == com_setting_freq) {
      frequency = value / 100;
      printf("Freq updated = %u\n", frequency);
    } else if (reg == com_control_cmd) {
      status = value;

      char json[128];
      snprintf(json, sizeof(json),
               "{\n"
               "  \"response\": {\n"
               "    \"status\": %u,\n"
               "    \"frequency\": %u,\n"
               "    \"device_address\": %u\n"
               "  }\n"
               "}",
               status, frequency, address);
      printf("MQTT TX: %s\n", json);

      MQTT_Publish(&MQTT_Ctrl, "test1", json, 0, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
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
