#include "modbus_payload_handle.h"

static void m_modbusph_parse_inverter_data(void *register_value) {
    static st_inverter_gd20_t payload;
    switch (reg_addr) {
        case GD20_REG_STATUS:
            payload.status 
            switch (value) {
                    case GD20_STATUS_RUN:   ESP_LOGW("STATUS", "Biến tần: ĐANG CHẠY THUẬN");break;
                    case GD20_STATUS_REV:   ESP_LOGW("STATUS", "Biến tần: ĐANG CHẠY NGHỊCH");break;
                    case GD20_STATUS_STOP:  ESP_LOGW("STATUS", "Biến tần: ĐANG DỪNG");break;
                    default:                ESP_LOGE("STATUS", "Biến tần: LỖI/KHÔNG XÁC ĐỊNH (%d)", value);break;

            }
            break;

        case GD20_OPERATION_FREQ:
            payload.frequency = (float)value / 100.0;
            break;

        case GD20_OUTPUT_CURRENT:
            payload.out_i = (float)value / 10.0;
            break;

        case GD20_OUTPUT_VOLTAGE:
            payload.out_v = (float)value;
            break;

        default:
            ESP_LOGI("DATA", "Thanh ghi 0x%04X: Giá trị = %d", reg_addr, value);
            break;
    }
}

static void m_modbusph_parse_sht20_data(void *register_value) {
    static st_sensor_sht20_t payload;
}

void m_modbusph_payload_handle(void *payload, size_t payload_size)
{
    st_modbus_device_info_t *info = (st_modbus_device_info_t*)payload;
    switch(info->device_type)
    {
        case MODBUS_DEVICE_INVERTER_GD20:
            m_modbusph_parse_inverter_data((void*)info->reg);
        break;
        case MODBUS_DEVICE_SHT20:
            m_modbusph_parse_sht20_data((void*)info->reg);
        break;

    }

}
