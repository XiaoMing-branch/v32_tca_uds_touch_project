/**
 *****************************************************************************
 * @brief   logging header file.
 *
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
/**
 * @brief  输出调试级别日志
 * @param  format - 格式化字符串
 * @param  args ... - 可变参数
 * @note   日志前缀 [D]，仅在 CFG_SUPPORT_LOG 使能时有效
 */
#define log_debug(format, args ...)  do{tc_printf("[D]"format, ## args); }while(0)
/**
 * @brief  输出信息级别日志
 * @param  format - 格式化字符串
 * @param  args ... - 可变参数
 * @note   日志前缀 [I]，仅在 CFG_SUPPORT_LOG 使能时有效
 */
#define log_info(format, args ...)   do{tc_printf("[I]"format, ## args); }while(0)
/**
 * @brief  输出警告级别日志
 * @param  format - 格式化字符串
 * @param  args ... - 可变参数
 * @note   日志前缀 [W]，仅在 CFG_SUPPORT_LOG 使能时有效
 */
#define log_warn(format, args ...)   do{tc_printf("[W]"format, ## args); }while(0)
/**
 * @brief  输出错误级别日志
 * @param  format - 格式化字符串
 * @param  args ... - 可变参数
 * @note   日志前缀 [E]，仅在 CFG_SUPPORT_LOG 使能时有效
 */
#define log_err(format, args ...)    do{tc_printf("[E]"format, ## args); }while(0)
#else
#define log_debug(format, args ...)
#define log_info(format, args ...)
#define log_warn(format, args ...)
#define log_err(format, args ...)
#endif

/**
 * @brief  日志模块初始化
 * @note   初始化底层串口日志并注册输出回调
 * @retval 无
 */
void logging_init(void);

/**
 * @brief  日志模块去初始化
 * @note   反初始化底层串口日志并清除输出回调
 * @retval 无
 */
void logging_deinit(void);

#ifdef __cplusplus
}
#endif
#endif
