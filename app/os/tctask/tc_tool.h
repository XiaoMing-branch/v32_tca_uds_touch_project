/**
*****************************************************************************
* @brief  tc tool header
* @file   tc_tool.h
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

#ifndef TC_TOOL_H__
#define TC_TOOL_H__

/**
 * @brief  任务运行时统计模块
 * @note   仅在TC_GENERATE_RUN_TIME_STATS使能时编译
 */
#if TC_GENERATE_RUN_TIME_STATS

/**
 * @brief  任务运行时统计信息结构体
 */
typedef struct
{
    int cpuPercent;     /**< CPU利用率百分比（0~100） */
} T_TcTaskStats;

/**
 * @brief  记录每个任务运行时间
 * @note   在SysTick中断中调用，累加当前任务的运行时间片计数
 *         每5秒将所有任务的计数器清零并保存到lastTickCnt中
 */
void TcRecordTaskRunTime(void);

#endif


#endif
