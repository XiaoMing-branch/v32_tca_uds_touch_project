/**
 *****************************************************************************
 * @brief   adc driver header.
 *
 * @file    tcae10_ll_adc.h
 * @author
 * @date    2024.04.20
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */
#ifndef __TCAE10_LL_ADC_H__
#define __TCAE10_LL_ADC_H__

#include "tcae10_ll_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief ADC ISR FUNC FLAG
*/
/* ADC int1 */
#define ADC_INT_CMP_FLAG             (0x00000001UL) //Mask ADC compare interrupt
#define ADC_INT_DONE_FLAG            (0x00000002UL) //Mask ADC trig done interrupt
#define ADC_INT_FIFO_OVF_FLAG        (0x00000004UL) //Mask ADC fifo overflow interrupt
#define ADC_INT_FIFO_RDY_FLAG        (0x00000008UL) //Mask ADC TRIG DONE data ready interrupt
#define ADC_INT_FIFO_UNF_FLAG        (0x00000010UL) //Mask ADC fifo underflow interrupt

/* power check int0*/
// #define OTP_INT_FLAG                 AFE_SYSCFG_IMR0_OTP_INT_MSK_MASK                        //Mask OTP interrupt
// #define VS_LVD_INT_FLAG              AFE_SYSCFG_IMR0_VS_LVD_INT_MSK_MASK                     //Mask VS LVD interrupt

typedef enum
{
    ADC_CHANNEL_TAO_TEST = 0,
    ADC_CHANNEL_VBAT,
    ADC_CHANNEL_VC0,
    ADC_CHANNEL_VC1,
    ADC_CHANNEL_VC2,
    ADC_CHANNEL_VPN0,
    ADC_CHANNEL_VPN1,
    ADC_CHANNEL_VPN2,
    ADC_CHANNEL_TEMP,
    ADC_CHANNEL_TEMP1,
    ADC_CHANNEL_LIN,
    ADC_CHANNEL_IO2_IO4,
    ADC_CHANNEL_IO4_GND,
    ADC_CHANNEL_IO2_GND,
    ADC_CHANNEL_TOUCH,
    ADC_CHANNEL_VCR,
    ADC_CHANNEL_MAX,
} adc_channel_e;

/**
 * @brief  ll adc temp channel enumeration
 */
typedef enum
{
    TEMP_CHANNEL_0 = 0,
    TEMP_CHANNEL_1,
    TEMP_CHANNEL_MAX,
} temp_channel_e;

/**
 * @brief  ll adc scan channel enumeration
 */
typedef enum
{
    ADC_SCAN_CHANNEL_0 = 0,
    ADC_SCAN_CHANNEL_1,
    ADC_SCAN_CHANNEL_2,
    ADC_SCAN_CHANNEL_3,
    ADC_SCAN_CHANNEL_4,
    ADC_SCAN_CHANNEL_5,
    ADC_SCAN_CHANNEL_6,
    ADC_SCAN_CHANNEL_7,
    ADC_SCAN_CHANNEL_MAX,
} adc_scan_channel_e;

/**
 * @brief  ll adc  trigger mode enumeration
 */
typedef enum
{
    TRIG_SOFTWARE        = 0U,
    TRIG_PWM0_RISEEDGE   = 0x01U,
    TRIG_PWM0_FALLEDGE   = 0x02U,
    TRIG_PWM1_RISEEDGE   = 0x04U,
    TRIG_PWM1_FALLEDGE   = 0x08U,
    TRIG_PWM2_RISEEDGE   = 0x10U,
    TRIG_PWM2_FALLEDGE   = 0x20U,
    TRIG_TOUCH           = 0x80U,
    TRIG_TIMER           = 0x100U,
    TRIG_LIN             = 0x200U,
} adc_trig_mode_e;

