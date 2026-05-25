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

//初始化lin
void LinInit(void);

//上电之后lin是否通信过
unsigned char LinCommSincePowerOn(void);

//销毁lin
void LinDestroy(void);

//返回1表示收到lin sleep命令后可以进入低功耗，0表示不可以
int32_t LinCanEnterSleep(void);

#endif
