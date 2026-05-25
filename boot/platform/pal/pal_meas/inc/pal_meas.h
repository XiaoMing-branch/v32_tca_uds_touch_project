/**
 *****************************************************************************
 * @brief   pal meas header file.
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

#ifndef __PAL_MEAS_H__
#define __PAL_MEAS_H__

#include "pal_func_def.h"
#include "pal_meas_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_SUPPORT_MEAS_INSTANNCE  1
#if CFG_SUPPORT_MEAS_INSTANNCE
#define MEAS_INSTANCE   static
#define MEAS_LED_CHANNEL_INVALID(channel) (false)
#else
#define MEAS_INSTANCE
#define MEAS_LED_CHANNEL_INVALID(channel) ((channel>=LED_CHANNEL_MAX) ? true:false)
#endif

#if defined (__TCPL01X__)
#define LED_VF_SAMP_CALC_NUM    (1u)
#elif defined __TCPL03X__
#define LED_VF_SAMP_CALC_NUM    (3u)
#endif

#if CFG_SUPPORT_MULTIPLEX_LED
#define MEAS_MULTIPLEX_NUM            2
#else
#define MEAS_MULTIPLEX_NUM            1
#endif
/**
 * @brief  测量电压类型枚举
 * @note   定义温度/电池/PN结电压测量类型
 */
typedef enum
{
    MEAS_VOLT_TEMP = 0U,
    MEAS_VOLT_VBAT,
    MEAS_VOLT_PN,
    MEAS_VOLT_PN_0 = MEAS_VOLT_PN,
    MEAS_VOLT_PN_1,
    MEAS_VOLT_PN_2,
    MEAS_VOLT_MAX,
} meas_volt_type_e;

/**
 * @brief  测量触发源枚举
 * @note   定义PWM通道的清除/计数最大值触发源
 */
typedef enum
{
    CH0_TRIG_PWM_CLR,
    CH0_TRIG_CNT_MAX,
    CH1_TRIG_PWM_CLR,
    CH1_TRIG_CNT_MAX,
    CH2_TRIG_PWM_CLR,
    CH2_TRIG_CNT_MAX,
    CH_TRIG_SRC_MAX,
} trig_src_e;

/**
 * @brief  LED VF状态标志枚举
 * @note   定义PN结电压采样的数据就绪/暂停/采集/静态采样状态
 */
typedef enum
{
    LED_VF_DATA_STATUS = (0x01U << 1),          /**< RGB PN结电压数据就绪 */
    LED_VF_SUSPEND_STATUS = (0x01U << 2),       /**< PN结电压采样暂停标志 */
    LED_VF_ACQUIRE_STATUS = (0x01U << 3),       /**< PN结电压采集标志 */
    LED_VF_STATIC_SAMP_STATUS = (0x01U << 4),   /**< PN结电压静态采样标志 */
} led_vf_status_type_e;

/**
 * @brief  VF通道状态结构体
 * @note   表示每个PWM通道的数据就绪状态
 */
typedef struct
{
    bool data_ready;
} vf_channel_status_t;

/**
 * @brief  VF采样状态位域结构体
 * @note   位域方式表示采样状态
 */
typedef struct
{
    uint16_t data_status : 1;
    uint16_t suspend_status : 1;
    uint16_t acquire_status : 1;
    uint16_t static_samp_status : 1;
} vf_sample_status_t;

/**
 * @brief  VF采样上下文结构体
 * @note   保存PWM通道、触发源、通道状态、VF码值及采样状态
 *         TCPL03X: vf_code[][0]=VF, [1]=AON_T温度, [2]=AON
 */
typedef struct
{
    uint8_t channel;
    trig_src_e trig_src;
    vf_channel_status_t ch_status[LED_TYPE_MAX];
    uint16_t vf_code[LED_TYPE_MAX][LED_VF_SAMP_CALC_NUM];
    union
    {
        uint16_t vf_status;
        vf_sample_status_t vf_status_bit;
    };

} vf_sample_ctx_t;

/**
 * @brief  LED测量上下文结构体
 * @note   保存采样就绪标志、RGB VF映射和VF采样上下文
 *         vf_samp为PWM0-2三通道(未做RGB映射)
 */
typedef struct
{
    bool sample_ready;
    uint8_t *rgb_vf_mux;
    vf_sample_ctx_t vf_samp;

} led_measure_context_t;

/**
 * @brief  LED RGB安全阈值结构体
 * @note   定义RGB通道的上下限阈值
 */
typedef struct
{
    uint16_t ceil_value;
    uint16_t floor_value;
} rgb_safty_threshold_t;

typedef struct
{
    uint16_t value_l;
    uint16_t value_h;
} sft_adpat_value_t;


#if !CFG_SUPPORT_MEAS_INSTANNCE
bool led_meas_init(led_channel_e channel);
bool led_meas_gains_config(led_channel_e channel);
bool led_meas_voltage_code_get(meas_volt_type_e type, uint16_t *value);
bool led_meas_volt_calc_func(meas_volt_type_e type, uint16_t raw_code, int16_t *value);

bool led_meas_pn_voltage_get(led_channel_e channel);
bool led_meas_pn_calc_func(led_channel_e channel, led_type_e rgb, int16_t *value);
bool led_meas_pn_status_reflash(led_channel_e channel);
bool led_meas_pn_process(led_channel_e channel, uint32_t pwm_isr);
bool led_meas_pn_suspend(led_channel_e channel);
bool led_meas_pn_resume(led_channel_e channel);
bool led_meas_pn_acquire(led_channel_e channel);
bool led_meas_pn_status_get(led_channel_e channel, led_vf_status_type_e type, uint8_t *status);
bool led_meas_pn_status_set(led_channel_e channel, led_vf_status_type_e type, uint8_t status);
#endif

#ifdef __cplusplus
}
#endif
#endif /*__PAL_MEAS_H__*/
