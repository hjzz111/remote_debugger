#include "device_manager.h"
#include "delay.h"
#include "device_control.h"

void deviceManagerTask(uint16_t *notification) {
    if (*notification & 0x000F) {
        if (*notification & 0x0001) {
            deviceCommunicateOff();
            delay_ms(5);
            devicePowerOff();
            *notification &= 0xFFFE;
            *notification |= 0x8000;
        }
        if (*notification & 0x0002) {
            devicePowerOn();
            delay_ms(5);
            deviceCommunicateOn();
            *notification &= 0xFFFD;
            *notification |= 0x8000;
        }
        if (*notification & 0x0004) {
            deviceResetEnable();
            *notification &= 0xFFFB;
            *notification |= 0x8000;
        }
        if (*notification & 0x0008) {
            deviceResetDisable();
            *notification &= 0xFFF7;
            *notification |= 0x8000;
        }
    }
}
