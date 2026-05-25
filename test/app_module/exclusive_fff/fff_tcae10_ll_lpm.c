/**
 *****************************************************************************
 * @brief   lpm Source file.
 *
 * @file    tcae10_ll_lpm.c
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

#include "fff_tcae10_ll_lpm.h"

DEFINE_FAKE_VOID_FUNC(ll_lpm_mcu_enter,sleep_mode_e,bool);
DEFINE_FAKE_VOID_FUNC(ll_lpm_afe_enter,sleep_mode_e);
DEFINE_FAKE_VOID_FUNC(ll_pmu_gpio_lowpower);
DEFINE_FAKE_VOID_FUNC(ll_pmu_ldo_dummy_enable,bool);