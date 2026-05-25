/**
 *****************************************************************************
 * @brief   wdg driver header.
 *
 * @file    tcae10_ll_wdg.h
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

#ifndef __TCAE10_LL_WDG_H__
#define __TCAE10_LL_WDG_H__

#include "tcae10_ll_def.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
    ll_clk_config_t clk_cfg;
    ll_isr_config_t isr_cfg;
    bool   reset_on_overflow;  //reset on overflow, otherwise will generate an interrupt
    uint32_t    max_count;          //the max count the wdg will count from 0xFFF
} wdg_config_t;

/**
 * @brief  去初始化看门狗模块
 */
void ll_wdg_deinit(void);
/**
 * @brief  初始化看门狗模块
 * @param config - 看门狗配置结构体指针
 * @param callback - 中断回调函数指针
 */
void ll_wdg_init(wdg_config_t *config, ISR_FUNC_CALLBACK callback);
/**
 * @brief  使能/禁能看门狗中断
 * @param enable - true: 使能，false: 禁能
 */
void ll_wdg_isr_enable(bool enable);
/**
 * @brief  清除看门狗中断标志
 */
void ll_wdg_interrupt_clear(void);
/**
 * @brief  使能/禁能看门狗
 * @param enable - true: 使能，false: 禁能
 */
void ll_wdg_enable(bool enable);
/**
 * @brief  重载看门狗计数器
 */
void ll_wdg_reload(void);

#if defined(__cplusplus)
}
#endif
#endif /* __TCAE10_LL_WDG_H__ */
