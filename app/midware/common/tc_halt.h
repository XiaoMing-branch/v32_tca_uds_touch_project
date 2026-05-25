/**
*****************************************************************************
* @brief  tc halt header
* @file   tc_halt.h
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

#ifndef TC_HALT_H__
#define TC_HALT_H__

/** @brief 最大注册唤醒过滤函数个数 */
#define MAX_FILTER_WAKEUP_CALLBACK_NUM      3
/** @brief 空闲超时时间（ms），超过此时间无活动则进入低功耗 */
#define ENTER_HALT_TIMEOUT_MS               500
/** @brief 上电后关闭SWD功能的时间（ms），0表示永不关闭 */
#define DISABLE_SWD_AFTER_POWERON_TIMEMS    60000

/**
 * @brief 触摸低功耗RTC触发计数器
 * @note  在TIMER_IRQHandler中置位，用于低功耗触摸唤醒计时
 */
extern volatile uint8_t touchHaltRtcTrigFlag;

/**
 * @brief 低功耗唤醒过滤函数
 * @retval 1 - 正式从低功耗唤醒，0 - 继续进入低功耗
 */
typedef int (*HALT_FILTER_WAKEUP_CALLBACK)(void);

/**
 * @brief  低功耗任务初始化，创建halt任务
 */
void HaltInit(void);

/**
 * @brief  注册低功耗唤醒后过滤函数
 * @param  callback - 过滤函数指针
 * @retval 1 - 注册成功，-1 - 注册缓冲区已满（最大MAX_FILTER_WAKEUP_CALLBACK_NUM个）
 */
int HaltFilterWakeupRegister(HALT_FILTER_WAKEUP_CALLBACK callback);

/**
 * @brief  清空所有已注册的唤醒过滤函数
 */
void HaltFilterWakeupClear(void);

/**
 * @brief  修改进入低功耗超时定时器的超时周期
 * @param  periodTick - 新的超时周期（tick数）
 */
void HaltTimeoutChgPeriod(uint32_t periodTick);

/**
 * @brief  重置进入低功耗超时定时器的超时时间
 */
void HaltTimeoutReset(void);

/** @brief 最大注册halt监控器个数 */
#define MAX_HALT_MONITOR_CALLBACK_NUM      2
/**
 * @brief halt监控器运行间隔
 * @note 单位为RTC wave计数值，范围0-15，越小间隔越大
 */
#define HALT_MONITOR_RUN_INTVAL            3

/**
 * @brief halt监控器回调接口
 * @note 每HALT_MONITOR_RUN_INTVAL时间被调用一次
 */
typedef void (*HALT_MONITOR_CALLBACK)(void);

/**
 * @brief  注册halt监控器回调
 * @param  callback - 监控器回调函数指针
 * @retval 1 - 注册成功，-1 - 注册缓冲区已满（最大MAX_HALT_MONITOR_CALLBACK_NUM个）
 */
int HaltMonitorRegister(HALT_MONITOR_CALLBACK callback);

/**
 * @brief  进入低功耗
 * @param  type - 低功耗模式类型（见SLEEP_MODE_E）
 * @param  v - 参数，暂未使用，填0
 * @retval 1 - 进入低功耗成功，-1 - 失败
 */
int HaltEnter(uint16_t type, uint16_t v);

/**
 * @brief  判断是否处于halt低功耗模式
 * @retval 1 - 处于halt模式，0 - 正常运行
 */
uint8_t IsHaltMode(void);

/**
 * @brief  获取当前低功耗模式
 * @retval 当前低功耗模式类型
 */
uint16_t GetHaltMode(void);

/**
 * @brief  设置低功耗模式
 * @param  mode - 低功耗模式类型
 */
void SetHaltMode(uint16_t mode);

#endif
