/**
*****************************************************************************
* @brief  si touch port source
* @file   si_touch_port.c
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

#include "fff_tc.h"
#include "fff_tc_log.h"
#include "fff_tcae10_ll_def.h"
// #include "fff_si_include.h"
#include "fff_touch_config.h"
#include "fff_tc_usermsg.h"
#include "fff_si_touch_port.h"
// #include "fff_touch_tool.h"
#include "fff_tc_halt.h"
#include "fff_app.h"

static const char *TAG = "TOUCH_PORT";

TOUCH_HalDispatch_Type *touchDispatch = NULL;           //touch分发器

T_TcTask *touchTaskHandle = NULL;

static uint8_t enableSamp = 1;       //使能采集touch数据
static uint8_t forceWakeup = 0;     //强制从低功耗唤醒
static uint8_t forceSetTaskBitFlag = 0; //强制设置任务标志

//触摸任务
static void TouchTask(uint32_t msg, void *param);

//touch低功耗监控器
static void TouchHaltMonitorCallback(void);

DEFINE_FAKE_VOID_FUNC(TouchInit);
DEFINE_FAKE_VALUE_FUNC(int, TouchHaltFilterWakeupCallback);
DEFINE_FAKE_VOID_FUNC(TouchHaltLockLpSiObject);
DEFINE_FAKE_VOID_FUNC(TouchHaltUnlockLpSiObject);
DEFINE_FAKE_VOID_FUNC(TouchHaltLockSiObject);
DEFINE_FAKE_VOID_FUNC(TouchHaltUnlockSiObject);
DEFINE_FAKE_VOID_FUNC(TouchEnableSamp, uint8_t);
DEFINE_FAKE_VOID_FUNC(TouchForceWakeup);
DEFINE_FAKE_VOID_FUNC(TouchForceRunAlgoOnce);
DEFINE_FAKE_VOID_FUNC(TouchSetTaskBitForce, uint8_t);