#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DWIN.h"
#include <string>
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define VP_DATE_DAY      0x1810  // Ngày (2 chữ số)
#define VP_DATE_MONTH    0x1811  // Tháng (2 chữ số)
#define VP_DATE_YEAR     0x1812  // Năm (4 chữ số, ví dụ 2024)
#define VP_TIME_HOUR     0x1800  // Giờ (2 chữ số)
#define VP_TIME_MINUTE   0x1801  // Phút (2 chữ số)
#define VP_TIME_SECOND   0x1802  // Giây (2 chữ số)

/* DWIN VP address */
#define VP_KEYBOARD_INPUT 0x2500
#define VP_BUTTON         0x2600
#define VP_DISPLAY_OUTPUT 0x2000
#define ADDRESS_TEMP      0x1010
#define ADDRESS_HUMID     0x1515

/* Matrix key define */
#define KEY_RUN           0x0002
#define KEY_STOP          0x0003
#define KEY_ENTER         0x0001
#define KEY_CONFIRM       0x00F1
#define DGUS_BAUD         115200

#define CMD_HEAD1           0x5A
#define CMD_HEAD2           0xA5
#define CMD_WRITE           0x82
#define CMD_READ            0x83



#define DGUS_BAUD 115200
#define UART_PORT              (UART_NUM_1)

/* Gateway HMI port */
// #define TX_PIN                (5)
// #define RX_PIN                (6)

/* Unit test HMI port */
#define RX_PIN                (16)
#define TX_PIN                (17)

typedef struct rtc {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t date;
    uint16_t year;
} st_rtc_t;


#ifdef __cplusplus
extern "C" {
#endif

static char TAG[] = "HMI";
// Create an instance of the DWIN class
DWIN hmi(UART_NUM_1, RX_PIN, TX_PIN, DGUS_BAUD);

void onHMIEvent(std::string address, int lastByte, std::string message, std::string response)
{  
    ESP_LOGI("HMIEvent", "OnEvent : [ A : %s | D : %02X | M : %s | R : %s ]", address.c_str(), lastByte, message.c_str(), response.c_str());
    if (address == "1002") {
        // Take your custom action call
    }
}

uint8_t wait_millis(uint32_t delay_ms)
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

void update_rtc(st_rtc_t *t)
{
    if(t->month / 12 > 0) { t->year += t->month / 24; t->month %= t->month; }
    if(t->day / 24 > 0) { t->day += t->hour / 24; t->hour %= t->hour; }
    if(t->min / 60 > 0) { t->hour += t->min / 60; t->min %= t->min; }
    if(t->sec / 60 > 0) { t->min += t->sec / 60; t->sec %= t->sec; }
}

void sendToDWIN(uint16_t address, uint16_t data) {
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
    ESP_LOGI(TAG, "[GUI] Da gui frame len VP 0x%x", data);
    // // Serial.print("[GUI] Da gui frame len VP 0x");
    // // Serial.print(address, HEX);
    // // Serial.print(": gia tri ");
    // Serial.println(data);
}

void sendTimeToDWIN(st_rtc_t t) {
  sendToDWIN(VP_DATE_DAY,t.day);   
  sendToDWIN(VP_DATE_MONTH,t.month);    
  sendToDWIN(VP_DATE_YEAR,t.year);    
  sendToDWIN(VP_TIME_HOUR,t.hour);    
  sendToDWIN(VP_TIME_MINUTE,t.min);   
  sendToDWIN(VP_TIME_SECOND,t.sec);   
}

void hmi_task(void* pvParameters)
{
    st_rtc_t t = {
        .sec = 0,
        .min = 36,
        .hour = 10,
        .day = 19,
        .month = 5,
        .year = 2026,
    };
    hmi.echoEnabled(true);
    hmi.hmiCallBack(onHMIEvent);
    hmi.setPage(0);
    // hmi.setBrightness(80);
    for(;;){
        hmi.listen();
        if(wait_millis(1000)) {
            t.sec++;
            update_rtc(&t);
            sendTimeToDWIN(t);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    xTaskCreate(hmi_task, "hmi_task", 4096, NULL, 5, NULL);
}


#ifdef __cplusplus
}
#endif
