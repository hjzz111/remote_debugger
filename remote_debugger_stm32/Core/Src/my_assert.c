#include "my_assert.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////////////  
//支持单片机的断言函数
//修改日期:2026/9/24
//版本：V1.0
//********************************************************************************
//修改说明
//////////////////////////////////////////////////////////////////////////////////

/**
  * @brief 断言函数具体实现
  * @retval None
  */
void my_assert_failed(const char *file, uint32_t line, const char *expr) {
//    __disable_irq();

    printf("ASSERT FAILED!\r\n");
    printf("File: %s\r\n", file);
    printf("Line: %lu\r\n", line);
    printf("Expr: %s\r\n", expr);

    while (1) {
        // 可以翻转LED、等待调试器等
    }
}
