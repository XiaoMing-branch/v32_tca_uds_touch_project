/**
*****************************************************************************
* @brief  tc debug source
* @file   tc_debug.c
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
 * @brief  调试打印模块
 * @note   仅在TC_DEBUG_PRINT使能时编译，通过printf输出各内核结构体的详细信息
 */
#if TC_DEBUG_PRINT

/**
 * @brief  打印任务结构体内容
 * @param  task - 目标任务指针
 * @param  desp - 描述字符串（可NULL，非NULL时先打印描述信息）
 * @note   输出task地址、taskId、isUsed、name、callback等关键字段
 *         若使能TC_GENERATE_RUN_TIME_STATS，还会输出运行统计信息
 */
void TcPrintTask(T_TcTask *task, const char *desp)
{
    printf("*********T_TcTask*********\r\n");

    if (desp)
    {
        printf("\t%s\r\n", desp);
    }

    printf("\ttask addr : %p\r\n", task);
    printf("\ttask->taskId : %d\r\n", task->taskId);
    printf("\ttask->isUsed : %d\r\n", task->isUsed);
    if (task->name)
    {
        printf("\ttask->name : %s\r\n", task->name);
    }
    else
    {
        printf("\ttask->name : %p\r\n", task->name);
    }
    printf("\ttask->callback : %p\r\n", task->callback);

    /*统计任务运行状态*/
#if TC_GENERATE_RUN_TIME_STATS
    printf("\ttask->lastTickCnt : %u\r\n", task->lastTickCnt);
    printf("\ttask->tickCntPer5S : %u\r\n", task->tickCntPer5S);
#endif

    printf("**************************\r\n");
}

/**
 * @brief  打印消息结构体内容
 * @param  msg  - 目标消息指针
 * @param  desp - 描述字符串（可NULL）
 * @note   输出消息地址、链表前后指针、taskId、消息类型msg、参数param
 */
void TcPrintMsg(T_TcMsg *msg, const char *desp)
{
    printf("*********T_TcMsg*********\r\n");

    if (desp)
    {
        printf("\t%s\r\n", desp);
    }

    printf("\tmsg addr : %p\r\n", msg);
    printf("\t&msg->list : %p\r\n", &msg->list);
    printf("\tmsg->list.prev : %p\r\n", msg->list.prev);
    printf("\tmsg->list.next : %p\r\n", msg->list.next);
    printf("\tmsg->taskId : %d\r\n", msg->taskId);
    printf("\tmsg->msg : %u\r\n", msg->msg);
    printf("\tmsg->param : %p\r\n", msg->param);

    printf("*************************\r\n");
}

/**
 * @brief  打印链表头结构体内容
 * @param  list - 目标链表头指针
 * @param  desp - 描述字符串（可NULL）
 * @note   输出链表头地址、prev和next指针
 */
void TcPrintListHead(struct T_TcListHead *list, const char *desp)
{
    printf("*********T_TcListHead*********\r\n");

    if (desp)
    {
        printf("\t%s\r\n", desp);
    }

    printf("\tlist addr : %p\r\n", list);
    printf("\tlist->prev : %p\r\n", list->prev);
    printf("\tlist->next : %p\r\n", list->next);

    printf("******************************\r\n");
}

/**
 * @brief  打印内存池结构体内容
 * @param  mem  - 目标内存池指针
 * @param  desp - 描述字符串（可NULL）
 * @note   输出内存池地址、缓冲区地址、块大小、总块数、空闲块数
 *         并遍历显示空闲链表中的所有块地址链
 */
void TcPrintMem(T_TcMem *mem, const char *desp)
{
    int i;
    void *memAddr = NULL;

    printf("*********T_TcMem*********\r\n");

    if (desp)
    {
        printf("\t%s\r\n", desp);
    }

    printf("\tmem addr : %p\r\n", mem);
    printf("\tmem->memAddr : %p\r\n", mem->memAddr);
    printf("\tmem->memFreeList : %p\r\n", mem->memFreeList);
    printf("\tmem->memBlkSize : %u\r\n", mem->memBlkSize);
    printf("\tmem->memNBlks : %u\r\n", mem->memNBlks);
    printf("\tmem->memNFree : %u\r\n", mem->memNFree);

    printf("\tmem content list : ");
    memAddr = mem->memFreeList;
    for (i = 0; i < mem->memNFree - 1; i++)
    {
        printf("%p->", memAddr);
        memAddr = (void *)(*(uint32_t *)memAddr);
    }
    printf("%p\r\n", memAddr);

    printf("*************************\r\n");
}

/**
 * @brief  打印定时器结构体内容
 * @param  timer - 目标定时器指针
 * @param  desp  - 描述字符串（可NULL）
 * @note   输出定时器地址、链表指针、类型、状态、所属任务、周期、起始时间、参数和回调
 */
void TcPrintTimer(T_TcTimer *timer, const char *desp)
{
    printf("*********T_TcTimer*********\r\n");

    if (desp)
    {
        printf("\t%s\r\n", desp);
    }

    printf("\ttimer addr : %p\r\n", timer);
    printf("\t&timer->list : %p\r\n", &timer->list);
    printf("\ttimer->list.prev : %p\r\n", timer->list.prev);
    printf("\ttimer->list.next : %p\r\n", timer->list.next);
    printf("\ttimer->type : %d\r\n", timer->type);
    printf("\ttimer->state : %d\r\n", timer->state);
    printf("\ttimer->task : %p\r\n", timer->task);
    printf("\ttimer->period : %u\r\n", timer->period);
    printf("\ttimer->startt : %u\r\n", timer->startt);
    printf("\ttimer->param : %p\r\n", timer->param);
    printf("\ttimer->callback : %p\r\n", timer->callback);

    printf("***************************\r\n");
}

#endif

/**
 * @brief  CPU利用率打印模块
 * @note   仅在TC_GENERATE_RUN_TIME_STATS使能时编译
 */
#if TC_GENERATE_RUN_TIME_STATS

/**
 * @brief  打印所有任务的CPU利用率
 * @note   遍历所有优先级的所有任务，对处于使用中的任务调用
 *         TcGetTaskRunStats获取其CPU占用百分比并打印
 */
void TcPrintCpuUsage(void)
{
    int i, j;

    for (i = 0; i < sizeof(tcTasks) / sizeof(tcTasks[0]); i++)
    {
        for (j = 0; j < sizeof(tcTasks[0]) / sizeof(tcTasks[0][0]); j++)
        {
            if (tcTasks[i][j].isUsed)
            {
                T_TcTaskStats stats;
                TcGetTaskRunStats(&tcTasks[i][j], &stats);
                printf("task:%s : %d\n", tcTasks[i][j].name, stats.cpuPercent);
            }
        }
    }
}
#endif
