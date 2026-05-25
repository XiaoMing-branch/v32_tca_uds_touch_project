/**
 *****************************************************************************
 * @brief   pal systick source file.
 *
 * @file    pal_systick.c
 * @author  AE/FAE team
 * @date    2024.01.01
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */

#include "pal_func_def.h"
#include "pal_systick.h"

static volatile uint32_t systick_count = 0;

extern void os_task_update(void);

/**
 * @brief  OS任务更新回调(弱符号，可重写)
 * @param  无
 * @retval 无
 */
__attribute__((weak)) void os_task_update(void)
{
}

/**
 * @brief  SysTick中断服务函数
 * @param  无
 * @note   调用OS任务更新，递增系统滴答计数器
 * @retval 无
 */
void SysTick_Handler(void)
{
    os_task_update();

    /* systick increase */
    systick_count++;
}

/**
 * @brief  获取当前系统滴答计数值
 * @param  无
 * @retval 当前滴答计数值
 */
uint32_t systick_count_get(void)
{
    return (systick_count);
}

/**
 * @brief  计算两次滴答之间的时间差(支持溢出)
 * @param  start_tick - 起始滴答值
 * @retval 从start_tick到当前的时间差
 */
uint32_t systick_diff(uint32_t start_tick)
{
    uint32_t tick_diff = 0;

    if (systick_count_get() >= start_tick)
    {
        tick_diff = systick_count_get() - start_tick;
    }
    else
    {
        // tick overflow
        tick_diff = 0xFFFFFFFF - start_tick + systick_count_get();
    }

    return (tick_diff);
}

/**
 * @brief  毫秒级延时
 * @param  ms - 延时毫秒数
 * @note   基于SysTick计数的阻塞延时
 * @retval 无
 */
void delay_ms(uint32_t ms)
{
    interrupt_disable();
    uint32_t curret_systick = systick_count_get();
    interrupt_enable();

    while (ms > systick_diff(curret_systick));
}

/**
 * @brief  微秒级延时
 * @param  us - 延时微秒数
 * @note   基于SysTick计数器寄存器的精准阻塞延时
 * @retval 无
 */
void delay_us(uint32_t us)
{
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    uint32_t reload;

    reload = SysTick->LOAD;

    ticks = us * (DEFAULT_SYSTEM_CLOCK / 1000000);

    tcnt = 0;
    told = SysTick->VAL;

    while (1)
    {
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }

            told = tnow;

            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}
