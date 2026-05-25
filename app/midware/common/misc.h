/**
*****************************************************************************
* @brief  misc header
* @file   misc.h
* @author AE/FAE team
* @date   28/JUL/2023
*****************************************************************************
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <b>&copy; Copyright (c) 2023 Tinychip Microelectronics Co.,Ltd.</b>
*
*****************************************************************************
*/

#ifndef MISC_H__
#define MISC_H__

#include "tcae10_ll_adc.h"

/**
 * @brief  获取VBAT电压值（单位mV），通过ADC多次采样取平均
 * @param  vref - ADC参考电压选择
 * @param  sampCount - 采样次数，多次采样求平均
 * @retval VBAT电压值（mV），参数无效时返回0
 */
int GetVbatMv(adc_vref_e vref, int sampCount);

/**
 * @brief  获取温度传感器ADC原始码值，多次采样求平均
 * @param  index - 温度传感器编号（0或1）
 * @param  vref - ADC参考电压选择
 * @param  sampCount - 采样次数，多次采样求平均
 * @retval ADC原始码平均值
 */
int GetTempCode(uint8_t index, adc_vref_e vref, int sampCount);

/**
 * @brief  将LIN高压口配置为GPIO模式
 * @param  pullup_enable - 内部30k上拉电阻使能
 */
void LinAsGpioInit(bool pullup_enable);

/**
 * @brief  设置LIN口GPIO输出电平
 * @param  state - 输出状态（true=高，false=低）
 * @note   输出0时电压到不了0V：上拉10k时约0.7V，上拉30k时约0.6V
 */
void LinAsGpioOutput(bool state);

/**
 * @brief  独立看门狗初始化
 */
void WdgInit(void);

/**
 * @brief  打印复位原因（从ASYSCFG寄存器读取）
 */
void PrintRstCause(void);

/**
 * @brief  RTC定时器触发配置（使用TIM_LITE）
 * @param  freq - 中断频率（Hz）
 * @param  sw - 开关（1开启，0关闭）
 */
void RtcTrigConfig(uint8_t freq, uint8_t sw);

/**
 * @brief  将GPIO6配置为外部ADC参考电压输入
 */
void AdcExtVrefInit(void);

/**
 * @brief  VS电源LVD阈值选择
 * @note   rising=电压上升阈值，falling=电压下降阈值
 */
typedef enum
{
    LVD_THD_R40F35 = 0,         /**< 上升4.0V，下降3.5V */
    LVD_THD_R45F40,             /**< 上升4.5V，下降4.0V */
    LVD_THD_R50F45,             /**< 上升5.0V，下降4.5V */
    LVD_THD_R55F50,             /**< 上升5.5V，下降5.0V */
    LVD_THD_R60F55,             /**< 上升6.0V，下降5.5V */
    LVD_THD_R65F60,             /**< 上升6.5V，下降6.0V */
    LVD_THD_R70F65,             /**< 上升7.0V，下降6.5V */
    LVD_THD_R75F70,             /**< 上升7.5V，下降7.0V */
} vs_lvd_threshold_e;

/**
 * @brief  VS电源LVD中断触发方式
 * @note   LVD中断信号：低于阈值=1，高于阈值=0；
 *         电压从高到低跌落时应选择上升沿中断
 */
typedef enum
{
    LVD_INT_POSEDGE = 0,        /**< 上升沿中断 */
    LVD_INT_NEGEDGE,            /**< 下降沿中断 */
    LVD_INT_HIGHLEVEL,          /**< 高电平中断 */
    LVD_INT_NONE                /**< 无中断 */
} vs_lvd_interrupt_e;

/**
 * @brief  VS电源LVD初始化配置
 * @param  threshold - LVD阈值选择
 * @param  int_type - 中断触发方式
 */
void VsLvdInit(vs_lvd_threshold_e threshold, vs_lvd_interrupt_e int_type);

#endif
