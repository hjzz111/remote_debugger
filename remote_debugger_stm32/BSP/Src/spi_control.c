#include "spi_control.h"

static SPI_HandleTypeDef spi1_handler;

void spiInit(spi_init_configitem *configitem) {
    if (configitem->spi_inited == 1) {
        spiDeinit(configitem);
    }
    spi1_handler.Instance = SPI1;
    spi1_handler.Init = configitem->init;
    HAL_SPI_Init(&spi1_handler);
    configitem->spi_inited = 1;
}

void spiDeinit(spi_init_configitem *configitem) {
    
    spi1_handler.Instance = SPI1;
    HAL_SPI_DeInit(&spi1_handler);
    configitem->spi_inited = 0;
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(spiHandle->Instance==SPI1)
    {
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**SPI1 GPIO Configuration
        PA4     ------> SPI1_NSS
        PA5     ------> SPI1_SCK
        PA6     ------> SPI1_MISO
        PA7     ------> SPI1_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
          
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        if (spiHandle->Init.Mode == SPI_MODE_MASTER) {
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        }
        else {
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        }
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
    if(spiHandle->Instance==SPI1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();

        /**SPI1 GPIO Configuration
        PA4     ------> SPI1_NSS
        PA5     ------> SPI1_SCK
        PA6     ------> SPI1_MISO
        PA7     ------> SPI1_MOSI
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
    }
}

void spiChangeBytes(const uint8_t *pTxData, uint8_t *pRxData, uint16_t Size) {
    HAL_SPI_TransmitReceive(&spi1_handler, pTxData, pRxData, Size, HAL_MAX_DELAY);
}
