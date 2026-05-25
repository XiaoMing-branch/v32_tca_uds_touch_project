/**
*****************************************************************************
* @brief  tc list header
* @file   tc_list.h
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

#ifndef TC_LIST_H__
#define TC_LIST_H__

/**
 * @brief  双向循环链表节点
 * @note   TCTask内核的基础数据结构，用于管理任务消息队列、定时器链表等
 *         next指向下一个节点，prev指向上一个节点，空链表时next和prev均指向自身
 */
struct T_TcListHead
{
    struct T_TcListHead *next;
    struct T_TcListHead *prev;
};

/**
 * @brief   获取结构体成员在类型中的偏移量
 * @param   type    - 结构体类型
 * @param   member  - 成员名称
 * @retval  成员相对于结构体起始地址的字节偏移
 */
#define OFFSETOF(type, member) ((size_t) &((type *)0)->member)

/**
 * @brief   通过结构体成员指针获取包含该成员的整个结构体指针
 * @param   ptr     - 成员指针
 * @param   type    - 包含该成员的结构体类型
 * @param   member  - 成员名称
 * @retval  包含该成员的结构体的基地址指针
 */
#define TC_CONTAINER_OF(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - OFFSETOF(type,member) );})

/**
 * @brief   链表节点到结构体的转换宏
 * @note    等同于TC_CONTAINER_OF，用于从链表节点指针获取宿主结构体
 */
#define LIST_ENTRY(ptr, type, member) TC_CONTAINER_OF(ptr, type, member)

/**
 * @brief  初始化链表头
 * @param  head - 链表头指针
 * @note   将头节点的next和prev都指向自身，形成一个空链表
 */
#define TcListInit(head)    do{     \
                                 (head)->next = (head);      \
                                 (head)->prev = (head);      \
                              }while(0)

/**
 * @brief  在链表头部添加元素
 * @param  entry - 待添加的节点
 * @param  head  - 链表头
 */
#define TcListAdd(entry,head)   do{ \
                                   (entry)->next = (head)->next;  \
                                   (entry)->prev = (head);    \
                                   (head)->next = (entry);    \
                                   (entry)->next->prev = (entry); \
                                 }while(0)

/**
 * @brief  在链表尾部添加元素
 * @param  entry - 待添加的节点
 * @param  head  - 链表头
 */
#define TcListAddTail(entry,head)   do{ \
                                      (entry)->prev = (head)->prev;  \
                                      (entry)->next = (head);    \
                                      (head)->prev = (entry);    \
                                      (entry)->prev->next = (entry); \
                                    }while(0)

/**
 * @brief  从链表中删除元素
 * @param  entry - 待删除的节点
 * @note   仅从链表中移除，不释放节点内存
 */
#define TcListDel(entry) do{		\
                             (entry)->next->prev = (entry)->prev;	\
                             (entry)->prev->next = (entry)->next;	\
                           }while(0)

/**
 * @brief  遍历链表（安全遍历，不修改链表结构时使用）
 * @param  pos    - 当前遍历到的宿主结构体指针
 * @param  head   - 链表头
 * @param  member - 链表节点在宿主结构体中的成员名称
 */
#define TcListForEachEntry(pos, head, member)                \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member);    \
         &pos->member != (head);     \
         pos = LIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * @brief  遍历链表（安全遍历，允许在遍历时删除当前元素）
 * @param  pos    - 当前遍历到的宿主结构体指针
 * @param  n      - 下一个宿主结构体指针（暂存，用于安全删除）
 * @param  head   - 链表头
 * @param  member - 链表节点在宿主结构体中的成员名称
 * @note   使用额外的指针n暂存下一节点，确保在删除当前节点后仍能继续遍历
 */
#define TcListForEachEntrySafe(pos, n, head, member)          \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member),     \
          n = LIST_ENTRY(pos->member.next, typeof(*pos), member);     \
          &pos->member != (head);                         \
          pos = n, n = LIST_ENTRY(n->member.next, typeof(*n), member))

/**
 * @brief  反向遍历链表
 * @param  pos    - 当前遍历到的宿主结构体指针
 * @param  head   - 链表头
 * @param  member - 链表节点在宿主结构体中的成员名称
 */
#define TcListForEachEntryReverse(pos, head, member)                  \
        for (pos = LIST_ENTRY((head)->prev, typeof(*pos), member);      \
             &pos->member != (head);    \
             pos = LIST_ENTRY(pos->member.prev, typeof(*pos), member))

/**
 * @brief  获取链表第一个元素
 * @param  ptr    - 链表头指针
 * @param  type   - 宿主结构体类型
 * @param  member - 链表节点成员名称
 * @retval 链表第一个节点所对应的宿主结构体指针（链表不能为空）
 */
#define TcListFirstEntry(ptr, type, member) LIST_ENTRY((ptr)->next, type, member)

/**
 * @brief  获取链表最后一个元素
 * @param  ptr    - 链表头指针
 * @param  type   - 宿主结构体类型
 * @param  member - 链表节点成员名称
 * @retval 链表最后一个节点所对应的宿主结构体指针（链表不能为空）
 */
#define TcListLastEntry(ptr, type, member) LIST_ENTRY((ptr)->prev, type, member)

/**
 * @brief  判断链表是否为空
 * @param  head - 链表头指针
 * @retval 1 - 链表为空；0 - 链表非空
 */
#define TcListEmpty(head)   ((head)->next == (head))

#endif
