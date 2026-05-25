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

/*双向循环链表*/
struct T_TcListHead
{
    struct T_TcListHead *next;
    struct T_TcListHead *prev;
};

#define OFFSETOF(type, member) ((size_t) &((type *)0)->member)

#define TC_CONTAINER_OF(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - OFFSETOF(type,member) );})

#define LIST_ENTRY(ptr, type, member) TC_CONTAINER_OF(ptr, type, member)

/*链表初始化*/
#define TcListInit(head)    do{     \
                                 (head)->next = (head);      \
                                 (head)->prev = (head);      \
                              }while(0)

/*链表头添加元素*/
#define TcListAdd(entry,head)   do{ \
                                   (entry)->next = (head)->next;  \
                                   (entry)->prev = (head);    \
                                   (head)->next = (entry);    \
                                   (entry)->next->prev = (entry); \
                                 }while(0)

/*链表尾添加元素*/
#define TcListAddTail(entry,head)   do{ \
                                     (entry)->prev = (head)->prev;  \
                                     (entry)->next = (head);    \
                                     (head)->prev = (entry);    \
                                     (entry)->prev->next = (entry); \
                                   }while(0)

/*链表中删除元素*/
#define TcListDel(entry) do{		\
                             (entry)->next->prev = (entry)->prev;	\
                             (entry)->prev->next = (entry)->next;	\
                           }while(0)

/*链表遍历*/
#define TcListForEachEntry(pos, head, member)                \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member);    \
         &pos->member != (head);     \
         pos = LIST_ENTRY(pos->member.next, typeof(*pos), member))

/*链表遍历,在遍历链表时需要删除链表中元素时使用*/
#define TcListForEachEntrySafe(pos, n, head, member)          \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member),     \
          n = LIST_ENTRY(pos->member.next, typeof(*pos), member);     \
          &pos->member != (head);                         \
          pos = n, n = LIST_ENTRY(n->member.next, typeof(*n), member))

/*反向遍历链表*/
#define TcListForEachEntryReverse(pos, head, member)                  \
        for (pos = LIST_ENTRY((head)->prev, typeof(*pos), member);      \
             &pos->member != (head);    \
             pos = LIST_ENTRY(pos->member.prev, typeof(*pos), member))

/*链表第一个元素,链表不能为空*/
#define TcListFirstEntry(ptr, type, member) LIST_ENTRY((ptr)->next, type, member)

/*链表最后一个元素,链表不能为空*/
#define TcListLastEntry(ptr, type, member) LIST_ENTRY((ptr)->prev, type, member)

/*判断链表为空*/
#define TcListEmpty(head)   ((head)->next == (head))

#endif
