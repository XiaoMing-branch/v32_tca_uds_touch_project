/**
 *****************************************************************************
 * @brief   logging source file.
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
#include "tc_printf.h"

/**
 * @brief  初始化日志模块，配置UART日志输出
 * @note   通过 pal_log_init 初始化串口(115200bps)，
 *         并通过 log_out_func 注册 PAL 层输出函数
 * @retval 无
 */
void logging_init(void)
{
#if 1 == CFG_SUPPORT_LOG
    pal_log_init(115200);

    log_out_func(pal_log_print);
#endif
}

/**
 * @brief  反初始化日志模块，关闭UART日志输出
 * @note   调用 pal_log_deinit 关闭串口，并将输出函数指针置空
 * @retval 无
 */
void logging_deinit(void)
{
#if 1 == CFG_SUPPORT_LOG
    pal_log_deinit();

    log_out_func(NULL);
#endif
}

