/**
 *****************************************************************************
 * @brief   pal meas def header file.
 *
 * @file    pal_meas.h
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

#ifndef __PAL_LED_DEF_H__
#define __PAL_LED_DEF_H__

#include "pal_led.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief  LED初始化函数指针类型
 */
typedef void (*ctrl_init)(led_channel_e);

/**
 * @brief  LED使能函数指针类型
 */
typedef void (*ctrl_enable)(led_channel_e, bool);

/**
 * @brief  LED刹车函数指针类型
 */
typedef void (*ctrl_break)(led_channel_e, bool);

/**
 * @brief  LED占空比设置函数指针类型
 */
typedef void (*ctrl_duty_set)(led_channel_e, uint16_t *);

/**
 * @brief  LED占空比获取函数指针类型
 */
typedef void (*ctrl_duty_get)(led_channel_e, uint16_t *);

/**
 * @brief  LED电流设置函数指针类型
 */
typedef void (*ctrl_current_set)(led_channel_e, uint8_t *);

/**
 * @brief  LED电流获取函数指针类型
 */
typedef void (*ctrl_current_get)(led_channel_e, uint8_t *);

/**
 * @brief  静态PN结电压采样设置函数指针类型
 */
typedef void (*ctrl_static_pnvolt_set)(led_channel_e, bool);

/**
 * @brief  LED通道多路复用获取函数指针类型
 */
typedef void (*ctrl_channel_mux_get)(led_channel_e, uint8_t **);

/**
 * @brief  控制管理器实例结构体
 * @note   通过函数指针表实现LED控制接口的抽象管理
 */
typedef struct ctrl_manager_instance_t_ {
    void (*ctrl_init)(led_channel_e);
    void (*ctrl_enable)(led_channel_e, bool);
    void (*ctrl_break)(led_channel_e, bool);
    void (*ctrl_duty_set)(led_channel_e, uint16_t *);
    void (*ctrl_duty_get)(led_channel_e, uint16_t *);
    void (*ctrl_current_set)(led_channel_e, uint8_t *);
    void (*ctrl_current_get)(led_channel_e, uint8_t *);
    void (*ctrl_static_pnvolt_set)(led_channel_e, bool);
    void (*ctrl_channel_mux_get)(led_channel_e, uint8_t **);
} ctrl_manager_instance_t;

#ifdef __cplusplus
}
#endif
#endif /*__LED_CONTROL_DEF_H__*/
