#ifndef __USB_TASK_H
#define __USB_TASK_H

#include "stdint.h"
#include "usb_control.h"

typedef enum {
    connect_confirm = 0x00,
    ota_start       = 0x01,
    device_powerctl = 0x02,
    device_rstctl   = 0x03,
    spi_control     = 0x04
} usb_cmdtype;

#define MIN_PROCESS_LEN     7

#define usbInitTask()       usbControlInit()

uint8_t usbReceiveTask(uint16_t *notification);

void usbTransmitTask(uint16_t *notification);

#endif
