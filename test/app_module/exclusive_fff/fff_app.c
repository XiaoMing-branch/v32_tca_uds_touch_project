/**
  ******************************************************************************
  * @brief  application main file.
  *
  * @file   app.c
  * @author AE/FAE team
  * @date
  ******************************************************************************
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
  ******************************************************************************
  */

#include "fff_tcae10.h"
#include "fff_tcae10_ll_def.h"
#include "fff_tc_log.h"
#include "fff_tc_halt.h"
#include "fff_misc.h"
#include "fff_tc.h"
#include "fff_app.h"
#include "fff_lin_task.h"
#include "fff_si_touch_port.h"
#include "fff_touch_config.h"
#include "fff_lin_frame.h"
#include "fff_store_manager.h"
#include "fff_diagnosticIII.h"

STATIC const char *TAG = "APP";

STATIC void AppTask(uint32_t msg, void *param);    //App任务
STATIC void FreeIoSet(void);        //不用io设置，防止低功耗漏电
STATIC void FreePerSet(void);       //关闭不用外设，降低功耗

STATIC void DoorGpioInit(void);
extern void SysDoFlashRoutine27Service(void);

extern DoorSt_T door_st;

STATIC void HandleDoorPwm(void);      //处理pwm输出
STATIC void DoorPwmStart(void);
STATIC void DoorPwmStop(void);
STATIC struct
{
    uint8_t keymask;
    uint8_t changed;
    uint8_t fsm;
    uint32_t begin_t;
} pwmCtrl = {0};

extern const char g_seres_app_software_version[21];
extern const char g_lin_sequence_num_version[24];
