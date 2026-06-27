/* -------------------------- LICENSE placeholder -------------------------- */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "wifi_app.h"
#include "mqtts_app.h"
#include <ctype.h>
#include <cJSON.h>

static const char *TAG = "network";

typedef enum {
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_FLOAT,
    TYPE_DOUBLE,
} e_data_type_t;

typedef struct map_str_to_value {
    const char* str;
    int val;
}st_map_str_to_value_t;

typedef struct param_descriptor {
    const char* paramkey;
    e_data_type_t datatype;
    void * p_val;
} st_param_descriptor_t;

typedef struct modbus_topic_descriptor{
    uint32_t  modbus_function;
    st_map_str_to_value_t modbus_reg;
    st_param_descriptor_t param_desc[2];
}st_modbus_topic_descriptor_t;

static void error_handle(esp_mqtt_error_codes_t *error);
static void handle_topic_request(esp_mqtt_event_handle_t event);
static void handle_topic_gd20_control(void *vParameters);
static void mqtt_log(const char* status, const char * message);

EventGroupHandle_t network_event_group;
st_modbus_data_t g_mdata;

const st_modbus_topic_descriptor_t topic_gd20_control[] = {
    {
        0x03, //Modbus function: Holding
        {"command", 0x2000}, // parameter:command
        {
            {"value", TYPE_U16, (void*)&g_mdata.value},
            {"modbus address", TYPE_U8, (void*)&g_mdata.addr},
        }
    },
    {
        0x03, //Modbus function: Holding
        {"frequency", 0x2001}, // parameter:frequency
        {
            {"value", TYPE_U16, (void*)&g_mdata.value},
            {"modbus address", TYPE_U8, (void*)&g_mdata.addr},
        }
    }

};

const st_map_str_to_value_t topic_gd20_query[] = {
    {"status"   , 0x2100},
    {"id"       , 0x2103},
};

#define TOTAL_TOPIC(_X_) TOTAL_INDEX(_X_, const char*)
const char *sub_topic_list[] = {
    // "/topic/project/lora/gateway/#",
    "/topic/project/lora/node/+/devices/+/control",
    "/topic/project/lora/node/+/devices/+/query",
    "/topic/project/lora/node/+/config/",
};

const char *pub_topic_list[] = {
    "/topic/project/lora/node/2/devices/gd20/data",
    "/topic/project/lora/gateway/status",
};

const char *example_topic_list[] = {
    "/topic/example/#",
};

/*
char *mqtt_gd20_data_json_format = "\
{                                   \
    \"Ouput Current\":%.2f A   ,    \
    \"Frequency\":%.2f     Hz  ,    \
    \"Out Voltage\":%.2f   V   ,    \
    \"Speed\":%.2f         rpm ,    \
    \"Temperature\":%.2f   °C       \
}";
*/

char *mqtt_gd20_data_json_format =\
" { \"Ouput Current\":%.2f, \"Frequency\":%.2f, \"Out Voltage\":%.2f, \"Speed\":%.2f, \"Temperature\":%.2f }";

char *mqtt_gateway_status_json_format =\
"{\t\n\"status\":\"%s\",\n\t\"message\":\"%s\"\n}";

