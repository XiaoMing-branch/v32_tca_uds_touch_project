/**
*****************************************************************************
* @brief  tc task header
* @file   tc_task.h
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

#ifndef TC_TASK_H__
#define TC_TASK_H__

/**
 * @brief  任务优先等级定义
 * @note   数值越小优先级越高
 *         CRITICAL > HIGH > MID > LOW
 */
#define TC_TASK_PRIO_LOW                3   /**< 低优先级 */
#define TC_TASK_PRIO_MID                2   /**< 中优先级 */
#define TC_TASK_PRIO_HIGH               1   /**< 高优先级 */
#define TC_TASK_PRIO_CRITICAL           0   /**< 关键优先级（最高） */

/**
 * @brief  系统保留的消息类型
 * @note   TCTask内核使用以下消息类型驱动任务生命周期和通信
 *         用户自定义消息从MSG_TASK_USER_BASE开始
 */
#define MSG_TASK_INIT                   0   /**< 任务初始化消息（任务被创建时自动发送） */
#define MSG_TASK_DESTROY                1   /**< 任务销毁消息（任务被销毁时自动发送） */
#define MSG_TASK_TIMER                  2   /**< 软件定时器到期消息 */
#define MSG_TASK_QUEUE                  3   /**< 队列数据到达消息（任务关联的队列收到数据时发送） */
#define MSG_TASK_BITFLAG                4   /**< 任务标记位消息（bitFlag被置位时由调度器发送） */

/**
 * @brief  用户自定义消息起始值
 * @note   系统保留0~1023，用户消息从1024开始
 */
#define MSG_TASK_USER_BASE          1024

/**
 * @brief  任务回调函数类型
 * @param  msg   - 消息ID
 * @param  param - 消息参数
 */
typedef void (*TC_TASK_CALLBACK)(uint32_t msg, void *param);

/**
 * @brief  消息节点结构体
 * @note   所有待处理消息通过双向链表串联管理
 *         消息节点从固定大小的消息内存池中分配
 */
typedef struct
{
    struct T_TcListHead list;   /**< 链表节点（所有消息通过链表管理） */
    uint16_t taskId;            /**< 目标任务ID */
    uint32_t msg;               /**< 消息类型 */
    void *param;                /**< 消息参数 */
} T_TcMsg;

/**
 * @brief  任务控制块结构体
 * @note   每个任务由优先级和索引编码成唯一的taskId
 *         taskId = priority * TC_TASK_NUM + index
 *         isUsed标志任务槽位是否已被占用
 *         bitFlag为用户自定义的32位标记位，非线程安全
 */
typedef struct
{
    uint16_t taskId;                    /**< 任务ID（编码方式：优先级×TC_TASK_NUM + 索引） */
    uint16_t isUsed;                    /**< 使用标志（0-未使用，1-使用中） */
    volatile uint32_t bitFlag;          /**< 32位用户自定义标记位 */
    const char *name;                   /**< 任务名称字符串 */
    TC_TASK_CALLBACK callback;          /**< 任务回调函数指针 */

#if TC_GENERATE_RUN_TIME_STATS
    uint32_t lastTickCnt;               /**< 上一次5秒统计周期内的CPU时间片数 */
    uint32_t tickCntPer5S;              /**< 当前5秒周期内的CPU时间片计数 */
#endif

} T_TcTask;

/**
 * @brief  全局任务描述数组
 * @note   tcTasks[priority][index] 以优先级分组存储所有任务
 *         每个优先级的任务数不超过TC_TASK_NUM
 */
extern T_TcTask tcTasks[TC_TASK_PRIO_LOW + 1][TC_TASK_NUM];

/**< 当前正在执行的任务指针 */
extern T_TcTask *currentTask;

/**
 * @brief  任务模块初始化
 * @note   清空任务数组、消息数组，初始化各优先级的待处理消息链表
 *         创建消息内存池（tcMsgsMem）用于管理T_TcMsg节点
 * @retval 1  - 初始化成功
 * @retval -1 - 初始化失败
 */
int TcTaskInit(void);

/**
 * @brief  创建任务
 * @param  name     - 任务名称字符串
 * @param  callback - 任务回调函数（不能为NULL）
 * @param  param    - 传递给MSG_TASK_INIT消息的参数
 * @param  priority - 任务优先级（TC_TASK_PRIO_CRITICAL ~ TC_TASK_PRIO_LOW）
 * @note   在指定优先级中查找空闲槽位，创建成功后自动向任务发送MSG_TASK_INIT消息
 *         若初始化消息发送失败则回滚（任务槽位置为未使用）
 * @retval T_TcTask* - 创建成功
 * @retval NULL      - 创建失败（优先级无效/槽位已满/MSG_TASK_INIT发送失败）
 */
