
/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _DATA_TYPEDEF_H_
#define _DATA_TYPEDEF_H_

#include <stdint.h>
typedef enum core_data_id{
    COREDATA_ID_NET,
    COREDATA_ID_MB_DATA,
    COREDATA_ID_MB_CFG,
    COREDATA_ID_SDCARD,
    COREDATA_ID_LORA_DATA,
    COREDATA_ID_LORA_CFG_REG,
    COREDATA_ID_HMI,
    COREDATA_ID_SHT20_TEMP,
    COREDATA_ID_SHT20_HUMID,
    COREDATA_ID_GD20_ID,
    COREDATA_ID_GD20_SPEED,
    COREDATA_ID_GD20_POWER,
    COREDATA_ID_GD20_TORQUE,
    COREDATA_ID_GD20_STATUS,
    COREDATA_ID_GD20_FREQ,
    COREDATA_ID_GD20_CURRENT,
    COREDATA_ID_GD20_VOLTAGE,
    COREDATA_ID_GD20_CONVETER_TEMP,
} e_core_data_id_t;

typedef struct core_data{
    uint8_t cdataid;
    /* WARNING: Do not use pointer for cdata due to free allocated memory or local variable */
    uint8_t cdata[20];
} st_core_data_t;

typedef struct modbus_data {
    uint8_t addr;
    uint8_t modbus_function;
    uint16_t reg;
    uint16_t value;
}st_modbus_data_t;

typedef struct network_data {
}st_network_data_t;

typedef struct sdcard_data {
}st_sdcard_data_t;

typedef struct hmi_data {
}st_hmi_data_t;

typedef struct lora_data {
}st_lora_data_t;


typedef struct lora_config_reg {
    uint8_t sf;
    uint8_t cr;
    uint8_t bw;
    uint32_t freq;
}st_lora_cfg_reg_t;

#endif // _DATA_TYPEDEF_H_
