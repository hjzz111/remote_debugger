#include "spi_task.h"
#include "ring_buffer.h"
#include "spi_control.h"

extern ring_buffer_typedef spi_tx_buffer;

spi_init_configitem spi_configitem;

void spiTask(void) {
    /* 确保接收到足够多数据后再处理 */
    if (bufferGetLength(&spi_tx_buffer) < bufferPeekByte(&spi_tx_buffer, 1) + 2) {
        return;
    }
    
    uint8_t spi_data[130] = {0};
    uint8_t spi_rx_data[128] = {0};
    if (bufferRead(&spi_tx_buffer, spi_data, bufferPeekByte(&spi_tx_buffer, 1) + 2)) {
        switch (spi_data[0]) {
            /* 配置spi模式 */
            case spi_init:
                spi_configitem.init.Direction = SPI_DIRECTION_2LINES;
                spi_configitem.init.TIMode = SPI_TIMODE_DISABLE;
                spi_configitem.init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
            
                if (spi_data[2] == spi_master) {
                    spi_configitem.init.Mode = SPI_MODE_MASTER;
                    spi_configitem.init.NSS = SPI_NSS_HARD_OUTPUT;
                }
                else {
                    spi_configitem.init.Mode = SPI_MODE_SLAVE;
                    spi_configitem.init.NSS = SPI_NSS_HARD_INPUT;
                }
                
                if (spi_data[3] == spi_data_8b) {
                    spi_configitem.init.DataSize = SPI_DATASIZE_8BIT;
                }
                else {
                    spi_configitem.init.DataSize = SPI_DATASIZE_16BIT;
                }
                
                if (spi_data[4] == spi_msb_first) {
                    spi_configitem.init.FirstBit = SPI_FIRSTBIT_MSB;
                }
                else {
                    spi_configitem.init.FirstBit = SPI_FIRSTBIT_LSB;
                }
                
                switch (spi_data[5]) {
                    case spi_prescaler2:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
                        break;
                    case spi_prescaler4:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
                        break;
                    case spi_prescaler8:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
                        break;
                    case spi_prescaler16:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
                        break;
                    case spi_prescaler32:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
                        break;
                    case spi_prescaler64:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
                        break;
                    case spi_prescaler128:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
                        break;
                    case spi_prescaler256:
                        spi_configitem.init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
                        break;
                }
                
                if (spi_data[6] == spi_clock_low) {
                    spi_configitem.init.CLKPolarity = SPI_POLARITY_LOW;
                }
                else {
                    spi_configitem.init.CLKPolarity = SPI_POLARITY_HIGH;
                }
                
                if (spi_data[7] == spi_clock_1edge) {
                    spi_configitem.init.CLKPhase = SPI_PHASE_1EDGE;
                }
                else {
                    spi_configitem.init.CLKPhase = SPI_PHASE_2EDGE;
                }
                
                spiInit(&spi_configitem);
                break;
            
            /* 发送spi数据，暂时不考虑接收 */
            case spi_tx:
                spiChangeBytes(spi_data+2, spi_rx_data, spi_data[1]);
                break;
            
            default:
                break;
        }
    }
}
