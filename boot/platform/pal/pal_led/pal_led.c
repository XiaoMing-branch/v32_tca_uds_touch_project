/**
 *****************************************************************************
 * @brief   pal led source file.
 *
 * @file    pal_led.c
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

#include "pal_led.h"
#include "pal_led_def.h"

/**
* @brief frequency of static acquisition for PN junction voltage
*/
#define LED_STATIC_SAMPLE_FREQ   0X4FFF

/**
* @brief LED driver frequency
*/
#define LED_DRIVER_PWM_FREQ      0XFFFF

extern void  pwm_callback_handle(uint32_t isr);

led_control_context_t led_ctrl_context[LED_CHANNEL_MAX];

__attribute__((weak)) void pwm_callback_handle(uint32_t isr)
{
    //do noting
}

/**
 * @brief  LED初始化(PWM配置、电流配置、多路复用开关配置)
 * @param  channel - LED通道号
 * @retval 无
 */
void pal_led_init(led_channel_e channel)
{
    /**
    * @brief led pwm config
    */
    pwm_config_t pwm_config =
    {
        .clk_cfg = {
            .clk_source = FCLK_SRC_48M,
            .fclk_div = 0,
        },
        .isr_cfg = {
            .isr = PWM_INT_CH0_CNT_MAX,
            .isr_enable = true,
            .priority = 2,
        },
        .out_mode = PWM_MODE_INDEPENDENT,
        .polarity = PWM_POLARITY_HIGH,
        .hvio_mode = HVIO_MODE_LED,
        .period = LED_DRIVER_PWM_FREQ,
    };

    ll_pwm_init((ll_pwm_bus_e)channel, &pwm_config, pwm_callback_handle);

    led_control_context_t *ctx = &led_ctrl_context[channel];
    ctx->channel.rgb[LED_R] = LED_CH_R;
    ctx->channel.rgb[LED_G] = LED_CH_G;
    ctx->channel.rgb[LED_B] = LED_CH_B;

    for (uint8_t i = 0; i < 3; i++)
    {
#ifdef __TCPL01X__
        ll_led_diag_current_config((pwm_channel_e)ctx->channel.rgb[i], (led_diag_current_e)LED_DIAG_1000UA);
#endif
        ll_led_driver_current_config((pwm_channel_e)ctx->channel.rgb[i], (led_driver_current_e)LED_DRIVER_CURRENT);
    }

    // #if !CFG_SUPPORT_LOG
    //     gpio_config_t test_gpio =
    //     {
    //         .gpio_pin = GPIO_PIN_2,
    //         .mode = GPIO_MODE_OUT_PP,
    //         .pull_mode = GPIO_PULL_UP,
    //         .pull_down_type = GPIO_PULLDOWN_SW_ONLY,

    //         .afio = AFIO_MUX_0,
    //         .trigger_flag = GPIO_TRIGGER_NULL,
    //     };
    //     ll_gpio_init(&test_gpio, NULL);
    //     ll_gpio_output(GPIO_PIN_2, false);
    // #endif

#if CFG_SUPPORT_MULTIPLEX_LED

#if CFG_MULTIPLEX_SWITCH_SW
    gpio_config_t led_sw_config =
    {
        .gpio_pin = (gpio_pin_e)CFG_MULTIPLEX_SWITCH_GPIO,
        .mode = GPIO_MODE_OUT_PP,
        .pull_mode = GPIO_PULL_UP,
        .pull_down_type = GPIO_PULLDOWN_SW_ONLY,
#if defined (__TCPL01X__)
        .afio = AFIO_MUX_0,
#elif defined (__TCPL03X__)
        .afio = AFIO_MUX_1,
#endif
        .trigger_flag = GPIO_TRIGGER_NULL,
    };
    ll_gpio_init(&led_sw_config, NULL);
    ll_gpio_output(GPIO_PIN_0, false);
    ll_pwm_enable(HVIO_MODE_LED, true);
#else
    ll_led_switch_init((ll_pwm_bus_e)channel);
#endif /* CFG_MULTIPLEX_SWITCH_SW */
#endif /* CFG_SUPPORT_MULTIPLEX_LED */
    ll_pwm_isr_enable((ll_pwm_bus_e)channel, true);
}

/**
 * @brief  设置LED R/G/B驱动电流
 * @param  channel - LED通道号
 * @param  current - R/G/B电流配置数组
 * @retval 无
 */
void pal_led_current_set(led_channel_e channel, uint8_t *current)
{
    led_control_context_t *ctx = &led_ctrl_context[channel];

    if (NULL == ctx)
    {
        return;
    }

    for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
    {
        ll_led_driver_current_config((pwm_channel_e)ctx->channel.rgb[rgb], (led_driver_current_e)current[rgb]);
    }
}

/**
 * @brief  获取LED R/G/B驱动电流
 * @param  channel - LED通道号
 * @param  current - 输出R/G/B电流配置数组
 * @retval 无
 */
void pal_led_current_get(led_channel_e channel, uint8_t *current)
{
    led_control_context_t *ctx = &led_ctrl_context[channel];
    led_driver_current_e value;

    if (NULL == ctx)
    {
        return;
    }

    for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
    {
        ll_led_driver_current_get((pwm_channel_e)ctx->channel.rgb[rgb], &value);
        current[rgb] = value;
    }
}

