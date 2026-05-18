/* -------------------------- LICENSE placeholder -------------------------- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "modbus_application_driver.h"
#include "gd20_inverter.h"
#include "lora_protocol.h"


static const char *TAG = "modbus";

void modbus_task(void* pvParameters)
{
  modbus_init();
  // st_inverter_gd20_t gd20_data;
  for(;;){
    // gd20_data.frequency = modbud_read_single_register(GD20_SLAVE_ID, GD20_OPERATION_FREQ, 1);
    // gd20_data.speed = modbud_read_single_register(GD20_SLAVE_ID, GD20_OPERATION_SPEED, 1);
    // gd20_data.out_i = modbud_read_single_register(GD20_SLAVE_ID, GD20_OUTPUT_CURRENT, 1);
    // gd20_data.out_v = modbud_read_single_register(GD20_SLAVE_ID, GD20_OUTPUT_VOLTAGE, 1);
    // gd20_data.frequency = 5000;
    // gd20_data.speed = 380;
    // gd20_data.out_i = 3.5;
    // gd20_data.out_v = 220;
    // ESP_LOGI(TAG, "GD20 Data - Freq: %.2f Hz, Speed: %.2f rpm, Current: %.2f A, Voltage: %.2f V", gd20_data.frequency, gd20_data.speed, gd20_data.out_i, gd20_data.out_v);
    // uint8_t out_buf[MAX_LORA_PACKET];
    // e_lora_protocol_err_t protocol_err = m_lora_protocol_frame_pack(out_buf,
    //                                                             sizeof(out_buf),  
    // vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
 