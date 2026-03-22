#include "ethernet_w5500.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "ethernet_init.h"

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_bit_defs.h"
#include "esp_err.h"
#include "ethernet_init.h"
#include "hal/gpio_types.h"
#include "hal/spi_types.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

// static void cs_high();
// static void cs_low();
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

void w5500_init() {
  uint8_t eth_port_cnt = 0;
  esp_eth_handle_t *eth_handles;
  /* Create default event loop that running in background to handle Ethernet
   * events */
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ethernet_init_all(&eth_handles, &eth_port_cnt);
  // Create instance(s) of esp-netif for Ethernet(s)
  if (eth_port_cnt == 1) {
    // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and
    // you don't need to modify default esp-netif configuration parameters.
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    // Attach Ethernet driver to TCP/IP stack
    ESP_ERROR_CHECK(
        esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handles[0])));
  } else {
    // Use ESP_NETIF_INHERENT_DEFAULT_ETH when multiple Ethernet interfaces are
    // used and so you need to modify esp-netif configuration parameters for
    // each interface (name, priority, etc.).
    esp_netif_inherent_config_t esp_netif_config =
        ESP_NETIF_INHERENT_DEFAULT_ETH(); // esp_netif_defaults
    esp_netif_config_t cfg_spi = {.base = &esp_netif_config,
                                  .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH};
    char if_key_str[10];
    char if_desc_str[10];
    for (int i = 0; i < eth_port_cnt; i++) {
      sprintf(if_key_str, "ETH_%d", i);
      sprintf(if_desc_str, "eth%d", i);
      esp_netif_config.if_key = if_key_str;
      esp_netif_config.if_desc = if_desc_str;
      esp_netif_config.route_prio -= i * 5;
      esp_netif_t *eth_netif = esp_netif_new(&cfg_spi);

      // Attach Ethernet driver to TCP/IP stack
      ESP_ERROR_CHECK(
          esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handles[i])));
      ESP_ERROR_CHECK(esp_eth_start(eth_handles[i]));
    }
  }
}

// static void cs_high() { gpio_set_level(SPI2_CS, 1); }

// static void cs_low() { gpio_set_level(SPI2_CS, 0); }
