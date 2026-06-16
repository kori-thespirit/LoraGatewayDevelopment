#include <GD20_Modbus_Slave.h>
#include "main.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

GD20_Slave_t GD20_Slave_Config;

void setup(void) {
	GD20_Slave_Config = (GD20_Slave_t ) { .VFD_SLAVE_ID = 0x03,
					.Uart_Handle.huart = &huart1, };
	GD20_Slave_RTU_Init(&GD20_Slave_Config);

}
void main_loop(void) {
	GD20_Slave_ProcessTx(&GD20_Slave_Config);
//	uint16_t parametter_address = 0x0020;
//	uint16_t Command_address =  0x0100;
//	GD20_Slave_SendWriteResponse(&GD20_Slave_Config,CMD_Write,parametter_address,Command_address);
//	HAL_Delay(1000);

}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		GD20_Slave_TxDone(&GD20_Slave_Config);
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if (huart->Instance == USART1) {
		GD20_Slave_ProcessRx(&GD20_Slave_Config,Size);
	}
}