void network_task(void* pvParameters) 
{
    ESP_LOGI(TAG, "task created");
    network_event_group = xEventGroupCreate();
    printf("LOG START\r\n");
    uint32_t bit_to_wait = 
        BIT_TO_VALUE(NET_WIFI_IS_CONNECTED) |
        BIT_TO_VALUE(NET_ETH_IS_CONNECTED);
#if CONFIG_NETWORK_WIFI
    wifi_station_init(&network_event_group);
#elif CONFIG_NETWORK_ETH 
    w5500_init(&network_event_group);
#endif
    EventBits_t bits = xEventGroupWaitBits(network_event_group,
            bit_to_wait,
            pdFALSE,
            pdFALSE,
            10000 / portTICK_PERIOD_MS);
    if(CHECK_BIT(bits, bit_to_wait)){
        ESP_LOGI(TAG, "Connecting to %s ... broker", MQTT_URI);
        ESP_ERROR_CHECK(mqtts_app_start(&network_event_group));
        ESP_ERROR_CHECK(mqtts_app_register_callback(handle_topic_request, error_handle));
        ESP_ERROR_CHECK(mqtts_app_use_subscribe_list(sub_topic_list, TOTAL_INDEX(sub_topic_list)));
        ESP_LOGI(TAG, "MQTT connected");
    }
    else {
        ESP_LOGE(TAG, "Failed to connect to network");
    }

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


static void handle_topic_request(esp_mqtt_event_handle_t event)
{
    char *topic = (char*)calloc(event->topic_len + 1, sizeof(char));
    char *json_str = (char*)calloc(event->data_len + 1, sizeof(char));
    memcpy(topic, event->topic, event->topic_len);
    memcpy(json_str, event->data, event->data_len);
    // ESP_LOGI(TAG, "topic len:%d", event->topic_len);
    // ESP_LOGI(TAG, "json_str len:%d", event->data_len);

    char project_topic[] = "/topic/project/lora/";
    size_t project_topic_length = strlen(project_topic);

    /* Get the device topic, e.g: node/2/gd20/control 
     * Extracted from topic /topic/project/lora/node/2/gd20/control */
    char *extracted_device_topic = topic + project_topic_length;
    // ESP_LOGI(TAG, "extracted_device_topic:%s", extracted_device_topic);

    char *token = strtok(extracted_device_topic, "/");
    char token_storage[5][10] = {0};
    uint8_t i = 0;
    while (token != NULL) {
        strcpy(token_storage[i++], token);
        // ESP_LOGI(TAG, "token_storage[%u]:%s", i - 1, token_storage[i - 1]);
        token = strtok(NULL, "/");
    }

    for(uint8_t i = 0; i < strlen(token_storage[1]); i++) {
        char test = token_storage[1][i];
        if(!isdigit(test)) {
            ESP_LOGE(TAG, "%s:Lora address is not digit, refuse to process",__func__);
            return;
        }
    }
    uint8_t lora_address = atoi(token_storage[1]);
    ESP_LOGI(TAG, "lora_address:%u", lora_address);
    mqtt_log("OK", "Parsed lora_address successfully");
    char* device_name = token_storage[3];
    ESP_LOGI(TAG, "Handle topic of device: %s", device_name);

    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error at: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        return;
    }

    // ESP_LOGI(TAG, "token_storage[4]:%s", token_storage[4]);
    if(!strcmp(token_storage[4],"control")){
        mqtt_log("OK", "Handle GD20 control");
        handle_topic_gd20_control((void*)json);
    }

    cJSON_Delete(json);
    free(topic);
    free(json_str);
}

static void get_value_from_json_key(cJSON *key, const char *str, e_data_type_t datatype, void *p_val)
{
    double tempvalue = 0;
    if((datatype >= TYPE_U8 && datatype <= TYPE_DOUBLE) && cJSON_IsNumber(key)) {
        tempvalue = cJSON_GetNumberValue(key);
    }
    switch(datatype){
        case TYPE_BOOL:
            break;
        case TYPE_STRING:
            break;
        case TYPE_U8:
            uint8_t u8val = (uint8_t)tempvalue;
            memcpy((void*)p_val, (void*)&u8val, sizeof(u8val));
            break;
        case TYPE_U16:
            uint16_t u16val = (uint16_t)tempvalue;
            memcpy((void*)p_val, (void*)&u16val, sizeof(u16val));
            break;
        case TYPE_U32:
            uint32_t u32val = (uint32_t)tempvalue;
            memcpy((void*)p_val, (void*)&u32val, sizeof(u32val));
            break;
        case TYPE_I8:
            int8_t i8val = (int8_t)tempvalue;
            memcpy((void*)p_val, (void*)&i8val, sizeof(i8val));
            break;
        case TYPE_I16:
            int16_t i16val = (int16_t)tempvalue;
            memcpy((void*)p_val, (void*)&i16val, sizeof(i16val));
            break;
        case TYPE_I32:
            int32_t i32val = (int32_t)tempvalue;
                memcpy((void*)p_val, (void*)&i32val, sizeof(i32val));
            break;
        case TYPE_FLOAT:
            float fval = (float)tempvalue;
            memcpy((void*)p_val, (void*)&fval, sizeof(fval));
            break;
        default:
            ESP_LOGE(TAG, "%s, %d: datatype not found:%d", __func__, __LINE__, datatype);
            break;
    }

}

