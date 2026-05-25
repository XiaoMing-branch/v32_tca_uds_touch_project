/**
*****************************************************************************
* @brief  tc memory source
* @file   tc_mem.c
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

/*内存池定义*/
static T_TcMem tcMems[TC_MEM_NUM];

static void * memFreeList = NULL;

/*初始化内存池*/
void TcMemInit(void)
{
    int i;

    memset(tcMems,0x0,sizeof(tcMems));

    /*将所有内存池节点，放到memFreeList空闲链表中*/
    memFreeList = &tcMems[0];
    for(i = 0; i < TC_MEM_NUM-1; i++)
    {
        tcMems[i].memFreeList = &tcMems[i+1];
    }
}

/*创建内存池，创建完成后才可使用，创建失败返回NULL，blkSize不能小于4，否则也返回NULL，建议addr要字对齐，blkSize也为字的整数倍，可以提高运行效率*/
T_TcMem * TcMemCreate(void *addr,uint32_t nBlks,uint32_t blkSize)
{
    uint32_t i;
    T_TcMem *mem = NULL;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    /*内存池已经分配完*/
    if(!memFreeList || blkSize<4 || !addr)
    {
        return NULL;
    }

    TC_ENTER_CRITICAL();

    mem = memFreeList;
    memFreeList = ((T_TcMem *)memFreeList)->memFreeList;

    TC_EXIT_CRITICAL();

    mem->memAddr = addr;
    mem->memFreeList = NULL;
    mem->memBlkSize = blkSize;
    mem->memNBlks = nBlks;
    mem->memNFree = nBlks;

    memset(addr,0x0,nBlks*blkSize);
    /*串接空闲内存块*/
    for(i = 0; i < nBlks-1; i++)
    {
        *(uint32_t *)(&(((uint8_t *)addr)[i*blkSize])) = (uint32_t)&(((uint8_t *)addr)[(i+1)*blkSize]);
    }
    mem->memFreeList = addr;

    return mem;
}

/*获取内存池中内存，获取失败，返回NULL*/
void * TcMemGet(T_TcMem *pmem)
{
    void *emptyMem;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!pmem)
    {
        return NULL;
    }
#endif

    TC_ENTER_CRITICAL();

    if(pmem->memNFree<=0)
    {
        TC_EXIT_CRITICAL();
        return NULL;
    }

    emptyMem = pmem->memAddr;

    --pmem->memNFree;
    pmem->memAddr = (void *)(*(uint32_t *)pmem->memAddr);

    TC_EXIT_CRITICAL();

    return emptyMem;
}

/*释放内存到内存池中*/
void TcMemPut(T_TcMem *pmem,void *pblk)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!pmem || !pblk)
    {
        return;
    }
#endif

    TC_ENTER_CRITICAL();

    *(uint32_t *)pblk = (uint32_t)pmem->memAddr;
    pmem->memAddr = pblk;
    ++pmem->memNFree;

    TC_EXIT_CRITICAL();
}

/*释放内存池，释放内存池之前要保证内存池中没有被使用的内存块，否则会出现内存泄漏风险，返回-1，正常返回0*/
int TcMemDestroy(T_TcMem *pmem)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!pmem)
    {
        return -1;
    }
#endif

    TC_ENTER_CRITICAL();

    /*内存池中有被使用的内存块*/
    if(pmem->memNFree != pmem->memNBlks)
    {
        TC_EXIT_CRITICAL();
        return -1;
    }

    pmem->memFreeList = memFreeList;
    memFreeList = pmem->memFreeList;

    TC_EXIT_CRITICAL();

    return 1;
}
