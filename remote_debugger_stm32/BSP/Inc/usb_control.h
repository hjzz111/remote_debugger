#ifndef __USB_CONTROL_H
#define __USB_CONTROL_H

#include "stdint.h"
#include "usbd_cdc_if.h"

#define usbControlTx(Buf, Len)    CDC_Transmit_FS(Buf, Len)

void usbControlInit(void);
void usbControlEnable(void);
void usbControlDisable(void);

#endif
