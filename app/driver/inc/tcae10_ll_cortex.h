/**
 *****************************************************************************
 * @brief   tcae10_ll_cortex header file.
 *
 * @file    tcae10_ll_cortex.h
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

#ifndef __TCAE10_LL_CORTEX_H__
#define __TCAE10_LL_CORTEX_H__

#include "tcae10.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  默认中断优先级等级
 */
#define TCAE10_DEFAULT_IRQ_LEVEL        3

/**
 * @brief  使能/禁能NVIC中断
 * @param irq - 中断号
 * @param level - 中断优先级
 * @param en - true: 使能，false: 禁能
 */
void EnableNvic(uint32_t irq, uint8_t level, bool en);
/**
 * @brief  全局使能中断
 */
void interrupt_enable(void);
/**
 * @brief  全局禁能中断
 */
void interrupt_disable(void);

#ifdef __cplusplus
}
#endif
#endif /* __TCAE10_LL_CORTEX_H__ */
