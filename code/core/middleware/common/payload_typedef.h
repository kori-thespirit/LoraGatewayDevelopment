/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _PAYLOAD_TYPEDEF_H_
#define _PAYLOAD_TYPEDEF_H_

#include "gd20_inverter.h"

typedef struct modbus_sensor_sht20{ //nên có sensor_sht20 để ide hiểu
  float temperature;
  float humidity;
} st_modbus_sensor_sht20_t;

typedef struct modbus_inverter_gd20{
    e_gd20_status_t status;
    float frequency;
    float speed;
    float out_i;
    float out_v;
} st_modbus_inverter_gd20_t;

#endif // _PAYLOAD_TYPEDEF_H_
