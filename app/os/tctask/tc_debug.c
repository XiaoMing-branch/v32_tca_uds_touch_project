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

#if TC_DEBUG_PRINT

/*打印任务结构体*/
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

/*打印消息结构体*/
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

/*打印链表结构体*/
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

/*打印内存结构体*/
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

/*打印定时器结构体*/
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

#if TC_GENERATE_RUN_TIME_STATS
/*打印CPU利用率*/
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
