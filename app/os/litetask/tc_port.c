/**
*****************************************************************************
* @brief  tc port source
* @file   tc_port.c
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
#include "tc_port.h"

/*任务系统时钟*/
volatile uint32_t TcSystick = TC_SYSTICK_INIT_VALUE;                    //1000Hz心跳，预计在10秒后发生溢出，可用于排查由于systick溢出导致的逻辑bug

volatile uint32_t g_TcSystickIntCnt = 0;     // SysTick 中断计数器

/*tick定时器初始化*/
static void TcTimerTickInit(void);
/*每Tick调用一次，驱动定时器运行，需要放到定时中断中运行*/
static void TcTimerTickHandler(void);

/*systick中断回调接口*/
systickCallback_t systickCallback;

/*硬件相关初始化*/
void TcPortInit(void)
{
    /*初始化tick定时器*/
    TcTimerTickInit();

    memset(&systickCallback, 0x0, sizeof(systickCallback));

    (void)TcSystickCallbackRegister(TcTimerTickHandler, SYSTICK_COUNTS);
}

/*注册Systick中断回调函数,1表示成功，-1表示失败
  nPeriod - 回调函数调用的周期，单位Systick中断周期
  */
int TcSystickCallbackRegister(void (*callback)(void), uint8_t nPeriod)
{
    if (systickCallback.num >= sizeof(systickCallback.callback) / sizeof(systickCallback.callback[0]))
    {
        return -1;
    }
    systickCallback.nPeriod[systickCallback.num] = nPeriod;
    systickCallback.nPastTime[systickCallback.num] = 0;
    systickCallback.callback[systickCallback.num] = callback;
    systickCallback.num++;
    return (int)systickCallback.num;
}

/*tick定时器初始化*/
static void TcTimerTickInit(void)
{
    (void)SysTick_Config(SystemCoreClock / SYSTICK_FREQ_HZ);
}

/*每Tick调用一次，驱动定时器运行，需要放到定时中断中运行*/
static void TcTimerTickHandler(void)
{
    TcSystick++;
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

//1ms period
#if !(TC_SAVE_RAM_MODE)
__attribute__((section("RAMCODE")))
#endif
void SysTick_Handler(void)                         //systick的中断处理函数
{
    uint8_t i;

    g_TcSystickIntCnt++;
    for (i = 0; i < systickCallback.num; i++)
    {
        systickCallback.nPastTime[i]++;
        if (systickCallback.nPastTime[i] >= systickCallback.nPeriod[i])
        {
            systickCallback.nPastTime[i] = 0;
            (*systickCallback.callback[i])();
        }
    }
}

void SysTick_Switch(boolean_t bEn)
{
    if (bEn != 0)
    {
        SysTick->CTRL  |= SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
    }
    else
    {
        SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;                        /* disable SysTick IRQ and SysTick Timer */
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
