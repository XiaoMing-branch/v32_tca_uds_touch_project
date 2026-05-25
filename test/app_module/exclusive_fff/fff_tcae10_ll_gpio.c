/**
 *****************************************************************************
 * @brief   gpio driver source file.
 *
 * @file    tcae10_ll_gpio.c
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

#include "fff_tcae10_ll_gpio.h"

DEFINE_FAKE_VOID_FUNC(ll_gpio_deinit);
DEFINE_FAKE_VOID_FUNC(ll_gpio_init, gpio_config_t *, ISR_FUNC_CALLBACK);
DEFINE_FAKE_VALUE_FUNC(bool,ll_gpio_read, gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_output, gpio_pin_e, bool);
DEFINE_FAKE_VOID_FUNC(ll_gpio_toggle, gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_isr_enable, gpio_pin_e, bool);
DEFINE_FAKE_VALUE_FUNC(bool,ll_gpio_interrupt_flag_get, gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_interrupt_clear, gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_afio_config, gpio_pin_e, gpio_afio_mux_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_ano_lpm_config, gpio_pin_e, bool);
DEFINE_FAKE_VOID_FUNC(ll_gpio_reset_enable, bool);

static ISR_FUNC_CALLBACK gpio_isr_callback = NULL;
