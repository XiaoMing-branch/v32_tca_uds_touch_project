/**
*****************************************************************************
* @brief  tc memory header
* @file   tc_mem.h
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

#ifndef TC_MEM_H__
#define TC_MEM_H__

/*内存管理节点*/
typedef struct
{
    void *memAddr;                  //内存缓冲区，以单向链表方式串接，本字段指向链表头
    void *memFreeList;              //以链表方式串接所有空闲内存池
    uint32_t memBlkSize;            //每个块大小
    uint32_t memNBlks;              //总块数
    uint32_t memNFree;              //空闲块数
} T_TcMem;

/*初始化内存池*/
void TcMemInit(void);

/*创建内存池，创建完成后才可使用，创建失败返回NULL，blkSize不能小于4，否则也返回NULL，建议addr要字对齐，blkSize也为字的整数倍，可以提高运行效率*/
T_TcMem * TcMemCreate(void *addr,uint32_t nBlks,uint32_t blkSize);

/*获取内存池中内存，获取失败，返回NULL*/
void * TcMemGet(T_TcMem *pmem);

/*释放内存到内存池中*/
void TcMemPut(T_TcMem *pmem,void *pblk);

/*释放内存池，释放内存池之前要保证内存池中没有被使用的内存块，否则会出现内存泄漏风险，返回-1，正常返回0*/
int TcMemDestroy(T_TcMem *pmem);

/*空闲内存块数*/
#define TcMemFreeNum(pmem)  ((pmem)->memNFree)

/*总内存块数*/
#define TcMemTotalNum(pmem) ((pmem)->memNBlks)

#endif