typedef enum
{
    ADC_VCR_SEL_236_7 = 0,
    ADC_VCR_SEL_315_6,
    ADC_VCR_SEL_394_54,
    ADC_VCR_SEL_437_45,
    ADC_VCR_SEL_552_4,
    ADC_VCR_SEL_631_3,
    ADC_VCR_SEL_710_17,
    ADC_VCR_SEL_789_1,
    ADC_VCR_MAX,
} adc_vcr_e;

typedef enum
{
    ADC_GAIN_X1  = 0,
    ADC_GAIN_X2,
    ADC_GAIN_X3,
    ADC_GAIN_X4,
    ADC_GAIN_X5,
    ADC_GAIN_X6,
    ADC_GAIN_X7,
    ADC_GAIN_X8,
    ADC_GAIN_X9,
    ADC_GAIN_X10,
    ADC_GAIN_X11,
    ADC_GAIN_X12,
    ADC_GAIN_X13,
    ADC_GAIN_X14,
    ADC_GAIN_X15,
    ADC_GAIN_X16,
    ADC_GAIN_MAX,
} adc_gain_e;

typedef enum
{
    ADC_VCM_SEL_NULL = 0,
    ADC_VCM_SEL_165,
    ADC_VCM_SEL_125,
    ADC_VCM_SEL_205,
    ADC_VCM_MAX,
} adc_vcm_e;

typedef enum
{
    ADC_IBIAS_1x = 0,
    ADC_IBIAS_0p5x
} adc_ibais_e;

typedef enum
{
    ADC_VREF_2500 =  0,
    ADC_VREF_2000,
    ADC_VREF_1500,
    ADC_VREF_EXT,
    ADC_VREF_MAX,
} adc_vref_e;

/**
 * @brief  ll adc scan config struct
 */
typedef union
{
    uint16_t    scan_channel_cfg;
    struct
    {
        uint16_t channel            : 5;
        uint16_t pga_bypass         : 1;
        uint16_t pga_bufn_bypass    : 1;
        uint16_t pga_bufp_bypass    : 1;
        uint16_t pga_gain           : 4;
        uint16_t reserved           : 4;
    } scan_config_bit;
} adc_scan_config_t;
typedef struct
{
    uint8_t ratio;

#if 1
    bool adc_buf_en;
    bool adc_buf_bypass;

    bool adc_pag_en;
    bool adc_pag_bypass;
#endif
    bool vcr_enable;
    adc_vcr_e vcr;
    adc_gain_e gain;

    adc_vcm_e vcm;
    adc_vref_e vref;
} adc_cfg_t;

/**
 * @brief  ll adc enumeration
 */
typedef struct
{
    ll_clk_config_t clk_cfg;
    ll_isr_config_t isr_cfg;
    uint8_t trig_num;
    adc_trig_mode_e trig_mode;
} adc_config_t;

/**
 * @brief  获取ADC原始码（从FIFO数据寄存器读取）
 * @retval ADC转换原始码（12位有符号数）
 */
static __INLINE int16_t ll_adc_getcode(void)
{
    return ((int16_t)(ADC->FIFO_DATA << 2)) >> 2;
}

/**
 * @brief  去初始化ADC模块
 */
void ll_adc_deinit(void);
/**
 * @brief  初始化ADC模块
 * @param config - ADC配置参数结构体指针
 * @param callback - 中断回调函数指针
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_init(adc_config_t *config, ISR_FUNC_CALLBACK callback);
/**
 * @brief  配置ADC参考电压
 * @param vref - 参考电压选择 @ref adc_vref_e
 * @retval LL_OK 成功，LL_PARAM_INVALID 参数无效
 */
ll_status_e ll_adc_vref_config(adc_vref_e vref);
/**
 * @brief  配置ADC通道增益
 * @param channel - ADC通道选择 @ref adc_channel_e
 * @param cfg - ADC配置参数结构体指针（含增益、VCR等）
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_gain_config(adc_channel_e channel, adc_cfg_t *cfg);
/**
 * @brief  使能/禁能ADC中断
 * @param enable - true: 使能，false: 禁能
 * @retval LL_OK 成功
 */
