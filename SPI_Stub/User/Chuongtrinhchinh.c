#include "Chuongtrinhchinh.h"
#include "stdbool.h"
#include "stdint.h"
#include "main.h"
/* USER CODE BEGIN 0 */
uint8_t rxBuffer[2] = { 0 };
uint8_t txBuffer[2] = { 0 };
bool flag = 0;
bool test = 0;
uint32_t start = 0;

extern SPI_HandleTypeDef hspi1;
void setup(void){
	 HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

}
void main_loop(void){
	if (HAL_GPIO_ReadPin(CS_GPIO_Port, CS_Pin) == 0) {
	    // Nhận 2 byte từ master
	    HAL_SPI_Receive(&hspi1, rxBuffer, 2, HAL_MAX_DELAY);

	    if (rxBuffer[0] == 0x37) {
	        txBuffer[0] = 0x80;
	        start = HAL_GetTick();
	        flag = 1;
	    } else {
	        txBuffer[0] = 0x00;
	    }
	    txBuffer[1] = 0x00;  // byte thứ hai trả về (có thể tùy ý)

	    // Gửi 2 byte trả lời
	    HAL_SPI_Transmit(&hspi1, txBuffer, 2, HAL_MAX_DELAY);
	}
}
