/**
 *****************************************************************************
 * @brief   pal wdg header file.
 *
 * @file    pal_wdg.h
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

#ifndef __PAL_WDG_H__
#define __PAL_WDG_H__

#include "pal_func_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  看门狗初始化
 * @param  cnt_ms - 超时时间(ms)
 */
void wdg_init(uint16_t cnt_ms);

/**
 * @brief  看门狗重载(喂狗)
 */
void wdg_reload(void);

/**
 * @brief  看门狗使能控制
 * @param  enable - true:使能, false:禁能
 */
void wdg_enable(bool enable);

#ifdef __cplusplus
}
#endif
#endif
