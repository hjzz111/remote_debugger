#ifndef __SPI_CONTROL_H
#define __SPI_CONTROL_H

#include "stm32f1xx_hal.h"
#include "proconfig.h"

typedef struct {
    uint8_t spi_inited;
    
    SPI_InitTypeDef init;
} spi_init_configitem;

BaseType_t spiInit(spi_init_configitem *configitem);
void spiDeinit(spi_init_configitem *configitem);
BaseType_t spiChangeBytes(const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);

#endif
