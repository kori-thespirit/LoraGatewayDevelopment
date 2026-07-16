#ifndef _RTC_DS3231_H
#define _RTC_DS3231_H

#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t second; 
  uint8_t minute; 
  uint8_t hour; 
  uint8_t day;
  uint8_t date; 
  uint8_t month; 
  uint16_t year;
} st_rtc_time_t;

typedef struct {
  st_rtc_time_t t;
  uint8_t i2c_addr;
  const i2c_config_t conf;
} st_rtc_handle_t;


void ds3231_init();
void ds3231_set_datetime(uint8_t second, 
    uint8_t minute, 
    uint8_t hour, 
    uint8_t day, 
    uint8_t date, 
    uint8_t month, 
    uint16_t year);
void ds3231_readtime();
esp_err_t ds3231_get_time(st_rtc_time_t *time_out);

#ifdef __cplusplus
}
#endif

#endif // _RTC_DS3231_H
