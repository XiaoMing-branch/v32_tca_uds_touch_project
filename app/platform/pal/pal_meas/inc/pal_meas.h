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
#define MEAS_LED_CHANNEL_INVALID(channel) ((channel>=LED_CHANNLE_MAX) ? true:false)
#endif

/**
 * @brief 测量电压类型枚举
 */
typedef enum
{
    MEAS_VOLT_TEMP = 0U,    /**< 温度传感器电压 */
    MEAS_VOLT_VBAT,         /**< 电池电压 */
    MEAS_VOLT_PN,           /**< PN结电压起始 */
    MEAS_VOLT_PN_0 = MEAS_VOLT_PN, /**< 通道0 PN结电压 */
    MEAS_VOLT_PN_1,         /**< 通道1 PN结电压 */
    MEAS_VOLT_PN_2,         /**< 通道2 PN结电压 */
    MEAS_VOLT_MAX,          /**< 类型最大值 */
} meas_volt_type_e;

/**
 * @brief PWM触发源枚举（选择PWM中断触发ADC采样的时机）
 */
typedef enum
{
    CH0_TRIG_PWM_CLR,   /**< 通道0 PWM清零时刻触发 */
    CH0_TRIG_CNT_MAX,   /**< 通道0 PWM计数最大值触发 */
    CH1_TRIG_PWM_CLR,   /**< 通道1 PWM清零时刻触发 */
    CH1_TRIG_CNT_MAX,   /**< 通道1 PWM计数最大值触发 */
    CH2_TRIG_PWM_CLR,   /**< 通道2 PWM清零时刻触发 */
    CH2_TRIG_CNT_MAX,   /**< 通道2 PWM计数最大值触发 */
    CH_TRIG_SRC_MAX,    /**< 触发源最大值 */
} trig_src_e;

/**
 * @brief VF采样状态标志枚举（位标志，支持组合）
 */
typedef enum
{
    LED_VF_SAMPLE_STATUS = (0x01U << 0),        /**< PN结采样进行中标志 */
    LED_VF_DATA_STATUS = (0x01U << 1),          /**< RGB PN结数据就绪标志 */
    LED_VF_SUSPEND_STATUS = (0x01U << 2),       /**< PN结采样暂停标志 */
    LED_VF_ACQUIRE_STATUS = (0x01U << 3),       /**< PN结采集进行中标志 */
    LED_VF_STATIC_SAMP_STATUS = (0x01U << 4),   /**< 静态采样标志 */
} led_vf_status_type_e;

/**
 * @brief VF通道状态结构体（记录各通道亮度和数据就绪状态）
 */
typedef struct
{
    uint16_t intensity;  /**< 通道亮度（PWM占空比） */
    bool data_ready;     /**< 数据就绪标志 */
} vf_channel_status_t;

/**
 * @brief VF采样状态位域结构体
 */
typedef struct
{
    uint16_t sample_status : 1;       /**< 采样进行中 */
    uint16_t data_status : 1;         /**< 数据就绪 */
    uint16_t suspend_status : 1;      /**< 暂停状态 */
    uint16_t acquire_status : 1;      /**< 采集中 */
    uint16_t static_samp_status : 1;  /**< 静态采样 */
} vf_sample_status_t;

/**
 * @brief VF采样上下文结构体，管理采样通道、触发源、状态和数据
 */
typedef struct
{
    uint8_t channel;                  /**< 当前采样PWM通道 */
    trig_src_e trig_src;              /**< PWM触发源选择 */
    vf_channel_status_t ch_status[RGB_TYPE_MAX]; /**< 各通道状态 */
    uint16_t vf_code[RGB_TYPE_MAX];   /**< VF原始码值 */
    union
    {
        uint16_t vf_status;           /**< 状态字（整体访问） */
        vf_sample_status_t vf_status_bit; /**< 状态字（位域访问） */
    };

} vf_sample_ctx_t;

/**
 * @brief LED测量上下文结构体，管理灯珠参数和VF采样
 */
typedef struct
{
    uint8_t serial_num;    /**< 灯珠串联数 */
    uint8_t *rgb_vf_mux;   /**< RGB通道到VF通道的映射表 */
    vf_sample_ctx_t vf_samp; /**< VF采样上下文 */
} led_measure_context_t;

