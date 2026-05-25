/**
*****************************************************************************
* @brief  demo example source file.
* @file   lin_hw_cfg.h
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
#ifndef __FFF_LIN_HW_CFG_H
#define __FFF_LIN_HW_CFG_H
#include "fff.h"
#include "fff_tcae10.h"

typedef enum
{
   SCI0,
   SCI1,
   SCI2,
   SCI3,
   SCI4,
   SCI5,
   G_PIO,
   SLIC
} lin_hardware_name;

/* MCU type definition */
#define _TCPL_ 0

#define SCI_VT 0

/* SCI version */
#define SCI_VERSION SCI_VT

/* Type of MCU */
#define _MCU_ _TCPL_

/* Resynchronization support */
#define __RESYN_EN 0

/* Autobaud support */
#define AUTOBAUD 0

/* Interface type that MCU uses */
#define XGATE_SUPPORT 0
#define _LIN_XGATE_ 0
#define _LIN_SCI_ 1
#define _LIN_UART_ 0
#define _LIN_SLIC_ 0
#define _LIN_GPIO_ 0
/***********  SCI HARDWARE SECTION  *********/
#define NUM_OF_SCI_CHANNEL 1

/* SCI Base Register definition */
#define SCI0_ADDR 0x0700

/* Use SCI Channel  */
#define _SCI0_ 1
#define _SCI1_ 0
#define _SCI2_ 0
#define _SCI3_ 0
#define _SCI4_ 0
#define _SCI5_ 0

/* MCU bus frequency */
#define MCU_BUS_FREQ 48000000

/* Default interrupt period of the timer for LIN is TIME_BASE_PERIOD micro seconds */
#define TIME_BASE_PERIOD 1000

/* max idle timeout for all networks = idle_timeout_value*1000000/time_base_period */
#define _MAX_IDLE_TIMEOUT_ 5000 /* idle_timeout_value = 5s */

#define LIN_DEBUG_EN 1

#endif
