/**
 *****************************************************************************
 * @brief   lin wakeup header file.
 *
 * @file    lin_wakeup.h
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

#ifndef __LIN_WAKEUP_H__
#define __LIN_WAKEUP_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  系统低功耗初始化
 * @param  无
 * @retval 无
 */
void system_low_power_init(void);

/**
 * @brief  进入睡眠模式
 * @param  无
 * @note   检查lin_goto_sleep_flg标志，进入低功耗后清除测量数据并恢复LED
 * @retval 无
 */
void sleep_mode_enter(void);

#ifdef __cplusplus
}
#endif
#endif
