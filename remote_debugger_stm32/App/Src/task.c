#include "task.h"
#include "ota.h"
#include "usb_task.h"
#include "device_manager.h"
#include "spi_task.h"

#include "ring_buffer.h"

#include "delay.h"

ring_buffer_typedef usb_rx_buffer;
ring_buffer_typedef ota_buffer;
ring_buffer_typedef spi_tx_buffer;
ring_buffer_typedef usb_tx_buffer;

uint16_t task_notification = 0;

void mainTask(void) {
    delay_init(72);
    
    otaInit();
    
    ringBufferInit(&usb_rx_buffer, 1024);
    ringBufferInit(&ota_buffer, 1024);
    ringBufferInit(&spi_tx_buffer, 256);
    ringBufferInit(&usb_tx_buffer, 1024);
    usbInitTask();
    while (1) {
        usbReceiveTask(&task_notification);
        
        deviceManagerTask(&task_notification);
        
        spiTask(&task_notification);
        
        usbTransmitTask(&task_notification);
        
        otaTask(&task_notification);
    }
}
