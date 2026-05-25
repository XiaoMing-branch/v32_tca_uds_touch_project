/**
*****************************************************************************
* @brief  touch tool header
* @file   touch_tool.h
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

#ifndef TOUCH_TOOL_H__
#define TOUCH_TOOL_H__

#include "tcae10.h"

void Touch_IOConfig(uint8_t channel);
//被关闭的touch io口模式为：推挽输出低
void Touch_IOEnable(uint8_t channel, uint8_t enable);
void Touch_Reset(void);

//RTC触发touch配置，sw:1开0关
void TouchRtcTrigConfig(uint8_t freq, uint8_t sw);

uint32_t TouchGetTime(void);            //获取当前时间，单位ms

uint32_t lpParaAdjusterSelectCallback(void);    //低功耗参数调节选择器回调接口，非低功耗返回0，低功耗返回1

#endif