/**
 * @brief RGB安全阈值结构体（用于PN结电压异常检测）
 */
typedef struct
{
    uint16_t ceil_value;   /**< 上限阈值 */
    uint16_t floor_value;  /**< 下限阈值 */
} rgb_safty_threshold_t;

#if !CFG_SUPPORT_MEAS_INSTANNCE
/**
 * @brief  测量通道初始化
 * @param  channel - LED通道
 * @retval true 成功 false 失败
 */
bool led_meas_init(led_channel_e channel);
/**
 * @brief  增益配置
 * @param  channel    - LED通道
 * @param  serial_num - 串数指针
 * @retval true 成功 false 失败
 */
bool led_meas_gains_config(led_channel_e channel, uint8_t *serial_num);
/**
 * @brief  获取电压ADC码值
 * @param  type  - 电压类型
 * @param  value - 输出码值
 * @retval true 成功 false 失败
 */
bool led_meas_voltage_code_get(meas_volt_type_e type, uint16_t *value);
/**
 * @brief  电压码值转实际值
 * @param  type     - 电压类型
 * @param  raw_code - 原始码值
 * @param  value    - 输出电压值
 * @retval true 成功 false 失败
 */
bool led_meas_volt_calc_func(meas_volt_type_e type, uint16_t raw_code, int16_t *value);

/**
 * @brief  读取PN结ADC码值
 * @param  channel - LED通道
 * @retval true 成功 false 失败
 */
bool led_meas_pn_code_read(led_channel_e channel);
/**
 * @brief  获取PN结电压码值
 * @param  channel - LED通道
 * @param  rgb     - RGB索引
 * @param  value   - 输出电压码值
 * @retval true 成功 false 失败
 */
bool led_meas_pn_voltage_get(led_channel_e channel, rgb_type_e rgb, uint16_t *value);

/**
 * @brief  PN结电压计算
 * @param  channel  - LED通道
 * @param  rgb      - RGB索引
 * @param  raw_code - 原始码值
 * @param  value    - 输出电压值
 * @retval true 成功 false 失败
 */
bool led_meas_pn_calc_func(led_channel_e channel, rgb_type_e rgb, uint16_t raw_code, int16_t *value);
/**
 * @brief  刷新PN采样状态
 * @param  channel - LED通道
 * @retval true 成功 false 失败
 */
bool led_meas_pn_status_reflash(led_channel_e channel);
/**
 * @brief  PN采样处理
 * @param  channel - LED通道
 * @param  pwm_isr - PWM中断标志
 * @retval true 成功 false 失败
 */
bool led_meas_pn_process(led_channel_e channel, uint32_t pwm_isr);
/**
 * @brief  暂停PN采样
 * @param  channel - LED通道
 * @retval true 成功 false 失败
 */
bool led_meas_pn_suspend(led_channel_e channel);
/**
 * @brief  恢复PN采样
 * @param  channel - LED通道
 * @retval true 成功 false 失败
 */
bool led_meas_pn_resume(led_channel_e channel);
/**
 * @brief  启动PN采样采集
 * @param  channel - LED通道
 * @retval true 成功 false 失败
 */
bool led_meas_pn_acquire(led_channel_e channel);
/**
 * @brief  获取PN采样状态
 * @param  channel - LED通道
 * @param  type    - 状态类型
 * @param  status  - 输出状态
 * @retval true 成功 false 失败
 */
bool led_meas_pn_status_get(led_channel_e channel, led_vf_status_type_e type, uint8_t *status);
/**
 * @brief  设置PN采样状态
 * @param  channel - LED通道
 * @param  type    - 状态类型
 * @param  status  - 状态值
 * @retval true 成功 false 失败
 */
bool led_meas_pn_status_set(led_channel_e channel, led_vf_status_type_e type, uint8_t status);
/**
 * @brief  获取PN安全阈值
 * @param  channel - LED通道
 * @param  rgb     - RGB索引
 * @param  value   - 输出阈值
 * @retval true 成功 false 失败
 */
bool led_meas_pn_threshold_get(led_channel_e channel, rgb_type_e rgb, rgb_safty_threshold_t *value);
#endif


#ifdef __cplusplus
}
#endif

#endif /*__PAL_MEAS_H__*/
