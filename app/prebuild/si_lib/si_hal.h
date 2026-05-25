/**
*****************************************************************************
* @brief  si hal header
* @file   si_hal.h
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

#ifndef SI_HAL_H__
#define SI_HAL_H__

typedef struct
{
    uint8_t enable;             //1表示开启倍采样功能
    T_SiData dummy1;
    T_SiData dummy2;
} TOUCH_HalDoubleSamp_Type;     //倍采样

typedef struct
{
    uint16_t coarseTune;                      //粗调
    uint16_t fineTune;                        //微调
    uint16_t fineStep;                        //微调步进，建议为1
    T_SiData dummy1;
    T_SiData dummy2;
} TOUCH_HalNoiseAvoid_Type;     //噪音抑制器

typedef struct
{
    uint8_t gain;               //增益器，建议为1，0表示关闭
    T_SiData dummy1;
    T_SiData dummy2;
} TOUCH_HalLowPassFilter_Type;  //低通滤波器

T_SiData calc_double_samp(TOUCH_HalDoubleSamp_Type *double_samp_table, uint8_t channel, T_SiData cur_data, int is_sleep);    //计算倍采样值，is_sleep为1表示为sleep通道
T_SiData calc_noise_avoid(TOUCH_HalNoiseAvoid_Type *noise_avoid_table, uint8_t channel, T_SiData cur_data, int is_sleep);    //计算噪音抑制器值，is_sleep为1表示为sleep通道
T_SiData calc_low_pass(TOUCH_HalLowPassFilter_Type *low_pass_table, uint8_t channel, T_SiData cur_data, int is_sleep);    //计算低通滤波器值，is_sleep为1表示为sleep通道

#endif
