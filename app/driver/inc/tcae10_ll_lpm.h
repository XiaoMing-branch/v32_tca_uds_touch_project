/**
 *****************************************************************************
 * @brief   lpm header file.
 *
 * @file    tcae10_ll_lpm.h
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

#ifndef __TCAE10_LL_LPM_H__
#define __TCAE10_LL_LPM_H__

#include "tcae10_ll_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  空闲模式
 */
#define SLPMODE_IDLE            0
/**
 * @brief  睡眠模式
 */
#define SLPMODE_SLEEP           1
/**
 * @brief  睡眠漫步模式
 */
#define SLPMODE_SLEEPWALK       2
/**
 * @brief  深度睡眠模式
 */
#define SLPMODE_DEEPSLEEP       3

typedef enum
{
    IDLE_MODE       = 0,
    SLEEP_MODE      = 1,
    SLEEPWALK_MODE  = 2,
    DEEPSLEEP_MODE  = 3,
    SLEEP_MODE_MAX  = 4
} sleep_mode_e;

/**
 * @brief  MCU进入低功耗模式
 * @param state - 睡眠模式 @ref sleep_mode_e
 * @param on_exit - true: 退出时唤醒，false: 持续睡眠
 */
void ll_lpm_mcu_enter(sleep_mode_e state, bool on_exit);
/**
 * @brief  AFE模拟前端进入低功耗模式
 * @param mode - 睡眠模式 @ref sleep_mode_e
 */
void ll_lpm_afe_enter(sleep_mode_e mode);
/**
 * @brief  PMU GPIO进入低功耗状态
 */
void ll_pmu_gpio_lowpower(void);
/**
 * @brief  使能/禁能PMU LDO虚拟负载
 * @param enable - true: 使能，false: 禁能
 */
void ll_pmu_ldo_dummy_enable(bool enable);

#ifdef __cplusplus
}
#endif
#endif /* __TCAE10_LL_LPM_H__ */
