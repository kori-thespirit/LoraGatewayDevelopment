#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdint.h>
#include "gd20_inverter.h"
#include "modbus_application_driver.h"

void app_main(void) {
  modbus_init();
  modbud_read_register(GD20_SLAVE_ID, GD20_REG_ID, 1);
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
