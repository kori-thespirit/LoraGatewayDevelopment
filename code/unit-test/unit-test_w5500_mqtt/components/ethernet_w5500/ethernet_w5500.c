#include "ethernet_w5500.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "ethernet_init.h"
#include "mqtt_client.h"

#include <stddef.h>
#include <stdint.h>
extern const uint8_t
    mosquitto_org_crt_start[] asm("_binary_mosquitto_org_crt_start");
extern const uint8_t
    mosquitto_org_crt_end[] asm("_binary_mosquitto_org_crt_end");

static char TAG[] = "MQTT";

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

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32,
           base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
    ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ",
             event->msg_id, (uint8_t)*event->data);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x",
               event->error_handle->esp_tls_last_esp_err);
      ESP_LOGI(TAG, "Last tls stack error number: 0x%x",
               event->error_handle->esp_tls_stack_err);
      ESP_LOGI(TAG, "Last captured errno : %d (%s)",
               event->error_handle->esp_transport_sock_errno,
               strerror(event->error_handle->esp_transport_sock_errno));
    } else if (event->error_handle->error_type ==
               MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
      ESP_LOGI(TAG, "Connection refused error: 0x%x",
               event->error_handle->connect_return_code);
    } else {
      ESP_LOGW(TAG, "Unknown error type: 0x%x",
               event->error_handle->error_type);
    }
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

esp_err_t mqtt_app_start(void) {
  const esp_mqtt_client_config_t mqtt_cfg = {
      .broker = {.address.uri = "mqtts://test.mosquitto.org",
                 .verification.certificate =
                     (const char *)mosquitto_org_crt_start},
  };

  ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes",
           esp_get_free_heap_size());
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  return esp_mqtt_client_start(client);
}
