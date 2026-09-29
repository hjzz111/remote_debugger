#include "device_manager.h"
#include "delay.h"

void devicePowerOn(void);
void devicePowerOff(void);
void deviceCommunicateOn(void);
void deviceCommunicateOff(void);
void deviceResetEnable(void);
void deviceResetDisable(void);

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

void devicePowerOn(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
}

void devicePowerOff(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
}

void deviceCommunicateOn(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}

void deviceCommunicateOff(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
}

void deviceResetEnable(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

void deviceResetDisable(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
}
