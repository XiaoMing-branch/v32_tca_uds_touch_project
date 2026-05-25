/**
 ******************************************************************************
 * @brief  delay header file.
 *
 * @file   tcae10_ll_delay.h
 * @author AE/FAE team
 * @date
 ******************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
 *
 ******************************************************************************
 */

#ifndef __FFF_TCAE10_LL_DELAY_H__
#define __FFF_TCAE10_LL_DELAY_H__

#include "fff_tcae10.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define SysTick_CLKSource_HCLK 0
#define SysTick_CLKSource_STCLK 1

    DECLARE_FAKE_VOID_FUNC(delay1ms, uint32_t);
    DECLARE_FAKE_VOID_FUNC(delay100us, uint32_t);
    DECLARE_FAKE_VOID_FUNC(delay1us, uint32_t);
    DECLARE_FAKE_VOID_FUNC(ll_systick_clkconfig, uint32_t);
    // void delay1ms(uint32_t u32Cnt);
    // void delay100us(uint32_t u32Cnt);
    // void delay1us(uint32_t u32Cnt);
    // void ll_systick_clkconfig(uint32_t source);

#ifdef __cplusplus
}
#endif
#endif /* __TCAE10_LL_DELAY_H__ */
