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
#include "app.h"

/**
* @brief 静态pn结电压检测频率
*/
#define LED_STATIC_SAMPLE_FREQ   0X4FFF
/**
* @brief LED正常驱动频率
*/
#define LED_DRIVER_PWM_FREQ      0XFFFF

extern void  pwm_callback_handle(uint32_t isr);

/**
 * @brief LED控制实例指针数组（按通道索引）
 */
led_control_instance_t *led_ctrl_instance[LED_CHANNLE_MAX];

/**
 * @brief PWM中断回调弱函数（用户可重写）
 * @param  isr - PWM中断状态标志
 * @retval None
 */
__attribute__((weak)) void pwm_callback_handle(uint32_t isr)
{
    //do noting
}

/**
 * @brief LED0控制实例，定义RGB通道映射、驱动电流及回调函数
 * @note  RGB通道对应LED_CH_R/G/B，驱动电流30mA，诊断电流1000uA（TCPL01X）
 */
led_control_instance_t led0_ctrl_instance =
{
    .channel.rgb = { LED_CH_R, LED_CH_G, LED_CH_B },
    .current =
    {
        .driver = LED_DRIVER_30MA,
#ifdef __TCPL01X__
        .diagnose = LED_DIAG_1000UA,
#endif
    },
    .callback = pwm_callback_handle,
};

/**
 * @brief LED PWM全局配置实例
 * @note  时钟源48MHz，独立输出模式，高电平有效，HVIO模式LED，周期0xFFFF
 */
pwm_config_t config =
{
    .clk_cfg = {
        .clk_source = FCLK_SRC_48M,
        .fclk_div = 0,
    },
    .isr_cfg = {
        .isr = PWM_INIT_FLAG,
        .isr_enable = true,
    },
    .out_mode = PWM_MODE_INDEPENDENT,
    .polarity = PWM_POLARITY_HIGH,
    .hvio_mode = HVIO_MODE_LED,
    .period = LED_DRIVER_PWM_FREQ,
};

/**
 * @brief  LED初始化，配置PWM、驱动电流和诊断电流
 * @param  channel  - LED通道编号
 * @param  instance - LED控制实例指针
 * @retval None
 */
void pal_led_init(led_channel_e channel, led_control_instance_t *instance)
{

    if (NULL == instance)
    {
        return;
    }

    ll_pwm_init((ll_pwm_bus_e)channel, &config, instance->callback);

    for (uint8_t i = 0; i < 3; i++)
    {
#ifdef __TCPL01X__
        ll_led_diag_current_config((pwm_channel_e)instance->channel.rgb[i], (led_diag_current_e)instance->current.diagnose);
#endif
        ll_led_driver_current_config((pwm_channel_e)instance->channel.rgb[i], (led_driver_current_e)instance->current.driver);
    }

    led_ctrl_instance[channel] = instance;
}

/**
 * @brief  设置LED驱动电流
 * @param  channel - LED通道编号
 * @param  current - 驱动电流值
 * @retval None
 */
void pal_led_current_set(led_channel_e channel, uint8_t current)
{
    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance)
    {
        return;
    }

    for (rgb_type_e rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
    {
        ll_led_driver_current_config((pwm_channel_e)instance->channel.rgb[rgb], (led_driver_current_e)current);
    }

    instance->current.driver = current;
}

/**
 * @brief  获取LED当前驱动电流
 * @param  channel - LED通道编号
 * @param  current - 输出电流值指针
 * @retval None
 */
void pal_led_current_get(led_channel_e channel, uint8_t *current)
{
    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance)
    {
        return;
    }

    *current = instance->current.driver;
}

/**
 * @brief  开启/关闭LED静态PN结电压采样模式
 * @note   开启时将PWM频率切换为低频采样频率，关闭时恢复驱动频率
 * @param  channel - LED通道编号
 * @param  enable  - true:开启静态采样 false:关闭
 * @retval None
 */
void pal_led_dutcycle_set(led_channel_e channel, uint16_t *duty_cycle)
{
    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance || NULL == duty_cycle)
    {
        return;
    }

    for (rgb_type_e rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
    {
        ll_pwm_channel_threshold_config((pwm_channel_e)instance->channel.rgb[rgb], duty_cycle[rgb], 0);
    }
}

/**
 * @brief  获取LED RGB各通道当前PWM占空比
 * @param  channel    - LED通道编号
 * @param  duty_cycle - 输出RGB三通道占空比数组指针
 * @retval None
 */
void pal_led_dutcycle_get(led_channel_e channel, uint16_t *duty_cycle)
{
    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance || NULL == duty_cycle)
    {
        return;
    }

    for (rgb_type_e rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
    {
        ll_pwm_high_threshold_get((pwm_channel_e)instance->channel.rgb[rgb], &duty_cycle[rgb]);
    }
}

/**
 * @brief  设置LED RGB各通道PWM占空比
 * @param  channel    - LED通道编号
 * @param  duty_cycle - RGB三通道占空比数组指针
 * @retval None
 */
void pal_led_enable(led_channel_e channel, bool enable)
{
    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance)
    {
        return;
    }

    ll_pwm_enable(HVIO_MODE_LED, enable);
    instance->is_open = enable;
}

/**
 * @brief  使能/禁能LED输出
 * @param  channel - LED通道编号
 * @param  enable  - true:使能 false:禁能
 * @retval None
 */
void pal_led_break(led_channel_e channel, bool enable)
{
    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance)
    {
        return;
    }

    ll_pwm_break_set(enable);
}

/**
 * @brief  设置LED刹车/断路功能
 * @param  channel - LED通道编号
 * @param  enable  - true:使能刹车 false:禁能
 * @retval None
 */
void pal_led_static_pnvolt_set(led_channel_e channel, bool enable)
{
    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance)
    {
        return;
    }

    if (enable != instance->static_sample.valid)
    {
        instance->static_sample.valid = enable;

        if (enable)
        {
            if (!instance->is_open)
            {

                for (rgb_type_e rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
                {
                    ll_pwm_high_threshold_get((pwm_channel_e)instance->channel.rgb[rgb], &instance->static_sample.duty_cycle[rgb]);
                    ll_pwm_channel_threshold_config((pwm_channel_e)instance->channel.rgb[rgb], 0, 0);
                }
            }

        }
        else
        {
            if (instance->is_open)
            {
                for (rgb_type_e rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
                {
                    ll_pwm_channel_threshold_config((pwm_channel_e)instance->channel.rgb[rgb], instance->static_sample.duty_cycle[rgb], 0);
                }
            }
        }

        uint32_t pwm_freq = (enable && !instance->is_open) ? LED_STATIC_SAMPLE_FREQ : LED_DRIVER_PWM_FREQ;

        for (rgb_type_e rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
        {
            ll_pwm_channel_period_config((pwm_channel_e)instance->channel.rgb[rgb], pwm_freq);
        }

        if (!instance->is_open)
        {
            ll_pwm_enable(HVIO_MODE_LED, true);
        }
    }
}

/**
 * @brief  获取LED通道的RGB多路复用映射表
 * @param  channel      - LED通道编号
 * @param  channel_mux  - 输出RGB通道映射数组指针的指针
 * @retval None
 */
void pal_led_channel_mux_get(led_channel_e channel, uint8_t **channel_mux)
{

    led_control_instance_t *instance = led_ctrl_instance[channel];

    if (NULL == instance)
    {

        return;
    }

    *channel_mux = instance->channel.rgb;
}
