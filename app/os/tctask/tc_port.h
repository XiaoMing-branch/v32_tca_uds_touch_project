/**
*****************************************************************************
* @brief  tc port header
* @file   tc_port.h
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

#ifndef TC_PORT_H__
#define TC_PORT_H__

#include "tcae10_ll_delay.h"
#include "tc_type.h"
#include "tc_conf.h"


/**
 * @brief  SysTick定时器频率
 * @note   默认1KHz（1ms中断一次），可由tc_conf.h覆盖
 */
#ifndef SYSTICK_FREQ_HZ
    #define SYSTICK_FREQ_HZ     1000       //1K 频率
#endif

/**
 * @brief  SysTick计数值（用于软件定时器分频）
 * @note   SYSTICK_COUNTS = SYSTICK_FREQ_HZ / TC_SYSTICK_HZ
 *         例如SYSTICK_FREQ_HZ=1000, TC_SYSTICK_HZ=1000, 则SYSTICK_COUNTS=1
 */
#define SYSTICK_COUNTS      (SYSTICK_FREQ_HZ / TC_SYSTICK_HZ)

/**
 * @brief  SysTick中断回调函数最大注册数量
 */
#define MAX_SYSTICK_ISR_CALLBACK_NUM        5

/**
 * @brief  临界区保护宏
 * @note   若TC_NOTUSED_IN_ISR使能，表示ISR中不使用tctask接口，
 *         临界区宏为空操作（不关中断）
 *         否则使用TC_CPU_SR_Save/TC_CPU_SR_Restore保存恢复PRIMASK
 */
#if TC_NOTUSED_IN_ISR
#define  TC_ENTER_CRITICAL()  do{   \
                                           cpu_sr = cpu_sr;  \
                                         }while(0)
#define  TC_EXIT_CRITICAL()   {}
#else                              //ISR中可以使用tctask
#define  TC_ENTER_CRITICAL()  {cpu_sr = TC_CPU_SR_Save();}
#define  TC_EXIT_CRITICAL()   {TC_CPU_SR_Restore(cpu_sr);}
#endif

/**
 * @brief  保存CPU状态寄存器（关闭中断）
 * @retval 保存前的PRIMASK值
 */
TC_CPU_SR  TC_CPU_SR_Save(void);

/**
 * @brief  恢复CPU状态寄存器
 * @param  cpu_sr - 先前保存的PRIMASK值
 */
void       TC_CPU_SR_Restore(TC_CPU_SR cpu_sr);

/**
 * @brief  硬件相关初始化
 * @note   初始化SysTick定时器，注册系统tick回调函数
 */
void TcPortInit(void);

/**
 * @brief  注册SysTick中断回调函数
 * @param  callback - 回调函数指针
 * @param  nPeriod  - 回调执行周期（以SysTick中断周期为单位）
 * @retval >0 - 注册成功，返回注册序号（1-based）
 * @retval -1 - 注册失败（回调数量已达上限）
 */
int TcSystickCallbackRegister(void (*callback)(void), uint8_t nPeriod);

/**
 * @brief  SysTick使能/禁能开关
 * @param  bEn - 0：禁能SysTick；非0：使能SysTick
 */
void SysTick_Switch(boolean_t bEn);

/**
 * @brief  SysTick中断处理函数
 * @note   遍历所有已注册的回调函数，按周期调用
 */
void SysTick_Handler(void);

/**
 * @brief  SysTick回调管理结构体
 * @note   管理多个SysTick中断回调函数及执行周期
 *         callback[]数组存储回调函数指针
 *         nPeriod[]存储各回调的执行周期（n个SysTick中断执行一次）
 *         nPastTime[]记录当前周期已过的中断次数
 */
typedef struct
{
    uint8_t num;                                            /**< 已注册的回调数量 */
    uint8_t nPeriod[MAX_SYSTICK_ISR_CALLBACK_NUM];          /**< 各回调的执行周期 */
    uint8_t nPastTime[MAX_SYSTICK_ISR_CALLBACK_NUM];        /**< 当前周期累计时间 */
    void (*callback[MAX_SYSTICK_ISR_CALLBACK_NUM])(void);   /**< 回调函数指针数组 */
} systickCallback_t;

/**< SysTick中断计数，每次SysTick中断加1 */
extern volatile uint32_t g_TcSystickIntCnt;

#endif
