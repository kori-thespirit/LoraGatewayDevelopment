#ifndef _RingBuffer_H
#define _RingBuffer_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef struct{
	volatile bool flag_is_full;
	volatile bool flag_is_empty;
	volatile bool Enable_Transfer;
}Ringbuffer_Flag;

typedef enum Ring_Buffer_State{
	Ring_Buffer_unit 			= 0,
	Ring_Buffer_Ready  			= 1,
	Ring_Buffer_Writing  		= 2,
	Ring_Buffer_Write_Complete	= 3,
	Ring_Buffer_Reading 		= 4,
	Ring_Buffer_Read_Complete 	= 5
}Ring_Buffer_State;

typedef struct {
	Ring_Buffer_State Ring_Buffer_State;
	uint8_t *Cicular_Buffer;
	uint8_t  buffer_size;
	volatile uint16_t Head;
	volatile uint16_t Tail;
	Ringbuffer_Flag Flag;
}Ringbuffer_Context;

bool Is_Buffer_Full(Ringbuffer_Context *ctx);
bool Is_Buffer_Empty(Ringbuffer_Context *ctx);


bool Ring_Buffer_Init(Ringbuffer_Context* ctx , uint8_t buffer_size);
bool Ring_Buffer_Deinit(Ringbuffer_Context* ctx);

void Ring_Buffer_Write_To_Buffer(Ringbuffer_Context* ctx,uint8_t* data,uint16_t len);
uint16_t Ring_Buffer_Read_From_Buffer(Ringbuffer_Context* ctx,uint8_t* destinate,uint16_t len);
size_t Ringbuffer_avalable(Ringbuffer_Context *ctx);






size_t Ringbuffer_avalable(Ringbuffer_Context* ctx);



#endif
