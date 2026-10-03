#include "sys.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////////////  
//单片机系统层方法
//包括printf
//修改日期:2026/9/24
//版本：V1.0
//********************************************************************************
//修改说明
//////////////////////////////////////////////////////////////////////////////////

#if USE_USB
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#endif

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)

#if USE_USB
extern USBD_HandleTypeDef hUsbDeviceFS;
static uint8_t printf_tx_byte;
#endif

//标准库需要的支持函数
struct __FILE {
    int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x) {
    (void)x;

    while (1) {
    }
}
//重定义fputc函数 
int fputc(int ch, FILE *f) {
    #if USE_USB
    USBD_CDC_HandleTypeDef *hcdc;

    (void)f;

    /*
     * USB 尚未枚举时直接丢弃字符。
     * 返回 ch 而不是 EOF，避免 stdout 被设置为错误状态，
     * 影响后续枚举完成后的 printf。
     */
    if ((hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) ||
        (hUsbDeviceFS.pClassData == NULL)) {
        return ch;
    }

    hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;

    while (hcdc->TxState != 0U);    // 等待上一字符发送完成

    printf_tx_byte = (uint8_t)ch;

    if (CDC_Transmit_FS(&printf_tx_byte, 1U) != USBD_OK) {
        /* 本次字符丢弃，不能在这里无限等待。 */
    }
    #endif

    return ch;
}
#endif
