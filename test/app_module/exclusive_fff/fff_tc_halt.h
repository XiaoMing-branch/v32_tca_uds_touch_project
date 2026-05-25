/**
*****************************************************************************
* @brief  tc halt header
* @file   tc_halt.h
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

#ifndef __FFF_TC_HALT_H__
#define __FFF_TC_HALT_H__

#include "fff.h"
#include <stdint.h>

#define MAX_FILTER_WAKEUP_CALLBACK_NUM 3       // 最大注册wakeup过滤函数个数
#define ENTER_HALT_TIMEOUT_MS 500              // 多长时间超时，进入低功耗
#define DISABLE_SWD_AFTER_POWERON_TIMEMS 60000 // 通电后多长时间关闭SWD功能，0表示永不关闭

extern volatile uint8_t touchHaltRtcTrigFlag; // 触摸低功耗rtc触发计数器

/*低功耗唤醒后，调用此过滤函数，如果函数返回1，正式从低功耗唤醒，否则返回0继续进入低功耗*/
typedef int (*HALT_FILTER_WAKEUP_CALLBACK)(void);

DECLARE_FAKE_VOID_FUNC(HaltInit);
DECLARE_FAKE_VALUE_FUNC(int, HaltFilterWakeupRegister, HALT_FILTER_WAKEUP_CALLBACK);
DECLARE_FAKE_VOID_FUNC(HaltFilterWakeupClear);
DECLARE_FAKE_VOID_FUNC(HaltTimeoutChgPeriod, uint32_t);
DECLARE_FAKE_VOID_FUNC(HaltTimeoutReset);

// void HaltInit(void);        //低功耗任务初始化

// int HaltFilterWakeupRegister(HALT_FILTER_WAKEUP_CALLBACK callback); //注册低功耗唤醒后过滤函数,1成功，-1注册缓冲区已满，最大注册个数见：MAX_FILTER_WAKEUP_CALLBACK_NUM
// void HaltFilterWakeupClear(void);       //清空注册的过滤函数

// void HaltTimeoutChgPeriod(uint32_t periodTick);       //修改进入低功耗超时定时器的超时周期
// void HaltTimeoutReset(void);                                        //重置进入低功耗超时定时器的超时时间

#define MAX_HALT_MONITOR_CALLBACK_NUM 2 // 最大注册halt监控器个数
#define HALT_MONITOR_RUN_INTVAL 3       // 多长时间halt监控器运行一次，单位为rtc wave的cnt值，0-15，越小间隔越大

/*halt监控器回调接口，每HALT_MONITOR_RUN_INTVAL时间被调用一次*/
typedef void (*HALT_MONITOR_CALLBACK)(void);

DECLARE_FAKE_VALUE_FUNC(int, HaltMonitorRegister, HALT_MONITOR_CALLBACK);
// int HaltMonitorRegister(HALT_MONITOR_CALLBACK callback); // 注册halt监控器,1成功，-1注册缓冲区已满，最大注册个数见：MAX_HALT_MONITOR_CALLBACK_NUM

/*
type:进入低功耗的类型，见：SLEEP_MODE_E
v:对应的参数，暂未使用，填0
*/

DECLARE_FAKE_VALUE_FUNC(int, HaltEnter, uint16_t, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint8_t, IsHaltMode);
DECLARE_FAKE_VALUE_FUNC(uint16_t, GetHaltMode);
DECLARE_FAKE_VOID_FUNC(SetHaltMode, uint16_t);
// int HaltEnter(uint16_t type, uint16_t v); // 进入低功耗，返回1表示进入低功耗成功，返回-1表示失败

// uint8_t IsHaltMode(void); // 是否处在halt模式

// uint16_t GetHaltMode(void);      // 获取低功耗模式
// void SetHaltMode(uint16_t mode); // 设置低功耗模式

#endif
