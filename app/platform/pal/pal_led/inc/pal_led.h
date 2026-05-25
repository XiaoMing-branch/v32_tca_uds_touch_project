/**
 *****************************************************************************
 * @brief   pal led header file.
 *
 * @file    pal_led.h
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
#ifndef __PAL_LED_H__
#define __PAL_LED_H__

#include "pal_func_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief LED功能回调函数类型
 * @param  isr - 中断状态标志
 */
typedef void (*LED_FUNC_CALLBACK)(uint32_t);

/**
  * @brief  RGB通道映射结构体
  */
typedef struct
{
    uint8_t rgb[RGB_TYPE_MAX];
} rgb_channel_t;

/**
  * @brief  LED电流配置结构体
  */
typedef struct
{
    uint8_t driver;   /**< 驱动电流 */
    uint8_t diagnose; /**< 诊断电流 */
} led_current_t;

/**
  * @brief  静态PN结电压采样结构体
  */
typedef struct
{
    uint16_t duty_cycle[RGB_TYPE_MAX]; /**< 各通道占空比备份 */
    bool valid;                        /**< 采样有效标志 */
} static_pnvolt_sample_t;

/**
  * @brief  LED控制实例结构体，管理RGB通道映射、电流、状态和回调
  */
typedef struct
{
    rgb_channel_t channel;               /**< RGB通道映射 */
    led_current_t current;               /**< 电流配置 */
    bool is_open;                        /**< LED是否开启 */
    static_pnvolt_sample_t static_sample;/**< 静态采样参数 */
    ISR_FUNC_CALLBACK  callback;         /**< 中断回调函数 */
} led_control_instance_t;

/**
 * @brief  LED初始化
 * @param  channel  - LED通道
 * @param  instance - 控制实例
 */
void pal_led_init(led_channel_e channel, led_control_instance_t *instance);
/**
 * @brief  使能/禁能LED
 * @param  channel - LED通道
 * @param  enable  - 使能标志
 */
void pal_led_enable(led_channel_e channel, bool enable);
/**
 * @brief  设置LED刹车
 * @param  channel - LED通道
 * @param  enable  - 使能标志
 */
void pal_led_break(led_channel_e channel, bool enable);
/**
 * @brief  设置RGB占空比
 * @param  channel    - LED通道
 * @param  duty_cycle - 占空比数组
 */
void pal_led_dutcycle_set(led_channel_e channel, uint16_t *duty_cycle);
/**
 * @brief  获取RGB占空比
 * @param  channel    - LED通道
 * @param  duty_cycle - 输出占空比数组
 */
void pal_led_dutcycle_get(led_channel_e channel, uint16_t *duty_cycle);
/**
 * @brief  设置驱动电流
 * @param  channel - LED通道
 * @param  current - 电流值
 */
void pal_led_current_set(led_channel_e channel, uint8_t current);
/**
 * @brief  获取驱动电流
 * @param  channel - LED通道
 * @param  current - 输出电流指针
 */
void pal_led_current_get(led_channel_e channel, uint8_t *current);
/**
 * @brief  设置静态PN结采样
 * @param  channel - LED通道
 * @param  enable  - 使能标志
 */
void pal_led_static_pnvolt_set(led_channel_e channel, bool enable);
/**
 * @brief  获取通道多路复用映射
 * @param  channel      - LED通道
 * @param  channel_mux  - 输出映射数组
 */
void pal_led_channel_mux_get(led_channel_e channel, uint8_t **channel_mux);

/**
 * @brief LED0控制实例（外部定义）
 */
extern led_control_instance_t led0_ctrl_instance;

#ifdef __cplusplus
}
#endif
#endif /*__PAL_LED_H__*/


