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

#ifndef __FFF_TCAE10_LL_LPM_H__
#define __FFF_TCAE10_LL_LPM_H__

#include "fff_tcae10_ll_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SLPMODE_IDLE            0
#define SLPMODE_SLEEP           1
#define SLPMODE_SLEEPWALK       2
#define SLPMODE_DEEPSLEEP       3

typedef enum
{
    IDLE_MODE       = 0,
    SLEEP_MODE      = 1,
    SLEEPWALK_MODE  = 2,
    DEEPSLEEP_MODE  = 3,
    SLEEP_MODE_MAX  = 4
} sleep_mode_e;

DECLARE_FAKE_VOID_FUNC(ll_lpm_mcu_enter,sleep_mode_e,bool);
DECLARE_FAKE_VOID_FUNC(ll_lpm_afe_enter,sleep_mode_e);
DECLARE_FAKE_VOID_FUNC(ll_pmu_gpio_lowpower);
DECLARE_FAKE_VOID_FUNC(ll_pmu_ldo_dummy_enable,bool);

// void ll_lpm_mcu_enter(sleep_mode_e state, bool on_exit);
// void ll_lpm_afe_enter(sleep_mode_e mode);
// void ll_pmu_gpio_lowpower(void);
// void ll_pmu_ldo_dummy_enable(bool enable);

#ifdef __cplusplus
}
#endif
#endif /* __FFF_TCAE10_LL_LPM_H__ */
