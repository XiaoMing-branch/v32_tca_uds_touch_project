/**
 *****************************************************************************
 * @brief   LIN休眠/唤醒管理模块头文件。
 *          声明低功耗初始化和休眠模式进入函数。
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

/** @brief 初始化系统低功耗管理模块 */
void system_low_power_init(void);
/** @brief 进入休眠模式的主入口函数（包含唤醒后的恢复处理） */
void sleep_mode_enter(void);

#ifdef __cplusplus
}
#endif
#endif
