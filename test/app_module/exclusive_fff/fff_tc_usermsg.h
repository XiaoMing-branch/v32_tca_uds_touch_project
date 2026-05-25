/**
*****************************************************************************
* @brief  tc usermsg header
* @file   tc_usermsg.h
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

#ifndef __FFF_TC_USERMSG_H__
#define __FFF_TC_USERMSG_H__

#define MSG_TASK_PREENTER_HALT          (MSG_TASK_USER_BASE+0)      //准备进入低功耗消息，用户不用捕获此消息
#define MSG_TASK_ENTER_HALT             (MSG_TASK_USER_BASE+1)      //进入低功耗消息，用户可以捕获此消息，在进入低功耗前处理一些事务
#define MSG_TASK_WAKE_UP                (MSG_TASK_USER_BASE+2)      //从低功耗唤醒消息，用户可以捕获此消息，做一些从低功耗唤醒后的初始化工作等

#endif
