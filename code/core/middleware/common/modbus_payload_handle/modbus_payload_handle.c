#include "modbus_payload_handle.h"
#include "esp_log.h"
#define TOTAL_INDEX(_ARRAY_SIZE_,_INDEX_SIZE_) ((sizeof(_ARRAY_SIZE_))/(sizeof(_INDEX_SIZE_)))
#define TOTAL_MODBUS_DEVICE TOTAL_INDEX(dev, st_modbus_device_info_t)
#define TOTAL_DESCRIPTOR(_X_) TOTAL_INDEX(_X_, st_modbus_params_descriptor_t)
static const char TAG[] = "modbus_payload_handle";

static p_modbus_tx_complete_cb  _g_p_rx_cb;
static p_modbus_rx_complete_cb  _g_p_tx_cb;
static p_modbus_error_cb        _g_p_err_cb;
const st_modbus_params_descriptor_t desc_inverter_gd20[] = {
    {.addr = GD20_REG_ID         , .unit = ""     , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ}, 
    {.addr = GD20_OUTPUT_SPEED   , .unit = "rpm"  , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 65535 },
    {.addr = GD20_OUTPUT_POWER   , .unit = "%"    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = -300, .range.max = 300   },
    {.addr = GD20_OUTPUT_TORQUE  , .unit = "%"    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = -250, .range.max = 250   },
    {.addr = GD20_REG_STATUS     , .unit = ""     , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 1   , .range.max = 5     },
    {.addr = GD20_OPERATION_FREQ , .unit = "Hz"   , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 100   },
    {.addr = GD20_OUTPUT_CURRENT , .unit = "A"    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 5000  },
    {.addr = GD20_OUTPUT_VOLTAGE , .unit = "V"    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ , .range.min = 0   , .range.max = 1200  },
    {.addr = GD20_REG_CONTROL_CMD, .unit = ""     , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_WRITE, .range.min = 1   , .range.max = 8     },
    {.addr = GD20_REG_SET_FREQ   , .unit = ""     , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_WRITE, .range.min = 1   , .range.max = 8     },
};
static const st_modbus_params_descriptor_t desc_sensor_sht20[] = {
    {.addr = 0x0001    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ}, // Temperature
    {.addr = 0x0002    , .size = 1, .type = MB_PARAM_HOLDING, .perm = PERM_READ}, // Humidity
};
static const st_modbus_device_info_t dev[2] = {
    {.address = 1, .name = "GD20 Inverter"  , .desc = desc_inverter_gd20 , .total_idx = TOTAL_DESCRIPTOR(desc_inverter_gd20) },
    {.address = 3, .name = "Sensor SHT20"   , .desc = desc_sensor_sht20  , .total_idx = TOTAL_DESCRIPTOR(desc_sensor_sht20)  },
};


void m_modbus_payload_handle(uint8_t *modbus_payload, bool is_send)
{
    e_modbus_payload_err_t err = MB_PAYLOAD_OK;
    /* To check whether modbus target address is known in device info */
    static uint8_t send_addr = 0;
    static uint8_t desc_idx = 0;
    /* To know which register is currently sent */
    static uint16_t register_to_send = 0; 
    /* Get the modbus function from modbus frame */
    uint8_t modbus_func = *(modbus_payload + 1);
    uint8_t dev_addr = *(modbus_payload + 0);
    uint16_t value = (*(modbus_payload + 3) << 8) | *(modbus_payload + 4);
    static const st_modbus_params_descriptor_t *descriptor;
    ESP_LOGI(TAG, "Total modbus devices: %d", TOTAL_MODBUS_DEVICE);
        
    if(is_send) { /* In sending modbus message phase */
        /* Get known modbus device */
        for(uint8_t i = 0; i < TOTAL_MODBUS_DEVICE; i++) {
            if(dev_addr == dev[i].address) {
                ESP_LOGI(TAG, "Found %s", dev[i].name);
                send_addr = dev_addr;
                descriptor = dev[i].desc;
                /* Save the requested register */
                register_to_send = (*(modbus_payload + 2) << 8) | *(modbus_payload + 3);
                /* This is invalid index, if none of Descriptor found */
                desc_idx = dev[i].total_idx;
                /* Get modbus parameter descriptor matched register to send */
                for(uint8_t j = 0; j < dev[i].total_idx ; j++) {
                    if((descriptor + j)->addr == register_to_send) {
                        desc_idx = j;
                        ESP_LOGI(TAG, "Found descriptor:%u", desc_idx);
                        break;
                    }
                }
                if(desc_idx == dev[i].total_idx) {
                    err = MB_PAYLOAD_ERR_DESCRIPTOR_NOT_FOUND;
                    goto error_callback;
                }
                break;
            }
        }

        if(!send_addr) {
            err = MB_PAYLOAD_ERR_DEVICE_NOT_FOUND;
            goto error_callback;
        }
        e_modbus_permission_t permission = (descriptor + desc_idx)->perm;
        int range_min = (descriptor + desc_idx)->range.min;
        int range_max = (descriptor + desc_idx)->range.max;
        ESP_LOGI(TAG,"register_to_send: 0x%x", register_to_send);
        switch(modbus_func) {
            case MB_FUNC_R:
                if(permission != PERM_READ && permission != PERM_READWRITE) {
                    err = MB_PAYLOAD_ERR_PERMISSON;
                    goto error_callback;
                }
                break;
            case MB_FUNC_W:
                if(permission != PERM_WRITE && permission != PERM_READWRITE) {
                    err = MB_PAYLOAD_ERR_PERMISSON;
                    goto error_callback;
                }
                if(range_min <= value && value <= range_max) {}
                else {
                    err = MB_PAYLOAD_ERR_RANGE_INVALID;
                    goto error_callback;
                }
                _g_p_tx_cb(NULL);
                break;
            default:
                break;
        }
    }
    else { /* In get response message phase */
        ESP_LOGI(TAG,"total payload: %u, value:%u", *(modbus_payload + 2), value);
        uint8_t receive_payload_size = *(modbus_payload + 2);
        /* Check device address between send and receive phase */
        if(send_addr != dev_addr) {
            err = MB_PAYLOAD_ERR_ADDRESS_MISMACTH;
            goto error_callback;
        }
        /* word to byte must multiply by 2 */
        uint8_t descriptor_payload_size = (descriptor + desc_idx)->size * 2;
        if(descriptor_payload_size != receive_payload_size) {
            err = MB_PAYLOAD_ERR_DATA_SIZE_MISMACTH;
            goto error_callback;
        }
        _g_p_rx_cb((void*)&value);

    }
    return;

error_callback:
    if(_g_p_err_cb)
        _g_p_err_cb((void*)&err);
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
    _g_p_rx_cb  = p_modbus_rx_complete_cb;
    _g_p_tx_cb  = p_modbus_tx_complete_cb;
    _g_p_err_cb = p_modbus_error_cb;
    return MB_PAYLOAD_OK;
}

