#ifndef __DELAY_H
#define __DELAY_H

#include "main.h"
//////////////////////////////////////////////////////////////////////////////////  
//使用SysTick的普通计数模式对延迟进行管理(支持rtos)
//包括delay_us,delay_ms
//修改日期:2026/8/15
//版本：V1.1
//********************************************************************************
//修改说明
//V1.1 增加裸机支持
//////////////////////////////////////////////////////////////////////////////////

#define SYSTEM_SUPPORT_OS   0                   // 系统是否使用RTOS

void delay_init(uint8_t SYSCLK);
void delay_us(uint32_t nus);
void delay_ms(uint32_t nms);
#if SYSTEM_SUPPORT_OS                           // FreeRTOS使用
void delay_xms(uint32_t nms);
#endif
#endif





























