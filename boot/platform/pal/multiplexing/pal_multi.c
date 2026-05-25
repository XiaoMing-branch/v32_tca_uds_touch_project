/**
 *****************************************************************************
 * @brief   pal multi source file.
 *
 * @file    pal_multi.c
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

#include "pal_multi.h"

#if defined (__TCPL01X__)
/* cnt interrupt time = (RTC_RELOAD_VALUE + 1) * 30.5us */
#define RTC_RELOAD_VALUE    0x60

static rtc_config_t config =
{
    .isr_cfg = {
        .isr = RTC_INT_CNT_FLAG,
        .isr_enable = true,
    },
    .period_cnt = RTC_RELOAD_VALUE,
};

/**
 * @brief  RTC GPIO中断回调(预留)
 * @param  isr - 中断状态
 * @retval 无
 */
static void rtc_gpio_callback(uint32_t isr)
{
}

/**
 * @brief  RTC GPIO复用初始化
 * @param  无
 * @note   初始化RTC定时器用于GPIO功能复用
 * @retval 无
 */
void rtc_gpio_init(void)
{
    ll_rtc_init(&config, rtc_gpio_callback);

    ll_rtc_isr_enable(true);
}

/**
 * @brief  获取当前RTC时间
 * @param  time - 输出RTC时间结构体
 * @retval 无
 */
void curretn_rtc_get(rtc_time_t *time)
{
    ll_rtc_time_get(time);
}
#endif
