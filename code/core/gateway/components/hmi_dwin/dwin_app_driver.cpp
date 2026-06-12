#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dwin_app_driver.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "DWIN.h"
#include "dwin_keyboard.h"

#ifdef __cplusplus
extern "C" {
#endif

static char TAG[] = "dwin_app_driver";
// Create an instance of the DWIN class
DWIN hmi(UART_PORT, RX_PIN, TX_PIN, DGUS_BAUD);

void send_text_to_display(uint16_t vpaddress, std::string &text)
{

}

void send_data_to_display(uint16_t address, uint16_t data) 
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
    ESP_LOGI(TAG, "send data 0x%x", data);
}


static void hmi_event_cb(std::string address, int lastByte, std::string message, std::string response)
{
    ESP_LOGI(TAG, "OnEvent : [ A : %s | D : %02X | M : %s | R : %s ]", address.c_str(), lastByte, message.c_str(), response.c_str());
    DWIN_Keyboard kb;
    if(std::stoi(address) == VP_KEYBOARD_INPUT) {
        int result = kb.processKey(lastByte);
        std::string buffer = kb.getBuffer();
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
            case 4:
                value = 0;
                break;
            case 5:
                if(!buffer.empty()) // If buffer is not empty
                    int value = std::stoi(buffer);
                kb.clearBuffer();
                break;
            default:break;

        }
        send_data_to_display(VP_DISPLAY_OUTPUT, value);

    }
    else if(std::stoi(address) == VP_BUTTON) {

        std::string buffer = kb.getBuffer();
        int value = std::stoi(buffer);
        switch(lastByte) {
            case 0x0001:
                ESP_LOGI(TAG, "[BUTTON] Confirm");
                if(!kb.getBuffer().empty()) {
                    int value = std::stoi(kb.getBuffer());
                    send_data_to_display(VP_DISPLAY_OUTPUT, value);
                    ESP_LOGI(TAG, "Frequency: %d Hz", value);
                    kb.clearBuffer();
                }
                break;
            case 0x0002: ESP_LOGI(TAG, "[BUTTON] STOP"); break;
            case 0x0003: ESP_LOGI(TAG, "[BUTTON] RUN"); break;
            case 0x0004: ESP_LOGI(TAG, "[BUTTON] BACK TO KEYBOARD"); break;
            case 0x0005: ESP_LOGI(TAG, "[BUTTON] FORWARD MOTOR"); break;
            case 0x0006: ESP_LOGI(TAG, "[BUTTON] REVERSE MOTOR"); break;
            default:
                ESP_LOGI(TAG, "Unknown command"); break;
        }

    }
}

void hmi_start()
{
    hmi.echoEnabled(true);
    hmi.hmiCallBack(hmi_event_cb);
    hmi.setPage(0);
}

void hmi_register_callback()
{

}

void hmi_listen() { hmi.listen(); }

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

#ifdef __cplusplus
}
#endif
