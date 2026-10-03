#ifndef __HEAP_H
#define __HEAP_H

#include "stm32f1xx_hal.h"

#define MAX_HEAP_SIZE       4096

uint8_t heapMalloc(uint16_t size, uint8_t **start_addr, uint8_t **end_addr);

#endif
