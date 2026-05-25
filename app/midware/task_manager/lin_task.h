/**
*****************************************************************************
* @brief  lin task header
* @file   lin_task.h
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

#ifndef LIN_TASK_H__
#define LIN_TASK_H__

/**
 * @brief  LIN任务初始化，创建LIN任务
 */
void LinInit(void);

/**
 * @brief  获取上电后LIN总线是否通信过的标志
 * @retval 1 - 已通信过，0 - 未通信过
 */
unsigned char LinCommSincePowerOn(void);

/**
 * @brief  销毁LIN任务，释放相关资源并关闭LIN时钟
 */
void LinDestroy(void);

/**
 * @brief  检查是否允许进入低功耗（收到LIN休眠命令后调用）
 * @retval 1 - 可以进入低功耗，0 - 不可以进入
 */
int32_t LinCanEnterSleep(void);

#endif
