/**
*****************************************************************************
* @brief  tc halt source
* @file   tc_halt.c
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

#include "fff_tcae10_ll_def.h"
#include "fff_tc.h"
#include "fff_tc_log.h"
#include "fff_tc_halt.h"
#include "fff_tc_usermsg.h"

static const char *TAG = "HALT";

#define EMC_TEST_EN     0

#define GEN_HALTMSG_PARAM(type,v)   (((uint32_t)(type)<<16) | (v))
#define PARSE_HALTMSG_PARAM_TYPE(param)     ((uint16_t)((param)>>16))
#define PARSE_HALTMSG_PARAM_VALUE(param)    ((uint16_t)((param)&0xFFFFU))

volatile uint8_t touchHaltRtcTrigFlag = 0;           //触摸低功耗rtc触发计数器
static uint8_t inHaltFlag = 0;  //低功耗标记
static int haltMonitorTimerCount = 0;

static struct
{
    uint8_t num;
    HALT_FILTER_WAKEUP_CALLBACK callback[MAX_FILTER_WAKEUP_CALLBACK_NUM];
} filterWakeupCallback = {0};               //低功耗唤醒后过滤函数

static T_TcTask *haltTask = NULL;
static T_TcTimer *haltTimeoutTimer = NULL;            //低功耗超时定时器

static struct
{
    uint8_t num;
    uint8_t waveFlag;       //检测到RTC wave标志，需要调用回调接口
    HALT_MONITOR_CALLBACK callback[MAX_HALT_MONITOR_CALLBACK_NUM];
} haltMonitorCallback = {0};               //halt监控器回调接口

static uint16_t haltmode = SLEEPWALK_MODE;

static void HaltTask(uint32_t msg, void *param);        //低功耗任务
static void LowLevelHaltEnter(uint16_t type, uint16_t v);   //底层进入低功耗
static void SystemLowPowerInit(void);                   //降低功耗相关

static void HaltMonitorRtcWaveSw(uint8_t sw);       //halt监控器rtc wave开关

static void SetSwdDisableOnDemand(uint8_t disable);      //按需开关swd功能


DEFINE_FAKE_VOID_FUNC(HaltInit);
DEFINE_FAKE_VALUE_FUNC(int, HaltFilterWakeupRegister, HALT_FILTER_WAKEUP_CALLBACK);
DEFINE_FAKE_VOID_FUNC(HaltFilterWakeupClear);
DEFINE_FAKE_VOID_FUNC(HaltTimeoutChgPeriod, uint32_t);
DEFINE_FAKE_VOID_FUNC(HaltTimeoutReset);
DEFINE_FAKE_VALUE_FUNC(int, HaltMonitorRegister, HALT_MONITOR_CALLBACK);
DEFINE_FAKE_VALUE_FUNC(int, HaltEnter, uint16_t, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint8_t, IsHaltMode);
DEFINE_FAKE_VALUE_FUNC(uint16_t, GetHaltMode);
DEFINE_FAKE_VOID_FUNC(SetHaltMode, uint16_t);
