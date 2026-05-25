/**
*****************************************************************************
* @brief  tc tmr header
* @file   tc_tmr.h
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

#ifndef TC_TMR_H__
#define TC_TMR_H__

/*系统时钟初始值*/
#define TC_SYSTICK_INIT_VALUE       (0xFFFFD8EFU)

/*任务系统时钟*/
extern volatile uint32_t TcSystick;

/*每Tick调用一次，驱动定时器运行，需要放到定时中断中运行*/
void TcTimerTickHandler(void);

/*延迟函数，忙等待延迟，单位为tick*/
void TcTimeDly(uint32_t dly);

/*获取系统时钟，单位为tick*/
#define TcTimeGet()     TcSystick

/*获取当前时间，单位为us*/
uint64_t TcGetTimeUS(void);

/*获取当前时间，单位为ms*/
uint32_t TcGetTimeMS(void);

#endif
