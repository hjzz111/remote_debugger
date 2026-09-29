#include "usb_app.h"
#include "delay.h"

void AppUSBInit(void) {
    AppUSBDisable();
    delay_ms(10);
    AppUSBEnable();
}

void AppUSBEnable(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}

void AppUSBDisable(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
}
