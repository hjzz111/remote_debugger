#include "usb_control.h"
#include "delay.h"

void usbControlInit(void) {
    usbControlDisable();
    delay_ms(10);
    usbControlEnable();
}

void usbControlEnable(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}

void usbControlDisable(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
}
