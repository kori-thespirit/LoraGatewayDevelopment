/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "driver/usb_serial_jtag.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#define BUF_SIZE (1024)
#define ECHO_TASK_STACK_SIZE (4096)
const char *tag = "USB Serial";
typedef enum { IDLE, START, RUN, STOP, ERROR } State_Test;
State_Test para_cmd(char *cmd) {
  if (strcmp(cmd, "IDLE") == 0)
    return IDLE;
  else if (strcmp(cmd, "START") == 0)
    return START;
  else if (strcmp(cmd, "RUN") == 0)
    return RUN;
  else if (strcmp(cmd, "STOP") == 0)
    return STOP;
  else
    return ERROR;
}
static void echo_task(void *arg) {
  // Configure USB SERIAL JTAG
  usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
      .rx_buffer_size = BUF_SIZE,
      .tx_buffer_size = BUF_SIZE,
  };

  ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
  ESP_LOGI("usb_serial_jtag echo", "USB_SERIAL_JTAG init done");

  // Configure a temporary buffer for the incoming data
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  if (data == NULL) {
    ESP_LOGE("usb_serial_jtag echo", "no memory for data");
    return;
  }

  while (1) {

    /*ESP_LOGE("Tag", "Hello world");
    vTaskDelay(1000 / portTICK_PERIOD_MS);*/
    int len = usb_serial_jtag_read_bytes(data, (BUF_SIZE - 1),
                                         20 / portTICK_PERIOD_MS);

    // Write data back to the USB SERIAL JTAG
    if (len) {
      char buff[10] = {};
      data[len] = '\0';
      /*
      ESP_LOG_BUFFER_HEXDUMP("Recv str: ", data, len, ESP_LOG_INFO);
      */
      sscanf((char *)data, "%s", buff);

      // printf("response : %s\n", buff);
      // ESP_LOGI(tag, "response :%s\n", buff);
      // usb_serial_jtag_write_bytes(buff, strlen(buff), 20 /
      // portTICK_PERIOD_MS);
      switch (para_cmd(buff)) {
      case IDLE:
        ESP_LOGI("IDLE", "Program is ready to test\n");
        break;
      case START:
        ESP_LOGI("START", "Program is start to test\n");
        break;
      case RUN:
        ESP_LOGI("RUN", "Program is running\n");
        break;
      case STOP:
        ESP_LOGI("STOP", "Program is stop\n");
        ESP_LOGI("STOP", "Closing....\n");
        break;
      case ERROR:
        ESP_LOGI("ERROR", "Wrong command\n");
        break;
      default:
        break;
      }
    }
  }
}
void app_main(void) {
  xTaskCreate(echo_task, "USB SERIAL JTAG_echo_task", ECHO_TASK_STACK_SIZE,
              NULL, 10, NULL);
}
