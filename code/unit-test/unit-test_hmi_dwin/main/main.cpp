#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DWIN.h"
#include <string>
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define DGUS_BAUD 115200
#define TX_PIN                (5)
#define RX_PIN                (6)

#ifdef __cplusplus
extern "C" {
#endif

void onHMIEvent(std::string address, int lastByte, std::string message, std::string response)
{  
    ESP_LOGI("HMIEvent", "OnEvent : [ A : %s | D : %02X | M : %s | R : %s ]", address.c_str(), lastByte, message.c_str(), response.c_str());
    if (address == "1002") {
        // Take your custom action call
    }
}

// Create an instance of the DWIN class
DWIN hmi(UART_NUM_1, RX_PIN, TX_PIN, DGUS_BAUD);

void hmi_task(void* pvParameters)
{
    hmi.echoEnabled(true);
    hmi.hmiCallBack(onHMIEvent);
    hmi.setPage(0);
    for(;;){
        hmi.listen();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    xTaskCreate(hmi_task, "hmi_task", 4096, NULL, 5, NULL);
}


#ifdef __cplusplus
}
#endif
