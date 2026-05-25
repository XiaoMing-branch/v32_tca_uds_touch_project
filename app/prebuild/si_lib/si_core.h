/**
*****************************************************************************
* @brief  算法核心文件
* @file   si_core.h
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

#ifndef __SI_CORE_H__
#define __SI_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup core 算法核心
 * 定义了信号集描述符、算法描述符、噪音检测描述符等算法核心结构
 * @{
 */

#define SI_ALGO_VERSION          ((5*100) + (2))   /*!< 算法版本5.2 */

#define SI_CH_MAXNUM              8    /*!< 最大支持信号数 */
#define SI_PNODE_CH_MAXNUM        5    /*!< 每个算法节点最大支持信号数 */

//算法描述符
struct T_SiAlgoObject;

/**
* @brief        信号集描述符
* @details      每个信号集描述符对象，定义一个信号处理单元，每个描述符包含独立的滤波器、基线追踪器、判决器等算法执行单元
* @attention    不要直接操作本结构体内容
*/
struct T_SiObject
{
    //! @cond
#ifdef SI_PC_DEBUG
#define SI_OBJECT_NAME_LEN  100
    char name[SI_OBJECT_NAME_LEN];
#endif
    T_SiType type;                   /*!< 信号类型，暂未使用 */
    T_SiObjectStatus status;         /*!< 信号集状态 */
    uint8_t noiseDetected;           /*!< 信号集是否检测到噪声，按T_SiNoiseBase的typeId位运算，bit为1检测到噪声，bit为0未检测到噪声 */
    uint8_t noiseAction;             /*!< 检测到噪音时执行的动作，具体见T_SiNoiseAction */
#define NO_GUARD_KEY         0xFF    /*!< T_SiObject.guardKey设置为本值，表示无屏蔽按键 */
    uint8_t guardKey;                /*!< 信号集内屏蔽层信号编号，0xFF表示无屏蔽层信号，默认为0xFF */
    uint8_t keyNum;                  /*!< 信号集内信号个数 */
    T_SiGrowDirect growDirect;       /*!< 信号增长方向 */
    T_SiBitMask lockMask;            /*!< 信号集内被锁定信号位掩码 */
    const uint8_t *keyMap;           /*!< 信号集内信号映射关系表 */
    T_SiData *rawData;               /*!< 信号集原始数据缓冲区 */
    const T_SiGrowDirect *growDirectMt;    /*!< 多通道信号增长方向，当growDirect为SI_GROW_DIRECT_MT时，本值有效 */

#if SI_AMP_ENABLE
    void *amp;          /*!< 放大器 */
#endif
    void *filter;       /*!< 滤波器 */
    void *baseline;     /*!< 基线更新器 */
    void *arbiter;      /*!< 仲裁器 */
    void *paraAdjuster; /*!< 参数调节器 */

#if SI_RAWDATA_PREHANDLE_ENABLE
    const void *rawDataHandlePara;  /*!< 原始数据预处理参数 */
    void (*rawDataHandleFunc)(struct T_SiObject *obj, const void *para);    /*!< 原始数据预处理接口，算法必须是无状态的，通常用于防水、温补等通道 */
#endif

    struct T_SiAlgoObject *algo;   /*!< 算法描述符 */

    struct T_SiObject *link;    /*!< 单向链表 */
    //! @endcond       //doxygen中隐藏
};
typedef struct T_SiObject T_SiObject;

/**
* @brief        噪音检测描述符
* @attention    不要直接操作本结构体内容
*/
struct T_SiNoiseObject
{
    //! @cond
#ifdef SI_PC_DEBUG
#define SI_NOISE_OBJECT_NAME_LEN  100
    char name[SI_NOISE_OBJECT_NAME_LEN];
#endif
    uint8_t status;                  /*!< 噪音检测器状态 */
    uint8_t keyNum;                  /*!< 噪音检测通道个数 */
    const uint8_t *keyMap;           /*!< 噪音检测通道映射关系表 */
    T_SiData *rawData;               /*!< 噪音检测通道原始数据缓冲区 */

    void *noise;                     /*!< 噪音检测器 */

    struct T_SiAlgoObject *algo;     /*!< 算法描述符 */

    struct T_SiNoiseObject *link;    /*!< 单向链表 */
    //! @endcond       //doxygen中隐藏
};
typedef struct T_SiNoiseObject T_SiNoiseObject;

