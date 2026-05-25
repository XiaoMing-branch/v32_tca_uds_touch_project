/**
*****************************************************************************
* @brief  全部算法库头文件
* @file   si_include.h
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

#ifndef __SI_INCLUDE_H__
#define __SI_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef SI_PC_DEBUG
#else
#include "tcae10.h"
#endif

#include "si_config.h"
#include "si_type.h"
#include "si_para_adjuster.h"
#include "si_core.h"
#include "si_tool.h"
#include "si_misc.h"
#include "si_pr.h"
#include "si_amp.h"
#include "si_filter.h"
#include "si_filter_dctor.h"
#include "si_noise_exit.h"
#include "si_noise.h"
#include "si_arbiter.h"
#include "si_arbiter_interp_condition_key.h"
#include "si_arbiter_interp_condition_set.h"
#include "si_arbiter_interp_action_key.h"
#include "si_arbiter_interp_action_set.h"
#include "si_arbiter_interp_event.h"
#include "si_arbiter_interp_matcher.h"
#include "si_arbiter_interp.h"
#include "si_baseline.h"
#include "si_scheduler.h"
#include "si_log.h"
#include "si_hal.h"

#ifdef __cplusplus
}
#endif

#endif
