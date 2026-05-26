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

/** @brief 系统心跳计数值（1000Hz），约10秒溢出，可排查systick溢出导致的逻辑bug */
volatile uint32_t TcSystick = TC_SYSTICK_INIT_VALUE;

/** @brief 系统定时器心跳处理（每Tick调用一次，需放入定时中断）
 *  @note 驱动TcSystick递增，供任务调度和时间管理使用
 *  @retval 无
 */
void TcTimerTickHandler(void)
{
    TcSystick++;

    /*统计任务运行状态*/
#if TC_GENERATE_RUN_TIME_STATS

    TcRecordTaskRunTime();

#endif
}

/** @brief 忙等待延迟（单位为tick）
 *  @param[in] dly 需要延迟的tick数（1tick=1ms @ 1000Hz系统时钟）
 *  @retval 无
 */
void TcTimeDly(uint32_t dly)
{
    uint32_t beginTick;

    beginTick = TcSystick;
    while (TcSystick - beginTick < dly)
    {
        continue;
    }
}

/** @brief 获取当前时间（微秒）
 *  @detail 基于TcSystick和SysTick当前计数值计算出高精度微秒时间，精度取决于内核时钟频率
 *  @retval 当前系统运行时间，单位为微秒（us）
 */
uint64_t TcGetTimeUS(void)
{
    uint64_t usTime = ((uint64_t)g_TcSystickIntCnt * 1000);
    usTime += ((((uint64_t)SystemCoreClock / SYSTICK_FREQ_HZ) - SysTick->VAL) / ((uint64_t)SystemCoreClock / 1000000));

    return usTime;
}

/** @brief 获取当前时间（毫秒）
 *  @retval 当前系统运行时间，单位为毫秒（ms）
 */
uint32_t TcGetTimeMS(void)
{
	return g_TcSystickIntCnt;
}