/**
* @brief        算法描述符
* @details      每个算法描述符，定义一种算法类型，内含信号集列表、噪音检测器列表以及算法调度器等
* @attention    不要直接操作本结构体内容
*/
struct T_SiAlgoObject
{
    //! @cond
    T_SiData siRawData[SI_CH_MAXNUM];       /*!< 原始采样数据，需要由用户在读取采样数据后更新 */
#if SI_RAWDATA_PREHANDLE_ENABLE
    T_SiData siInitData[SI_CH_MAXNUM];      /*!< 原始采样数据初始值 */
#endif
    T_SiObject *siObjHeader;                /*!< 信号集表头 */
    T_SiNoiseObject *siNoiseObjHeader;      /*!< 噪音检测器表头 */
    T_SiErrRt(*scheduler)(struct T_SiAlgoObject *algo);            /*!< 调度器 */
    uint32_t scheduleCount;                 /*调度计数次数*/
    //! @endcond       //doxygen中隐藏
};
typedef struct T_SiAlgoObject T_SiAlgoObject;

/**
* @brief    获取系统时间函数指针
* @note     用户在使用算法库时，需要给本函数指针正确赋值，否则会导致部分功能失效
* @param[in]   无
* @retval      当前系统时间，单位ms
*/
extern uint32_t (*SiGetTimeMs)(void);

/**
* @brief    算法初始化
* @param[in]   algo 算法描述符
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiAlgoInit(T_SiAlgoObject *algo);

/**
* @brief    运行算法
* @param[in]   algo 算法描述符
* @retval      SI_RT_OK 运行成功
* @retval      SI_RT_WKUP 检测到唤醒信号
* @retval      other 运行失败
*/
#define SiAlgoProcress(algo)     ((*(algo)->scheduler)(algo))

/**
* @brief    向算法缓冲区中写入原始数据
* @param[in]   algo 算法描述符
* @param[in]   i 通道号
* @param[in]   v 原始数据值
* @retval      无
*/
#if SI_RAWDATA_PREHANDLE_ENABLE
#define SiAlgoSetRawData(algo,i,v)  do{                 \
                                        (algo)->siRawData[(i)] = (v);                   \
                                        if ((algo)->siInitData[(i)] <= 0)                   \
                                        {                   \
                                          (algo)->siInitData[(i)] = (algo)->siRawData[(i)];                   \
                                        }                   \
                                      } while (0)
#else
#define SiAlgoSetRawData(algo,i,v)  do{                 \
                                        (algo)->siRawData[(i)] = (v);                   \
                                      } while (0)
#endif
/**
* @brief    从算法缓冲区中获取原始数据
* @param[in]   algo 算法描述符
* @param[in]   i 通道号
* @retval      原始数据值
*/
#define SiAlgoGetRawData(algo,i)  ((algo)->siRawData[(i)])

/**
* @brief    锁住按键通道，被锁住通道不被调度器处理
* @param[in]   obj 信号集对象
* @param[in]   keyMask 按键通道掩码，置1表示对应通道被锁定
* @retval      无
*/
void SiLockKey(struct T_SiObject *obj, T_SiBitMask keyMask);

/**
* @brief    解锁按键通道
* @param[in]   obj 信号集对象
* @param[in]   keyMask 按键通道掩码，置1表示对应通道被解锁
* @retval      无
*/
void SiUnlockKey(struct T_SiObject *obj, T_SiBitMask keyMask);

/**
* @brief    设置保护通道
* @note     guardKey必须是信号集的最后一个按键，否则滑条判决器会返回错误
* @param[in]   obj 信号集对象
* @param[in]   guardKey 保护通道编号，0xFF表示无保护通道，guardKey必须是信号集的最后一个按键，否则滑条判决器会返回错误
* @retval      无
*/
void SiSetGuard(struct T_SiObject *obj, uint8_t guardKey);

/**
* @brief    设置原始数据预处理器，通常用于防水、温补等通道
* @note     算法必须是无状态的
* @param[in]   obj 信号集对象
* @param[in]   func 预处理回调函数
* @param[in]   para 传递给预处理回调函数的参数
* @retval      无
*/
void SiSetRawDataHandle(struct T_SiObject *obj, void (*func)(struct T_SiObject *obj, const void *para), const void *para);

/**
* @brief    设置多通道信号增长方向
* @param[in]   obj 信号集对象
* @param[in]   growDirectMt 信号增长方向描述表
* @retval      无
*/
void SiSetGrowDirectMt(struct T_SiObject *obj, const T_SiGrowDirect *growDirectMt);

