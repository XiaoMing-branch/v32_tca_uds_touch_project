/**
*****************************************************************************
* @brief  算法调度器
* @file   si_scheduler.h
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

#ifndef __SI_SCHEDULER_H__
#define __SI_SCHEDULER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup core
 * 设置算法调度器
 * @{
 */

/**
* @brief    设置算法调度器
* @param[in]   algo 算法对象
* @param[in]   scheduler 调度器类型
* @retval      SI_RT_OK 设置成功
* @retval      other 设置失败
*/
T_SiErrRt SiSetScheduler(T_SiAlgoObject *algo, T_SiScheduler scheduler);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
