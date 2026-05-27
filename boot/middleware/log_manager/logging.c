/**
 *****************************************************************************
 * @brief   logging source file.
 *
 * @file    logging.c
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

#include "pal_log.h"
#include "logging.h"

/**
 * @brief  日志模块初始化
 * @param[in]  无
 * @note   初始化底层串口日志（波特率115200），注册日志输出回调函数
 * @retval 无
 */
void logging_init(void)
{
#if 1 == CFG_SUPPORT_LOG                                            /* 日志功能全局使能开关，由CFG_SUPPORT_LOG控制 */
    pal_log_init(115200);                                           /**< 初始化底层UART日志模块，波特率设为115200bps */

    log_out_func(pal_log_print);                                    /**< 注册日志输出回调函数，将日志定向到UART输出 */
#endif                                                              /* CFG_SUPPORT_LOG条件编译结束 */
}

/**
 * @brief  日志模块去初始化
 * @param[in]  无
 * @note   反初始化底层串口日志，清除日志输出回调函数
 * @retval 无
 */
void logging_deinit(void)
{
#if 1 == CFG_SUPPORT_LOG                                            /* 日志功能全局使能开关，与logging_init保持一致 */
    pal_log_deinit();                                               /**< 反初始化底层UART日志模块，释放占用的硬件资源 */

    log_out_func(NULL);                                             /**< 清除日志输出回调函数，防止悬空指针调用 */
#endif                                                              /* CFG_SUPPORT_LOG条件编译结束 */
}

