#include <stdio.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "string.h"

#define TXD_PIN             (GPIO_NUM_21)  //++ UART TX and RX Pins for Communication
#define RXD_PIN             (GPIO_NUM_47)
#define EN_GPIO             (3)  //++ Set any GPIO to connect to Simcomm ENABLE Pin
#define RX_BUF_SIZE         (127)
#define MAX_LENGTH          (100)  //++ Max Buffer length for array to store AT Commands
#define INDEX               (14)        //++ Total Number of AT Commands
uint8_t data[RX_BUF_SIZE + 1];         //++ Buffer to store AT Responses from Simcomm
uint8_t esp_chip_id[6];       //++ Array to get Chip ID
int count = 0;

char AT_COMMANDS[INDEX][MAX_LENGTH] = {
    {"AT+CPIN?\r\n"},  //++ All the Necessary AT Commands
    {"AT\r\n"},
    {"AT+CSQ\r\n"},
    {"AT+CGDCONT=1,\"IP\",\"M2MISAFE\"\r\n"},
    {"AT+CGACT=1,1\r\n"},
    {"AT+CNTP=\"asia.pool.ntp.org\",0\r\n"},  //++ SNTP for Asia
    {"AT+CNTP\r\n"},
    {"AT+CCLK?\r\n"},
    {"AT+CLBS=1\r\n"},  //++ AT Command for Lat-Long Co-ordinates
    {"AT+CMQTTSTART\r\n"},
    {"AT+CMQTTACCQ=0,\"CLIENT 4G\"\r\n"},
    {"AT+CMQTTCFG=\"argtopic\",0,1,1\r\n"},
    {"AT+CMQTTCONNECT=0,\"tcp://test.mosquitto.org:1883\",60,1\r\n"},  //++ Enter your MQTT Broker here
    {"AT+CMQTTSUB=0,\"4GBOARD_SIMCOMM/cmd\",2,1\r\n"}};                //++ Topic to Subscribe

void simcomm_uart_init(void)  //++ Initializing UART
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int send_cmd_to_simcomm(const char* logName, const char* data)  //++ Sending AT Commands to Simcomm via UART
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    return txBytes;
}
void simcomm_response_parser(const char* data)  //++ Parser to parse AT Responses from Simcomm
{
    for (int i = 0; i <= strlen(data); i++) {
        if (data[i] == 'O' && data[i + 1] == 'K')  //++ AT Response for OK
        {
            printf(" AT Successful\n");
        }
    }
}
void app_main(void) {
    simcomm_uart_init();                            //++ Call UART Initializing Function
    gpio_set_direction(EN_GPIO, GPIO_MODE_OUTPUT);  //++ Set GPIO Pin Directions
    gpio_set_level(EN_GPIO, 1);                     //++ Set the ENABLE Pin HIGH to Power ON the Simcomm Module
    printf("LOG START\r\n");
    ESP_LOGI("MAIN", "\n\n-----------------------\nSystem started!!!!");
    vTaskDelay(8000 / portTICK_PERIOD_MS);  //++ Delay to allow the Simcomm Module to Power ON
    
    while(count<5) {
        send_cmd_to_simcomm("UNIT_TEST", AT_COMMANDS[1]);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // esp_efuse_mac_get_default(esp_chip_id);  //++ Get the ESP Chip ID
        
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 300 / portTICK_PERIOD_MS);

        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI("PARSE", "Read %d bytes: '%s'", rxBytes, data);
            simcomm_response_parser((const char*)data);  //++ Call the Parser Function to Parse the AT Response
        }
        count++;
        vTaskDelay(pdMS_TO_TICKS(1000));  // Nghỉ 1 giây giữa các lần thử[cite: 1]
    }
    printf("LOG END\r\n");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}