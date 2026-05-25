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
 * @brief  SysTick中断计数器
 * @note   每次SysTick_Handler执行时递增
 *         用于TcGetTimeUS/TcGetTimeMS的时间计算
 */
volatile uint32_t g_TcSystickIntCnt = 0;

/**< SysTick定时器硬件初始化函数 */
static void TcTimerTickInit(void);

/**< SysTick中断回调管理器实例 */
systickCallback_t systickCallback;

/**
 * @brief  硬件相关初始化
 * @note   配置SysTick定时器产生SYSTICK_FREQ_HZ频率的中断
 *         清空回调结构体，注册系统tick处理函数TcTimerTickHandler
 *         TcTimerTickHandler负责驱动系统时钟TcSystick递增
 */
void TcPortInit(void)
{
    /*初始化tick定时器*/
    TcTimerTickInit();

    memset(&systickCallback, 0x0, sizeof(systickCallback));

    (void)TcSystickCallbackRegister(TcTimerTickHandler, SYSTICK_COUNTS);
}

/**
 * @brief  注册SysTick中断回调函数
 * @param  callback - 回调函数指针
 * @param  nPeriod  - 回调执行周期（以SysTick中断次数为单位）
 *                    例如nPeriod=10表示每10次SysTick中断执行1次回调
 * @note   回调函数存储在systickCallback结构体中
 *         最多可注册MAX_SYSTICK_ISR_CALLBACK_NUM个回调
 * @retval >0 - 注册成功，返回当前回调总数
 * @retval -1 - 注册失败（已达上限）
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
 * @brief  SysTick定时器初始化
 * @note   调用SysTick_Config配置SysTick中断
 *         中断频率 = SystemCoreClock / SYSTICK_FREQ_HZ
 */
static void TcTimerTickInit(void)
{
    (void)SysTick_Config(SystemCoreClock / SYSTICK_FREQ_HZ);
}

/**
 * @brief  SysTick中断处理函数
 * @note   每1ms执行一次（由SYSTICK_FREQ_HZ=1000决定）
 *         递增g_TcSystickIntCnt计数器
 *         遍历所有已注册的回调函数，按各自的周期分频调用
 *         当TC_SAVE_RAM_MODE=0时，此函数被放置在RAMCODE段中运行
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
        if(systickCallback.nPastTime[i] >= systickCallback.nPeriod[i])
        {
            systickCallback.nPastTime[i] = 0;
            (*systickCallback.callback[i])();
        }
    }
}

/**
 * @brief  使能或禁能SysTick定时器
 * @param  bEn - 0：禁能SysTick；非0：使能SysTick
 * @note   直接操作SysTick->CTRL寄存器的ENABLE位
 */
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
