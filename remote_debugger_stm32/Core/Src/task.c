#include "task.h"
#include "delay.h"
#include "usb_app.h"
#include "my_assert.h"
#include "ring_buffer.h"

#include "usb_task.h"
#include "device_manager.h"
#include "ota.h"

ring_buffer_typedef usb_ring_buffer;
ring_buffer_typedef ota_buffer;

uint16_t task_notification = 0;

void mainTask(void) {
    delay_init(72);
    
    otaInit();
    
    ringBufferInit(&usb_ring_buffer, 1024);
    ringBufferInit(&ota_buffer, 1024);
    AppUSBInit();
    while (1) {
        usbReceiveTask(&task_notification);
        
        deviceManagerTask(&task_notification);
        
        usbTransmitTask(&task_notification);
        
        otaTask(&task_notification);
    }
}
