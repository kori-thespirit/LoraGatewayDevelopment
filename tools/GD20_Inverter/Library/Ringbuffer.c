#include "Ringbuffer.h"

bool Is_Buffer_Full(Ringbuffer_Context *ctx) {
//	if ((((ctx->Head + 1) % ctx->buffer_size) == ctx->Tail)) {
//		ctx->Flag.flag_is_full = 1;
//		return true;
//	}
	if(ctx == NULL ) return false;
	uint16_t next_head = ctx->Head + 1;
	if (next_head >= ctx->buffer_size) {
		next_head = 0;
	}
	return (next_head == ctx->Tail);
}
bool Is_Buffer_Empty(Ringbuffer_Context *ctx) {

	return (ctx->Head == ctx->Tail); // true neu head == tail , failse thi 0
}
bool Ring_Buffer_Init(Ringbuffer_Context *ctx, uint8_t buffer_size) {
	if(ctx == NULL || buffer_size == 0) return false;
	ctx->Head = 0;
	ctx->Tail = 0;

	ctx->buffer_size = buffer_size;
	ctx->Cicular_Buffer = (uint8_t*) malloc(buffer_size * sizeof(uint8_t));
	if (ctx->Cicular_Buffer == NULL) {
		return false;
	}
	memset(ctx->Cicular_Buffer, 0, ctx->buffer_size);
	ctx->Ring_Buffer_State = Ring_Buffer_Ready;
	return true;
}
bool Ring_Circular_Deinit(Ringbuffer_Context *ctx) {
	if (ctx->Cicular_Buffer == NULL)
		return false;
	free(ctx->Cicular_Buffer);
	ctx->Cicular_Buffer = NULL;
	ctx->Ring_Buffer_State = Ring_Buffer_unit;
	return true;
}

void Ring_Buffer_Write_To_Buffer(Ringbuffer_Context *ctx, uint8_t *data,
		uint16_t len) {
	if (ctx == NULL || data == NULL)
		return;
	for (uint16_t i = 0; i < len; i++) {
		if (!Is_Buffer_Full(ctx)) {
			ctx->Ring_Buffer_State = Ring_Buffer_Writing;
			ctx->Cicular_Buffer[ctx->Head++] = data[i];
			if (ctx->Head >= ctx->buffer_size) {
				ctx->Head = 0;
			}
		}else {
			break;
		}
	}
	ctx->Ring_Buffer_State = Ring_Buffer_Write_Complete;
}
uint16_t Ring_Buffer_Read_From_Buffer(Ringbuffer_Context *ctx, uint8_t *destinate,
		uint16_t len) {
	if (ctx == NULL || destinate == NULL)
		return -1;
	uint16_t count = 0 ;
	while(!Is_Buffer_Empty(ctx) && count < len ){

		*(destinate + count) = ctx->Cicular_Buffer[ctx->Tail++];
		count++;
		if (ctx->Tail >= ctx->buffer_size) {
			ctx->Tail = 0;
		}
		ctx->Ring_Buffer_State = Ring_Buffer_Reading;

	}
	ctx->Ring_Buffer_State = Ring_Buffer_Read_Complete;
	return count;

}
size_t Ringbuffer_avalable(Ringbuffer_Context *ctx) {
	if(ctx == NULL) {
		return -1;
	}
	if (ctx->Head >= ctx->Tail)
		return (ctx->Head - ctx->Tail);
	return ctx->buffer_size - (ctx->Tail - ctx->Head);
}

