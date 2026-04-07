#ifndef GD20_MODBUS_DRIVER_H
#define GD20_MODBUS_DRIVER_H

#include "esp_err.h"
#include "sdkconfig.h"
#include <stdint.h>
// Number of UART port used for Modbus connection
#define MB_PORT_NUM (CONFIG_MB_UART_PORT_NUM)
// The communication speed of the UART
#define MB_DEV_SPEED (CONFIG_MB_UART_BAUD_RATE)

// Note: Some pins on target chip cannot be assigned for UART communication.
// See UART documentation for selected board and target to configure pins using
// Kconfig.

// The number of parameters that intended to be used in the particular control
// process
#define MASTER_MAX_CIDS num_device_parameters

// Number of reading of parameters from slave
#define MASTER_MAX_RETRY (10)

// Timeout to update cid over Modbus
#define UPDATE_CIDS_TIMEOUT_MS (500)
#define UPDATE_CIDS_TIMEOUT_TICS (UPDATE_CIDS_TIMEOUT_MS / portTICK_PERIOD_MS)

// Timeout between polls
#define POLL_TIMEOUT_MS (1)
#define POLL_TIMEOUT_TICS (POLL_TIMEOUT_MS / portTICK_PERIOD_MS)

// The macro to get offset for parameter in the appropriate structure
#define HOLD_OFFSET(field)                                                     \
  ((uint16_t)(offsetof(holding_reg_params_t, field) + 1))
#define INPUT_OFFSET(field)                                                    \
  ((uint16_t)(offsetof(input_reg_params_t, field) + 1))
#define COIL_OFFSET(field) ((uint16_t)(offsetof(coil_reg_params_t, field) + 1))
// Discrete offset macro
#define DISCR_OFFSET(field)                                                    \
  ((uint16_t)(offsetof(discrete_reg_params_t, field) + 1))

#define STR(fieldname) ((const char *)(fieldname))
#define TEST_HOLD_REG_START(field) (HOLD_OFFSET(field) >> 1)
#define TEST_HOLD_REG_SIZE(field)                                              \
  (sizeof(((holding_reg_params_t *)0)->field) >> 1)

#define TEST_INPUT_REG_START(field) (INPUT_OFFSET(field) >> 1)
#define TEST_INPUT_REG_SIZE(field)                                             \
  (sizeof(((input_reg_params_t *)0)->field) >> 1)

#pragma pack(push, 1)
typedef struct {
  float holding_data0;
  float holding_data1;
  float holding_data2;
  float holding_data3;
  uint16_t test_regs[150];
  float holding_data4;
  float holding_data5;
  float holding_data6;
  float holding_data7;
  uint32_t holding_area1_end;
} holding_reg_params_t;

esp_err_t master_init(void);
void master_operation_func(void *arg);
#endif // !GD20_MODBUS_DRIVER_H
