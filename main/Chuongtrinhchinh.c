#include "STM32_Slave.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "hal/spi_types.h"

#include <stdio.h>

#define SCK_PIN GPIO_NUM_14
#define MOSI_PIN GPIO_NUM_13
#define MISO_PIN GPIO_NUM_12
#define CS_PIN GPIO_NUM_15

STM32_Slave_handle_t STM32_Slave_handle;
void setup(void) {
  SPI_config esp32_master_cfg = {.host = SPI2_HOST,
                                 .SCK = SCK_PIN,
                                 .MOSI = MOSI_PIN,
                                 .MISO = MISO_PIN,
                                 .CS = CS_PIN};

  STM32_Slave_Init(&STM32_Slave_handle, &esp32_master_cfg);
}
void app_main(void) {

  setup();
  while (1) {
    STM32_Begin_Get_Info(&STM32_Slave_handle);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}