T_TcTask *TcTaskCreate(const char *name, TC_TASK_CALLBACK callback, void *param, uint32_t priority);

/**
 * @brief  向任务发送消息
 * @param  task  - 目标任务指针
 * @param  msg   - 消息类型
 * @param  param - 消息参数
 * @note   从消息内存池分配一个T_TcMsg节点，填充后加入任务对应优先级的待处理消息链表
 *         消息节点使用完后由调度器释放回内存池
 * @retval 1  - 发送成功
 * @retval -1 - 发送失败（消息内存池耗尽或参数无效）
 */
int TcTaskSendMsg(T_TcTask *task, uint32_t msg, void *param);

/**
 * @brief  清除指定任务的所有待处理消息
 * @param  task - 目标任务指针
 * @note   遍历任务对应优先级的待处理消息链表，删除所有taskId匹配的消息节点
 *         被删除的消息节点通过TcMemPut释放回消息内存池
 * @retval 1  - 清除成功
 * @retval -1 - 参数无效
 */
int TcTaskClrMsg(T_TcTask *task);

/**
 * @brief  清空所有优先级的所有待处理消息
 * @note   遍历tcMsgsTable所有优先级链表，释放所有消息节点回内存池
 */
void TcTaskClrAllMsg(void);

/**
 * @brief  获取任务的标记位
 * @param  task - 目标任务指针
 * @retval 任务的bitFlag值
 */
#define TcTaskGetBitFlag(task)      ((task)->bitFlag)

/**
 * @brief  设置任务标记位
 * @param  task - 目标任务指针
 * @param  bits - 要设置的位掩码
 * @note   bitFlag为非线程安全，用户需自行保证原子性
 */
#define TcTaskSetBitFlag(task,bits)         do{			\
                                                (task)->bitFlag |= (bits);	\
                                              }while(0)

/**
 * @brief  清除任务标记位
 * @param  task - 目标任务指针
 * @param  bits - 要清除的位掩码
 * @note   非线程安全，用户需自行保证原子性
 */
#define TcTaskClrBitFlag(task,bits)         do{			\
                                                (task)->bitFlag &= ~(bits);	\
                                              }while(0)

/**
 * @brief  销毁任务（安全销毁）
 * @param  task - 目标任务指针
 * @note   通过向任务自身发送MSG_TASK_DESTROY消息实现安全销毁
 *         任务在处理到该消息时将isUsed置0
 * @retval 1  - 销毁请求发送成功
 * @retval -1 - 发送MSG_TASK_DESTROY消息失败（通常为内存耗尽）
 */
int TcTaskDestroy(T_TcTask *task);
																						
/**
 * @brief  强制销毁任务（立即销毁）
 * @param  task - 目标任务指针
 * @note   直接将任务的isUsed置为0，不经过消息机制
 *         可能导致当前正在执行的任务状态不一致，需谨慎使用
 */																							
void TcTaskForceDestroy(T_TcTask *task);

/**
 * @brief  向所有任务广播消息
 * @param  msg   - 消息类型
 * @param  param - 消息参数
 * @note   遍历tcTasks数组所有优先级和槽位，对处于使用中的任务直接调用其回调函数
 *         不经过消息队列，由调用者同步执行
 */
void TcTaskBroadcastHandleMsg(uint32_t msg, void *param);

/**
 * @brief  任务调度执行函数
 * @note   TCTask的核心调度器，按优先级轮询待处理消息链表
 *         PRIO_CRITICAL和PRIO_HIGH优先处理，PRIO_MID和PRIO_LOW中会先执行定时器
 *         每处理完一条消息后释放消息节点回内存池
 *         最后处理所有任务的bitFlag标记位
 */
void TcTaskExec(void);

/**
 * @brief  任务运行时统计
 * @note   仅在TC_GENERATE_RUN_TIME_STATS使能时编译
 */
#if TC_GENERATE_RUN_TIME_STATS

    /**
     * @brief  清除所有任务的tickCntPer5S计数器
     * @note   将tickCntPer5S值保存到lastTickCnt中，然后将tickCntPer5S清零
     */
    void TcAllTaskClrTickCnt(void);

    /**
     * @brief  获取指定任务的CPU利用率
     * @param  task  - 目标任务指针
     * @param  stats - 输出统计信息结构体
     * @retval 1  - 获取成功
     * @retval -1 - 参数无效
     */
    int TcGetTaskRunStats(T_TcTask *task, T_TcTaskStats *stats);
#endif

#endif