ll_status_e ll_adc_isr_enable(bool enable);
/**
 * @brief  使能/禁能LIN自动寻址ADC触发
 * @param type - LIN自动寻址类型 @ref lin_aa_type_e
 * @param enable - true: 使能，false: 禁能
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_lin_aa_enable(lin_aa_type_e type, bool enable);
/**
 * @brief  选择ADC通道并配置参数
 * @param channel - ADC通道 @ref adc_channel_e
 * @param cfg - ADC配置参数（增益、VCM、VREF等）
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_select_channel(adc_channel_e channel, adc_cfg_t *cfg);
/**
 * @brief  获取ADC FIFO当前数据长度
 * @retval FIFO中有效数据个数
 */
uint8_t ll_adc_fifo_length_get(void);
/**
 * @brief  从ADC FIFO读取数据
 * @param buffer - 数据缓冲区指针
 * @param length - 要读取的数据个数
 * @retval 实际读取的数据个数
 */
uint16_t ll_adc_fifo_get(uint16_t *buffer, uint16_t length);
/**
 * @brief  清除ADC FIFO数据
 * @retval 清除的数据个数
 */
uint16_t ll_adc_fifo_clear(void);
/**
 * @brief  读写ADC控制寄存器
 * @param value - 写入的值或读出的值指针
 * @param is_write - true: 写入，false: 读取
 * @retval LL_OK 成功
 */
ll_status_e ll_adc_crtl_config(uint32_t *value, bool is_write);
/**
 * @brief  软件触发ADC启动
 * @param enable - true: 启动，false: 停止
 */
void ll_adc_softwart_start(bool enable);
/**
 * @brief  计算ADC电压因子（VF）值
 * @param channel - ADC通道 @ref adc_channel_e
 * @param buffer - ADC原始数据缓冲区
 * @param value - 计算后的VF值输出指针
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_vf_calculate_func(adc_channel_e channel, uint16_t *buffer, uint16_t *value);
/**
 * @brief  计算ADC电压值
 * @param code - ADC转换原始码
 * @param cfg - ADC配置参数（参考电压、增益等）
 * @param value - 计算后的电压值输出指针（单位mV）
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_volt_calculate_func(int16_t code, adc_cfg_t *cfg, uint16_t *value);
/**
 * @brief  计算VBAT电池电压
 * @param code - ADC转换原始码
 * @param value - 计算后的电压值输出指针（单位mV）
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_vbat_calculate_func(int16_t code, uint16_t *value);
/**
 * @brief  计算ADC温度传感器值
 * @param channel - 温度通道 @ref temp_channel_e
 * @param code - ADC转换原始码
 * @param cfg - ADC配置参数
 * @param value - 计算后的温度值输出指针
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_temp_calculate_func(temp_channel_e channel, int16_t code, adc_cfg_t *cfg,  uint16_t *value);
/**
 * @brief  简化温度值计算
 * @param channel - 温度通道 @ref temp_channel_e
 * @param code - ADC转换原始码
 * @retval 计算后的温度值
 */
int ll_adc_temp_calculate(temp_channel_e channel, int16_t code);
/**
 * @brief  启动指定ADC通道转换
 * @param channel - ADC通道 @ref adc_channel_e
 * @param cfg - ADC配置参数
 * @param buffer - 转换结果缓冲区
 * @param trig_num - 触发次数
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_adc_channnel_start(adc_channel_e channel, adc_cfg_t *cfg, uint16_t *buffer, uint8_t trig_num);
/**
 * @brief  使能/禁能ADC温度传感器
 * @param enable - true: 使能，false: 禁能
 * @retval LL_OK 成功
 */
ll_status_e ll_adc_tsensor_enable(bool enable);
/**
 * @brief  使能/禁能ADC偏置控制
 * @param enable - true: 使能，false: 禁能
 * @retval LL_OK 成功
 */
ll_status_e ll_bias_control_enable(bool enable);

#ifdef __cplusplus
}
#endif
#endif /* __TCAE10_LL_ADC_H__ */
