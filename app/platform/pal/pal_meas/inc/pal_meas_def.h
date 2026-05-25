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

#ifndef __PAL_MEAS_DEF_H__
#define __PAL_MEAS_DEF_H__

#include "pal_meas.h"
#include "pal_led.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED灯珠串联数定义
 */
#define LED_MEAS_SERIAL_1       (0u) /**< 1颗灯珠串联 */
#define LED_MEAS_SERIAL_2       (1u) /**< 2颗灯珠串联 */
#define LED_MEAS_SERIAL_3       (2u) /**< 3颗灯珠串联 */
#if defined (__TCPL01X__)
#define LED_MEAS_SERIAL_MAX     (3u) /**< TCPL01X最大3串 */
#elif defined __TCPL03X__
#define LED_MEAS_SERIAL_MAX     (1u) /**< TCPL03X最大1串 */
#endif

/**
 * @brief 默认LED灯珠串联数（出厂默认值）
 */
#define DEFAULT_LED_SERIAL_NUM  LED_MEAS_SERIAL_1

/**
 * @brief 测量初始化函数指针类型
 */
typedef bool (*meas_init)(led_channel_e);
/**
 * @brief 增益配置函数指针类型
 */
typedef bool (*meas_gain_config)(led_channel_e, uint8_t *);
/**
 * @brief 电压码值获取函数指针类型
 */
typedef bool (*meas_voltage_code_get)(meas_volt_type_e, uint16_t *);
/**
 * @brief 电压计算函数指针类型
 */
typedef bool (*meas_volt_calc_func)(meas_volt_type_e, uint16_t, int16_t *);
/**
 * @brief PN码值读取函数指针类型
 */
typedef void (*meas_pn_code_read)(led_channel_e);
/**
 * @brief PN电压获取函数指针类型
 */
typedef bool (*meas_pn_voltage_get)(led_channel_e, rgb_type_e, uint16_t *);
/**
 * @brief PN电压计算函数指针类型
 */
typedef bool (*meas_pn_calc_func)(led_channel_e, rgb_type_e, uint16_t, int16_t *);
/**
 * @brief PN状态刷新函数指针类型
 */
typedef void (*meas_pn_status_reflash)(led_channel_e);
/**
 * @brief PN采样处理函数指针类型
 */
typedef bool (*meas_pn_process)(led_channel_e, uint32_t);
/**
 * @brief PN采样暂停函数指针类型
 */
typedef bool (*meas_pn_suspend)(led_channel_e);
/**
 * @brief PN采样恢复函数指针类型
 */
typedef bool (*meas_pn_resume)(led_channel_e);
/**
 * @brief PN采样采集函数指针类型
 */
typedef void (*meas_pn_acquire)(led_channel_e);
/**
 * @brief PN状态获取函数指针类型
 */
typedef bool (*meas_pn_status_get)(led_channel_e, led_vf_status_type_e, uint8_t *);
/**
 * @brief PN状态设置函数指针类型
 */
typedef bool (*meas_pn_status_set)(led_channel_e, led_vf_status_type_e, uint8_t);
/**
 * @brief PN阈值获取函数指针类型
 */
typedef bool (*meas_pn_threshold_get)(led_channel_e, rgb_type_e, rgb_safty_threshold_t *);

/**
 * @brief 测量管理器实例结构体，聚合所有测量操作接口
 */
typedef struct meas_manager_instance_t_
{
    bool (*meas_init)(led_channel_e);                           /**< 初始化接口 */
    bool (*meas_gain_config)(led_channel_e, uint8_t *);         /**< 增益配置接口 */
    bool (*meas_voltage_code_get)(meas_volt_type_e, uint16_t *);/**< 电压码值获取接口 */
    bool (*meas_volt_calc_func)(meas_volt_type_e, uint16_t, int16_t *); /**< 电压计算接口 */

    bool (*meas_pn_code_read)(led_channel_e);                   /**< PN码值读取接口 */
    bool (*meas_pn_voltage_get)(led_channel_e, rgb_type_e, uint16_t *); /**< PN电压获取接口 */
    bool (*meas_pn_calc_func)(led_channel_e, rgb_type_e, uint16_t, int16_t *); /**< PN电压计算接口 */

    bool (*meas_pn_status_reflash)(led_channel_e);              /**< PN状态刷新接口 */
    bool (*meas_pn_process)(led_channel_e, uint32_t);           /**< PN采样处理接口 */
    bool (*meas_pn_suspend)(led_channel_e);                     /**< PN暂停接口 */
    bool (*meas_pn_resume)(led_channel_e);                      /**< PN恢复接口 */
    bool (*meas_pn_acquire)(led_channel_e);                     /**< PN采集接口 */
    bool (*meas_pn_status_get)(led_channel_e, led_vf_status_type_e, uint8_t *); /**< PN状态获取接口 */
    bool (*meas_pn_status_set)(led_channel_e, led_vf_status_type_e, uint8_t); /**< PN状态设置接口 */
    bool (*meas_pn_threshold_get)(led_channel_e, rgb_type_e, rgb_safty_threshold_t *); /**< PN阈值获取接口 */
} meas_manager_instance_t;

#ifdef __cplusplus
}
#endif

#endif /*__PAL_MEAS_DEF_H__*/
