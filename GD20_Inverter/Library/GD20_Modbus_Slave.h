#ifndef _GD20_VDF_H
#define _GD20_VDF_H

#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "stm32f1xx_hal.h"// thay the ten cua chip khac vi du stm32f4xx_hal.h // sai cho STM32F405
#ifdef _cplusplus
extern "C" {
#endif

#define CMD_Read 		     0x03
#define CMD_Write			 0x06
//#define CMD_Continuous_Write 0x10



typedef struct __attribute__((packed)){
	uint8_t inverter_address;
	uint8_t function_code;
	uint16_t parametter_address;
	uint16_t command_address;
	uint16_t CRC_CHECK;
}RTU_Contex_Write_t;

typedef struct __attribute__((packed)){
	uint8_t inverter_address;
	uint8_t function_code;
	uint8_t data_address;
	uint16_t data_content;
	uint16_t CRC_CHECK;
}RTU_Contex_Read_t;

typedef enum{
    Motor_FORWARD_RUN      = 1,
    Motor_REVERSE_RUN      = 2,
    Motor_FORWARD_JOGGING  = 3,
	Motor_REVERSE_JOGGING  = 4,
    Motor_STOP        	   = 5,
    Motor_EMG_STOP		   = 6,
	Motor_FAULT_RESET	   = 7,
	Motor_JOGGING_STOP	   = 8
}GD20_Motor_Control;
#define GD20_Reg_Motor_Control 0x2000

typedef enum {
    SW1_FORWARD_RUN = 1,
    SW1_REVERSE_RUN = 2,
    SW1_STOP        = 3,
    SW1_FAULT       = 4,
    SW1_POFF        = 5,
    SW1_PRE_EXCITE  = 6
}GD20_SW1_Status;
#define GD20_Reg_SW1_Status  0x2100

typedef struct{
	GD20_SW1_Status SW1_Status;
	GD20_Motor_Control Motor_Control;
}GD20_Inverter_Status;

typedef struct {
    uint8_t DMA_RX_Complete;
    uint8_t DMA_TX_Complete;
}flags;

typedef struct {
    UART_HandleTypeDef *huart;
    GD20_Inverter_Status GD20_Status;
    flags flags;
    RTU_Contex_Write_t rx_frame;
    RTU_Contex_Write_t tx_frame;
    RTU_Contex_Read_t  tx_read_frame;
    uint8_t VFD_SLAVE_ID;

} GD20_Slave_t;

bool GD20_Slave_RTU_Init(GD20_Slave_t *ctx);

void GD20_Slave_Process(GD20_Slave_t *ctx);
void GD20_Slave_SendReadResponse(GD20_Slave_t *ctx,uint8_t comnand,uint8_t number_of_byte ,uint16_t content);
void GD20_Slave_SendWriteResponse(GD20_Slave_t *ctx,uint8_t comnand,uint16_t parametter_address,uint16_t command_address);
#ifdef _cplusplus
}
#endif
#endif
