#ifndef ETH_W5500_APP_H
#define ETH_W5500_APP_H
#include "network_event_group_define.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
void w5500_init(EventGroupHandle_t *net_eg);
#endif // ETH_W5500_APP_H
