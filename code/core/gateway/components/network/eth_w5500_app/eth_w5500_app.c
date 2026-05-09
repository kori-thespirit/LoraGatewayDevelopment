#include "eth_w5500_app.h"
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

static const char *TAG = "ETH W5500";
static EventGroupHandle_t *eg;

void w5500_init(EventGroupHandle_t *net_eg) {
  uint8_t eth_port_cnt = 0;
  eg = net_eg;
  esp_eth_handle_t *eth_handles;
  /* Create default event loop that running in background to handle Ethernet
   * events */
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ethernet_init_all(&eth_handles, &eth_port_cnt);
  ESP_ERROR_CHECK(esp_netif_init());
  // Create instance(s) of esp-netif for Ethernet(s)
  // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and
  // you don't need to modify default esp-netif configuration parameters.
  esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
  esp_netif_t *eth_netif = esp_netif_new(&cfg);
  // Attach Ethernet driver to TCP/IP stack
  ESP_ERROR_CHECK(
  esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handles[0])));
  // Start Ethernet driver state machine
  ESP_ERROR_CHECK(esp_eth_start(eth_handles[0]));
  xEventGroupSetBits(*eg, NET_EG_BIT_GET_VALUE(NET_ETH_IS_CONNECTED));
}
