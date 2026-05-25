/**
 *****************************************************************************
 * @brief   pal timer source file.
 *
 * @file    pal_timer.c
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

#include "pal_timer.h"

extern void timer_callback_handle(uint32_t isr);
/**
 * @brief  定时器中断回调函数(弱符号，可重写)
 * @param  isr - 中断状态
 * @retval 无
 */
__attribute__((weak)) void timer_callback_handle(uint32_t isr)
{

}

/**
 * @brief  定时器控制实例默认配置
 * @note   定时器频率 = src / clk_div / count
 *         定时器周期 = 1/频率
 *         示例: 频率 = 48M/10/480 = 10000Hz, 周期 = 1/10000s = 0.1ms = 100us
 */
timer_control_instance_t timer_ctrl_instance =
{
    .src = FCLK_SRC_48M,
    .clk_div = TIMER_SRC_FDIV,
    .count = TIMER_BASE_PERIOD,
    .dis_repeat = false,
    .callback = timer_callback_handle,
};

/**
 * @brief  硬件定时器初始化
 * @param  instance - 定时器控制实例(包含时钟源、分频、计数周期等配置)
 * @note   初始化后自动使能中断和定时器
 * @retval 无
 */
void pal_timer_init(timer_control_instance_t *instance)
{
    timer_config_t config =
    {
        .clk_cfg = {
            .clk_source = instance->src,
            .fclk_div = instance->clk_div,
        },
        .isr_cfg = {
            .isr_enable = true,
            .priority = 2,
        },
        .initial_value = instance->count,
        .repeat_disable = instance->dis_repeat,

    };
    ll_timer_init(&config, instance->callback);
    ll_timer_isr_enable(true);
    ll_timer_enable(true);
}

/**
 * @brief  硬件定时器去初始化
 * @param  instance - 定时器控制实例(去初始化后清零)
 * @retval 无
 */
void pal_timer_deinit(timer_control_instance_t *instance)
{
    ll_timer_deinit();
    memset(instance, 0, sizeof(timer_control_instance_t));
}

