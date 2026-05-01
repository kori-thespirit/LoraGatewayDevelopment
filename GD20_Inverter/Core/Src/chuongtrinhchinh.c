#include <GD20_Modbus_Slave.h>
#include "main.h"



extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;






GD20_Slave_t GD20_Slave_Config;


void setup(void){
	GD20_Slave_Config = (GD20_Slave_t){
	    .VFD_SLAVE_ID = 0x03,
	    .huart = &huart1
	};
	GD20_Slave_RTU_Init(&GD20_Slave_Config);

}
void main_loop(void){
	GD20_Slave_Process(&GD20_Slave_Config);

}
