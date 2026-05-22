#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/uart.h"
#include "DWIN_Keyboard.h"
#include "DWIN.h"
#include <string>
#include <cstring>
#include <cstdio>

// ============================================================================
// ĐỊNH NGHĨA HẰNG SỐ
// ============================================================================
#define VP_KEYBOARD_INPUT   0x2500
#define VP_BUTTON           0x2600
#define VP_DISPLAY_OUTPUT   0x2000
#define ADDRESS_TEMP        0x1010
#define ADDRESS_HUMID       0x1515

// Thời gian
#define VP_TIME_HOUR        0x1800
#define VP_TIME_MINUTE      0x1801
#define VP_TIME_SECOND      0x1802
#define VP_DATE_DAY         0x1810
#define VP_DATE_MONTH       0x1811
#define VP_DATE_YEAR        0x1812

// Giá trị key - LƯU Ý: KEY_ENTER đã được định nghĩa trong DWIN.h là 0x00F1
// Không định nghĩa lại ở đây
#define KEY_RUN             0x0002
#define KEY_STOP            0x0003

// Cấu hình UART
#define UART_PORT           UART_NUM_2
#define RX_PIN              16
#define TX_PIN              17
#define DGUS_BAUD           115200

// Cấu hình hệ thống
#define MAX_INPUT_DIGITS    2
#define MAX_TEMP_VALUE      70
#define SENSOR_UPDATE_MS    2000
#define TIME_UPDATE_MS      1000

static const char* TAG = "DWIN_APP";

// ============================================================================
// CẤU TRÚC DỮ LIỆU
// ============================================================================

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_t;

typedef struct {
    float temperature;
    float humidity;
    bool isSystemRunning;
} sensor_data_t;

// ============================================================================
// BIẾN TOÀN CỤC
// ============================================================================
static rtc_t currentTime;
static sensor_data_t sensorData;
static DWIN_Keyboard keyboard;  // Dùng thư viện bàn phím
static QueueHandle_t dwin_event_queue = NULL;
uint8_t Frequency;
// ============================================================================
// HÀM SETVPWORD - GỬI DỮ LIỆU LÊN DWIN
// ============================================================================

void setVPWord(uint16_t address, uint16_t data) {
    uint8_t frame[8];
    
    frame[0] = 0x5A;
    frame[1] = 0xA5;
    frame[2] = 0x05;
    frame[3] = 0x82;
    frame[4] = (address >> 8) & 0xFF;
    frame[5] = address & 0xFF;
    frame[6] = (data >> 8) & 0xFF;
    frame[7] = data & 0xFF;
    
    uart_write_bytes(UART_PORT, (const char*)frame, 8);
    
    ESP_LOGI(TAG, "[GUI] Frame gui len VP 0x%04X: gia tri %d", address, data);
    ESP_LOGD(TAG, "Frame: %02X %02X %02X %02X %02X %02X %02X %02X", 
             frame[0], frame[1], frame[2], frame[3], 
             frame[4], frame[5], frame[6], frame[7]);
}

// ============================================================================
// CÁC HÀM GỬI DỮ LIỆU
// ============================================================================

void sendTemperature(float temperature) {
    uint16_t tempValue = (uint16_t)(temperature * 100);
    setVPWord(ADDRESS_TEMP, tempValue);
    ESP_LOGI(TAG, "Da gui nhiet do: %.1f°C (gia tri: %d)", temperature, tempValue);
}

void sendHumidity(float humidity) {
    uint16_t humValue = (uint16_t)(humidity * 100);
    setVPWord(ADDRESS_HUMID, humValue);
    ESP_LOGI(TAG, "Da gui do am: %.0f%% (gia tri: %d)", humidity, humValue);
}

void sendTimeToDWIN(int hour, int minute) {
    setVPWord(VP_TIME_HOUR, (uint16_t)hour);
    setVPWord(VP_TIME_MINUTE, (uint16_t)minute);
    ESP_LOGI(TAG, "[TIME] Da gui len DWIN: %02d:%02d:%02d", hour, minute, currentTime.second);
}

void sendDateToDWIN(int day, int month, int year) {
    setVPWord(VP_DATE_DAY, (uint16_t)day);
    setVPWord(VP_DATE_MONTH, (uint16_t)month);
    setVPWord(VP_DATE_YEAR, (uint16_t)year);
    ESP_LOGI(TAG, "[DATE] Da gui len DWIN: %02d/%02d/%04d", day, month, year);
}

