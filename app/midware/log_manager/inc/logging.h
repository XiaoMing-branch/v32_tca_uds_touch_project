/**
 *****************************************************************************
 * @brief   logging header file.
 * @file    logging.h
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

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "tc_printf.h"
#include "pal_func_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if CFG_SUPPORT_LOG
/** @brief  输出调试级别日志，加 "[D]" 前缀 */
#define log_debug(format, args ...)  do{tc_printf("[D]"format, ## args); }while(0)
/** @brief  输出信息级别日志，加 "[I]" 前缀 */
#define log_info(format, args ...)   do{tc_printf("[I]"format, ## args); }while(0)
/** @brief  输出警告级别日志，加 "[W]" 前缀 */
#define log_warn(format, args ...)   do{tc_printf("[W]"format, ## args); }while(0)
/** @brief  输出错误级别日志，加 "[E]" 前缀 */
#define log_err(format, args ...)    do{tc_printf("[E]"format, ## args); }while(0)
#else
/** @brief  调试日志（空宏，日志功能关闭时使用） */
#define log_debug(format, args ...)
/** @brief  信息日志（空宏，日志功能关闭时使用） */
#define log_info(format, args ...)
/** @brief  警告日志（空宏，日志功能关闭时使用） */
#define log_warn(format, args ...)
/** @brief  错误日志（空宏，日志功能关闭时使用） */
#define log_err(format, args ...)
#endif

/**
 * @brief  初始化日志模块
 * @retval 无
 */
void logging_init(void);

/**
 * @brief  反初始化日志模块
 * @retval 无
 */
void logging_deinit(void);

#ifdef __cplusplus
}
#endif
#endif
