/**
*****************************************************************************
* @brief  tc header
* @file   tc.h
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

#ifndef TC_H__
#define TC_H__

#include <stdio.h>
#include <string.h>

#include "tc_conf.h"
#include "tc_type.h"
#include "tc_list.h"

#include "tc_mem.h"
#include "tc_tool.h"
#include "tc_task.h"
#include "tc_timer.h"
#include "tc_queue.h"
#include "tc_tmr.h"
#include "tc_debug.h"

#include "tc_port.h"

/*软件版本*/
#define TC_TASK_VERSION		"1.1"

/*初始化tctask，失败返回-1，成功返回1*/
int TcInit(void);

#endif
