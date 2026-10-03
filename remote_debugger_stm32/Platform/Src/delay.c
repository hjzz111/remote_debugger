#include "delay.h"
#if SYSTEM_SUPPORT_OS                   // FreeRTOS使用
#include "cmsis_os.h"
#include "task.h"
#endif

//////////////////////////////////////////////////////////////////////////////////  
//使用SysTick的普通计数模式对延迟进行管理(支持rtos)
//包括delay_us,delay_ms
//修改日期:2026/8/15
//版本：V1.1
//********************************************************************************
//修改说明
//V1.1 增加裸机支持
//////////////////////////////////////////////////////////////////////////////////

static uint8_t  fac_us=0;                   // us延时倍乘数
#if SYSTEM_SUPPORT_OS                           // FreeRTOS使用
static uint16_t fac_ms=0;                   // ms延时倍乘数,在os下,代表每个节拍的ms数
#endif

//初始化延迟函数
//SYSTICK的时钟固定为AHB时钟，基础例程里面SYSTICK时钟频率为AHB/8
//这里为了兼容FreeRTOS，所以将SYSTICK的时钟频率改为AHB的频率！
//SYSCLK:系统时钟频率
void delay_init(uint8_t SYSCLK) {
    fac_us=SYSCLK;                          // 不论是否使用OS,fac_us都需要使用
    #if SYSTEM_SUPPORT_OS                   // FreeRTOS使用
    fac_ms=1000/configTICK_RATE_HZ;         // 代表OS可以延时的最少单位
    #endif
}

//延时nus
//nus:要延时的us数.
//nus:0~204522252(最大值即2^32/fac_us@fac_us=168)
void delay_us(uint32_t nus) {
    uint32_t ticks;
    uint32_t told,tnow,tcnt=0;
    uint32_t reload=SysTick->LOAD;              //LOAD的值
    ticks=nus*fac_us;                           //需要的节拍数 
    told=SysTick->VAL;                          //刚进入时的计数器值
    while(1) {
        tnow=SysTick->VAL;
        if(tnow!=told) {
            if(tnow<told) {
                tcnt+=told-tnow;                //这里注意一下SYSTICK是一个递减的计数器就可以了.
            }
            else {
                tcnt+=reload-tnow+told;
            }
            told=tnow;
            if(tcnt>=ticks) {
                break;                          //时间超过/等于要延迟的时间,则退出.
            }
        }  
    }
}

//延时nms
//nms:要延时的ms数
//nms:0~65535
void delay_ms(uint32_t nms) {
    #if SYSTEM_SUPPORT_OS                       // FreeRTOS使用
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED) {   //系统已经运行
        if(nms>=fac_ms) {                                       //延时的时间大于OS的最少时间周期 
            vTaskDelay(nms/fac_ms);                             //FreeRTOS延时
        }
        nms%=fac_ms;                                            //OS已经无法提供这么小的延时了,采用普通方式延时    
    }
    #endif
    delay_us((uint32_t)(nms*1000));             // 普通方式延时
}

//延时nms,不会引起任务调度，仅在RTOS中可用
//nms:要延时的ms数
#if SYSTEM_SUPPORT_OS                           // FreeRTOS使用
void delay_xms(uint32_t nms) {
    uint32_t i;
    for(i=0;i<nms;i++) {
        delay_us(1000);
    }
}
#endif


































