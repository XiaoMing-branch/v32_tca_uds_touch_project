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

#include "tcae10.h"
#include "tc_conf.h"

/**
 * @brief  系统时钟初始值
 * @note   设置初始值为0xFFFFD8EF，使系统运行约10秒后产生溢出
 *         用于在调试阶段排查由于systick溢出导致的逻辑bug
 */
#define TC_SYSTICK_INIT_VALUE       (0xFFFFD8EF)

/**
 * @brief  SysTick定时器频率
 */
#ifndef SYSTICK_FREQ_HZ
    #define SYSTICK_FREQ_HZ     1000       //1K 频率
#endif

/**
 * @brief  SysTick计数值
 */
#define SYSTICK_COUNTS      (SYSTICK_FREQ_HZ / TC_SYSTICK_HZ)

/**
 * @brief  CPU状态寄存器类型
 */
typedef uint32_t TC_CPU_SR;

/**
 * @brief  临界区保护宏
 * @note   根据TC_NOTUSED_IN_ISR配置选择是否实际关中断
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
 * @brief  保存PRIMASK（关中断）
 */
TC_CPU_SR  TC_CPU_SR_Save(void);

/**
 * @brief  恢复PRIMASK
 * @param  cpu_sr - 保存的PRIMASK值
 */
void       TC_CPU_SR_Restore(TC_CPU_SR cpu_sr);

/**< 系统滴答时钟，每tick递增1 */
extern volatile uint32_t TcSystick;

/**
 * @brief  硬件相关初始化
 */
void TcPortInit(void);

/**
 * @brief  注册SysTick中断回调
 * @param  callback - 回调指针
 * @param  nPeriod  - 执行周期
 * @retval >0 - 成功，-1 - 失败
 */
int TcSystickCallbackRegister(void (*callback)(void), uint8_t nPeriod);

/**
 * @brief  SysTick使能/禁能
 * @param  bEn - 使能标志
 */
void SysTick_Switch(boolean_t bEn);

/**
 * @brief  SysTick回调管理结构体
 */
typedef struct
{
    uint8_t num;                                            /**< 注册的回调数量 */
    uint8_t nPeriod[MAX_SYSTICK_ISR_CALLBACK_NUM];          /**< 各回调执行周期 */
    uint8_t nPastTime[MAX_SYSTICK_ISR_CALLBACK_NUM];        /**< 当前累计时间 */
    void (*callback[MAX_SYSTICK_ISR_CALLBACK_NUM])(void);   /**< 回调函数数组 */
} systickCallback_t;

/**
 * @brief  获取系统当前tick值
 */
#define TcTimeGet()     TcSystick

/**< SysTick中断计数器 */
extern volatile uint32_t g_TcSystickIntCnt;

/**
 * @brief  忙等待延迟
 * @param  dly - 延迟的tick数
 */
void TcTimeDly(uint32_t dly);

/**
 * @brief  获取当前时间（微秒）
 * @retval 当前时间，单位us
 */
uint64_t TcGetTimeUS(void);

/**
 * @brief  获取当前时间（毫秒）
 * @retval 当前时间，单位ms
 */
uint32_t TcGetTimeMS(void);

#endif
