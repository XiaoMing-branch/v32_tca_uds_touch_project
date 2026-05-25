/**
 *****************************************************************************
 * @brief   pal wdg source file.
 *
 * @file    pal_wdg.c
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

#include "pal_wdg.h"

/**
 * @brief  看门狗初始化
 * @param  cnt_ms - 看门狗超时时间(毫秒)
 * @note   TCPL01X和TCPL03X的分频计算方式不同
 * @retval 无
 */
void wdg_init(uint16_t cnt_ms)
{
    wdg_config_t wdg =
    {
#if defined (__TCPL01X__)
        .clk_cfg =
        {
            .fclk_div = 32 * 5,
        },
#endif
        .isr_cfg = {
            .isr_enable = false,
            .priority = 3,
        },
#if defined (__TCPL01X__)
        .max_count = cnt_ms / 5,
#elif defined (__TCPL03X__)
        .max_count = cnt_ms << 5, /* x32 */
#endif
        .reset_on_overflow = true,
    };
    ll_wdg_init(&wdg, NULL);

    // ll_wdg_isr_enable(true);
    ll_wdg_enable(true);
}

/**
 * @brief  看门狗喂狗(重载计数器)
 * @param  无
 * @retval 无
 */
void wdg_reload(void)
{
    ll_wdg_reload();
}

/**
 * @brief  看门狗使能/禁能
 * @param  enable - true:使能, false:禁能
 * @retval 无
 */
void wdg_enable(bool enable)
{
    ll_wdg_enable(enable);
}
