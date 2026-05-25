/**
  ******************************************************************************
  * @brief  Delay source file
  *
  * @file   delay.c
  * @author AE/FAE team
  * @date
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
  *
  ******************************************************************************
  */
  
  
  
#include "tcae10_ll_delay.h"
#include "system_tcae10.h"


extern uint32_t SystemCoreClock;


/**
 * @brief   配置SysTick时钟源
 * @param   source - 时钟源选择：SysTick_CLKSource_HCLK（内核时钟）或SysTick_CLKSource_STCLK（外部参考时钟）
 * @retval  None
 */
void ll_systick_clkconfig(uint32_t source)
{
    if (source == SysTick_CLKSource_HCLK)
    {
        SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;    /* 选择HCLK作为SysTick时钟源 */
    }
    else
    {
        SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;   /* 选择STCLK作为SysTick时钟源 */
    }
}

/**
 * @brief   毫秒级延时函数（阻塞式，基于SysTick）
 * @param   u32Cnt - 延时毫秒数
 * @note    使用SysTick定时器实现精确毫秒延时，系统时钟为SystemCoreClock
 * @retval  None
 */
void delay1ms(uint32_t u32Cnt)
{
    uint32_t u32end;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL  = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while(u32Cnt-- > 0)
    {
        SysTick->VAL  = 0;
        u32end = 0x1000000 - SystemCoreClock/1000;
        while(SysTick->VAL > u32end)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}

/**
 * @brief   32KHz时钟源下的毫秒级延时函数（阻塞式）
 * @param   u32Cnt - 延时毫秒数
 * @note    适用于32KHz低速时钟源的延时场景
 * @retval  None
 */
void delay1ms32K(uint32_t u32Cnt)
{
    int32_t temp;
    SysTick->LOAD=(int32_t)u32Cnt*32;      /* 设置SysTick重装载值 */
    SysTick->VAL =0x00;                    /* 清空当前计数值 */
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; /* 使能SysTick定时器 */
    {
        temp=SysTick->CTRL;                /* 读取控制寄存器状态 */
    }
    while(temp&0x01&&!(temp&(1<<16)));      /* 等待COUNTFLAG置位 */
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; /* 关闭SysTick */
    SysTick->VAL =0X00;                     /* 清空计数值 */

}


/**
 * @brief   100微秒级延时函数（阻塞式，基于SysTick）
 * @param   u32Cnt - 延时100微秒的个数
 * @note    每个计数约100us，以SystemCoreClock为基准计算
 * @retval  None
 */
void delay100us(uint32_t u32Cnt)
{
    uint32_t u32end;

    SysTick->LOAD = 0xFFFFFF;                                       /* 设置最大重装载值 */
    SysTick->VAL  = 0;                                              /* 清空计数值 */
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;  /* 使能SysTick，选择HCLK */

    while(u32Cnt-- > 0)
    {
        SysTick->VAL = 0;                                           /* 复位计数值 */

        u32end = 0x1000000 - SystemCoreClock/10000;                 /* 计算100us对应的阈值 */
        while(SysTick->VAL > u32end)                                 /* 等待计数值递减到阈值以下 */
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));   /* 关闭SysTick */
}

/**
 * @brief   微秒级延时函数（阻塞式，基于SysTick）
 * @param   u32Cnt - 延时微秒数
 * @note    使用SysTick定时器实现精确微秒延时，系统时钟为SystemCoreClock
 * @retval  None
 */
void delay1us(uint32_t u32Cnt)
{
    uint32_t u32end;

    SysTick->LOAD = 0xFFFFFF;                                       /* 设置最大重装载值 */
    SysTick->VAL  = 0;                                              /* 清空计数值 */
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;  /* 使能SysTick，选择HCLK */

    while(u32Cnt-- > 0)
    {
        SysTick->VAL = 0;                                           /* 复位计数值 */

        u32end = 0x1000000 - SystemCoreClock/1000000;               /* 计算1us对应的阈值 */
        while(SysTick->VAL > u32end)                                 /* 等待计数值递减到阈值以下 */
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));   /* 关闭SysTick */
}
