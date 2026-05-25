/**
*****************************************************************************
* @brief  tc debug header
* @file   tc_debug.h
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

#ifndef TC_DEBUG_H__
#define TC_DEBUG_H__

/**
 * @brief  调试打印模块
 * @note   仅在TC_DEBUG_PRINT使能时编译
 */
#if TC_DEBUG_PRINT

/**
 * @brief  打印任务结构体内容
 * @param  task - 目标任务指针
 * @param  desp - 描述字符串（可NULL）
 */
void TcPrintTask(T_TcTask * task,const char *desp);

/**
 * @brief  打印消息结构体内容
 * @param  msg  - 目标消息指针
 * @param  desp - 描述字符串（可NULL）
 */
void TcPrintMsg(T_TcMsg * msg,const char *desp);

/**
 * @brief  打印链表头结构体内容
 * @param  list - 目标链表头指针
 * @param  desp - 描述字符串（可NULL）
 */
void TcPrintListHead(struct T_TcListHead * list,const char *desp);

/**
 * @brief  打印内存池结构体内容
 * @param  mem  - 目标内存池指针
 * @param  desp - 描述字符串（可NULL）
 */
void TcPrintMem(T_TcMem * mem,const char *desp);

/**
 * @brief  打印定时器结构体内容
 * @param  timer - 目标定时器指针
 * @param  desp  - 描述字符串（可NULL）
 */
void TcPrintTimer(T_TcTimer * timer,const char *desp);

#endif

/**
 * @brief  CPU利用率打印模块
 * @note   仅在TC_GENERATE_RUN_TIME_STATS使能时编译
 */
#if TC_GENERATE_RUN_TIME_STATS

/**
 * @brief  打印所有任务CPU利用率
 * @note   遍历所有任务，调用TcGetTaskRunStats获取CPU占用率并打印
 */
void TcPrintCpuUsage(void);
#endif

#endif