// ============================================================================
// CẬP NHẬT THỜI GIAN
// ============================================================================

void updateRealTime() {
    currentTime.second++;
    
    if (currentTime.second >= 60) {
        currentTime.second = 0;
        currentTime.minute++;
        
        if (currentTime.minute >= 60) {
            currentTime.minute = 0;
            currentTime.hour++;
            
            if (currentTime.hour >= 24) {
                currentTime.hour = 0;
                currentTime.day++;
                
                int ngayTrongThang = 31;
                if (currentTime.month == 4 || currentTime.month == 6 || 
                    currentTime.month == 9 || currentTime.month == 11) {
                    ngayTrongThang = 30;
                } else if (currentTime.month == 2) {
                    bool namNhuan = (currentTime.year % 400 == 0) || 
                                    (currentTime.year % 4 == 0 && currentTime.year % 100 != 0);
                    ngayTrongThang = namNhuan ? 29 : 28;
                }
                
                if (currentTime.day > ngayTrongThang) {
                    currentTime.day = 1;
                    currentTime.month++;
                    
                    if (currentTime.month > 12) {
                        currentTime.month = 1;
                        currentTime.year++;
                    }
                }
            }
        }
    }
}

// ============================================================================
// CẬP NHẬT CẢM BIẾN MÔ PHỎNG
// ============================================================================

void updateTemperature() {
    if (sensorData.isSystemRunning) {
        sensorData.temperature += 0.25f;
        if (sensorData.temperature > 90.0f) {
            sensorData.temperature = 30.0f;
        }
        sendTemperature(sensorData.temperature);
    }
}

void updateHumidity() {
    if (sensorData.isSystemRunning) {
        sensorData.humidity += 0.5f;
        if (sensorData.humidity > 90.0f) {
            sensorData.humidity = 60.0f;
        }
        sendHumidity(sensorData.humidity);
    }
}

// ============================================================================
// XỬ LÝ SỰ KIỆN DWIN - SỬ DỤNG DWIN_Keyboard
// ============================================================================

void processDWINEvents() {
    uint8_t data[256];
    int length = uart_read_bytes(UART_PORT, data, sizeof(data), 10 / portTICK_PERIOD_MS);
    
    if (length > 0) {
        for (int i = 0; i < length - 5; i++) {
            if (data[i] == 0x5A && data[i+1] == 0xA5) {
                uint8_t cmd = data[i+3];
                if (cmd == 0x83) {
                    uint16_t address = (data[i+4] << 8) | data[i+5];
                    uint8_t dataLen = data[i+6];
                    
                    if (dataLen >= 1) {
                        // ====================================================
                        // QUAN TRỌNG: Với dataLen=1, key code nằm tại data[i+7+1]
                        // Vì có 1 byte dataLen ở vị trí i+6 và 1 byte dữ liệu ở i+7
                        // Nhưng theo raw data, key code nằm ở i+8!
                        // ====================================================
                        int lastByte = data[i+7+dataLen];  // i+7+1 = i+8
                        
                        ESP_LOGI(TAG, "======================================================================");
                        ESP_LOGI(TAG, "[EVENT] VP:0x%04X | dataLen:%d | KeyCode:0x%02X (%d)", 
                                 address, dataLen, lastByte, lastByte);
                        
                        if (address == VP_KEYBOARD_INPUT) {
                            int result = keyboard.processKey(lastByte);
                            std::string buffer = keyboard.getBuffer();
                            
                            if (result == 1 && !buffer.empty()) {
                                int value = std::stoi(buffer);
                                if (value > MAX_TEMP_VALUE) {
                                    value = MAX_TEMP_VALUE;
                                    keyboard.setBuffer(std::to_string(value));
                                }
                                setVPWord(VP_DISPLAY_OUTPUT, value);
                                ESP_LOGI(TAG, "[NHAP] So: %s", keyboard.getBuffer().c_str());
                            }
                            else if (result == 2) {
                                int value = buffer.empty() ? 0 : std::stoi(buffer);
                                setVPWord(VP_DISPLAY_OUTPUT, value);
                            }
                            else if (result == 4) {
                                setVPWord(VP_DISPLAY_OUTPUT, 0);
                            }
                            else if (result == 5 && !buffer.empty()) {
                                int value = std::stoi(buffer);
                                setVPWord(VP_DISPLAY_OUTPUT, value);
                                keyboard.clearBuffer();
                                ESP_LOGI(TAG, "[CONFIRM] %d", value);
                            }
                        }
                        else if (address == VP_BUTTON) {
                            switch (lastByte) {
                                case 0x0001:
                                    ESP_LOGI(TAG, "[BUTTON] CONFIRM");
                                    if (!keyboard.getBuffer().empty()) {
                                        int value = std::stoi(keyboard.getBuffer());
                                        setVPWord(VP_DISPLAY_OUTPUT, value);
                                        keyboard.clearBuffer();
                                        Frequency = value;
                                            ESP_LOGI(TAG, "FREQUENCY:%d Hz",Frequency);
                                    }
                                    break;
                                case 0x0002:
                                    ESP_LOGI(TAG, "[BUTTON] RUN");
                                    sensorData.isSystemRunning = true;
                                    break;
                                case 0x0003:
                                    ESP_LOGI(TAG, "[BUTTON] STOP");
                                    //sensorData.isSystemRunning = false;
                                    break;
                                case 0x0004:
                                    ESP_LOGI(TAG, "[BUTTON] BACK TO KEYBOARD");
                                    //sensorData.isSystemRunning = false;
                                    break;
                                case 0x0005:
                                    ESP_LOGI(TAG, "[BUTTON] FORWARD MOTOR");
                                    //sensorData.isSystemRunning = false;
                                    break;
                                case 0x0006:
                                    ESP_LOGI(TAG, "[BUTTON] REVERSE MOTOR");
                                    //sensorData.isSystemRunning = false;
                                    break;
                                default:
                                    ESP_LOGI(TAG, "[BUTTON] Unknown: 0x%02X", lastByte);
                                    break;
                            }
                        }
                        ESP_LOGI(TAG, "======================================================================");
                    }
                }
            }
        }
    }
}

