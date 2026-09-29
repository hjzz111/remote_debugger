#ifndef __MY_ASSERT_H
#define __MY_ASSERT_H

#include "stm32f1xx_hal.h"

//////////////////////////////////////////////////////////////////////////////////  
//支持单片机的断言函数
//修改日期:2026/9/24
//版本：V1.0
//********************************************************************************
//修改说明
//////////////////////////////////////////////////////////////////////////////////

#define MY_ASSERT(expr) ((expr) ? (void)0 : my_assert_failed(__FILE__, __LINE__, #expr))

void my_assert_failed(const char *file, uint32_t line, const char *expr);

#endif
