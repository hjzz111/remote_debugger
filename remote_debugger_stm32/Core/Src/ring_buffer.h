#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "stm32f1xx_hal.h"

//////////////////////////////////////////////////////////////////////////////////  
//环形缓冲区，需结合内存管理代码使用
//修改日期:2026/9/24
//版本：V1.0
//********************************************************************************
//修改说明
//////////////////////////////////////////////////////////////////////////////////

typedef struct {
    volatile uint16_t readIndex;      // 循环缓冲区读索引
    volatile uint16_t writeIndex;     // 循环缓冲区写索引
    
    uint8_t *heapStartAddr; // 堆首地址
    uint8_t *heapEndAddr;   // 堆末地址
    uint16_t size;           // 缓冲区大小
} ring_buffer_typedef;

uint8_t ringBufferInit(ring_buffer_typedef *pring_buffer_struct, uint16_t size);
void bufferAddReadIndex(ring_buffer_typedef *pring_buffer_struct, uint16_t length);
uint8_t bufferPeekByte(ring_buffer_typedef *pring_buffer_struct, uint16_t i);
uint16_t bufferGetLength(ring_buffer_typedef *pring_buffer_struct);
uint16_t bufferWrite(ring_buffer_typedef *pring_buffer_struct, uint8_t *data, uint16_t length);
uint16_t bufferRead(ring_buffer_typedef *pring_buffer_struct, uint8_t *data, uint16_t len);
uint16_t bufferCRC16(ring_buffer_typedef *pring_buffer_struct, uint16_t start_index, uint16_t len);

#endif