// ============================================================================
// FREE RTOS TASKS
// ============================================================================

void timeUpdateTask(void* pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(TIME_UPDATE_MS);
    
    while (1) {
        vTaskDelayUntil(&lastWakeTime, interval);
        updateRealTime();
        sendTimeToDWIN(currentTime.hour, currentTime.minute);
        sendDateToDWIN(currentTime.day, currentTime.month, currentTime.year);
    }
}

void sensorUpdateTask(void* pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(SENSOR_UPDATE_MS));
        updateTemperature();
        updateHumidity();
    }
}

void hmiMainTask(void* pvParameters) {
    // Gửi dữ liệu khởi tạo
    sendTimeToDWIN(currentTime.hour, currentTime.minute);
    sendDateToDWIN(currentTime.day, currentTime.month, currentTime.year);
    sendTemperature(sensorData.temperature);
    sendHumidity(sensorData.humidity);
    setVPWord(VP_DISPLAY_OUTPUT, 0);
    
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, "  DWIN HMI System Ready - With Keyboard");
    ESP_LOGI(TAG, "  UART%d: %d baud, RX=%d, TX=%d", UART_PORT, DGUS_BAUD, RX_PIN, TX_PIN);
    ESP_LOGI(TAG, "  VP_KEYBOARD_INPUT = 0x2500");
    ESP_LOGI(TAG, "  VP_DISPLAY_OUTPUT = 0x2000");
    ESP_LOGI(TAG, "==========================================");
    
    while (1) {
        processDWINEvents();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ============================================================================
// KHỞI TẠO DỮ LIỆU
// ============================================================================

void initSystemData() {
    currentTime.second = 0;
    currentTime.minute = 45;
    currentTime.hour = 16;
    currentTime.day = 18;
    currentTime.month = 7;
    currentTime.year = 2026;
    
    sensorData.temperature = 30.0f;
    sensorData.humidity = 60.0f;
    sensorData.isSystemRunning = true;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

extern "C" void app_main() {
    initSystemData();
    
    // Khởi tạo UART cho DWIN
    uart_config_t uart_config = {
        .baud_rate = DGUS_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 1024 * 2, 1024 * 2, 0, NULL, 0));
    
    ESP_LOGI(TAG, "UART%d initialized at %d baud", UART_PORT, DGUS_BAUD);
    
    // Khởi tạo UART0 cho debug
    uart_config_t uart_debug_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = 0
    };
    
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_debug_config));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    
    // Tạo các task
    xTaskCreatePinnedToCore(hmiMainTask, "HMI_Task", 8192, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(timeUpdateTask, "Time_Task", 4096, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(sensorUpdateTask, "Sensor_Task", 4096, NULL, 3, NULL, 1);
    
    ESP_LOGI(TAG, "System initialized successfully!");
}