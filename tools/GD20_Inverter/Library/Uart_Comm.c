#include <Uart_Comm.h>

static char *Temp;
void Serial_Begin(Uart_context *ctx) {
	if(ctx == NULL) return;
	Ring_Buffer_Init(&ctx->Ringbuffer_handle, 200);
	ctx->flags.DMA_TX_Complete = 1;
	ctx->flags.Enable_Transfer = 0;
	Temp = (char* )malloc(sizeof(char));
	HAL_UARTEx_ReceiveToIdle_DMA(ctx->huart,(uint8_t *)ctx->rxframe, sizeof(ctx->rxframe));
}
void Serial_printf(Uart_context *ctx, const char *fmt, ...) {
	if (ctx == NULL || fmt == NULL)
		return;
	char buff[256];
	va_list arg;
	va_start(arg, fmt);
	int len = vsnprintf(buff, sizeof(buff), fmt, arg);
	va_end(arg);

	if (len <= 0)
		return;

	Ring_Buffer_Write_To_Buffer(&ctx->Ringbuffer_handle, (uint8_t*) buff, len);
}
void Serial_Write(Uart_context *ctx, uint8_t *data,uint16_t len)
{
    if(ctx == NULL || data == NULL)
        return;

    Ring_Buffer_Write_To_Buffer(
            &ctx->Ringbuffer_handle,
            data,
            len);
}
void Serial_Processing(Uart_context *ctx) {
	if (ctx == NULL)
		return;

	if (ctx->flags.DMA_TX_Complete  && ctx->flags.Enable_Transfer) {
		ctx->flags.DMA_TX_Complete = 0;
		ctx->flags.Enable_Transfer = 0;
		memset(ctx->txframe, 0 , sizeof(ctx->txframe));

		uint16_t len = Ring_Buffer_Read_From_Buffer(&ctx->Ringbuffer_handle, (uint8_t*) ctx->txframe, sizeof(ctx->txframe));
		if (len > 0) {
			HAL_UART_Transmit_DMA(ctx->huart, (uint8_t*) ctx->txframe,
					len); // tru ky tu '\0'
		}else{
			ctx->flags.DMA_TX_Complete = 1; // tranh truong hop kh co du lieu van bat flag cho truong hop tiep theo
		}
	}
}
void Serial_Enable_Comm(Uart_context *ctx) {
	if(ctx == NULL) return;
	ctx->flags.Enable_Transfer = 1;

}
void Serial_IsDmaTxDone(Uart_context *ctx){
	if(ctx == NULL) return;
	ctx->flags.DMA_TX_Complete = 1;
}
void Serial_Listening(Uart_context *ctx,uint16_t size)
{
    if(ctx == NULL) return;
    if(ctx->flags.DMA_RX_Complete == 1) return;

    ctx->flags.DMA_RX_Complete = 1;

    char *newPtr = realloc(Temp, size + 1);
    if(newPtr == NULL) return;

    Temp = newPtr;

    memcpy(Temp, ctx->rxframe, size);
    ctx->rxlen = size;
    Temp[ctx->rxlen ] = '\0';
    HAL_UARTEx_ReceiveToIdle_DMA(ctx->huart,(uint8_t *)ctx->rxframe, sizeof(ctx->rxframe));
}
//void Serial_Disable_Comm(Uart_context *ctx){
//
//}
char *Serial_avalabile(Uart_context *ctx){
	if(ctx == NULL) return 0;
	if(ctx->flags.DMA_RX_Complete){
		ctx->flags.DMA_RX_Complete = 0;
		return Temp;
	}
	return 0;
}


