/**
 *****************************************************************************
 * @brief   pal timer header file.
 *
 * @file    pal_timer.h
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

#ifndef __PAL_TIMER_H__
#define __PAL_TIMER_H__

#include <stdint.h>
#include "pal_func_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIMER_BASE_PERIOD   (480)
#define TIMER_SRC_FDIV      (0xA)

/**
 * @brief  定时器控制实例结构体
 * @note   包含时钟源、分频系数、计数周期和回调函数
 */
typedef struct
{
    fclk_src_e src;
    uint16_t clk_div;
    uint16_t count;
    bool dis_repeat;
    ISR_FUNC_CALLBACK callback;

} timer_control_instance_t;

extern timer_control_instance_t timer_ctrl_instance;

/**
 * @brief  定时器初始化
 * @param  instance - 定时器配置实例
 */
void pal_timer_init(timer_control_instance_t *instance);

/**
 * @brief  定时器去初始化
 * @param  instance - 定时器配置实例
 */
void pal_timer_deinit(timer_control_instance_t *instance);

#ifdef __cplusplus
}
#endif
#endif
