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

#ifndef __FFF_MISC_H__
#define __FFF_MISC_H__

#include "fff_tcae10_ll_adc.h"

int GetVbatMv(adc_vref_e vref, int sampCount);  //sampCount：采样计数器，多次采样求平均，获取vbat电压，单位mv
int GetTempCode(uint8_t index, adc_vref_e vref, int sampCount);  //index：传感器编号0-1，sampCount：采样计数器，多次采样求平均
int GetTemp(uint8_t index);//index：传感器编号0-1

void LinAsGpioInit(bool pullup_enable);     //lin高压口配置成gpio，pullup_enable内部30k上拉电阻使能
void LinAsGpioOutput(bool state);   //设置lin口输出，注意输出0电压到不了0V（当上拉10k时，输出0电压对应0.7v左右，当上拉30k时，输出0电压对应0.6v左右）

void WdgInit(void);     //看门狗初始化
void PrintRstCause(void);   //打印复位原因

void RtcTrigConfig(uint8_t freq, uint8_t sw);    //RTC触发配置，freq:中断频率,sw:1开0关

void AdcExtVrefInit(void);  //GPIO6作为外部adc vref

typedef enum
{
    LVD_THD_R40F35 = 0,         //rising 4.0v,falling 3.5v
    LVD_THD_R45F40,             //rising 4.5v,falling 4.0v
    LVD_THD_R50F45,             //rising 5.0v,falling 4.5v
    LVD_THD_R55F50,             //rising 5.5v,falling 5.0v
    LVD_THD_R60F55,             //rising 6.0v,falling 5.5v
    LVD_THD_R65F60,             //rising 6.5v,falling 6.0v
    LVD_THD_R70F65,             //rising 7.0v,falling 6.5v
    LVD_THD_R75F70,             //rising 7.5v,falling 7.0v
} vs_lvd_threshold_e;
typedef enum
{
    LVD_INT_POSEDGE = 0,      //posedge irq
    LVD_INT_NEGEDGE,          //negedge irq
    LVD_INT_HIGHLEVEL,        //high level irq
    LVD_INT_NONE              //none irq
} vs_lvd_interrupt_e;         //lvd中断信号低于阈值是1，高于阈值是0，所以电压从高到低跌落，需要选择上升沿中断
void VsLvdInit(vs_lvd_threshold_e threshold,vs_lvd_interrupt_e int_type);

#endif