/**
 * @brief  设置LED PWM占空比(R/G/B)
 * @param  channel - LED通道号
 * @param  duty_cycle - R/G/B占空比数组
 * @retval 无
 */
void pal_led_dutcycle_set(led_channel_e channel, uint16_t *duty_cycle)
{
    led_control_context_t *ctx = &led_ctrl_context[channel];

    if (NULL == ctx || NULL == duty_cycle)
    {
        return;
    }

    for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
    {
        ll_pwm_channel_threshold_config((pwm_channel_e)ctx->channel.rgb[rgb], duty_cycle[rgb], 0);
    }

#if CFG_SUPPORT_MULTIPLEX_LED
#if CFG_MULTIPLEX_SWITCH_SW
    ll_gpio_output(GPIO_PIN_0, channel);
#endif /* CFG_MULTIPLEX_SWITCH_SW */
#endif /* CFG_SUPPORT_MULTIPLEX_LED */
}

/**
 * @brief  获取LED PWM占空比(R/G/B)
 * @param  channel - LED通道号
 * @param  duty_cycle - 输出R/G/B占空比数组
 * @retval 无
 */
void pal_led_dutcycle_get(led_channel_e channel, uint16_t *duty_cycle)
{
    led_control_context_t *ctx = &led_ctrl_context[channel];

    if (NULL == ctx || NULL == duty_cycle)
    {
        return;
    }

    for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
    {
        ll_pwm_high_threshold_get((pwm_channel_e)ctx->channel.rgb[rgb], &duty_cycle[rgb]);
    }
}

/**
 * @brief  LED使能/禁能(PWM输出)
 * @param  channel - LED通道号
 * @param  enable - true:使能输出, false:禁能输出
 * @retval 无
 */
void pal_led_enable(led_channel_e channel, bool enable)
{
    led_control_context_t *ctx = &led_ctrl_context[channel];

    if (NULL == ctx)
    {
        return;
    }

    ll_pwm_enable(HVIO_MODE_LED, enable);
    ctx->is_open = enable;
}

/**
 * @brief  LED刹车控制(紧急关闭PWM输出)
 * @param  channel - LED通道号
 * @param  enable - true:刹车(关闭), false:释放(恢复)
 * @retval 无
 */
void pal_led_break(led_channel_e channel, bool enable)
{
    led_control_context_t *ctx = &led_ctrl_context[channel];

    if (NULL == ctx)
    {
        return;
    }

    ll_pwm_break_set(enable);
}

/**
 * @brief  设置静态PN结电压采样模式
 * @param  channel - LED通道号
 * @param  enable - true:进入静态采样模式, false:退出
 * @note   进入时保存当前占空比并清零PWM阈值以关闭LED;
 *         退出时恢复占空比;切换PWM频率至静态采样频率
 * @retval 无
 */
void pal_led_static_pnvolt_set(led_channel_e channel, bool enable)
{
    led_control_context_t *ctx = &led_ctrl_context[channel];

    if (NULL == ctx)
    {
        return;
    }

    if (enable != ctx->static_sample.valid)
    {
        ctx->static_sample.valid = enable;

        if (enable)
        {
            if (!ctx->is_open)
            {

                for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
                {
                    ll_pwm_high_threshold_get((pwm_channel_e)ctx->channel.rgb[rgb], &ctx->static_sample.duty_cycle[rgb]);
                    ll_pwm_channel_threshold_config((pwm_channel_e)ctx->channel.rgb[rgb], 0, 0);
                }
            }

        }
        else
        {
            if (ctx->is_open)
            {
                for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
                {
                    ll_pwm_channel_threshold_config((pwm_channel_e)ctx->channel.rgb[rgb], ctx->static_sample.duty_cycle[rgb], 0);
                }
            }
        }

        uint32_t pwm_freq = (enable && !ctx->is_open) ? LED_STATIC_SAMPLE_FREQ : LED_DRIVER_PWM_FREQ;

        for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
        {
            ll_pwm_channel_period_config((pwm_channel_e)ctx->channel.rgb[rgb], pwm_freq);
        }

        if (!ctx->is_open)
        {
            ll_pwm_enable(HVIO_MODE_LED, true);
        }
    }
}

/**
 * @brief  获取LED通道的RGB PWM通道映射
 * @param  channel - LED通道号
 * @param  channel_mux - 输出RGB映射数组指针
 * @retval 无
 */
void pal_led_channel_mux_get(led_channel_e channel, uint8_t **channel_mux)

/**
 * @brief  LED通道切换(多路复用预留接口)
 * @param  channel - 目标通道号
 * @retval 无
 */
void pal_led_channel_switch(led_channel_e channel)
{

}


const ctrl_manager_instance_t led_ctrl_instance_def =
{
    .ctrl_init = pal_led_init,
    .ctrl_enable = pal_led_enable,
    .ctrl_break = pal_led_break,
    .ctrl_duty_set = pal_led_dutcycle_set,
    .ctrl_duty_get = pal_led_dutcycle_get,
    .ctrl_current_set = pal_led_current_set,
    .ctrl_current_get = pal_led_current_get,
    .ctrl_static_pnvolt_set = pal_led_static_pnvolt_set,
    .ctrl_channel_mux_get = pal_led_channel_mux_get,
};
