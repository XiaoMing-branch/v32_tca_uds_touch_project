/**
*****************************************************************************
* @brief  tc tool source
* @file   tc_tool.c
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

/**
 * @brief  任务运行时统计模块
 * @note   仅在TC_GENERATE_RUN_TIME_STATS使能时编译
 *         每5秒统计一次各个任务的CPU使用率
 */
#if TC_GENERATE_RUN_TIME_STATS

static uint32_t TcRecordTaskFreshTime = 0;  /**< 上次刷新时间戳，每5秒刷新一次 */

/**
 * @brief  记录每个任务运行时间
 * @note   在SysTick中断中调用，每次中断时累加当前任务的tickCntPer5S计数
 *         当系统运行时间达到5秒时，调用TcAllTaskClrTickCnt()清零计数器
 *         并将累计值保存到lastTickCnt中供后续计算CPU利用率
 */
void TcRecordTaskRunTime(void)
{
    T_TcTask * taskLock = NULL;

    /*增加当前任务的时间片*/
    if((taskLock = currentTask) != NULL)
    {
        taskLock->tickCntPer5S++;
    }

    /*每5秒清空一次任务时间片计数器*/
    if(TcSystick - TcRecordTaskFreshTime >= 5000*TC_SYSTICK_HZ/1000)
    {
        TcRecordTaskFreshTime = TcSystick;
        /*刷新所有任务的计数器*/
        TcAllTaskClrTickCnt();
    }
}

#endif


