#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dwin_app_driver.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <string>
#include "DWIN.h"

static char TAG[] = "dwin_app_driver";
// Create an instance of the DWIN class
DWIN hmi(UART_PORT, RX_PIN, TX_PIN, DGUS_BAUD);

void set_VP(uint16_t vpaddress, uint16_t data)
{
    uint8_t frame[8];
    frame[0] = 0x5A;
    frame[1] = 0xA5;
    frame[2] = 0x05;
    frame[3] = 0x82;
    frame[4] = (vpaddress >> 8) & 0xFF;
    frame[5] = vpaddress & 0xFF;
    frame[6] = (data >> 8) & 0xFF;
    frame[7] = data & 0xFF;
    uart_write_bytes(UART_PORT, (const char*)frame, 8);
}

void send_text_dwin(uint16_t vpaddress, std::string &text)

void send_to_dwin(uint16_t address, uint16_t data) {
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
    ESP_LOGI(TAG, "send data 0x%x", data);
}


static void hmi_event_cb(std::string address, int lastByte, std::string message, std::string response)
{  
    ESP_LOGI(TAG, "OnEvent : [ A : %s | D : %02X | M : %s | R : %s ]", address.c_str(), lastByte, message.c_str(), response.c_str());
    if(address == VP_KEYBOARD_INPUT) {
        DWIN_Keyboard kb;
        int result = kb.processKey(lastByte);
        std::string buffer = kb.getBuffer();
        switch(result){
            case 1:
                int value = 0;
                if(!buffer.empty())  // If buffer is not empty
                    value = std::stoi(buffer);
                if (value > MAX_VALUE)
                    value = MAX_VALUE;
                set_VP(VP_DISPLAY_OUTPUT, value);
                break;
            case 2:
                int value = buffer.empty() ? 0 : std:stoi(buffer);
                set_VP(VP_DISPLAY_OUTPUT, value);
                break;
            case 4:
                set_VP(VP_DISPLAY_OUTPUT, 0);
                break;
            case 5:
                if(!buffer.empty()) // If buffer is not empty
                    int value = std::stoi(buffer);
                set_VP(VP_DISPLAY_OUTPUT, value);
                kb.clearBuffer();
        }

    }
    else if(address == VP_BUTTON) {

        std::string buffer = kb.getBuffer();
        value = std::stoi(buffer);
        switch(lastByte) {
            case 0x0001:
                ESP_LOGI(TAG, "[BUTTON] Confirm");
                if(!kb.getBuffer().empty) {
                    int value = std::stoi;

                }
        }

    }
}

void hmi_start()
{
    hmi.echoEnabled(true);
    hmi.hmiCallBack(hmi_event_cb);
    hmi.setPage(0);
}

static uint8_t wait_millis(uint32_t delay_ms)
{
    static uint8_t is_started = 0;
    static uint32_t start_time = 0;
    if(is_started == 0) {
        start_time = esp_timer_get_time() / 1000;
        is_started = 1;
    }
    uint32_t current_tick = esp_timer_get_time() / 1000;
    if(current_tick - start_time > delay_ms && is_started) {
        start_time = current_tick;
        is_started = 0;
        return 1;
    }
    return 0;
}

static void update_rtc(st_rtc_t *t)
{
    if(t->month / 12 > 0) { t->year += t->month / 24; t->month %= t->month; }
    if(t->day / 24 > 0) { t->day += t->hour / 24; t->hour %= t->hour; }
    if(t->min / 60 > 0) { t->hour += t->min / 60; t->min %= t->min; }
    if(t->sec / 60 > 0) { t->min += t->sec / 60; t->sec %= t->sec; }
}
