

/**
 ******************************************************************************
 * @brief  Delay source file
 *
 * @file   delay.c
 * @author AE/FAE team
 * @date
 ******************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
 *
 ******************************************************************************
 */

#include "fff_tcae10_ll_delay.h"
#include "fff_system_tcae10.h"
DEFINE_FAKE_VOID_FUNC(delay1ms, uint32_t);
DEFINE_FAKE_VOID_FUNC(delay100us, uint32_t);
DEFINE_FAKE_VOID_FUNC(delay1us, uint32_t);
DEFINE_FAKE_VOID_FUNC(ll_systick_clkconfig, uint32_t);
