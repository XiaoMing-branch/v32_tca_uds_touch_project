/**
*****************************************************************************
* @brief  si touch port header
* @file   si_touch_port.h
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

#ifndef SI_TOUCH_PORT_H__
#define SI_TOUCH_PORT_H__

#include "tc.h"
#include "touch_haldispatch.h"

extern T_TcTask *touchTaskHandle;               //touch任务
extern TOUCH_HalDispatch_Type *touchDispatch;           //touch分发器

//触摸初始化
void TouchInit(void);

//低功耗唤醒源过滤回调函数
int TouchHaltFilterWakeupCallback(void);

//锁定管理低功耗的T_SiObject
void TouchHaltLockLpSiObject(void);
//解锁管理低功耗的T_SiObject
void TouchHaltUnlockLpSiObject(void);

//锁定常规的T_SiObject
void TouchHaltLockSiObject(void);
//解锁常规的T_SiObject
void TouchHaltUnlockSiObject(void);

//使能touch采集，1表示开启，0表示关闭
void TouchEnableSamp(uint8_t enable);

//强制将touch从低功耗唤醒
void TouchForceWakeup(void);

//强制touch扫描并运行一次算法
void TouchForceRunAlgoOnce(void);

//touch中断强制设置MSG_TASK_BITFLAG，1表示强制设置，0表示不强制
void TouchSetTaskBitForce(uint8_t force);

#endif
