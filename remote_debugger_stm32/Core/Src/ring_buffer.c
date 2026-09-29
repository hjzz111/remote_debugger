#include "ring_buffer.h"
#include "string.h"
#include "heap.h"

//////////////////////////////////////////////////////////////////////////////////  
//环形缓冲区，需结合内存管理代码使用
//修改日期:2026/9/24
//版本：V1.0
//********************************************************************************
//修改说明
//////////////////////////////////////////////////////////////////////////////////

/**
  * @brief 环形缓冲区初始化
  * @param pring_buffer_struct  缓冲区结构体
  *        size                 缓冲区大小，单位字节
  * @retval 0                   创建失败
  */
uint8_t ringBufferInit(ring_buffer_typedef *pring_buffer_struct, uint16_t size) {
    pring_buffer_struct->readIndex = 0;
    pring_buffer_struct->writeIndex = 0;
    pring_buffer_struct->size = size;
    
    return heapMalloc(size, &pring_buffer_struct->heapStartAddr, &pring_buffer_struct->heapEndAddr);
}

/**
  * @brief 环形缓冲区增加读索引
  * @param pring_buffer_struct  缓冲区结构体
  *        length               增加读索引大小
  */
void bufferAddReadIndex(ring_buffer_typedef *pring_buffer_struct, uint16_t length) {
    pring_buffer_struct->readIndex += length;
    pring_buffer_struct->readIndex %= pring_buffer_struct->size;
}

/**
  * @brief 环形缓冲区读取指定位置字节，不改变读索引，读取缓冲区绝对索引处数据
  * @param pring_buffer_struct  缓冲区结构体
  *        i                    读取字节索引
  */
uint8_t bufferReadIndex(ring_buffer_typedef *pring_buffer_struct, uint16_t i) {
    uint16_t index = i % pring_buffer_struct->size;
    return *(pring_buffer_struct->heapStartAddr + index);
}

/**
  * @brief 环形缓冲区读取指定索引字节，不改变读索引，读取缓冲区相对索引处数据
  * @param pring_buffer_struct  缓冲区结构体
  *        i                    读取字节索引
  */
uint8_t bufferPeekByte(ring_buffer_typedef *pring_buffer_struct, uint16_t i) {
    uint16_t index = (pring_buffer_struct->readIndex + i) % pring_buffer_struct->size;
    return *(pring_buffer_struct->heapStartAddr + index);
}

/**
  * @brief 环形缓冲区计算未处理的数据长度
  * @param pring_buffer_struct  缓冲区结构体
  * @retval 0                   缓冲区为空
  *         1-BUFFER_SIZE-2     未处理的数据长度
  *         BUFFER_SIZE-1       缓冲区已满
  */
uint16_t bufferGetLength(ring_buffer_typedef *pring_buffer_struct) {
    return (pring_buffer_struct->writeIndex + pring_buffer_struct->size - pring_buffer_struct->readIndex)
        % pring_buffer_struct->size;
}

/**
  * @brief 环形缓冲区计算剩余空间
  * @param pring_buffer_struct  缓冲区结构体
  * @retval BUFFER_SIZE-1       缓冲区为空
  *         1-BUFFER_SIZE-2     剩余空间
  *         0                   缓冲区已满
  */
uint16_t bufferGetRemain(ring_buffer_typedef *pring_buffer_struct) {
    return pring_buffer_struct->size - bufferGetLength(pring_buffer_struct) - 1;
}

/**
  * @brief 环形缓冲区写数据
  * @param pring_buffer_struct  缓冲区结构体
  *        data                 写入数据地址
  *        length               写入数据长度
  * @retval 0                   写失败
  */
uint16_t bufferWrite(ring_buffer_typedef *pring_buffer_struct, uint8_t *data, uint16_t length) {
    // 如果缓冲区不足 则不写入数据 返回0
    if (bufferGetRemain(pring_buffer_struct) < length) {
        return 0;
    }
    // 使用memcpy函数将数据写入缓冲区
    if (pring_buffer_struct->writeIndex + length < pring_buffer_struct->size) {
        memcpy(pring_buffer_struct->heapStartAddr + pring_buffer_struct->writeIndex, data, length);
        pring_buffer_struct->writeIndex += length;
    } else {
        uint16_t firstLength = pring_buffer_struct->size - pring_buffer_struct->writeIndex;
        memcpy(pring_buffer_struct->heapStartAddr + pring_buffer_struct->writeIndex, data, firstLength);
        memcpy(pring_buffer_struct->heapStartAddr, data + firstLength, length - firstLength);
        pring_buffer_struct->writeIndex = length - firstLength;
    }
    return length;
}

/**
  * @brief 环形缓冲区读取固定长度字节
  * @param pring_buffer_struct  缓冲区结构体
  *        data                 存放读取数据指针
  *        len                  读取数据长度
  * @retval 0                   读失败
  */
uint16_t bufferRead(ring_buffer_typedef *pring_buffer_struct, uint8_t *data, uint16_t len) {
    if (len == 0) {
        len = bufferGetLength(pring_buffer_struct);     // 0表示读取全部
    }
    if (bufferGetLength(pring_buffer_struct) < len) {   // 剩余数据长度不够则返回0
        return 0;
    }
    for (uint16_t i = 0; i < len; i ++) {
        data[i] = bufferReadIndex(pring_buffer_struct, pring_buffer_struct->readIndex + i);
    }
    bufferAddReadIndex(pring_buffer_struct, len);
    return len;
}

uint16_t bufferCRC16(ring_buffer_typedef *pring_buffer_struct, uint16_t start_index, uint16_t len) {
    uint16_t crc16 = 0;
    for (uint16_t i = 0; i < len; i ++) {
        crc16 = crc16 + bufferPeekByte(pring_buffer_struct, (start_index + i) % pring_buffer_struct->size);
    }
    return crc16;
}
