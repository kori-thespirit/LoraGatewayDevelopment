/* -------------------------- LICENSE placeholder -------------------------- */
#ifndef _NODE_DEVICE_TYPEDEF_H
#define _NODE_DEVICE_TYPEDEF_H

#include "gd20_inverter.h"

typedef struct sensor_sht20{ //nên có sensor_sht20 để ide hiểu
  float temperature;
  float humidity;
} st_sensor_sht20_t;

typedef struct inverter_gd20{
    e_gd20_status_t status;
    float frequency;
    float speed;
    float out_i;
    float out_v;
} st_inverter_gd20_t;

#endif // _NODE_DEVICE_TYPEDEF_H
