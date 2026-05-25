/**
 *****************************************************************************
 * @brief   pal pmu source file.
 *
 * @file    pal_pmu.c
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

#include "pal_pmu.h"

/**
 * @brief  进入低功耗模式
 * @param  mode - 睡眠模式选择
 * @note   关闭PWM中断、GPIO低功耗模式、自动寻址、ADC温度传感器、
 *         偏置电流和看门狗;使能LIN唤醒;依次进入AFE和MCU睡眠
 * @retval 无
 */
void pmu_lpm_enter(sleep_mode_e mode)
{
    /* Need stop & clr intterrupt sources */
    ll_pwm_isr_flag_enable(LL_PWM_BUS_0, PWM_INIT_ALL_FLAG, false);
#if defined (__TCPL01X__)

#elif defined (__TCPL03X__)
    ll_pwm_isr_enable(LL_PWM_BUS_0, false);
#else
#endif

    /* change gpio function */
    ll_pmu_gpio_lowpower();

    /* Auto addressing  ana disable */
    ll_lin_aa_disable(LL_SCI_BUS_1);

    /* disable adc t-sensor */
    ll_adc_tsensor_enable(false);

    /* disable bias */
    ll_bias_control_enable(false);

    //    ll_adc_deinit();
    //    ll_pwm_deinit(0);
    //    ll_sci_deinit(0);
    //    ll_sci_deinit(1);
    //    ll_gpio_deinit();
    //    ll_timer_deinit();
    //    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    //    /* Disable timer */
    //    ll_timer_isr_enable(false);
    //    ll_timer_enable(false);
#if CFG_SUPPORT_WDG_EN
    ll_wdg_enable(false);
#endif
    /* Enable lin Wake up control */
    ll_lin_wakeup_enable(LL_SCI_BUS_1, true);

    /* AFE enter sleep mode */
    ll_lpm_afe_enter(mode);

#if defined (__TCPL03X__)
    ll_pmu_ldo_dummy_enable(true);
#endif
    /* MCU enter sleep mode */
    ll_lpm_mcu_enter(mode, false);

    __NOP();
    __NOP();
    __NOP();
    __NOP();

    /* Disable lin Wake up control */
    ll_lin_wakeup_enable(LL_SCI_BUS_1, false);
}

/**
 * @brief  退出低功耗模式
 * @param  无
 * @note   恢复看门狗、ADC温度传感器和偏置电流
 * @retval 无
 */
void pmu_lpm_exit(void)
{
    /* Enable timer */
    // ll_timer_isr_enable(true);
    // ll_timer_enable(true);
#if CFG_SUPPORT_WDG_EN
    ll_wdg_enable(true);
#endif

    /* enable adc t-sensor */
    ll_adc_tsensor_enable(true);

    /* enable bias */
    ll_bias_control_enable(true);

#if defined (__TCPL03X__)
    ll_pmu_ldo_dummy_enable(false);
#endif
}

/**
 * @brief  低功耗模式初始化
 * @param  无
 * @note   配置GPIO低功耗模式，禁用LIN自动寻址
 * @retval 无
 */
void pmu_lpm_init(void)
{
    ll_pmu_gpio_lowpower();

#if defined (__TCPL03X__)
    ll_pmu_ldo_dummy_enable(false);
#endif

    /* Auto addressing disable */
    ll_lin_aa_disable(LL_SCI_BUS_1);
}

/**
 * @brief  使能/禁能OTP(过温保护)
 * @param  enable - true:使能, false:禁能
 * @retval 无
 */
void pmu_otp_enable(bool enable)
{
#if defined (__TCPL03X__)
    ll_syscfg_otp_enable(true);
#endif
}

/**
 * @brief  获取OTP(过温保护)状态
 * @param  无
 * @retval true - 过温保护触发, false - 正常
 */
bool pmu_otp_status(void)
{
#if defined (__TCPL03X__)
    return (ll_syscfg_otp_status());
#else
    return false;
#endif
}
