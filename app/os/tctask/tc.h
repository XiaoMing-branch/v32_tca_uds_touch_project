/**
*****************************************************************************
* @brief  tc header
* @file   tc.h
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

#ifndef TC_H__
#define TC_H__

#include <stdio.h>
#include <string.h>

#include "tc_conf.h"
#include "tc_type.h"
#include "tc_list.h"

#include "tc_mem.h"
#include "tc_tool.h"
#include "tc_task.h"
#include "tc_timer.h"
#include "tc_queue.h"
#include "tc_tmr.h"
#include "tc_debug.h"

#include "tc_port.h"

/**
 * @brief  TCTask软件版本号
 */
#define TC_TASK_VERSION		"1.1"

/**
 * @brief  TCTask系统初始化
 * @note   按以下顺序初始化各模块：
 *         1. TcMemInit() - 内存管理模块
 *         2. TcTaskInit() - 任务管理模块
 *         3. TcTimerInit() - 定时器管理模块
 *         4. TcQueueInit() - 队列管理模块
 *         5. TcPortInit() - 硬件相关初始化（SysTick等）
 *         任一模块初始化失败将中止并返回错误码
 * @retval 1  - 初始化成功
 * @retval -1 - 初始化失败（任务或队列模块初始化失败）
 */
int TcInit(void);

#endif
