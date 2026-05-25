/**
*****************************************************************************
* @brief  tc source
* @file   tc.c
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

#include "tc.h"

/**
 * @brief  TCTask系统初始化入口
 * @note   依次初始化内存管理、任务管理、定时器管理、队列管理和硬件端口
 *         内存管理最先初始化，因为后续模块依赖内存池分配节点（T_TcMem）
 * @retval 1  - 全部初始化成功
 * @retval -1 - 任务模块或队列模块初始化失败
 */
int TcInit(void)
{
    int rt;

    /*初始化内存管理模块*/
    TcMemInit();

    /*初始化任务管理模块*/
    if ((rt = TcTaskInit()) < 0)
    {
        return rt;
    }

    /*初始化定时任务模块*/
    (void)TcTimerInit();

    /*初始化队列管理模块*/
    if ((rt = TcQueueInit()) < 0)
    {
        return rt;
    }

    /*硬件相关初始化*/
    TcPortInit();

    return 1;
}