static void handle_topic_gd20_control(void *vParameters)
{
    cJSON *json = (cJSON *)vParameters;
    cJSON *temp = cJSON_GetObjectItemCaseSensitive(json, "parameter");
    if(temp == NULL) {
        ESP_LOGE(TAG, "%s, %d: required key not found: %s", 
                __func__, 
                __LINE__, 
                "parameter");
        return;
    }

    uint8_t i;
    char *mb_reg = NULL;
    if(cJSON_IsString(temp))
        mb_reg = cJSON_GetStringValue(temp);
    else {
        ESP_LOGE(TAG, "%s, %d - mb_reg is not string", __func__, __LINE__);
        return;
    }

    ESP_LOGI(TAG, "mb_reg:%s", mb_reg);
    
    for(i = 0; i < TOTAL_INDEX(topic_gd20_control); i++) {
        if(!strcmp(topic_gd20_control[i].modbus_reg.str, mb_reg)) {
            ESP_LOGI(TAG, "Found register:%s, reg_value:0x%x at topic %u", topic_gd20_control[i].modbus_reg.str, topic_gd20_control[i].modbus_reg.val, i);
            g_mdata.reg = topic_gd20_control[i].modbus_reg.val;
            g_mdata.modbus_function = topic_gd20_control[i].modbus_function;
            const st_param_descriptor_t *param_desc = topic_gd20_control[i].param_desc;
            for(uint8_t descriptor = 0; descriptor < TOTAL_INDEX(topic_gd20_control[i].param_desc); descriptor++){
                cJSON *temp = cJSON_GetObjectItemCaseSensitive(json,(param_desc + descriptor)->paramkey);
                if(temp == NULL) {
                    ESP_LOGE(TAG, "%s, %d: required key not found: %s", 
                            __func__, 
                            __LINE__, 
                            (param_desc + descriptor)->paramkey);
                    return;
                }
                ESP_LOGI(TAG, "paramkey:%s, datatype:%d", 
                        (param_desc + descriptor)->paramkey, 
                        (param_desc + descriptor)->datatype);
                get_value_from_json_key(temp,
                        (param_desc + descriptor)->paramkey,
                        (param_desc + descriptor)->datatype,
                        (param_desc + descriptor)->p_val);
            }
            break;
        }
    }
    ESP_LOGI(TAG, "g_mdata - value:%u, addr:%u", g_mdata.value, g_mdata.addr);
    mqtt_log("OK", "Parse received data completed");
}

static void error_handle(esp_mqtt_error_codes_t *error)
{
    ESP_LOGE(TAG, "Func:%s, line:%d",__func__, __LINE__);

}

static void mqtt_log(const char* status, const char * message)
{
    char buffer[sizeof(mqtt_gateway_status_json_format) + 150] = {0};
    uint16_t total_len = sizeof(mqtt_gateway_status_json_format) + strlen(message) + strlen(status);
    ESP_LOGI(TAG, "total_len:%u", total_len);
    if(total_len > sizeof(buffer)) {
        ESP_LOGE(TAG, "%s - message is too long:%d, expect:%d", __func__, total_len, sizeof(mqtt_gateway_status_json_format) + 150);
    }
    if(strcmp(status, "OK") && strcmp(status, "ERROR")) {
        ESP_LOGE(TAG, "status must be OK or ERROR:%s", status);
    }
    sprintf(buffer, mqtt_gateway_status_json_format, status, message);
    ESP_ERROR_CHECK(mqtts_app_publish(*(pub_topic_list + 1), buffer));
}
