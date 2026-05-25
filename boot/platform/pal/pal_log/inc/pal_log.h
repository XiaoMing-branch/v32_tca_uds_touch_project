/**
 *****************************************************************************
 * @brief   pal log header file.
 *
 * @file    pal_log.h
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

#ifndef __PAL_LOG_H__
#define __PAL_LOG_H__

#include <stdint.h>
#include "pal_func_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  日志模块初始化(串口)
 * @param  baudrate - 波特率
 */
void pal_log_init(uint32_t baudrate);

/**
 * @brief  日志模块去初始化
 */
void pal_log_deinit(void);

/**
 * @brief  日志打印一个字符
 * @param  ch - 字符
 */
void pal_log_print(uint8_t ch);

#ifdef __cplusplus
}
#endif
#endif
