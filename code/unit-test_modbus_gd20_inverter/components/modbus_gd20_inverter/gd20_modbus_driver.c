#include "gd20_modbus_driver.h"
#include "esp_err.h"
#include "esp_modbus_common.h"
#include "esp_modbus_master.h"
#include "mbcontroller.h"
#include <assert.h>
#include <stdint.h>

char TAG[] = "GD20";

// Enumeration of modbus device addresses accessed by master device
enum {
  MB_DEVICE_ADDR1 = 1 // Default address of GD20 inverter
};

enum {
  CID_GD20_ID = 0,
  CID_GD20_MOTOR_START,
  CID_GD20_MOTOR_STOP,

};

holding_reg_params_t holding_reg_params;
const mb_parameter_descriptor_t gd20_parameter[] = {
    // { CID, Param Name, Units, Modbus Slave Addr, Modbus Reg Type, Reg Start,
    // Reg Size, Instance Offset, Data Type, Data Size, Parameter Options,
    // Access Mode}
    {CID_GD20_ID,
     STR("GD_20 ID"),
     STR("--"),
     MB_DEVICE_ADDR1,
     MB_PARAM_HOLDING,
     0,
     2,
     2,
     PARAM_TYPE_U16,
     2,
     {.opt1 = 0, .opt2 = 0, .opt3 = 0},
     PAR_PERMS_READ},
};

static void *master_handle = NULL;
// The function to get pointer to parameter storage (instance) according to
// parameter description table
static void *
master_get_param_data(const mb_parameter_descriptor_t *param_descriptor) {
  assert(param_descriptor != NULL);
  void *instance_ptr = NULL;
  if (param_descriptor->param_offset != 0) {
    switch (param_descriptor->mb_param_type) {
    case MB_PARAM_HOLDING:
      ESP_LOGI(TAG, "param_offset: %lu", param_descriptor->param_offset);
      instance_ptr =
          ((void *)&holding_reg_params + param_descriptor->param_offset - 1);
      break;
    default:
      instance_ptr = NULL;
      break;
    }
  } else {
    ESP_LOGE(TAG, "Wrong parameter offset for CID #%u",
             (unsigned)param_descriptor->cid);
    assert(instance_ptr != NULL);
  }
  return instance_ptr;
}
// Modbus master initialization
esp_err_t master_init(void) {
  // Initialize Modbus controller
  mb_communication_info_t comm = {.ser_opts.port = MB_PORT_NUM,
                                  .ser_opts.mode = MB_RTU,
                                  .ser_opts.baudrate = MB_DEV_SPEED,
                                  .ser_opts.parity = MB_PARITY_NONE,
                                  .ser_opts.uid = 0,
                                  .ser_opts.response_tout_ms = 1000,
                                  .ser_opts.data_bits = UART_DATA_8_BITS,
                                  .ser_opts.stop_bits = UART_STOP_BITS_1};

  esp_err_t err = mbc_master_create_serial(&comm, &master_handle);
  MB_RETURN_ON_FALSE((master_handle != NULL), ESP_ERR_INVALID_STATE, TAG,
                     "mb controller initialization fail.");
  MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                     "mb controller initialization fail, returns(0x%x).",
                     (int)err);

  // Set UART pin numbers
  err = uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD, CONFIG_MB_UART_RXD,
                     CONFIG_MB_UART_RTS, UART_PIN_NO_CHANGE);
  MB_RETURN_ON_FALSE(
      (err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
      "mb serial set pin failure, uart_set_pin() returned (0x%x).", (int)err);

  // Set driver mode to Half Duplex
  err = uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);
  MB_RETURN_ON_FALSE(
      (err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
      "mb serial set mode failure, uart_set_mode() returned (0x%x).", (int)err);

  vTaskDelay(5);

  err = mbc_master_start(master_handle);
  MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                     "mb controller start fail, returned (0x%x).", (int)err);

  ESP_LOGI(TAG, "Modbus master stack initialized...");

  err = mbc_master_set_descriptor(master_handle, &gd20_parameter[0], 1);
  MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG,
                     "mb controller set descriptor fail, returns(0x%x).",
                     (int)err);
  return err;
}
// User operation function to read slave values and check alarm
void master_operation_func(void *arg) {
  esp_err_t err = ESP_OK;
  const mb_parameter_descriptor_t *param_descriptor = NULL;
  err = mbc_master_get_cid_info(master_handle, CID_GD20_ID, &param_descriptor);
  assert(err == ESP_OK && param_descriptor != NULL);
  void *temp_data_ptr = master_get_param_data(param_descriptor);
  assert(temp_data_ptr);
  uint8_t type = 0;
  err = mbc_master_get_parameter(master_handle, param_descriptor->cid,
                                 temp_data_ptr, &type);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Characteristic #%d (%s) read fail, err = 0x%x (%s).",
             (int)param_descriptor->cid, (char *)param_descriptor->param_key,
             (int)err, (char *)esp_err_to_name(err));
  } else {
    ESP_LOGI(TAG,
             "Characteristic #%d %s (%s) value = (0x%" PRIx16
             ") read successful.",
             (int)param_descriptor->cid, (char *)param_descriptor->param_key,
             (char *)param_descriptor->param_units, *(uint16_t *)temp_data_ptr);
  }
}

void master_test(void) {
  mb_param_request_t request = {
      .slave_addr = 1,
      .command = 0x11,
      .reg_start = 0x2103,
      .reg_size = 1,
  };
}
