/**
*****************************************************************************
* @brief  tc debug header
* @file   tc_debug.h
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

#ifndef TC_DEBUG_H__
#define TC_DEBUG_H__

#if TC_DEBUG_PRINT

/*打印任务结构体*/
void TcPrintTask(T_TcTask * task,const char *desp);

/*打印消息结构体*/
void TcPrintMsg(T_TcMsg * msg,const char *desp);

/*打印链表结构体*/
void TcPrintListHead(struct T_TcListHead * list,const char *desp);

/*打印内存结构体*/
void TcPrintMem(T_TcMem * mem,const char *desp);

/*打印定时器结构体*/
void TcPrintTimer(T_TcTimer * timer,const char *desp);

#endif

#if TC_GENERATE_RUN_TIME_STATS
/*打印CPU利用率*/
void TcPrintCpuUsage(void);
#endif

#endif
