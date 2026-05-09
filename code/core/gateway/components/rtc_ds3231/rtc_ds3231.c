#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rtc_ds3231.h"

// I2C pins for DS3231
#define I2C_MASTER_NUM     I2C_NUM_0
#define I2C_MASTER_SCL_IO  GPIO_NUM_10
#define I2C_MASTER_SDA_IO  GPIO_NUM_11
#define DS3231_ADDR        0x68

static const char *TAG = "rtc_ds3231";

// Function to convert decimal to BCD
static uint8_t dec_to_bcd(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

// Function to set date and time on DS3231
void ds3231_set_datetime(uint8_t second, 
    uint8_t minute, 
    uint8_t hour, 
    uint8_t day, 
    uint8_t date, 
    uint8_t month, 
    uint16_t year)
{
    uint8_t data[7];
    
    // Convert to BCD format
    data[0] = dec_to_bcd(second);
    data[1] = dec_to_bcd(minute);
    data[2] = dec_to_bcd(hour);
    data[3] = dec_to_bcd(day);      // Day of week (1=Sunday, 2=Monday, ...)
    data[4] = dec_to_bcd(date);
    data[5] = dec_to_bcd(month);
    data[6] = dec_to_bcd(year % 100);  // Only last 2 digits of year
    
    // Write to DS3231 registers starting from 0x00
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);  // Start from register 0x00
    i2c_master_write(cmd, data, 7, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Date and time set successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to set date and time!");
    }
}

void ds3231_readtime()
{
    uint8_t data[7];
    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 7, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        int second = ((data[0] >> 4) * 10) + (data[0] & 0x0F);
        int minute = ((data[1] >> 4) * 10) + (data[1] & 0x0F);
        int hour = ((data[2] >> 4) * 10) + (data[2] & 0x0F);
        int date = ((data[4] >> 4) * 10) + (data[4] & 0x0F);
        int month = ((data[5] >> 4) * 10) + (data[5] & 0x0F);
        int year = 2000 + ((data[6] >> 4) * 10) + (data[6] & 0x0F);
        
        ESP_LOGI(TAG, "Time: %02d:%02d:%02d - Date: %02d/%02d/%d", 
                 hour, minute, second, date, month, year);
    } else {
        ESP_LOGE(TAG, "Failed to read DS3231!");
    }

}

void ds3231_init()
{
    // Initialize I2C
    const i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    
    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C config failed!");
        return;
    }
    
    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C install failed!");
        return;
    }
    
    // Test DS3231 connection
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "not found! Check wiring.");
        ESP_LOGE(TAG, "SCL->GPIO10, SDA->GPIO11, VCC->3.3V, GND->GND");
        return;
    }
    ESP_LOGI(TAG, "connected successfully!");
}
