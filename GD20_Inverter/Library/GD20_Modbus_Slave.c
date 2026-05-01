#include <GD20_Modbus_Slave.h>

static UART_HandleTypeDef *uart;
static flags *_flags;
static uint16_t CRC_Check;
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

bool GD20_Slave_RTU_Init(GD20_Slave_t *ctx) {
	if(ctx == NULL) return false;

	uart = ctx->huart;
//	_flags->DMA_RX_Complete = 0;
	_flags = &ctx->flags;
	_flags->DMA_TX_Complete = 1;
	memset(&ctx->rx_frame, 0, sizeof(RTU_Contex_Write_t));
	memset(&ctx->tx_frame, 0, sizeof(RTU_Contex_Write_t));


	ctx->GD20_Status.Motor_Control = Motor_FORWARD_JOGGING;
	ctx->GD20_Status.SW1_Status    = SW1_STOP ;
	return true;

}
void GD20_Slave_Process(GD20_Slave_t *ctx){
	if (ctx == NULL) return;
	if(!_flags->DMA_TX_Complete) return;
	uint8_t Rx_Frame[8]= {0};

	if (HAL_UART_Receive(ctx->huart, Rx_Frame, 2, 10) != HAL_OK)
	    return;
	if(Rx_Frame[0] != ctx->VFD_SLAVE_ID ) return;

	switch(Rx_Frame[1]){
	case 0x06:// Master request Write
		if(HAL_UART_Receive(ctx->huart,&Rx_Frame[2],6 ,10) != HAL_OK) return;
		CRC_Check = modbus_crc16(Rx_Frame, 6);

		if(CRC_Check != (Rx_Frame[7] << 8 | Rx_Frame[6])) return; // HIGH /| LOW
		ctx->rx_frame.CRC_CHECK = (Rx_Frame[7] << 8 | Rx_Frame[6]);
		memcpy(&ctx->rx_frame,Rx_Frame, 8); // copy vào rx_Frame và gửi lại lệnh
		GD20_Slave_SendWriteResponse(ctx,CMD_Write,ctx->rx_frame.parametter_address,ctx->rx_frame.command_address);
		break;
	case 0x03:// Master request Read Satus
		if(HAL_UART_Receive(ctx->huart,&Rx_Frame[2],6 ,10) != HAL_OK) return;
		CRC_Check = modbus_crc16(Rx_Frame, 6);

		if(CRC_Check != (Rx_Frame[7] << 8 | Rx_Frame[6])) return; // HIGH /| LOW

//		ctx->rx_frame.CRC_CHECK = (Rx_Frame[7] << 8 | Rx_Frame[6]);

		memcpy(&ctx->rx_frame,Rx_Frame, 8); // copy vào rx_Frame và gửi lại lệnh
		uint16_t parametter__address = Rx_Frame[2] << 8 | Rx_Frame[3];

		if(parametter__address == GD20_Reg_Motor_Control){ // kiểm tra Status của Motor
			//Command_read , number of byte , Motor Status
			GD20_Slave_SendReadResponse(ctx,CMD_Read,2,ctx->GD20_Status.Motor_Control);
		}else if(parametter__address == GD20_Reg_SW1_Status){ // kiểm tra SW1
			GD20_Slave_SendReadResponse(ctx,CMD_Read,2,ctx->GD20_Status.SW1_Status);
		}
		break;
	case 0x10:// Master request continuous Writting
		break;
	default :
		break;
	}
}
void GD20_Slave_SendWriteResponse(GD20_Slave_t *ctx,uint8_t comnand,uint16_t parametter_address,uint16_t command_address){
		_flags->DMA_TX_Complete = 0;
		ctx->tx_frame = (RTU_Contex_Write_t) {
					.inverter_address = ctx->VFD_SLAVE_ID,
					.function_code = comnand,
					.parametter_address = parametter_address,
					.command_address = command_address,
		};
		uint16_t crc = modbus_crc16((uint8_t *)&ctx->tx_frame, 6);
		ctx->tx_frame.CRC_CHECK = (crc << 8) | (crc >> 8);

		HAL_UART_Transmit_DMA(ctx->huart,(uint8_t *)&ctx->tx_frame, 8);
//		HAL_UART_Transmit(ctx->huart, (uint8_t *)&ctx->tx_frame, sizeof(ctx->tx_frame), HAL_MAX_DELAY);

}
void GD20_Slave_SendReadResponse(GD20_Slave_t *ctx,uint8_t comnand,uint8_t number_of_byte ,uint16_t content){
		_flags->DMA_TX_Complete = 0;
		ctx->tx_read_frame = (RTU_Contex_Read_t) {
					.inverter_address = ctx->VFD_SLAVE_ID,
					.function_code = comnand,
					.data_address = number_of_byte,
					.data_content = content << 8 | content >> 8,
		};
		uint16_t crc = modbus_crc16((uint8_t *)&ctx->tx_read_frame, 5);
		ctx->tx_read_frame.CRC_CHECK = (crc << 8) | (crc >> 8);

		HAL_UART_Transmit_DMA(ctx->huart,(uint8_t *)&ctx->tx_read_frame, 7);
//		HAL_UART_Transmit(ctx->huart, (uint8_t *)&ctx->tx_frame, sizeof(ctx->tx_frame), HAL_MAX_DELAY);

}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {

	if(huart == uart){
		_flags->DMA_TX_Complete = 1 ;
	}
}

