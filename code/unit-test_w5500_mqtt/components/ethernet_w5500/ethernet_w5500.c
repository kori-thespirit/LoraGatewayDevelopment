#include "ethernet_w5500.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "ethernet_init.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "ping/ping_sock.h"

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

static void test_on_ping_success(esp_ping_handle_t hdl, void *args) {
  // optionally, get callback arguments
  // const char* str = (const char*) args;
  // printf("%s\r\n", str); // "foo"
  uint8_t ttl;
  uint16_t seqno;
  uint32_t elapsed_time, recv_len;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr,
                       sizeof(target_addr));
  esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
  esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time,
                       sizeof(elapsed_time));
  printf("%ld bytes from %s icmp_seq=%d ttl=%d time=%ld ms\n", recv_len,
         inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);
}

static void test_on_ping_timeout(esp_ping_handle_t hdl, void *args) {
  uint16_t seqno;
  ip_addr_t target_addr;
  esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
  esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr,
                       sizeof(target_addr));
  printf("From %s icmp_seq=%d timeout\n", inet_ntoa(target_addr.u_addr.ip4),
         seqno);
}

static void test_on_ping_end(esp_ping_handle_t hdl, void *args) {
  uint32_t transmitted;
  uint32_t received;
  uint32_t total_time_ms;

  esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted,
                       sizeof(transmitted));
  esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
  esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms,
                       sizeof(total_time_ms));
  printf("%ld packets transmitted, %lu received, time %lums\n", transmitted,
         received, total_time_ms);
}

int initialize_ping() {
  /* convert URL to IP address */
  ip_addr_t target_addr;
  struct addrinfo hint;
  struct addrinfo *res = NULL;
  memset(&hint, 0, sizeof(hint));
  memset(&target_addr, 0, sizeof(target_addr));
  getaddrinfo("www.google.com", NULL, &hint, &res);
  struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
  inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
  freeaddrinfo(res);

  esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
  ping_config.target_addr = target_addr;       // target IP address
  ping_config.count = ESP_PING_COUNT_INFINITE; // ping in infinite mode,
                                               // esp_ping_stop can stop it

  /* set callback functions */
  esp_ping_callbacks_t cbs;
  cbs.on_ping_success = test_on_ping_success;
  cbs.on_ping_timeout = test_on_ping_timeout;
  cbs.on_ping_end = test_on_ping_end;
  cbs.cb_args =
      "foo"; // arguments that feeds to all callback functions, can be NULL
  // cbs.cb_args = eth_event_group;

  esp_ping_handle_t ping;
  esp_ping_new_session(&ping_config, &cbs, &ping);
  ESP_RETURN_ON_FALSE(esp_ping_start(ping) == ESP_OK, -1, "ping_eth",
                      "esp_ping_start() failed");
  return 0;
}
