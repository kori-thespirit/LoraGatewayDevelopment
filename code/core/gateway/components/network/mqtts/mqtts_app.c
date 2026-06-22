/* -------------------------- LICENSE placeholder -------------------------- */
#include "mqtts_app.h"
#include "mqtt_client.h"
#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "MQTT";
static EventGroupHandle_t *eg;
const char **p_p_subscribe_list;
static p_mqtt_error_cb g_p_mqtt_err_cb;
static p_mqtt_data_cb  g_p_mqtt_data_cb;
static uint8_t total_topic_subscribe = 0;
static esp_mqtt_client_handle_t client;
extern const uint8_t server_cert_pem_start[] asm("_binary_kolabori_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_kolabori_pem_end");
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
            xEventGroupSetBits(*eg, BIT_TO_VALUE(NET_MQTT_IS_CONNECTED));
            for(uint8_t topic = 0; topic < total_topic_subscribe; topic++) {
                msg_id = esp_mqtt_client_subscribe(client, *(p_p_subscribe_list + topic), 0);
                ESP_LOGI(TAG, "Subscribe to %s, msg_id=%d", *(p_p_subscribe_list + topic), msg_id);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            xEventGroupClearBits(*eg, BIT_TO_VALUE(NET_MQTT_IS_CONNECTED));
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            xEventGroupSetBits(*eg, BIT_TO_VALUE(NET_MQTT_IS_SUBSCRIBED));
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ",
                    event->msg_id, (uint8_t)*event->data);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            xEventGroupClearBits(*eg, BIT_TO_VALUE(NET_MQTT_IS_SUBSCRIBED));
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            g_p_mqtt_data_cb(event);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            g_p_mqtt_err_cb(event->error_handle);
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

esp_err_t mqtts_app_register_callback(
        void (* p_mqtt_data_cb)     (esp_mqtt_event_handle_t),
        void (* p_mqtt_error_cb)    (esp_mqtt_error_codes_t *))
{
    if( NULL == p_mqtt_data_cb || 
        NULL == p_mqtt_error_cb) 
        return ESP_ERR_INVALID_ARG; 
     g_p_mqtt_data_cb = p_mqtt_data_cb;
     g_p_mqtt_err_cb = p_mqtt_error_cb;
    return ESP_OK;
}

esp_err_t mqtts_app_use_subscribe_list(const char **sublist, uint8_t total)
{
    if(!total) {
        ESP_LOGE(TAG, "Invalid total subscribe topic:%u", total);
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "Total topic list:%u", total);
    p_p_subscribe_list = sublist;
    total_topic_subscribe = total;
    return ESP_OK;
}

esp_err_t mqtts_app_publish(const char* topic, char* data)
{
    int msg_id = esp_mqtt_client_publish(client, topic, data, 0, 0, 0);
    ESP_LOGI(TAG, "Pushlish message id:%d", msg_id);
    return ESP_OK;
}

esp_err_t mqtts_app_start(EventGroupHandle_t *net_eg) {
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_URI,
        .broker.verification.certificate = (const char *)server_cert_pem_start,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };
    eg = net_eg;

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes",
            esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this
     * example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
            NULL);
    return esp_mqtt_client_start(client);
}
