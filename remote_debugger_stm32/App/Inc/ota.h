#ifndef __OTA_H
#define __OTA_H

#include "stm32f1xx_hal.h"

#define APP1_VERSION_ADDR       0x08001000
#define APP1_IOS_STATE          0x08001004
#define APP1_IOS_SIZE           0x08001008

#define APP1_PROGRAM_ADDR       0x08001400

#define APP2_VERSION_ADDR       0x0800100C
#define APP2_IOS_STATE          0x08001010
#define APP2_IOS_SIZE           0x08001014

#define APP2_PROGRAM_ADDR       0x08008C00

#define APP_PROGRAM_MAX_SIZE    0x00007400U

typedef enum {
    OTA_MSG_START = 0x01,
    OTA_MSG_DATA  = 0x02,
    OTA_MSG_END   = 0x03
} OTA_MsgType_t;

typedef enum {
    IOS_BROKEN   = 0x00,
    IOS_READY    = 0x01,
    IOS_UPDATING = 0x02,
    IOS_COPYING  = 0x03
} IOS_STATE;

void otaInit(void);
void otaTask(uint16_t *notification);

#endif
