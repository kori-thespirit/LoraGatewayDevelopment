#ifndef _Uart_H
#define _Uart_H

#ifdef _cplusplus
 extern "C" {
#endif
 /*Private Library*/
#include "stm32f1xx_hal.h"

#include "Ringbuffer.h"
#include "cmsis_compiler.h"
/*End setup Library*/


#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "stdbool.h"


typedef struct {
     volatile bool DMA_RX_Complete;
     volatile bool DMA_TX_Complete;
     volatile bool Enable_Transfer;
}flags;

typedef struct Uart_context{
	UART_HandleTypeDef *huart;
	char rxframe[256];
	char txframe[256];
	uint16_t rxlen;
	flags flags;
	Ringbuffer_Context Ringbuffer_handle;
}Uart_context;


void Serial_Begin(Uart_context *ctx);
void Serial_printf(Uart_context *ctx,const char *fmt, ...) ;
void Serial_Processing(Uart_context *ctx);

void Serial_Write(Uart_context *ctx, uint8_t *data,uint16_t len);
void Serial_Enable_Comm(Uart_context *ctx);
void Serial_IsDmaTxDone(Uart_context *ctx);
//void Serial_Disable_Comm(Uart_context *ctx);
void Serial_Listening(Uart_context *ctx,uint16_t size);
char *Serial_avalabile(Uart_context *ctx);
//void Serial_Restart_DMA(Uart_context *ctx);
#ifdef _cplusplus
 }
#endif

#endif
