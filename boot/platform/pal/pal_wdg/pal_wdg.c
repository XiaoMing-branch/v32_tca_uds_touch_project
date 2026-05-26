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
 *
 * @param[in] cnt_ms 看门狗超时时间(毫秒)，TCPL01X和TCPL03X的分频计算方式不同
 * @retval 无
 * @note   根据芯片型号自动选择分频系数：TCPL01X使用 (cnt_ms / 5)，TCPL03X使用 (cnt_ms << 5)
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
 *
 * @param[in] 无
 * @retval 无
 * @note   调用底层 ll_wdg_reload() 刷新看门狗定时器，防止系统复位
 */
void wdg_reload(void)
{
    ll_wdg_reload();
}

/**
 * @brief  看门狗使能/禁能
 *
 * @param[in] enable true: 使能看门狗, false: 禁能看门狗
 * @retval 无
 * @note   通过调用 ll_wdg_enable() 控制看门狗外设的启停
 */
void wdg_enable(bool enable)
{
    ll_wdg_enable(enable);
}
