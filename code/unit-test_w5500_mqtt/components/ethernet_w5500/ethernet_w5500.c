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
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

void w5500_init() {
  uint8_t eth_port_cnt = 0;
  esp_eth_handle_t *eth_handles;
  /* Create default event loop that running in background to handle Ethernet
   * events */
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ethernet_init_all(&eth_handles, &eth_port_cnt);
  ESP_ERROR_CHECK(esp_netif_init());
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
  // Start Ethernet driver state machine
  for (int i = 0; i < eth_port_cnt; i++) {
    ESP_ERROR_CHECK(esp_eth_start(eth_handles[i]));
  }
}
