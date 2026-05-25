/**
*****************************************************************************
* @brief  tc port header
* @file   tc_port.h
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

#ifndef __FFF_TC_PORT_H__
#define __FFF_TC_PORT_H__

#include "fff.h"
#include "fff_tcae10.h"
#include "fff_tc_conf.h"
#include <stdint.h>

/*系统时钟初始值*/
#define TC_SYSTICK_INIT_VALUE       (0xFFFFD8EF)

#ifndef SYSTICK_FREQ_HZ
    #define SYSTICK_FREQ_HZ     1000       //1K 频率
#endif
#define SYSTICK_COUNTS      (SYSTICK_FREQ_HZ / TC_SYSTICK_HZ)

/*任务系统时钟*/
extern volatile uint32_t TcSystick;

/*硬件相关初始化*/
DECLARE_FAKE_VOID_FUNC(TcPortInit);
// void TcPortInit(void);

/*注册Systick中断回调函数,1表示成功，-1表示失败*/
DECLARE_FAKE_VALUE_FUNC(int, TcSystickCallbackRegister, void *, uint8_t);
// int TcSystickCallbackRegister(void (*callback)(void), uint8_t nPeriod);

DECLARE_FAKE_VOID_FUNC(SysTick_Switch, boolean_t);
// void SysTick_Switch(boolean_t bEn);

typedef struct
{
    uint8_t num;
    uint8_t nPeriod[MAX_SYSTICK_ISR_CALLBACK_NUM];
    uint8_t nPastTime[MAX_SYSTICK_ISR_CALLBACK_NUM];
    void (*callback[MAX_SYSTICK_ISR_CALLBACK_NUM])(void);
} systickCallback_t;

/*获取系统时钟，单位为tick*/
#define TcTimeGet()     TcSystick

extern volatile uint32_t g_TcSystickIntCnt;

/*延迟函数，忙等待延迟，单位为tick*/
DECLARE_FAKE_VOID_FUNC(TcTimeDly, uint32_t);
// void TcTimeDly(uint32_t dly);

/*获取当前时间，单位为us*/
DECLARE_FAKE_VALUE_FUNC(uint64_t, TcGetTimeUS);
// uint64_t TcGetTimeUS(void);

/*获取当前时间，单位为ms*/
DECLARE_FAKE_VALUE_FUNC(uint32_t, TcGetTimeMS);
// uint32_t TcGetTimeMS(void);

#endif
