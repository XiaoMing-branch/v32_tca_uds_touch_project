/**
 *****************************************************************************
 * @brief   system_tcpl03x header file.
 *
 * @file    system_tcpl03x.h
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

#ifndef __SYSTEM_TCPL03X_H__
#define __SYSTEM_TCPL03X_H__

#include <stdint.h>
#include <stdbool.h>
#include "fff.h"
#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************
 * CPU Settings.
 *****************************************************************************/

/* Watchdog disable */
#ifndef DISABLE_WDOG
#define DISABLE_WDOG                 1
#endif

/* Value of the fast internal oscillator clock frequency in Hz  */
#ifndef CPU_INT_FAST_CLK_HZ
#define CPU_INT_FAST_CLK_HZ            DEFAULT_SYSTEM_CLOCK
#endif

/* Value of the slow internal oscillator clock frequency in Hz  */
#ifndef CPU_INT_SLOW_CLK_HZ
#define CPU_INT_SLOW_CLK_HZ            32000u
#endif

/**
 * @brief System clock frequency (core clock)
 *
 * The system clock frequency supplied to the SysTick timer and the processor
 * core clock. This variable can be used by the user application to setup the
 * SysTick timer or configure other parameters. It may also be used by debugger to
 * query the frequency of the debug timer or configure the trace clock speed
 * SystemCoreClock is initialized with a correct predefined value.
 */
extern uint32_t SystemCoreClock;

/*******************************************************************************
* API
******************************************************************************/

#define CONFIG_SYSTEM_CORE_CLK      HCLK_SRC_RC48M

typedef enum system_hclk_src
{
    HCLK_SRC_RC48M,
    HCLK_SRC_RC32K,
} enumSystemHclkSrc_t;

DECLARE_FAKE_VOID_FUNC(sys_core_clock_update);
DECLARE_FAKE_VALUE_FUNC(uint32_t,sys_hclk_freq_get); 
DECLARE_FAKE_VALUE_FUNC(uint32_t,sys_pclk_freq_get);
DECLARE_FAKE_VOID_FUNC(SystemInit);
DECLARE_FAKE_VOID_FUNC(system_remap_config, uint32_t *, bool *);

// void sys_core_clock_update(void);
// uint32_t sys_hclk_freq_get(void);
// uint32_t sys_pclk_freq_get(void);
// void SystemInit(void);
// void system_remap_config(uint32_t vetor_offset, bool enable);

#if defined(__cplusplus)
}
#endif
#endif /* __SYSTEM_TCPL03X_H__ */
