#include "usb_task.h"
#include "ring_buffer.h"

extern ring_buffer_typedef usb_rx_buffer;
extern ring_buffer_typedef ota_buffer;

extern ring_buffer_typedef spi_tx_buffer;

/* 数据包结构 */
/* -------------------------------------------- */
/* |   包头  |命令类型|帧长度（数据长度）|数据|  CRC16  | */
/* -------------------------------------------- */
/* |0x55 0xAA|  0xXX |     0xXX 0xXX    |    |0xXX 0xXX| */
/* -------------------------------------------- */

/* usb任务，写为非阻塞式 */
uint8_t usbReceiveTask(uint16_t *notification) {
    
    /* 判断是否有足够长的数据处理 */
    if (bufferGetLength(&usb_rx_buffer) < MIN_PROCESS_LEN) {
        return 0;
    }
    
    /* 找包头 */
    if (bufferPeekByte(&usb_rx_buffer, 0) != 0x55) {
        bufferAddReadIndex(&usb_rx_buffer, 1);
        return 0;
    }
    if (bufferPeekByte(&usb_rx_buffer, 1) != 0xAA) {
        bufferAddReadIndex(&usb_rx_buffer, 1);
        return 0;
    }
    
    /* 进行CRC校验，首先判断长度够不够 */
    uint16_t len = bufferPeekByte(&usb_rx_buffer, 3);
    len = (len << 8) + bufferPeekByte(&usb_rx_buffer, 4);;
    if (bufferGetLength(&usb_rx_buffer) < len + 7) {
        return 0;
    }
    
    /* CRC校验 */
    uint16_t crc16r = bufferCRC16(&usb_rx_buffer, 5, len);
    uint16_t crc16t = bufferPeekByte(&usb_rx_buffer, len + 5);
    crc16t = (crc16t << 8) + bufferPeekByte(&usb_rx_buffer, len + 6);
    if (crc16r != crc16t) {
        bufferAddReadIndex(&usb_rx_buffer, 1);
        return 0;
    }
    
    /* 这里我们认为传输的数据是正确的 */
    uint8_t cmd_type = bufferPeekByte(&usb_rx_buffer, 2);
    uint8_t ota_data[600] = {0};
    uint8_t ota_head[4] = {0};
    uint8_t spi_data[130] = {0};
    uint8_t spi_head[2] = {0};
    switch (cmd_type) {
        case connect_confirm:
            bufferAddReadIndex(&usb_rx_buffer, 7);
            *notification |= 0x8000;
            break;
        
        case ota_start:
            /* ota缓冲区结构 */
            /* ----------------------------------- */
            /* |ota数据类型| 数据长度 |帧编号|数据| */
            /* ----------------------------------- */
            /* |    0xXX   |0xXX 0xXX| 0xXX |    | */
            /* ----------------------------------- */
            bufferRead(&usb_rx_buffer, ota_data, len + 7);
            ota_head[0] = ota_data[5];
            ota_head[1] = (uint8_t)((len - 2) >> 8);
            ota_head[2] = (uint8_t)(len - 2);
            ota_head[3] = ota_data[6];
            bufferWrite(&ota_buffer, ota_head, 4);
            bufferWrite(&ota_buffer, ota_data + 7, len - 2);
            break;
        
        case device_powerctl:
            if (bufferPeekByte(&usb_rx_buffer, 5) == 0x00) {
                *notification = *notification | 0x0001;
                bufferAddReadIndex(&usb_rx_buffer, 8);
            }
            else if (bufferPeekByte(&usb_rx_buffer, 5) == 0x01) {
                *notification = *notification | 0x0002;
                bufferAddReadIndex(&usb_rx_buffer, 8);
            }
            else {
                bufferAddReadIndex(&usb_rx_buffer, 1);
                return 0;
            }
            break;
            
        case device_rstctl:
            if (bufferPeekByte(&usb_rx_buffer, 5) == 0x00) {
                *notification = *notification | 0x0004;
                bufferAddReadIndex(&usb_rx_buffer, 8);
            }
            else if (bufferPeekByte(&usb_rx_buffer, 5) == 0x01) {
                *notification = *notification | 0x0008;
                bufferAddReadIndex(&usb_rx_buffer, 8);
            }
            else {
                bufferAddReadIndex(&usb_rx_buffer, 1);
                return 0;
            }
            break;
            
        case spi_control:
            /* spi缓冲区结构 */
            /* ----------------------------------- */
            /* |spi数据类型|数据长度|数据| */
            /* ----------------------------------- */
            /* |    0xXX   |  0xXX |    | */
            /* ----------------------------------- */
            bufferRead(&usb_rx_buffer, spi_data, len + 7);
            spi_head[0] = ota_data[5];
            spi_head[1] = (uint8_t)(len - 1);   /* spi一次最多收128位数据 */
            bufferWrite(&spi_tx_buffer, spi_head, 2);
            bufferWrite(&spi_tx_buffer, spi_data + 7, len - 2);
            break;
        
        default:
            bufferAddReadIndex(&usb_rx_buffer, 1);
            return 0;
    }
    
    return 1;
}

void usbTransmitTask(uint16_t *notification) {
    if (*notification & 0x8000) {
        uint8_t ack = 0x00;
        usbControlTx(&ack, 1);
        *notification &= 0x7FFF;
    }
    if (*notification & 0x4000) {
        uint8_t ack = 0xFF;
        usbControlTx(&ack, 1);
        *notification &= 0xBFFF;
    }
}
