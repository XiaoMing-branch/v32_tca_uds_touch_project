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

typedef void (*LED_FUNC_CALLBACK)(uint32_t);

/**
 * @brief  RGB通道结构体
 * @note   存储R/G/B三个通道的映射值
 */
typedef struct
{
    uint8_t rgb[LED_TYPE_MAX];
} rgb_channel_t;

/**
 * @brief  LED电流配置结构体
 * @note   包含驱动电流和诊断电流
 */
typedef struct
{
    uint8_t driver;
    uint8_t diagnose;
} led_current_t;

/**
 * @brief  静态PN结电压采样结构体
 * @note   保存LED关闭时的PWM占空比快照和有效标志
 */
typedef struct
{
    uint16_t duty_cycle[LED_TYPE_MAX];
    bool valid;
} static_pnvolt_sample_t;

/**
 * @brief  LED控制实例结构体
 * @note   保存LED通道的RGB映射、开关状态及静态采样信息
 */
typedef struct
{
    rgb_channel_t channel;
    bool is_open;
    static_pnvolt_sample_t static_sample;
#if (CFG_SUPPORT_MULTIPLEX_LED && CFG_MULTIPLEX_SWITCH_SW)
    gpio_pin_e led_sw_gpio;
#endif
} led_control_context_t;

/**
 * @brief  LED初始化
 * @param  channel - LED通道号
 */
void pal_led_init(led_channel_e channel);

/**
 * @brief  LED使能/禁能
 * @param  channel - LED通道号
 * @param  enable - true:使能, false:禁能
 */
void pal_led_enable(led_channel_e channel, bool enable);

/**
 * @brief  LED刹车控制(紧急关闭)
 * @param  channel - LED通道号
 * @param  enable - true:刹车, false:释放
 */
void pal_led_break(led_channel_e channel, bool enable);

/**
 * @brief  设置LED PWM占空比
 * @param  channel - LED通道号
 * @param  duty_cycle - R/G/B占空比数组[0]=R,[1]=G,[2]=B
 */
void pal_led_dutcycle_set(led_channel_e channel, uint16_t *duty_cycle);

/**
 * @brief  获取LED PWM占空比
 * @param  channel - LED通道号
 * @param  duty_cycle - 输出R/G/B占空比数组
 */
void pal_led_dutcycle_get(led_channel_e channel, uint16_t *duty_cycle);

/**
 * @brief  设置LED驱动电流
 * @param  channel - LED通道号
 * @param  current - R/G/B电流配置数组
 */
void pal_led_current_set(led_channel_e channel, uint8_t *current);

/**
 * @brief  获取LED驱动电流
 * @param  channel - LED通道号
 * @param  current - 输出R/G/B电流配置数组
 */
void pal_led_current_get(led_channel_e channel, uint8_t *current);

/**
 * @brief  设置静态PN结电压采样
 * @param  channel - LED通道号
 * @param  enable - true:开启采样, false:关闭
 */
void pal_led_static_pnvolt_set(led_channel_e channel, bool enable);

/**
 * @brief  获取LED通道的RGB多路复用映射
 * @param  channel - LED通道号
 * @param  channel_mux - 输出RGB通道映射指针
 */
void pal_led_channel_mux_get(led_channel_e channel, uint8_t **channel_mux);

/**
 * @brief  LED通道切换(多路复用)
 * @param  channel - 目标通道号
 */
void pal_led_channel_switch(led_channel_e channel);

extern led_control_context_t led0_ctrl_instance;

#ifdef __cplusplus
}
#endif
#endif /*__PAL_LED_H__*/

