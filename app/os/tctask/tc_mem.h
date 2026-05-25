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

/**
 * @brief  固定大小内存块管理节点
 * @note   TCTask使用固定大小内存块（而非malloc）管理消息节点和队列节点
 *         所有空闲内存块通过单向链表串接，分配和释放均为O(1)操作
 */
typedef struct
{
    void *memAddr;              /**< 内存缓冲区空闲链表头指针 */
    void *memFreeList;          /**< 空闲内存池链表（用于TcMemDestroy释放回全局） */
    uint32_t memBlkSize;        /**< 每个内存块大小（字节） */
    uint32_t memNBlks;          /**< 内存块总数 */
    uint32_t memNFree;          /**< 当前空闲内存块数 */
} T_TcMem;

/**
 * @brief  初始化全局内存池管理器
 * @note   将所有T_TcMem节点串联到memFreeList空闲链表中
 *         在每个内存池创建前调用一次
 */
void TcMemInit(void);

/**
 * @brief  创建固定大小内存块池
 * @param  addr   - 预分配的内存缓冲区起始地址
 * @param  nBlks  - 内存块数量
 * @param  blkSize - 每个内存块大小（字节，不能小于4）
 * @note   addr建议字对齐，blkSize建议为4字节整数倍以提高访问效率
 *         创建时将addr中的内存块通过单向链表串联起来
 * @retval T_TcMem* - 创建成功，返回内存池指针
 * @retval NULL     - 创建失败（参数无效或无空闲内存池节点）
 */
T_TcMem * TcMemCreate(void *addr,uint32_t nBlks,uint32_t blkSize);

/**
 * @brief  从内存池中分配一个内存块
 * @param  pmem - 内存池指针
 * @retval void* - 分配成功，返回内存块地址
 * @retval NULL  - 分配失败（内存池已无空闲块或参数无效）
 */
void * TcMemGet(T_TcMem *pmem);

/**
 * @brief  释放一个内存块回内存池
 * @param  pmem - 内存池指针
 * @param  pblk - 待释放的内存块地址
 * @note   pblk必须是此前通过TcMemGet从同一内存池获取的地址
 */
void TcMemPut(T_TcMem *pmem,void *pblk);

/**
 * @brief  销毁内存池，释放回全局空闲链表
 * @param  pmem - 内存池指针
 * @note   销毁前必须保证所有已分配块均已归还（即memNFree == memNBlks）
 *         否则存在内存泄漏风险
 * @retval 1  - 销毁成功
 * @retval -1 - 销毁失败（仍有未归还的内存块或参数无效）
 */
int TcMemDestroy(T_TcMem *pmem);

/**
 * @brief  获取内存池中空闲内存块数量
 * @param  pmem - 内存池指针
 * @retval 当前空闲块数
 */
#define TcMemFreeNum(pmem)  ((pmem)->memNFree)

/**
 * @brief  获取内存池总内存块数量
 * @param  pmem - 内存池指针
 * @retval 总块数
 */
#define TcMemTotalNum(pmem) ((pmem)->memNBlks)

#endif
