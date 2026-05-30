#include "modbus_payload_handle.h"
#include "esp_log.h"
#define TOTAL_INDEX(_ARRAY_SIZE_,_INDEX_SIZE_) ((sizeof(_ARRAY_SIZE_))/(sizeof(_INDEX_SIZE_)))
#define TOTAL_MODBUS_DEVICE TOTAL_INDEX(dev, st_modbus_device_info_t)
static const char TAG[] = "modbus_payload_handle";
const st_modbus_params_descriptor_t desc_inverter_gd20[] = {
    {.addr = GD20_REG_ID         , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ}, 
    {.addr = GD20_OUTPUT_SPEED   , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 65535 },
    {.addr = GD20_OUTPUT_POWER   , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = -300, .range.max = 300   },
    {.addr = GD20_OUTPUT_TORQUE  , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = -250, .range.max = 250   },
    {.addr = GD20_REG_STATUS     , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 1   , .range.max = 5     },
    {.addr = GD20_OPERATION_FREQ , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 100   },
    {.addr = GD20_OUTPUT_CURRENT , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 5000  },
    {.addr = GD20_OUTPUT_VOLTAGE , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 1200  },
    {.addr = GD20_REG_CONTROL_CMD, .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_WRITE, .range.min = 1   , .range.max = 8     },
    {.addr = GD20_REG_SET_FREQ   , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_WRITE, .range.min = 1   , .range.max = 8     },
};
static const st_modbus_params_descriptor_t desc_sensor_sht20[] = {
    {.addr = 0x0001    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ}, // Temperature
    {.addr = 0x0002    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ}, // Humidity
};
static const st_modbus_device_info_t dev[2] = {
    {.address = 1, .name = "GD20 Inverter"  , .desc = desc_inverter_gd20},
    {.address = 3, .name = "Sensor SHT20"   , .desc = desc_sensor_sht20},
};

e_modbus_payload_err_t m_modbus_payload_handle(uint8_t *modbus_payload)
{
    static uint8_t addr_available = 0;
    for(uint8_t i = 0; i < TOTAL_MODBUS_DEVICE; i++) {
        if(*(modbus_payload + 0) == dev[i].address) {
            addr_available = 1;
            break;
        }
    }
    if(!addr_available) 
        return MB_PAYLOAD_ERR_DEVICE_NOT_FOUND;

    ESP_LOGI(TAG,"total payload: %u", *(modbus_payload + 2));
    // if(*(modbus_payload + 2))

    return MB_PAYLOAD_OK;
}

