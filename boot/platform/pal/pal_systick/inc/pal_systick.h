/**
 *****************************************************************************
 * @brief   pal systick header file.
 *
 * @file    pal_systick.h
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

#ifndef __PAL_SYSTICK_H__
#define __PAL_SYSTICK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  获取系统滴答计数值
 * @retval 当前滴答值
 */
uint32_t systick_count_get(void);

/**
 * @brief  计算滴答时间差
 * @param  start_tick - 起始值
 * @retval 时间差
 */
uint32_t systick_diff(uint32_t start_tick);

/**
 * @brief  毫秒延时
 * @param  ms - 毫秒数
 */
void delay_ms(uint32_t ms);

/**
 * @brief  微秒延时(基于SysTick)
 * @param  us - 微秒数
 */
void delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif
#endif
