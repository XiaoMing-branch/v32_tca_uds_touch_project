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
 * @brief  LED测量串联方式最大值
 * @note   TCPL01X支持3种串联, TCPL03X支持1种
 */
#if defined (__TCPL01X__)
#define LED_MEAS_SERIAL_MAX     (3u)
#elif defined __TCPL03X__
#define LED_MEAS_SERIAL_MAX     (1u)
#endif

/**
 * @brief  默认LED串联序号
 */
#define DEFAULT_LED_SERIAL_NUM  LED_MEAS_SERIAL_1

/**
 * @brief  LED RGB上下限阈值定义
 */
#ifndef LED_R_CEIL_VALUE
#define LED_R_CEIL_VALUE        (2020)
#endif //LED_R_CEIL_VALUE
#ifndef LED_R_FLOOR_VALUE
#define LED_R_FLOOR_VALUE       (1585)
#endif //LED_R_FLOOR_VALUE

#ifndef LED_G_CEIL_VALUE
#define LED_G_CEIL_VALUE        (3204)
#endif //LED_G_CEIL_VALUE
#ifndef LED_G_FLOOR_VALUE
#define LED_G_FLOOR_VALUE       (2217)
#endif //LED_G_FLOOR_VALUE

#ifndef LED_B_CEIL_VALUE
#define LED_B_CEIL_VALUE        (3032)
#endif //LED_B_CEIL_VALUE
#ifndef LED_B_FLOOR_VALUE
#define LED_B_FLOOR_VALUE       (2415)
#endif //LED_B_FLOOR_VALUE

/**
 * @brief  测量初始化函数指针类型
 */
typedef bool (*meas_init)(led_channel_e);

/**
 * @brief  测量增益配置函数指针类型
 */
typedef bool (*meas_gain_config)(led_channel_e);

/**
 * @brief  电压原始码获取函数指针类型
 */
typedef bool (*meas_voltage_code_get)(meas_volt_type_e, uint16_t *);

/**
 * @brief  电压值计算函数指针类型
 */
typedef bool (*meas_volt_calc_func)(meas_volt_type_e, uint16_t, int16_t *);

/**
 * @brief  PN结电压获取函数指针类型
 */
typedef void (*meas_pn_voltage_get)(led_channel_e);

/**
 * @brief  PN结电压计算函数指针类型
 */
typedef bool (*meas_pn_calc_func)(led_channel_e, led_type_e, uint16_t *, int16_t *);

/**
 * @brief  PN结状态刷新函数指针类型
 */
typedef void (*meas_pn_status_reflash)(led_channel_e);

/**
 * @brief  PN结处理函数指针类型
 */
typedef bool (*meas_pn_process)(led_channel_e, uint32_t);

/**
 * @brief  PN结暂停函数指针类型
 */
typedef bool (*meas_pn_suspend)(led_channel_e);

/**
 * @brief  PN结恢复函数指针类型
 */
typedef bool (*meas_pn_resume)(led_channel_e);

/**
 * @brief  PN结采集函数指针类型
 */
typedef void (*meas_pn_acquire)(led_channel_e);

/**
 * @brief  PN结状态获取函数指针类型
 */
typedef bool (*meas_pn_status_get)(led_channel_e, led_vf_status_type_e, uint8_t *);

/**
 * @brief  PN结状态设置函数指针类型
 */
typedef bool (*meas_pn_status_set)(led_channel_e, led_vf_status_type_e, uint8_t);

/**
 * @brief  测量管理器实例结构体
 * @note   通过函数指针表实现测量控制接口的抽象管理
 */
typedef struct meas_manager_instance_t_
{
    bool (*meas_init)(led_channel_e);
    bool (*meas_gain_config)(led_channel_e);
    bool (*meas_voltage_code_get)(meas_volt_type_e, uint16_t *);
    bool (*meas_volt_calc_func)(meas_volt_type_e, uint16_t, int16_t *);
    bool (*meas_pn_voltage_get)(led_channel_e);
    bool (*meas_pn_calc_func)(led_channel_e, led_type_e, uint16_t *, int16_t *);
    bool (*meas_pn_status_reflash)(led_channel_e);
    bool (*meas_pn_process)(led_channel_e, uint32_t);
    bool (*meas_pn_suspend)(led_channel_e);
    bool (*meas_pn_resume)(led_channel_e);
    bool (*meas_pn_acquire)(led_channel_e);
    bool (*meas_pn_status_get)(led_channel_e, led_vf_status_type_e, uint8_t *);
    bool (*meas_pn_status_set)(led_channel_e, led_vf_status_type_e, uint8_t);
} meas_manager_instance_t;

#ifdef __cplusplus
}
#endif
#endif /*__PAL_MEAS_DEF_H__*/
