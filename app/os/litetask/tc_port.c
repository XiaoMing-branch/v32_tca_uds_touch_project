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

/**
 * @brief  系统滴答时钟
 * @note   1000Hz心跳，每次TcTimerTickHandler递增1
 *         初始值设为0xFFFFD8EF使其约10秒后溢出
 *         用于排查systick溢出导致的逻辑bug
 */
volatile uint32_t TcSystick = TC_SYSTICK_INIT_VALUE;

/**
 * @brief  SysTick中断计数器
 * @note   每次SysTick_Handler执行时递增
 */
volatile uint32_t g_TcSystickIntCnt = 0;

/**< SysTick硬件初始化 */
static void TcTimerTickInit(void);

/**< 系统tick处理函数，每tick调用一次 */
static void TcTimerTickHandler(void);

/**< SysTick回调管理器实例 */
systickCallback_t systickCallback;

/**
 * @brief  硬件端口初始化
 * @note   初始化SysTick定时器并注册TcTimerTickHandler回调
 */
void TcPortInit(void)
{
    /*初始化tick定时器*/
    TcTimerTickInit();

    memset(&systickCallback, 0x0, sizeof(systickCallback));

    (void)TcSystickCallbackRegister(TcTimerTickHandler, SYSTICK_COUNTS);
}

/**
 * @brief  注册SysTick中断回调
 * @param  callback - 回调函数指针
 * @param  nPeriod  - 执行周期（SysTick中断次数）
 * @retval >0 - 成功，-1 - 失败（已达上限）
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

/**
 * @brief  SysTick定时器硬件初始化
 * @note   配置SysTick以SYSTICK_FREQ_HZ频率产生中断
 */
static void TcTimerTickInit(void)
{
    (void)SysTick_Config(SystemCoreClock / SYSTICK_FREQ_HZ);
}

/**
 * @brief  系统tick处理函数
 * @note   每tick调用一次，驱动TcSystick递增
 *         需要在定时中断中运行
 */
static void TcTimerTickHandler(void)
{
    TcSystick++;
}

/**
 * @brief  忙等待延迟
 * @param  dly - 延迟的tick数
 * @note   通过轮询TcSystick实现精确延时
 *         注意：此函数会阻塞CPU直到延迟时间到
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

/**
 * @brief  SysTick中断处理函数
 * @note   1ms周期（由SYSTICK_FREQ_HZ=1000决定）
 *         递增g_TcSystickIntCnt，遍历执行所有注册的回调
 *         当TC_SAVE_RAM_MODE=0时放入RAMCODE段运行
 */
//1ms period
#if !(TC_SAVE_RAM_MODE)
__attribute__((section("RAMCODE")))
#endif
void SysTick_Handler(void)
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

/**
 * @brief  SysTick使能/禁能
 * @param  bEn - 0：禁能；非0：使能
 */
void SysTick_Switch(boolean_t bEn)
{
    if (bEn != 0)
    {
        SysTick->CTRL  |= SysTick_CTRL_ENABLE_Msk;
    }
    else
    {
        SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;
    }
}

/**
 * @brief  获取当前时间（微秒）
 * @note   通过g_TcSystickIntCnt和SysTick->VAL计算高精度时间
 *         usTime = 中断次数*1000 + (重装载值 - 当前VAL) / (时钟频率/1000000)
 * @retval 当前时间，单位微秒（uint64_t精度）
 */
uint64_t TcGetTimeUS(void)
{
    uint64_t usTime = ((uint64_t)g_TcSystickIntCnt * 1000);
    usTime += ((((uint64_t)SystemCoreClock / SYSTICK_FREQ_HZ) - SysTick->VAL) / ((uint64_t)SystemCoreClock / 1000000));

    return usTime;
}

/**
 * @brief  获取当前时间（毫秒）
 * @retval 当前时间，单位毫秒（即g_TcSystickIntCnt值）
 */
uint32_t TcGetTimeMS(void)
{
	return g_TcSystickIntCnt;
}
