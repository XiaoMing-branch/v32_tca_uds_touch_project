/**
 *****************************************************************************
 * @brief   pal multi header file.
 *
 * @file    pal_multi.h
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

#ifndef __PAL_MULTI_H__
#define __PAL_MULTI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pal_func_def.h"

#if defined (__TCPL01X__)
/**
 * @brief  RTC GPIO复用初始化
 */
void rtc_gpio_init(void);

/**
 * @brief  获取当前RTC时间
 * @param  time - 输出时间结构体
 */
void curretn_rtc_get(rtc_time_t *time);
#endif

#ifdef __cplusplus
}
#endif

#endif /*__PAL_MULTI_H__*/