/**
* @brief    设置参数调节器，调节器会在不同条件下配置不同的算法参数
* @param[in]   obj 信号集对象
* @param[in]   paraAdjuster 参数调节器
* @retval      无
*/
void SiObjectSetParaAdjuster(T_SiObject *obj, T_SiParaAdjusterBase *paraAdjuster);

/**
* @brief    信号集对象节点初始化
* @param[in]   obj 信号集对象
* @param[in]   type 信号类型，暂未使用
* @param[in]   growDirect 信号增长方向
* @param[in]   keyNum 信号个数
* @param[in]   keyMap 信号通道映射表
* @param[in]   rawData 信号原始数据缓冲区
* @param[in]   filter 滤波器
* @param[in]   baseline 基线追踪器
* @param[in]   arbiter 判决器
* @param[in]   amp 放大器，可以为NULL
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiObjectNodeInit(T_SiObject *obj, T_SiType type, T_SiGrowDirect growDirect, uint8_t keyNum, const uint8_t *keyMap, T_SiData *rawData, void *filter, void *baseline, void *arbiter, void *amp);

/**
* @brief    注册信号集对象，向算法对象中注册信号集对象
* @param[in]   algo 算法对象
* @param[in]   obj 信号集对象
* @retval      SI_RT_OK 注册成功
* @retval      other 注册失败
*/
T_SiErrRt SiObjectRegister(T_SiAlgoObject *algo, T_SiObject *obj);

/**
* @brief    注销信号集对象，从算法对象中移除信号集对象
* @param[in]   algo 算法对象
* @param[in]   obj 信号集对象
* @retval      SI_RT_OK 注销成功
* @retval      other 注销失败
*/
T_SiErrRt SiObjectUnregister(T_SiAlgoObject *algo, T_SiObject *obj);

/**
* @brief    设置信号集对象状态，设置锁定、解锁状态等可控制信号集对象是否被调度器调度
* @param[in]   obj 信号集对象
* @param[in]   status 状态
* @retval      无
*/
void SiObjectSetStatus(T_SiObject *obj, T_SiObjectStatus status);

/**
* @brief    获取信号集对象状态
* @param[in]   obj 信号集对象
* @retval      status 状态
*/
#define SiObjectGetStatus(obj)  ((obj)->status)

/**
* @brief    复制信号集对象
* @param[in]   dstobj 目的对象
* @param[in]   srcobj 源对象
* @retval      SI_RT_OK 复制成功
* @retval      other 复制失败，信号集对象内的某些成员可能禁止拷贝
*/
T_SiErrRt SiObjectCopy(T_SiObject *dstobj, T_SiObject *srcobj);

/**
* @brief    噪音检测器对象初始化
* @param[in]   obj 噪音检测器对象
* @param[in]   keyNum 噪音检测通道个数
* @param[in]   keyMap 噪音检测通道映射表
* @param[in]   rawData 噪音原始数据缓冲区
* @param[in]   noise 噪音检测器
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseObjectNodeInit(T_SiNoiseObject *obj, uint8_t keyNum, const uint8_t *keyMap, T_SiData *rawData, void *noise);

/**
* @brief    注册噪音检测器对象，向算法对象中注册噪音检测器对象
* @param[in]   algo 算法对象
* @param[in]   obj 噪音检测器对象
* @retval      SI_RT_OK 注册成功
* @retval      other 注册失败
*/
T_SiErrRt SiNoiseObjectRegister(T_SiAlgoObject *algo, T_SiNoiseObject *obj);

/**
* @brief    注销噪音检测器对象，从算法对象中移除噪音检测器对象
* @param[in]   algo 算法对象
* @param[in]   obj 噪音检测器对象
* @retval      SI_RT_OK 注销成功
* @retval      other 注销失败
*/
T_SiErrRt SiNoiseObjectUnregister(T_SiAlgoObject *algo, T_SiNoiseObject *obj);

/**
* @brief    设置噪音检测器对象状态，设置锁定、解锁状态等可控制噪音检测器对象是否被调度器调度
* @param[in]   obj 噪音检测器对象
* @param[in]   status 状态
* @retval      无
*/
void SiNoiseObjectSetStatus(T_SiNoiseObject *obj, T_SiNoiseObjectStatus status);

/**
* @brief    获取算法版本
* @retval      版本号 版本格式：Major*100 + Minor
*/
uint16_t SiAlgoVersion(void);

/**
* @brief    获取算法编译时间
* @retval      编译时间字符串
*/
const char *SiAlgoCompileTime(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
