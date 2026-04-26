/*
 @name: Ta Thuan
 @brief: Communication
 */
#pragma once
#include "driver/gpio.h"
#include "driver/spi_master.h"
/// Configuration of spi_slave
#define SPI_Frequency 1 * 1000 * 1000 // 1MHz

typedef struct SPI_Stub_Register {
  uint8_t addr_reg;
  uint8_t hummidity;
  uint8_t temperature;
} SPI_Stub_Register;

typedef struct {
  gpio_num_t SCK;
  gpio_num_t MOSI;
  gpio_num_t MISO;
  gpio_num_t CS;
  spi_host_device_t host; // enum host 0 : defualt , 1 : user
} SPI_config;

typedef struct SPI_Stub_Context_t *SPI_Stub_handle_t;

esp_err_t SPI_Stub_Init(SPI_Stub_handle_t *ctx, SPI_config *cfg);
//
// esp_err_t SPI_init_bus(SPI_Stub_handle_t *ctx, uint16_t max_transfer);
// esp_err_t SPI_add_device(SPI_Stub_handle_t *ctx, SPI_config *cfg);
//
esp_err_t SPI_Stub_Begin_Get_Info(SPI_Stub_handle_t *ctx);
esp_err_t SPI_Stub_Set_Up(SPI_Stub_handle_t *ctx);

esp_err_t Send_SPI_Stub_Register(SPI_Stub_handle_t *ctx, uint8_t *data,
                                 size_t len);
esp_err_t SPI_Stub_Deinit(SPI_Stub_handle_t *ctx);
