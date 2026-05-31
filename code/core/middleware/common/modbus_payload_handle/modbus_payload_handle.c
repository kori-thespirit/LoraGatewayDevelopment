#include "modbus_payload_handle.h"
#include "esp_log.h"
#define TOTAL_INDEX(_ARRAY_SIZE_,_INDEX_SIZE_) ((sizeof(_ARRAY_SIZE_))/(sizeof(_INDEX_SIZE_)))
#define TOTAL_MODBUS_DEVICE TOTAL_INDEX(dev, st_modbus_device_info_t)
static const char TAG[] = "modbus_payload_handle";

static p_modbus_tx_complete_cb  _g_p_rx_cb;
static p_modbus_rx_complete_cb  _g_p_tx_cb;
static p_modbus_error_cb        _g_p_err_cb;

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


void m_modbus_payload_handle(uint8_t *modbus_payload, bool is_send)
{
    e_modbus_payload_err_t err = MB_PAYLOAD_OK;
    /* To check whether modbus target address is known in device info */
    static uint8_t addr_found = 0;
    /* To know which register is currently sent */
    static uint16_t register_to_send = 0; 
    /* Get the modbus function from modbus frame */
    uint8_t modbus_func = *(modbus_payload + 1);
    uint8_t dev_addr = *(modbus_payload + 0);
    uint16_t value = (*(modbus_payload + 4) << 8) | *(modbus_payload + 5);
    ESP_LOGI(TAG, "total modbus devices: %d", TOTAL_MODBUS_DEVICE);
    for(uint8_t i = 0; i < TOTAL_MODBUS_DEVICE; i++) {
        if(dev_addr == dev[i].address) {
            ESP_LOGI(TAG, "found %s", dev[i].name);
            addr_found = 1;
            break;
        }
    }
    if(!addr_found) {
        err = MB_PAYLOAD_ERR_DEVICE_NOT_FOUND;
        goto error_callback;
    }
        
    if(is_send) {
        /* In read request phase */
        /* Save the requested register */
        register_to_send = (*(modbus_payload + 2) << 8) | *(modbus_payload + 3);
        e_modbus_permission_t permission = dev[dev_addr].desc->perm; 
        int range_min = dev[dev_addr].desc->range.min; 
        int range_max = dev[dev_addr].desc->range.max;
        ESP_LOGI(TAG,"register: %u", register_to_send);
        switch(modbus_func) {
            case MB_FUNC_R:
                if(permission != PERM_READ && permission != PERM_READWRITE) {
                    ESP_LOGE(TAG, "no permission to read");
                    err = MB_PAYLOAD_ERR_PERMISSON;
                    goto error_callback;
                }
                break;
            case MB_FUNC_W:
                if(permission != PERM_WRITE && permission != PERM_READWRITE) {
                    ESP_LOGE(TAG, "no permission to write");
                    err = MB_PAYLOAD_ERR_PERMISSON;
                    goto error_callback;
                }
                if(range_min < value && value < range_max) {}
                else {
                    ESP_LOGE(TAG, "value is out of valid range");
                    err = MB_PAYLOAD_ERR_RANGE_INVALID;
                    goto error_callback;
                }
                break;
        }
    }
    else if (MB_FUNC_R == modbus_func){
        /* In read response phase */
        ESP_LOGI(TAG,"total payload: %u", *(modbus_payload + 2));
        uint8_t receive_payload_size = *(modbus_payload + 2);
        /* word to byte must multiply by 2 */
        uint8_t descriptor_payload_size = dev[dev_addr].desc->size * 2; 
        if(descriptor_payload_size != receive_payload_size) {
            err = MB_PAYLOAD_ERR_DATA_SIZE_MISMACTH;
            goto error_callback;
        }
    }
    else {
        /* Otherwise do nothing */
    }

error_callback:
    if(_g_p_err_cb)
        _g_p_err_cb((void*) err);
}

e_modbus_payload_err_t m_modbus_register_callback(
            void (* p_modbus_tx_complete_cb)(void *),
            void (* p_modbus_rx_complete_cb)(void *),
            void (* p_modbus_error_cb)      (void *))
{
    if( NULL == p_modbus_tx_complete_cb || 
        NULL == p_modbus_rx_complete_cb || 
        NULL == p_modbus_error_cb) 
        return MB_PAYLOAD_ERR_CALLBACK_IS_NULL; 
    _g_p_rx_cb  = p_modbus_tx_complete_cb;
    _g_p_tx_cb  = p_modbus_rx_complete_cb;
    _g_p_err_cb = p_modbus_error_cb;
    return MB_PAYLOAD_OK;
}

