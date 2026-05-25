/**
*****************************************************************************
* @brief  tc tmr source
* @file   tc_tmr.c
* @author AE/FAE team
* @date   28/JUL/2023
*****************************************************************************
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <b>&copy; Copyright (c) 2023 Tinychip Microelectronics Co.,Ltd.</b>
*
*****************************************************************************
*/

#include "tc.h"
#include "system_tcae10.h"

/*任务系统时钟*/
volatile uint32_t TcSystick = TC_SYSTICK_INIT_VALUE;                    //1000Hz心跳，预计在10秒后发生溢出，可用于排查由于systick溢出导致的逻辑bug

/*每Tick调用一次，驱动定时器运行，需要放到定时中断中运行*/
void TcTimerTickHandler(void)
{
    TcSystick++;

    /*统计任务运行状态*/
#if TC_GENERATE_RUN_TIME_STATS

    TcRecordTaskRunTime();

#endif
}

/*延迟函数，忙等待延迟，单位为tick*/
void TcTimeDly(uint32_t dly)
{
    uint32_t beginTick;

    beginTick = TcSystick;
    while (TcSystick - beginTick < dly)
    {
        continue;
    }
}

/*获取当前时间，单位为us*/
uint64_t TcGetTimeUS(void)
{
    uint64_t usTime = ((uint64_t)g_TcSystickIntCnt * 1000);
    usTime += ((((uint64_t)SystemCoreClock / SYSTICK_FREQ_HZ) - SysTick->VAL) / ((uint64_t)SystemCoreClock / 1000000));

    return usTime;
}

/*获取当前时间，单位为ms*/
uint32_t TcGetTimeMS(void)
{
	return g_TcSystickIntCnt;
}

