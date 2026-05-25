/**
 *****************************************************************************
 * @brief   pal gpio source file.
 *
 * @file    pal_gpio.c
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

#include "pal_gpio.h"

#if defined (__TCPL03X__)

/**
 * @brief  读取GPIO引脚电平
 * @param  gpio_pin - GPIO引脚号
 * @retval true - 高电平, false - 低电平
 */
bool pal_gpio_read(gpio_pin_e gpio_pin)
{
    return ll_gpio_read(gpio_pin);
}

/**
 * @brief  设置GPIO引脚输出电平
 * @param  gpio_pin - GPIO引脚号
 * @param  state - true:高电平, false:低电平
 * @retval 无
 */
void pal_gpio_output(gpio_pin_e gpio_pin, bool state)
{
    ll_gpio_output(gpio_pin, state);
}

/**
 * @brief  翻转GPIO引脚输出电平
 * @param  gpio_pin - GPIO引脚号
 * @retval 无
 */
void pal_gpio_toggle(gpio_pin_e gpio_pin)
{
    ll_gpio_toggle(gpio_pin);
}

/**
 * @brief  GPIO初始化
 * @param  cfg - GPIO配置参数
 * @param  callback - 中断回调函数(需配置触发标志)
 * @note   初始化完成后根据trigger_flag自动使能中断
 * @retval 无
 */
void pal_gpio_init(gpio_config_t *cfg, ISR_FUNC_CALLBACK callback)
{
    ll_gpio_init(cfg, callback);

    if (cfg->trigger_flag)
    {
        ll_gpio_isr_enable(cfg->gpio_pin, true);
    }
}
#endif
