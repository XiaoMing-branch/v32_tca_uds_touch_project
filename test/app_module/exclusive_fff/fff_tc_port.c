/**
*****************************************************************************
* @brief  tc port source
* @file   tc_port.c
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
#include "fff_system_tcae10.h"
#include "fff_tc_port.h"

/*任务系统时钟*/
volatile uint32_t TcSystick = TC_SYSTICK_INIT_VALUE;                    //1000Hz心跳，预计在10秒后发生溢出，可用于排查由于systick溢出导致的逻辑bug

volatile uint32_t g_TcSystickIntCnt = 0;     // SysTick 中断计数器

/*tick定时器初始化*/
static void TcTimerTickInit(void);
/*每Tick调用一次，驱动定时器运行，需要放到定时中断中运行*/
static void TcTimerTickHandler(void);

/*systick中断回调接口*/
systickCallback_t systickCallback = {0};

DEFINE_FAKE_VOID_FUNC(TcPortInit);
DEFINE_FAKE_VALUE_FUNC(int, TcSystickCallbackRegister, void *, uint8_t);
DEFINE_FAKE_VOID_FUNC(SysTick_Switch, boolean_t);
DEFINE_FAKE_VOID_FUNC(TcTimeDly, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint64_t, TcGetTimeUS);
DEFINE_FAKE_VALUE_FUNC(uint32_t, TcGetTimeMS);
