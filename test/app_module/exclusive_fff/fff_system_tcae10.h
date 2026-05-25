/**
 *****************************************************************************
 * @brief   system_tcae01 head file defination.
 *
 * @file   system_tcae10.h
 * @author AE/FAE team
 * @date   28/JUN/2020
 *****************************************************************************
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
 *****************************************************************************
 */

#ifndef __FFF_SYSTEM_TCAE10_H__
#define __FFF_SYSTEM_TCAE10_H__

#include "fff.h"
#include "fff_base_types.h"

/******************************************************************************* 
* Definitions 
******************************************************************************/
extern uint32_t SystemCoreClock;



/******************************************************************************* 
* API 
******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /*_cplusplus*/

/* Value of the fast internal oscillator clock frequency in Hz  */
#ifndef CPU_INT_FAST_CLK_HZ
#define CPU_INT_FAST_CLK_HZ            DEFAULT_SYSTEM_CLOCK
#endif

/* Value of the slow internal oscillator clock frequency in Hz  */
#ifndef CPU_INT_SLOW_CLK_HZ
#define CPU_INT_SLOW_CLK_HZ            32000u
#endif

#define CONFIG_SYSTEM_CORE_CLK      HCLK_SRC_RC48M

typedef enum system_hclk_src
{
    HCLK_SRC_RC48M,
    HCLK_SRC_RC32K,
} enumSystemHclkSrc_t;

DECLARE_FAKE_VALUE_FUNC(uint32_t,GetFclkVal);
DECLARE_FAKE_VOID_FUNC(SystemCoreClockUpdate);
DECLARE_FAKE_VALUE_FUNC(uint32_t, SystemGetHClkFreq);
DECLARE_FAKE_VALUE_FUNC(uint32_t, sys_hclk_freq_get);
DECLARE_FAKE_VALUE_FUNC(uint32_t, sys_pclk_freq_get);
DECLARE_FAKE_VOID_FUNC(SystemInit);

// uint32_t GetFclkVal(void);
// void SystemCoreClockUpdate (void);
// uint32_t SystemGetHClkFreq(void);
// uint32_t sys_hclk_freq_get(void);
// uint32_t sys_pclk_freq_get(void);
// void SystemInit ( void );

#if defined(__cplusplus)
}
#endif /*_cplusplus*/

#endif /* __FFF_SYSTEM_TCAE10_H__ */
