#include <GD20_Modbus_Slave.h>

static UART_HandleTypeDef *uart;

uint16_t CRC_Check;

static uint16_t modbus_crc16(uint8_t *buffer, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= buffer[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
bool GD20_Slave_RTU_Init(GD20_Slave_t *pHandle) {
	if(pHandle == NULL) return false;

	uart = pHandle->Uart_Handle.huart;
	Serial_Begin(&pHandle->Uart_Handle);
//	_flags->DMA_RX_Complete = 0;

	pHandle->GD20_Status.Motor_Control = Motor_FORWARD_JOGGING;
	pHandle->GD20_Status.SW1_Status    = SW1_STOP ;
	return true;

}
void GD20_Slave_ProcessTx(GD20_Slave_t *pHandle){
	if (pHandle == NULL) return;
	static uint8_t Rx_Frame[30] = {0};
	static bool printf_once = 0;
	char *Temp = Serial_avalabile(&pHandle->Uart_Handle);

	if(Temp){
		memcpy((char *)Rx_Frame,Temp,pHandle->Uart_Handle.rxlen);
		memcpy(&pHandle->rx_frame,Rx_Frame, 8);
		printf_once = 1;
	}
	if(pHandle->Uart_Handle.rxlen < 8) return;
	if(Rx_Frame[0] != pHandle->VFD_SLAVE_ID ) return;

	if(!printf_once){
		printf_once = 0;
		return;
	}
	switch(Rx_Frame[1]){
	case 0x06:// Master request Write
		CRC_Check = modbus_crc16((uint8_t *)Rx_Frame, 6);

		if(CRC_Check != (Rx_Frame[6]  | Rx_Frame[7] << 8)) return;
		printf_once = 1;
		GD20_Slave_SendWriteResponse(pHandle,CMD_Write,Rx_Frame);
		memset(Rx_Frame,0,sizeof(Rx_Frame));
		break;
	case 0x03:
		CRC_Check = modbus_crc16((uint8_t *)Rx_Frame, 6);

		if(CRC_Check != (Rx_Frame[6]  | Rx_Frame[7] << 8)) return;
		printf_once = 1;
		pHandle->rx_frame.CRC_CHECK = CRC_Check;

		uint16_t parametter__address = Rx_Frame[2] << 8 | Rx_Frame[3];
		memset(Rx_Frame,0,sizeof(Rx_Frame));
		if(parametter__address == GD20_Reg_Motor_Control){ // kiểm tra Status của Motor
			//Command_read , number of byte , Motor Status
			GD20_Slave_SendReadResponse(pHandle,CMD_Read,2,pHandle->GD20_Status.Motor_Control);
		}else if(parametter__address == GD20_Reg_SW1_Status){ // kiểm tra SW1
			GD20_Slave_SendReadResponse(pHandle,CMD_Read,2,pHandle->GD20_Status.SW1_Status);
		}
		break;
	case 0x10:// Master request continuous Writting
		break;
	default :
		break;
	}
}
void GD20_Slave_ProcessRx(GD20_Slave_t *pHandle,uint16_t size){
	Serial_Listening(&pHandle->Uart_Handle, size);
}
void GD20_Slave_TxDone(GD20_Slave_t *pHandle){
	Serial_IsDmaTxDone(&pHandle->Uart_Handle);
}
void GD20_Slave_SendWriteResponse(GD20_Slave_t *pHandle,uint8_t command,uint8_t *src){

		pHandle->tx_write_frame = (RTU_Contex_Write_t) {
					.inverter_address = pHandle->VFD_SLAVE_ID,
					.function_code = command,
					.parametter_address = src[2] << 8 | src[3],
					.command_address    = src[4] << 8 | src[5],
					.CRC_CHECK          = src[7] << 8 | src[6]
		};

		Serial_Write(&pHandle->Uart_Handle,(uint8_t *)&pHandle->tx_write_frame,8);
		Serial_Enable_Comm(&pHandle->Uart_Handle);
		Serial_Processing(&pHandle->Uart_Handle);

}
void GD20_Slave_SendReadResponse(GD20_Slave_t *pHandle,uint8_t command,uint8_t number_of_byte ,uint16_t content){

		pHandle->tx_read_frame = (RTU_Contex_Read_t) {
					.inverter_address = pHandle->VFD_SLAVE_ID,
					.function_code = command,
					.data_address = number_of_byte,
					.data_content = content << 8 | content >> 8,
		};
		uint16_t crc = modbus_crc16((uint8_t *)&pHandle->tx_read_frame, 5);
		pHandle->tx_read_frame.CRC_CHECK = crc;
		Serial_Write(&pHandle->Uart_Handle,(uint8_t *)&pHandle->tx_read_frame,8);
		Serial_Enable_Comm(&pHandle->Uart_Handle);
		Serial_Processing(&pHandle->Uart_Handle);

}


