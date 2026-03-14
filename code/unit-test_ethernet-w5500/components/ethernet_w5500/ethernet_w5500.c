#include "ethernet_w5500.h"

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_bit_defs.h"
#include "esp_err.h"
#include "hal/gpio_types.h"
#include "hal/spi_types.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static void cs_high();
static void cs_low();
static spi_device_handle_t spi;

void spi_bus_init() {
  const gpio_config_t spi_cs_cfg = {
      .pin_bit_mask = BIT64(SPI2_CS),
      .mode = GPIO_MODE_OUTPUT,
  };
  gpio_config(&spi_cs_cfg);
  esp_err_t ret = ESP_OK;
  spi_bus_config_t spi2_bus_cfg = {
      .miso_io_num = SPI2_MISO,
      .mosi_io_num = SPI2_MOSI,
      .sclk_io_num = SPI2_SCK,
      .quadhd_io_num = -1,
      .quadwp_io_num = -1,
      .max_transfer_sz = 32,
  };
  const spi_device_interface_config_t w5500 = {
      .command_bits = 8,
      .spics_io_num = SPI2_CS,
      .mode = 0,
      // clock_speed_hz range from 20MHz to 40MHz
      .clock_speed_hz = 10 * 1000 * 100,
      .flags = SPI_DEVICE_HALFDUPLEX,
      // .pre_cb = cs_low,
      // .post_cb = cs_high,
      .queue_size = 3};

  ret = spi_bus_initialize(SPI2_HOST, &spi2_bus_cfg, SPI_DMA_CH_AUTO);
  assert(!ret);

  ret = spi_bus_add_device(SPI2_HOST, &w5500, &spi);
  ESP_ERROR_CHECK(ret); // same as assert
}

void w5500_transmit(uint8_t *data, size_t datasize) {
  spi_transaction_t t = {0};
  t.length = 24;
  t.cmd = 0x10;
  /* Not use data input argument, use tx_data instead, command is 0x10 */
  t.flags = SPI_TRANS_USE_TXDATA;
  t.tx_data[0] = 0x01;
  t.tx_data[1] = 0x02;
  t.tx_data[2] = 0x03;
  t.tx_data[3] = 0x04;
  t.user = (void *)0; // this is transaction 0
  esp_err_t ret = spi_device_polling_transmit(spi, &t);

  /* Transfer burst data with command 0x22 */
  t.length = 8 * datasize;
  t.flags &= ~SPI_TRANS_USE_TXDATA;
  t.cmd = 0x22;
  t.tx_buffer = data;
  t.user = (void *)1; // this is transaction 1
  ret = spi_device_polling_transmit(spi, &t);

  assert(!ret);
}
static void cs_high() { gpio_set_level(SPI2_CS, 1); }

static void cs_low() { gpio_set_level(SPI2_CS, 0); }
