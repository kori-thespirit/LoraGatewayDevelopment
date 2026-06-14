#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dwin_app_driver.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dwin_app_driver.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "DWIN.h"
#include "dwin_keyboard.h"
#include  <vector>
#include  <cstring>
#include  <sstream>

#ifdef __cplusplus
extern "C" {
#endif

static char TAG[] = "dwin_app_driver";
// Create an instance of the DWIN class
DWIN hmi(UART_PORT, TX_PIN, RX_PIN, DGUS_BAUD);

static p_hmi_rx_cb g_p_rxhmi;
void hmi_send_text_to_display(uint16_t address, std::string &text)
{
    int textLength = text.length();
    int dataLen = 5 + textLength * 2; // cmd(1) + addr(2) + data(2*len) + terminator(2)
    uint8_t frame[256];
    int idx = 0;

    frame[idx++] = 0x5A;
    frame[idx++] = 0xA5;
    frame[idx++] = dataLen;
    frame[idx++] = 0x82;
    frame[idx++] = (address >> 8) & 0xFF;
    frame[idx++] = address & 0xFF;

    for (int i = 0; i < textLength; i++)
    {
        frame[idx++] = 0x00;
        frame[idx++] = (uint8_t)text[i];
    }

    frame[idx++] = 0xFF;
    frame[idx++] = 0xFF;

    uart_write_bytes(UART_PORT, (const char *)frame, idx);
}

void hmi_send_data_to_display(uint16_t address, uint16_t data) 
{
    uint8_t frame[8] = {
        CMD_HEAD1,
        CMD_HEAD2,
        0x05,
        CMD_WRITE,
        (uint8_t)((address >> 8) & 0xFF),
        (uint8_t)(address & 0xFF),
        (uint8_t)((data >> 8) & 0xFF),
        (uint8_t)(data & 0xFF),
    };
    uart_write_bytes(UART_PORT, (const char*)frame, 8);
}

static std::vector<std::string> tokenize(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    uint8_t items = 0;

    while (std::getline(ss, token, delimiter) && items < 10) {
        if (!token.empty()) { // Skip empty tokens
            tokens.push_back(token);
            items++;
        }
    }
    items = 0;
    return tokens;
}

static esp_err_t parse_response_str(std::string response_str, st_hmi_frame_t *hmiframe)
{
    std::vector<std::string> response_token = tokenize(response_str, ' ');
    uint8_t temp[10] = {0};
    for(uint8_t i = 0; i < response_token.size(); i++) {
        temp[i] = std::stoi(response_token[i], 0, 16);
    }
    if(CMD_HEAD1 == temp[0] && CMD_HEAD2 == temp[1]){
        hmiframe->data_length = temp[2];
        hmiframe->cmd_func = temp[3];
        hmiframe->vp = (uint16_t)(temp[4] << 8 | temp[5]);
        hmiframe->word_len = temp[6];
        hmiframe->lastbyte = (uint16_t)(temp[7] << 8 | temp[8]);
    }
    else {
        ESP_LOGE(TAG, "Response header invalid");
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

static void hmi_event_cb(std::string address, int lastByte, std::string message, std::string response)
{
    st_hmi_frame_t hmiframe;
    esp_err_t err = parse_response_str(response, &hmiframe);
    if(ESP_OK != err)
        return;
    DWIN_Keyboard kb;
    ESP_LOGI(TAG, "data_length:%u, cmd_func:0x%x, vp:0x%x, ret_len:%u word, lastbyte:0x%x",
            hmiframe.data_length, 
            hmiframe.cmd_func, 
            hmiframe.vp, 
            hmiframe.word_len, 
            hmiframe.lastbyte);
    if(hmiframe.vp == VP_KEYBOARD_INPUT) {
        int result = kb.processKey(hmiframe.lastbyte);
        std::string buffer = kb.getBuffer();
        ESP_LOGI(TAG, "result:%d, kbBuffer:%s",result, buffer);
        int value = 0;
        switch(result){
            case 1:
                value = buffer.empty() ? 0 : std::stoi(buffer);
                if (value > 70)
                    value = 70;
                break;
            case 2:
                value = buffer.empty() ? 0 : std::stoi(buffer);
                break;
            case 4: value = 0; break;
            case 5:
                // if(!buffer.empty()) // If buffer is not empty
                //     int value = std::stoi(buffer);
                // kb.clearBuffer();
                break;
            default:break;

        }
        hmi_send_data_to_display(VP_DISPLAY_OUTPUT, value);
    }
    else if(hmiframe.vp == VP_BUTTON) {
        ESP_LOGI(TAG, "On VP_BUTTON, lastbyte:%u",hmiframe.lastbyte);
        switch(hmiframe.lastbyte) {
            case 0x0001:
                ESP_LOGI(TAG, "[BUTTON] Confirm");
                if(!kb.getBuffer().empty()) {
                    int value = std::stoi(kb.getBuffer());
                    hmi_send_data_to_display(VP_DISPLAY_OUTPUT, value);
                    ESP_LOGI(TAG, "Frequency: %d Hz", value);
                    kb.clearBuffer();
                    g_p_rxhmi(hmiframe, (void*)&value);
                }
                break;
            default:
                 g_p_rxhmi(hmiframe, NULL);
                 break;
        }
    }
}

void hmi_start()
{
    hmi.echoEnabled(false);
    hmi.hmiCallBack(hmi_event_cb);
    hmi.setPage(0);

    // "5A A5 06 83 25 00 01 00 32"
    // "5A A5 06 83 25 00 01 00 31"
    // "5A A5 06 83 26 00 01 00 01"
    // Frequency
    // "5A A5 06 83 25 00 01 00 33"
    // "5A A5 06 83 25 00 01 00 30"
    // "5A A5 06 83 26 00 01 00 01"
    // "5A A5 06 83 26 00 01 00 05"
    // "5A A5 06 83 26 00 01 00 02"
    //5A
    //A5
    //03
    //82
    //4F
    //4B
    //string to integer
    //temp[0]:90
    //temp[1]:165
    //temp[2]:3
    //temp[3]:130
    //temp[4]:79
    //temp[5]:75
    //data_length:3, cmd_func:0x82, vp:0x4f4b, ret_len:0 word, lastbyte:0
}


void hmi_register_callback(void (* p_hmi_parse_cb)(st_hmi_frame_t, void *))
{
    g_p_rxhmi = p_hmi_parse_cb;
}

void hmi_listen() { hmi.listen(); }

// static uint8_t wait_millis(uint32_t delay_ms)
// {
//     static uint8_t is_started = 0;
//     static uint32_t start_time = 0;
//     if(is_started == 0) {
//         start_time = esp_timer_get_time() / 1000;
//         is_started = 1;
//     }
//     uint32_t current_tick = esp_timer_get_time() / 1000;
//     if(current_tick - start_time > delay_ms && is_started) {
//         start_time = current_tick;
//         is_started = 0;
//         return 1;
//     }
//     return 0;
// }

// static void update_rtc(st_rtc_t *t)
// {
//     if(t->month / 12 > 0) { t->year += t->month / 24; t->month %= t->month; }
//     if(t->day / 24 > 0) { t->day += t->hour / 24; t->hour %= t->hour; }
//     if(t->min / 60 > 0) { t->hour += t->min / 60; t->min %= t->min; }
//     if(t->sec / 60 > 0) { t->min += t->sec / 60; t->sec %= t->sec; }
// }

#ifdef __cplusplus
}
#endif

