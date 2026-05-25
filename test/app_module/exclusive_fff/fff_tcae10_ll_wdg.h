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

#ifndef __FFF_TCAE10_LL_WDG_H__
#define __FFF_TCAE10_LL_WDG_H__

#include "fff.h"
#include "fff_tcae10_ll_def.h"

#if defined(__cplusplus)
extern "C"
{
#endif

    typedef struct
    {
        ll_clk_config_t clk_cfg;
        ll_isr_config_t isr_cfg;
        bool reset_on_overflow; // reset on overflow, otherwise will generate an interrupt
        uint32_t max_count;     // the max count the wdg will count from 0xFFF
    } wdg_config_t;

    DECLARE_FAKE_VOID_FUNC(ll_wdg_deinit);
    DECLARE_FAKE_VOID_FUNC(ll_wdg_init, wdg_config_t *, ISR_FUNC_CALLBACK);
    DECLARE_FAKE_VOID_FUNC(ll_wdg_isr_enable, bool);
    DECLARE_FAKE_VOID_FUNC(ll_wdg_interrupt_clear);
    DECLARE_FAKE_VOID_FUNC(ll_wdg_enable, bool);
    DECLARE_FAKE_VOID_FUNC(ll_wdg_reload);

    // void ll_wdg_deinit(void);
    // void ll_wdg_init(wdg_config_t *config, ISR_FUNC_CALLBACK callback);
    // void ll_wdg_isr_enable(bool enable);
    // void ll_wdg_interrupt_clear(void);
    // void ll_wdg_enable(bool enable);
    // void ll_wdg_reload(void);

#if defined(__cplusplus)
}
#endif
#endif /* __TCAE10_LL_WDG_H__ */
