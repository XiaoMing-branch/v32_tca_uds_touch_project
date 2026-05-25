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

/**
 * @brief  全局内存池数组
 * @note   TC_MEM_NUM由tc_conf.h配置，预分配固定数量的内存池节点
 */
static T_TcMem tcMems[TC_MEM_NUM];

/**< 全局空闲内存池链表头，串联所有未被创建的内存池节点 */
static void * memFreeList = NULL;

/**
 * @brief  初始化全局内存池管理器
 * @note   清空tcMems数组，将所有T_TcMem节点通过memFreeList串联成空闲链表
 *         在TcInit()中最早被调用，为后续消息、队列、定时器等模块提供内存管理基础
 */
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

/**
 * @brief  创建固定大小内存块池
 * @param  addr    - 预分配的内存缓冲区起始地址
 * @param  nBlks   - 内存块数量
 * @param  blkSize - 每个内存块大小（字节，不能小于4）
 * @note   从全局空闲内存池中分配一个T_TcMem节点，然后将addr中的内存区域
 *         按固定大小分割成nBlks个块，通过单向链表串联（每块前4字节指向下一块）
 *         addr建议字对齐，blkSize建议为4字节整数倍
 * @retval T_TcMem* - 创建成功
 * @retval NULL     - 创建失败（全局池耗尽/参数无效）
 */
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

/**
 * @brief  从内存池中分配一个内存块
 * @param  pmem - 内存池指针
 * @note   O(1)分配，从链表头取出一个空闲块
 *         空闲块的前4字节存放的是下一个空闲块的地址
 * @retval void* - 分配成功，返回内存块地址
 * @retval NULL  - 分配失败（无空闲块或参数无效）
 */
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

/**
 * @brief  释放一个内存块回内存池
 * @param  pmem - 内存池指针
 * @param  pblk - 待释放的内存块地址
 * @note   O(1)释放，将pblk插入空闲链表头部
 *         pblk必须是此前通过TcMemGet从同一内存池获取的地址
 */
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

/**
 * @brief  销毁内存池，释放回全局空闲链表
 * @param  pmem - 内存池指针
 * @note   销毁前必须保证所有已分配的内存块均已归还（memNFree == memNBlks）
 *         否则返回-1表示存在内存泄漏风险
 * @retval 1  - 销毁成功
 * @retval -1 - 销毁失败（参数无效或存在未归还的内存块）
 */
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
