/**
*****************************************************************************
* @brief  demo example source file.
* @file   lin_frame.c
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
#include "fff_lin_frame.h"
#include "fff_lin_cfg.h"
#include "fff_lin.h"
#include "fff_app.h"
#include "fff_custom_diagnosticIII.h"
#include "fff_tc_log.h"
#include "fff_lin_process.h"

static const char *TAG = "LIN FRAME";

DoorSt_T door_st = {0};       // 门把手状态反馈信号，初始化为0
DoorCmd_T door_cmd = {0};     // ECU对门把手控制信号，初始化为0
extern user_cfg_t g_user_info;
extern volatile uint8_t lin_error;

DEFINE_FAKE_VOID_FUNC(App_LinControlMsg);