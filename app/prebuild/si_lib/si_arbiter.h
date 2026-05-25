/**
*****************************************************************************
* @brief  判决器
* @file   si_arbiter.h
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

#ifndef __SI_ARBITER_H__
#define __SI_ARBITER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup arbiter 判决器
 * 判断信号是否有效
 */
/** @} */

/**
 * @defgroup ArbiterInterface 公共接口
 * @ingroup arbiter
 * 判决器相关接口及数据结构
 * @{
 */

/**
* @brief        判决器基类
* @details      定义判决器内部实现方法
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiBitMask lockMask;                     /*!< 屏蔽按键掩码，1表示屏蔽，被屏蔽的按键不参与判决运算 */
    T_SiErrRt(*init)(T_SiObject *obj);        /*!< 安装判决器 */
    T_SiErrRt(*exit)(T_SiObject *obj);        /*!< 卸载判决器 */
    void (*hook)(T_SiObject *obj);            /*!< 判决器钩子函数 */
    void (*reset)(T_SiObject *obj);           /*!< 判决器复位 */

    int (*baselineLocked)(T_SiObject *obj, uint8_t keyNo);       /*!< 基线更新是否被判决器锁定，keyNo表示信号编号，返回值1表示被锁定，0表示未被锁定 */
    int (*canEnterHalt)(T_SiObject *obj);                        /*!< 是否可以进入低功耗，1表示可以，0表示不可以 */

    T_SiErrRt(*run)(T_SiObject *obj, uint8_t keyNum, const T_SiData *filterBuf, const T_SiData *baselineBuf);  /*!< 运行判决器，filterBuf滤波缓冲区，baselineBuf基线缓冲区 */
//! @endcond       //doxygen中隐藏
} T_SiArbiterBase;

/**
* @brief    注册判决器回调函数
* @details     调度器执行完判决器后，紧接着会执行其回调函数
* @param[in]   nd 判决器节点
* @param[in]   hookCallback 回调函数
* @retval      无
*/
#define SiArbiterRegisterHook(nd,hookCallback)      (((T_SiArbiterBase*)(nd))->hook = hookCallback)

/**
* @brief    复位判决器
* @param[in]   obj 信号集对象
* @retval      SI_RT_OK 复位成功
* @retval      other 复位失败
*/
T_SiErrRt SiArbiterReset(T_SiObject *obj);

/**
* @brief    判决器判断是否可以进入低功耗
* @param[in]   obj 信号集对象
* @retval      1 可以
* @retval      0 不可以
*/
int SiArbiterCanEnterHalt(T_SiObject *obj);

/**
* @brief    修改判决器算法参数
* @param[in]   nd 判决器节点
* @param[in]   newpara 新参数
* @retval      无
*/
#define SiArbiterSetPara(nd,newpara)        (memcpy(&(nd)->para,(newpara),sizeof((nd)->para)))

/**
* @brief    锁住判决器信号，被锁住的信号不会被判决器执行
* @param[in]   obj 信号集对象
* @param[in]   keyMask 信号掩码，每位对应一个信号
* @retval      无
*/
void SiArbiterLockKey(struct T_SiObject *obj, T_SiBitMask keyMask);

/**
* @brief    解锁判决器信号，被锁住的信号不会被判决器执行
* @param[in]   obj 信号集对象
* @param[in]   keyMask 信号掩码，每位对应一个信号
* @retval      无
*/
void SiArbiterUnlockKey(struct T_SiObject *obj, T_SiBitMask keyMask);

/** @} */

//************************************************************************************
//按键专用判决器

/**
 * @defgroup ArbiterKey 按键判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开
 * @{
 */
//******支持操作：单击

/**
* @brief        按键判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataKey;

/**
* @brief        按键判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;     /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;   /*!< 释放消抖次数--需要用户指定 */
    uint8_t releaseThreshold;   /*!< 释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定 */
    T_SiData pressThreshold;    /*!< 按压阈值--需要用户指定 */
    uint16_t forceReleaseTimeS; /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaKey;

/**
* @brief        按键判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;           /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKey para;        /*!< 判决器算法参数 */
    uint8_t dataLen;                /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataKey *pdata;      /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterKey;

/**
* @brief    按键判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyNodeInit(T_SiArbiterKey *nd, const T_SiArbiterParaKey *para, uint8_t dataLen, T_SiArbiterDataKey *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterKeyMt 每通道独立参数按键判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        每通道独立参数按键判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    uint8_t releaseThresholds[SI_PNODE_CH_MAXNUM];   /*!< 释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定 */
    T_SiData pressThresholds[SI_PNODE_CH_MAXNUM];    /*!< 按压阈值--需要用户指定 */
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t forceReleaseTimeS;                      /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t forceReleaseTimeMS;                     /*!< 强制释放时间，单位为毫秒--需要用户指定 */
} T_SiArbiterParaKeyMt;

/**
* @brief        每通道独立参数按键判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyMt para;       /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataKey *pdata;       /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterKeyMt;

/**
* @brief    每通道独立参数按键判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyMtNodeInit(T_SiArbiterKeyMt *nd, const T_SiArbiterParaKeyMt *para, uint8_t dataLen, T_SiArbiterDataKey *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterKeyForceMt 每通道独立参数按键判决器，回调中会携带forceRelease标志
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        每通道独立参数按键判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    uint8_t releaseThresholds[SI_PNODE_CH_MAXNUM];   /*!< 释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定 */
    T_SiData pressThresholds[SI_PNODE_CH_MAXNUM];    /*!< 按压阈值--需要用户指定 */
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t forceReleaseTimeS;                      /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t forceReleaseTimeMS;                     /*!< 强制释放时间，单位为毫秒--需要用户指定 */
} T_SiArbiterParaKeyForceMt;

typedef T_SiArbiterDataKey T_SiArbiterDataKeyForceMt;

/**
* @brief        每通道独立参数按键判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyForceMt para;  /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataKeyForceMt *pdata;       /*!< 判决器数据 */
    T_SiArbiterKeyForceValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterKeyForceMt;

/**
* @brief    每通道独立参数按键判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyForceMtNodeInit(T_SiArbiterKeyForceMt *nd, const T_SiArbiterParaKeyForceMt *para, uint8_t dataLen, T_SiArbiterDataKeyForceMt *pdata, T_SiArbiterKeyForceValueChangedCallback valueChangedCallback);
/** @} */

//! @cond

//******判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于向下释放阈值或大于向上释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
//******支持操作：单击

//判决器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint8_t eliminateCnt;           //消抖计数器
    uint8_t upReleaseFlag;          //向上释放阈值标志
    uint32_t pressingMs;            //按压时长计数器
    uint32_t upReleaseBeginMs;      //向上释放标志消失时间，消失后还得延迟upReleaseDelayS后才能work
} T_SiArbiterDataKeyMt2;
//判决器算法参数
typedef struct
{
    uint8_t pressEliminate;                        //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;                      //释放消抖次数--需要用户指定
    uint8_t downReleaseThresholds[SI_PNODE_CH_MAXNUM];   //向下释放阈值，范围3-9，表示：按键阈值的0.3-0.9，当diff值小于本值认为释放--需要用户指定
    uint8_t upReleaseThresholds[SI_PNODE_CH_MAXNUM];     //向上释放阈值，单位为pressThresholds的倍数，范围建议4以上，当diff值大于本值认为释放--需要用户指定
    T_SiData pressThresholds[SI_PNODE_CH_MAXNUM];        //按压阈值--需要用户指定
    uint8_t upReleaseDelayS;                       //到达向上释放阈值后，锁定判决器一段时间不工作，单位S--需要用户指定
    uint16_t forceReleaseTimeS;                    //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
} T_SiArbiterParaKeyMt2;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;            //判决器基类，必须放在开头
    T_SiArbiterParaKeyMt2 para;      //判决器算法参数
    uint8_t dataLen;                 //判决器数据缓冲区个数
    T_SiArbiterDataKeyMt2 *pdata;       //判决器数据
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; //值改变回调接口，keyNo按键编号，status按键状态
} T_SiArbiterKeyMt2;

//按键判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterKeyMt2NodeInit(T_SiArbiterKeyMt2 *nd, const T_SiArbiterParaKeyMt2 *para, uint8_t dataLen, T_SiArbiterDataKeyMt2 *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

//******判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，支持防水通道
//******支持操作：单击
//******防水原理：1：当防水通道diff值小于门限的时候，触摸通道正常工作
//                2：当防水通道diff值大于门限的时候，触摸通道延时guardPressDly时间后禁止工作，并将触摸通道释放，如果防水通道没有release的话将baseline一直跟随RAWDATA，直到防水通道释放，
//                   防水通道释放guardReleaseDly时间后，baseline不再跟随RAWDATA
//                3：防水通道的guardForceReleaseTimeS时间要尽可能长，要大于实验室喷水测试时间。这个要根据客户测试时间调整
//                4：pressEliminate和releaseEliminate对采样通道和防水通道都有效，releaseThreshold、pressThreshold、forceReleaseTimeS只对采样通道有效

//判决器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint8_t guardStatus;            //guard按键状态
    uint8_t eliminateCnt;           //消抖计数器
    uint32_t pressingMs;            //按压时长计数器
    uint32_t guardMs;               //guard通道计时器
} T_SiArbiterDataKeyGuard;
//判决器算法参数
typedef struct
{
    uint8_t pressEliminate;     //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;   //释放消抖次数--需要用户指定
    uint8_t releaseThreshold;   //释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定
    T_SiData pressThreshold;    //按压阈值--需要用户指定
    uint16_t forceReleaseTimeS; //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
    uint8_t guardReleaseThreshold;      //防水通道释放阈值，范围3-9，表示：触发阈值的0.3-0.9--需要用户指定
    T_SiData guardPressThreshold;       //防水通道触发阈值--需要用户指定
    uint16_t guardForceReleaseTimeS;    //强制释放时间，单位为秒，设为0时表示永不释放，要大于实验室喷水测试时间--需要用户指定
    uint16_t guardPressDly;             //单位ms，200-500--需要用户指定
    uint16_t guardReleaseDly;           //单位ms，50--需要用户指定
} T_SiArbiterParaKeyGuard;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;             //判决器基类，必须放在开头
    T_SiArbiterParaKeyGuard para;     //判决器算法参数
    uint8_t dataLen;                  //判决器数据缓冲区个数
    T_SiArbiterDataKeyGuard *pdata;        //判决器数据
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; //值改变回调接口，keyNo按键编号，status按键状态
} T_SiArbiterKeyGuard;

//按键判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterKeyGuardNodeInit(T_SiArbiterKeyGuard *nd, const T_SiArbiterParaKeyGuard *para, uint8_t dataLen, T_SiArbiterDataKeyGuard *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

//! @endcond       //doxygen中隐藏

/**
 * @defgroup ArbiterKey2 多操作按键判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开。支持单击、双击、长按
 * @{
 */
//******支持操作：单击、双击、长按

/**
* @brief        多操作按键判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t clkCnt;                 /*!< 单击计数器 */
    uint32_t waitDblClkMs;          /*!< 等待双击时间 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataKey2;

/**
* @brief        多操作按键判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;     /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;   /*!< 释放消抖次数--需要用户指定 */
    uint8_t releaseThreshold;   /*!< 释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定 */
    T_SiData pressThreshold;    /*!< 按压阈值--需要用户指定 */
    uint8_t dblClickIntval;     /*!< 判断按键双击时间间隔，单位0.1s--需要用户指定 */
    uint8_t longPressTime;      /*!< 判断按键长按时间，单位0.1s--需要用户指定 */
    uint16_t forceReleaseTimeS; /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaKey2;

/**
* @brief        多操作按键判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开。支持单击、双击、长按
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;         /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKey2 para;     /*!< 判决器算法参数 */
    uint8_t dataLen;              /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataKey2 *pdata;   /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterKey2;

/**
* @brief    多操作按键判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKey2NodeInit(T_SiArbiterKey2 *nd, const T_SiArbiterParaKey2 *para, uint8_t dataLen, T_SiArbiterDataKey2 *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterKey2Mt 每通道独立参数多操作按键判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开。支持单击、双击、长按
 * @{
 */
//******支持操作：单击、双击、长按

/**
* @brief        每通道独立参数多操作按键判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t clkCnt;                 /*!< 单击计数器 */
    uint32_t waitDblClkMs;          /*!< 等待双击时间 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataKey2Mt;

/**
* @brief        每通道独立参数多操作按键判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;     /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;   /*!< 释放消抖次数--需要用户指定 */
    uint8_t releaseThreshold[SI_PNODE_CH_MAXNUM];   /*!< 释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定 */
    T_SiData pressThreshold[SI_PNODE_CH_MAXNUM];    /*!< 按压阈值--需要用户指定 */
    uint8_t dblClickIntval;     /*!< 判断按键双击时间间隔，单位0.1s--需要用户指定 */
    uint8_t longPressTime;      /*!< 判断按键长按时间，单位0.1s--需要用户指定 */
    uint16_t forceReleaseTimeS; /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaKey2Mt;

/**
* @brief        每通道独立参数多操作按键判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开。支持单击、双击、长按
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;           /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKey2Mt para;     /*!< 判决器算法参数 */
    uint8_t dataLen;                /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataKey2Mt *pdata;   /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterKey2Mt;

/**
* @brief    每通道独立参数多操作按键判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKey2MtNodeInit(T_SiArbiterKey2Mt *nd, const T_SiArbiterParaKey2Mt *para, uint8_t dataLen, T_SiArbiterDataKey2Mt *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterKeyWkup 低功耗唤醒判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
 * @{
 */
//******支持操作：低功耗唤醒

/**
* @brief        低功耗唤醒判决器算法参数
*/
typedef struct
{
    T_SiData wkupThreshold;             /*!< 唤醒阈值--需要用户指定 */
} T_SiArbiterParaKeyWkup;

/**
* @brief        低功耗唤醒判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;                /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyWkup para;         /*!< 判决器算法参数 */
    uint8_t fsm;                         /*!< 状态机 */ //状态机
} T_SiArbiterKeyWkup;

/**
* @brief    低功耗唤醒判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyWkupNodeInit(T_SiArbiterKeyWkup *nd, const T_SiArbiterParaKeyWkup *para);
/** @} */

/**
 * @defgroup ArbiterKeyWkupMt 每通道独立参数低功耗唤醒判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
 * @{
 */
//******支持操作：低功耗唤醒

/**
* @brief        每通道独立参数低功耗唤醒判决器算法参数
*/
typedef struct
{
    T_SiData wkupThresholds[SI_PNODE_CH_MAXNUM];             /*!< 唤醒阈值--需要用户指定 */
} T_SiArbiterParaKeyMtWkup;

/**
* @brief        每通道独立参数低功耗唤醒判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;                /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyMtWkup para;       /*!< 判决器算法参数 */
    uint8_t fsm;                         /*!< 状态机 */
} T_SiArbiterKeyMtWkup;

/**
* @brief    每通道独立参数低功耗唤醒判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyMtWkupNodeInit(T_SiArbiterKeyMtWkup *nd, const T_SiArbiterParaKeyMtWkup *para);
/** @} */

/**
 * @defgroup ArbiterKeyFilterWkupMt 每通道独立参数低功耗唤醒判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
 * @{
 */
//******支持操作：低功耗唤醒

/**
* @brief        每通道独立参数低功耗唤醒判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 唤醒状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataKeyMtFilterWkup;

/**
* @brief        每通道独立参数低功耗唤醒判决器算法参数
*/
typedef struct
{
    T_SiData wkupThresholds[SI_PNODE_CH_MAXNUM];             /*!< 唤醒阈值--需要用户指定 */
    uint8_t wkupEliminates[SI_PNODE_CH_MAXNUM];              /*!< 唤醒消抖次数--需要用户指定 */
} T_SiArbiterParaKeyMtFilterWkup;

/**
* @brief        每通道独立参数低功耗唤醒判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;                       /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyMtFilterWkup para;        /*!< 判决器算法参数 */
    uint8_t dataLen;                            /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataKeyMtFilterWkup *pdata;      /*!< 判决器数据 */
} T_SiArbiterKeyMtFilterWkup;

/**
* @brief    每通道独立参数低功耗唤醒判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyMtFilterWkupNodeInit(T_SiArbiterKeyMtFilterWkup *nd, const T_SiArbiterParaKeyMtFilterWkup *para, uint8_t dataLen, T_SiArbiterDataKeyMtFilterWkup *pdata);
/** @} */

/**
 * @defgroup ArbiterKeyAbsWkup 绝对值低功耗唤醒判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值的绝对值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
 * @{
 */
//******支持操作：低功耗唤醒

/**
* @brief        绝对值低功耗唤醒判决器算法参数
*/
typedef struct
{
    T_SiData wkupThreshold;             /*!< 唤醒阈值--需要用户指定 */
} T_SiArbiterParaKeyAbsWkup;

/**
* @brief        绝对值低功耗唤醒判决器描述符
* @details      判决器原理：当滤波值和基线值的差值的绝对值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;                /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyAbsWkup para;      /*!< 判决器算法参数 */
    uint8_t fsm;                         /*!< 状态机 */
} T_SiArbiterKeyAbsWkup;

/**
* @brief    绝对值低功耗唤醒判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyAbsWkupNodeInit(T_SiArbiterKeyAbsWkup *nd, const T_SiArbiterParaKeyAbsWkup *para);
/** @} */

/**
 * @defgroup ArbiterKeyAbsWkupMt 每通道独立参数绝对值低功耗唤醒判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值的绝对值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
 * @{
 */
//******支持操作：低功耗唤醒

/**
* @brief        每通道独立参数绝对值低功耗唤醒判决器算法参数
*/
typedef struct
{
    T_SiData wkupThresholds[SI_PNODE_CH_MAXNUM];             //唤醒阈值--需要用户指定
} T_SiArbiterParaKeyMtAbsWkup;

/**
* @brief        每通道独立参数绝对值低功耗唤醒判决器描述符
* @details      判决器原理：当滤波值和基线值的差值的绝对值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;                /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyMtAbsWkup para;    /*!< 判决器算法参数 */
    uint8_t fsm;                         /*!< 状态机 */
} T_SiArbiterKeyMtAbsWkup;

/**
* @brief    每通道独立参数绝对值低功耗唤醒判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyMtAbsWkupNodeInit(T_SiArbiterKeyMtAbsWkup *nd, const T_SiArbiterParaKeyMtAbsWkup *para);
/** @} */

/**
 * @defgroup ArbiterKeyFilterAbsWkupMt 每通道独立参数低功耗唤醒判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
 * @{
 */
//******支持操作：低功耗唤醒

/**
* @brief        每通道独立参数低功耗唤醒判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 唤醒状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataKeyMtFilterAbsWkup;

/**
* @brief        每通道独立参数低功耗唤醒判决器算法参数
*/
typedef struct
{
    T_SiData wkupThresholds[SI_PNODE_CH_MAXNUM];             /*!< 唤醒阈值--需要用户指定 */
    uint8_t wkupEliminates[SI_PNODE_CH_MAXNUM];              /*!< 唤醒消抖次数--需要用户指定 */
} T_SiArbiterParaKeyMtFilterAbsWkup;

/**
* @brief        每通道独立参数低功耗唤醒判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的检测阈值时，发送TOUCH_RT_WKUP信号，从低功耗中唤醒
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;                       /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaKeyMtFilterAbsWkup para;        /*!< 判决器算法参数 */
    uint8_t dataLen;                            /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataKeyMtFilterAbsWkup *pdata;      /*!< 判决器数据 */
} T_SiArbiterKeyMtFilterAbsWkup;

/**
* @brief    每通道独立参数低功耗唤醒判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterKeyMtFilterAbsWkupNodeInit(T_SiArbiterKeyMtFilterAbsWkup *nd, const T_SiArbiterParaKeyMtFilterAbsWkup *para, uint8_t dataLen, T_SiArbiterDataKeyMtFilterAbsWkup *pdata);
/** @} */

//************************************************************************************
//滑条专用判决器

/**
 * @defgroup ArbiterSlider 滑条判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值后，认为滑条按下，开始计算滑动距离
 * @{
 */
//******注意：参考通道必须指向最后一个keyMap
//******支持操作：滑动

#define MIN_SLIDER_STEP     1                       /*!< 最小滑动识别等级 */
#define MAX_SLIDER_STEP     99                      /*!< 最大滑动识别等级 */

/**
* @brief        滑条判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiData curData;               /*!< 当前数据 */
    T_SiData curCoordinate;         /*!< 当前坐标 */
    int32_t chperSum;               /*!< 累加进位 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataSlider;

/**
* @brief        滑条判决器算法参数
*/
typedef struct
{
    uint16_t resolution;        /*!< 分辨率，512 - 1024，建议512 --需要用户指定 */
    uint8_t resolutionStep;     /*!< 几级分辨率，MIN_SLIDER_STEP - MAX_SLIDER_STEP --需要用户指定 */
    uint8_t groupSeed;          /*!< 5-15%，滤波系数，建议10 --需要用户指定 */
    uint8_t holdStepMargin;     /*!< 10-20，步偏移消抖 --需要用户指定 */
    uint8_t pressEliminate;     /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;   /*!< 释放消抖次数--需要用户指定 */
    uint8_t releaseThreshold;   /*!< 释放阈值，范围3-9，表示：按键阈值的0.3-0.9 --需要用户指定 */
    T_SiData pressThreshold;    /*!< 按压阈值 --需要用户指定 */
    uint16_t forceReleaseTimeS; /*!< 强制释放时间，单位为秒，设为0时表示永不释放 --需要用户指定 */
    uint8_t enableMergeDiffValue; /*!< 1使能合并diff按键值功能，0关闭，使能后所有按键的diff值累加后和pressThreshold比较 --需要用户指定 */
} T_SiArbiterParaSlider;

/**
* @brief        滑条判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值后，认为滑条按下，开始计算滑动距离
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;               /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaSlider para;         /*!< 判决器算法参数 */
    uint8_t status;                     /*!< 滑条状态 */
    uint8_t eliminateCnt;               /*!< 消抖计数器 */
    int8_t curStep;                     /*!< 当前步位置 */
    int8_t lastPressStep;               /*!< 上次按压步位置 */
    int8_t beginPressStep;              /*!< 首次按下时步位置 */
    int8_t releaseStep;                 /*!< 松开时步位置 */
    uint8_t dataLen;                    /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataSlider *pdata;       /*!< 判决器数据 */
    T_SiData curPosition;               /*!< 当前坐标位置 */
    T_SiData beginPressPosition;        /*!< 首次按压坐标位置 */
    uint32_t pressingMs;                /*!< 按压时长计数器 */
    T_SiArbiterSliderValueChangedCallback valueChangedCallback;     /*!< 滑条值改变回调函数 */
} T_SiArbiterSlider;

/**
* @brief    滑条判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   callback 滑条值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterSliderNodeInit(T_SiArbiterSlider *nd, const T_SiArbiterParaSlider *para, uint8_t dataLen, T_SiArbiterDataSlider *pdata, T_SiArbiterSliderValueChangedCallback callback);
/** @} */

//! @cond

//************************************************************************************
//入耳检测专用判决器

//************************************************************************************
//脚踢专用判决器

//******判决器原理：采用diff值比例和模式识别算法来判断脚踢是否有效，目前只支持2通道脚踢
//******注意：算法会用到math库和浮点运算，需要将栈空间设置大一些
//******支持操作：短踢

#define ARBITER_FOOT_CHANNEL_NUM        2           //脚踢支持的通道数
#define ARBITER_FOOT_PATTERN_REG_MAX_BUFLEN     40  //脚踢模式识别缓冲区最大长度，不能小于6

typedef void (*T_SiArbiterKickDetectCallback)(void);

//判决器数据
typedef struct
{
    uint8_t status;                 //脚踢通道状态
    T_SiData lastRawData;           //上一次采样原始数据
    uint32_t pressingMs;            //按压时长计数器
} T_SiArbiterDataKick;
//判决器算法参数
typedef struct
{
    struct
    {
        int16_t diff2ChannelRatioThreshold;            //当diff2ChannelRatio小于阈值，认为脚踢无效，可以为负数--需要用户指定
        uint8_t chDiffRatioIntegratorLowThreshold;     //diff积分器低阈值，范围0-100--需要用户指定
        uint8_t chDiffRatioIntegratorHighThreshold;    //diff积分器高阈值，范围0-100--需要用户指定
        uint8_t chMinDiffRatioLowThreshold;            //最小diff比例低阈值，用于计算minRatioScore得分，范围0-100--需要用户指定
        uint8_t chMinDiffRatioHighThreshold;           //最小diff比例高阈值，用于计算minRatioScore得分，范围0-100--需要用户指定
        //chDiffLowRatioThreshold和chDiffHighRatioThreshold，单位0.01，当两通道diff2Channel值比例，低于chDiffLowRatioThreshold认为脚踢有效，当比例高于chDiffHighRatioThreshold认为脚踢无效，当比例介于两者之间时启动模式识别算法做判断
        uint16_t chDiffLowRatioThreshold;       //chDiffRatio低阈值--需要用户指定
        uint16_t chDiffHighRatioThreshold;      //chDiffRatio高阈值--需要用户指定
        int16_t chDiffCompensateRatioThreshold;//补偿阈值，需要介于chDiffLowRatioThreshold和chDiffHighRatioThreshold之间，当chDiffRatio在chDiffLowRatioThreshold和chDiffCompensateRatioThreshold之间时，会有一定的得分补偿--需要用户指定
        int16_t gradientThreshold;             //斜率阈值--需要用户指定
        int16_t similarityFactor;              //相似度因子--需要用户指定
        uint8_t patternRecScoreThreshold;      //模式识别得分阈值0-100--需要用户指定
        T_SiData diffScoreThreshold;           //超过该阈值后，diffScore得100分,本值必须大于pressThreshold--需要用户指定
        uint8_t scoreWeightTable[6];           //得分权重表，累加值为100，分别对应：gradientScore,centerScore,similarityScore,integratorScore,diffScore,minRatioScore--需要用户指定
    } paras[3];
    //参数匹配参考值，控制算法选择哪个paras值，当前选择的度量标准是gradient，当gradient<parasMatchRefValue[0],选择paras[0],<parasMatchRefValue[1],选择paras[1],>=parasMatchRefValue[1],选择paras[2]
    int16_t parasMatchRefValue[2];
    int8_t diff2ChannelRatioSign;          //diff2ChannelRatio符号位，1表示符号位不变，-1表示符号位反向，0表示取绝对值--需要用户指定
    int8_t gradientSign;                   //斜率符号位，斜率为正填1，斜率为负填-1，0表示斜率取绝对值，转换后的斜率值为负数时会被丢弃--需要用户指定
    uint8_t releaseThreshold;              //释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定
    uint8_t patternRecSampIntval;          //模式识别采样间隔，单位为原始采样帧个数--需要用户指定
    uint16_t kickSpeedThreshold;           //踢腿过程中速度阈值，超过此阈值时会认为是一个噪声数据--需要用户指定
    uint16_t kickSpeedMaxIntvalMs;         //两次测速数据最大时间间隔，单位ms，建议设置为100--需要用户指定
    T_SiData patternRecRefValue;           //模式识别参考值，取总变化量的60%左右--需要用户指定
    T_SiData pressThreshold;               //按压阈值，取总变化量的20%左右--需要用户指定
    T_SiData cutoffThreshold;              //截止阈值，当阈值超过本值时，认为脚踢无效，0表示无穷大，脚踢有效范围为[pressThreshold,cutoffThreshold)--需要用户指定
    uint16_t kickDetectTimeoutMs;          //脚踢检测超时时间，脚踢超过该时间后认为无效，0表示不检测超时--需要用户指定
    uint16_t kickDetectMinMs;              //脚踢检测最小时间，脚踢低于该时间后认为无效，0表示不检测--需要用户指定
    uint16_t forceReleaseTimeS;            //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
} T_SiArbiterParaKick;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;        //判决器基类，必须放在开头
    T_SiArbiterParaKick para;    //判决器算法参数
    uint8_t status;              //脚踢状态
    uint8_t dataLen;             //判决器数据缓冲区个数
    T_SiArbiterDataKick *pdata;  //判决器数据
    T_SiArbiterKickDetectCallback detectCallback; //值改变回调接口
    struct
    {
        uint8_t patternRecIntvalCnt;                                    //模式识别间隔计数器
        uint8_t patternRecSampIntval;                                   //模式识别采样间隔
        uint8_t direct;                                                 //方向，1增大，0减小
        uint8_t centerIndex;                                            //中心点坐标
        uint8_t patternRecBufLen;                                       //模式识别缓冲区长度
        uint8_t gradientScore;                                          //斜率得分
        uint8_t centerScore;                                            //中心点得分
        uint8_t similarityScore;                                        //相似度得分
        uint8_t integratorScore;                                        //积分器得分
        uint8_t diffScore;                                              //diff值得分
        uint8_t minRatioScore;                                          //对diff2ChannelRatio最小值打分
        uint8_t score;                                                  //总得分
        uint8_t kickSpeedFsm;                                           //踢腿测速状态机
        int16_t diff2ChannelRatio;                                      //两通道diff值比例，单位0.01
        int16_t diff2ChannelRatioAvg;                                   //两通道diff值比例均值
        int32_t diff2ChannelRatioIntegrator;                            //两通道diff值比例积分器
        int16_t maxDiff2ChannelRatio;                                   //两通道diff值比例中最大值，单位0.01
        int16_t minDiff2ChannelRatio;                                   //两通道diff值比例中最小值，单位0.01
        uint16_t maxAbsDiff2ChannelRatio;                               //两通道diff值比例绝对值中最大值，单位0.01
        uint16_t minAbsDiff2ChannelRatio;                               //两通道diff值比例绝对值中最小值，单位0.01
        uint16_t pressCnt;                                              //按压计数器
        uint16_t lastPressCnt;                                          //上次采样计数值
        uint16_t kickSpeed1;                                            //踢腿过程中最大速度1
        uint16_t kickSpeed2;                                            //踢腿过程中最大速度2
        uint32_t lastKickSpeedTime;                                     //上一次踢腿数据采样时间
        T_SiData diff2Channel;                                          //两通道diff值
        T_SiData lastDiff2Channel;                                      //上一次两通道diff值
        T_SiData maxDiffValue;                                          //最大diff值
        float centerOffset;                                             //中心点偏移
        float gradient;                                                 //斜率
        float similarity;                                               //相似度
        struct
        {
            uint16_t x;
            T_SiData y;
        } patternRecBuf[ARBITER_FOOT_PATTERN_REG_MAX_BUFLEN];   //模式识别缓冲区
    } runData;      //运行时数据
} T_SiArbiterKick;

//脚踢判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterKickNodeInit(T_SiArbiterKick *nd, const T_SiArbiterParaKick *para, uint8_t dataLen, T_SiArbiterDataKick *pdata, T_SiArbiterKickDetectCallback detectCallback);

//******判决器原理：采用diff值比例和模式识别算法来判断脚踢是否有效，目前只支持2通道脚踢
//******注意：算法会用到math库和浮点运算，需要将栈空间设置大一些
//******支持操作：短踢

#define ARBITER_FOOT_GRADIENT_MAX_BUFLEN     12  //脚踢斜率缓冲区最大长度

//判决器数据
typedef struct
{
    uint8_t status;                                     //脚踢通道状态
    uint32_t pressingMs;                                //脚踢时长计数器
} T_SiArbiterDataKick2;
//判决器算法参数
typedef struct
{
    struct
    {
        T_SiData lowThreshold;                                 //当diff2Channel小于阈值，认为脚踢无效，可以为负数--需要用户指定
        T_SiData highThreshold;                                //当diff2Channel大于阈值，认为脚踢无效，可以为负数--需要用户指定
        T_SiData calcThresholds[ARBITER_FOOT_CHANNEL_NUM];     //任一通道diff值大于本阈值时，就会进行lowThreshold和highThreshold判断--需要用户指定
    } diff2ChannelThreshold;                                   //两通道差值阈值，满足1次及以上，如果想调整diff2Channel符号，可以调换按键映射表顺序

    struct
    {
        uint8_t sampIntval;                //斜率采样间隔，表示间隔几次采样数据计算斜率，范围0-ARBITER_FOOT_GRADIENT_MAX_BUFLEN-2 --需要用户指定
        T_SiData lowThreshold;             //当gradientValue小于阈值，认为脚踢无效--需要用户指定
        T_SiData highThreshold;            //当gradientValue大于阈值，认为脚踢无效--需要用户指定
        T_SiData calcThreshold;            //当diff值大于本阈值时，才会进行lowThreshold和highThreshold判断--需要用户指定
    } gradientThresholds[ARBITER_FOOT_CHANNEL_NUM];            //斜率阈值，取绝对值，满足0次及以上

    T_SiData pressThreshold;               //脚踢有效阈值，取总变化量的20%左右--需要用户指定
    uint8_t releaseThreshold;              //释放阈值，范围3-9，表示：脚踢有效阈值（pressThreshold）的0.3-0.9--需要用户指定
    T_SiData cutoffThreshold;              //截止阈值，当阈值超过本值时，认为脚踢无效，0表示无穷大，脚踢有效范围为[pressThreshold,cutoffThreshold)--需要用户指定

    uint16_t kickDetectTimeoutMs;          //脚踢检测超时时间，脚踢超过该时间后认为无效，0表示不检测超时--需要用户指定
    uint16_t kickDetectMinMs;              //脚踢检测最小时间，脚踢低于该时间后认为无效，0表示不检测--需要用户指定

    uint16_t forceReleaseTimeS;            //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
} T_SiArbiterParaKick2;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;        //判决器基类，必须放在开头
    T_SiArbiterParaKick2 para;    //判决器算法参数
    uint8_t status;              //脚踢状态
    uint8_t dataLen;             //判决器数据缓冲区个数
    T_SiArbiterDataKick2 *pdata;  //判决器数据
    T_SiArbiterKickDetectCallback detectCallback; //值改变回调接口
    struct
    {
        T_SiData diff2Channel;                                          //两通道diff值
        uint8_t gradientStatus;                                                               //斜率状态机
        uint8_t gradientBufLen;                                                               //斜率缓冲区长度
        T_SiData gradientDataBuf[ARBITER_FOOT_GRADIENT_MAX_BUFLEN][ARBITER_FOOT_CHANNEL_NUM]; //斜率缓冲区
        T_SiData gradientValue[ARBITER_FOOT_CHANNEL_NUM];                                     //计算后的斜率值
        uint8_t diff2ChannalStatus;                                                           //diff2Channel状态机
        uint8_t cutoffStatus;                                                                 //截止阈值状态机
        uint32_t beginKickMs;                                                                 //脚踢起始时间
    } runData;      //运行时数据
} T_SiArbiterKick2;

//脚踢判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterKick2NodeInit(T_SiArbiterKick2 *nd, const T_SiArbiterParaKick2 *para, uint8_t dataLen, T_SiArbiterDataKick2 *pdata, T_SiArbiterKickDetectCallback detectCallback);

//************************************************************************************
//点阵图专用判决器

//******判决器原理：记录按压起点和终点坐标，生成一张resolutionXWidth*resolutionYWidth大小的点阵图片

typedef void (*T_SiArbiterBitmapCallback)(const T_SiBitmap *bitmap);

//判决器数据
typedef struct
{
    T_SiData curData;               //当前数据
    T_SiData curCoordinate;         //当前坐标
    int32_t chperSum;               //累加进位
} T_SiArbiterDataBitmap;

//判决器算法参数
typedef struct
{
    uint16_t resolution;        //分辨率，512 - 1024，建议512 --需要用户指定
    uint16_t resolutionXWidth;  //X轴宽度--需要用户指定
    uint16_t resolutionYWidth;  //Y轴宽度--需要用户指定
    uint8_t xChNum;             //X轴通道个数，T_SiObject中的keyMap前xChNum个通道作为X轴，之后的yChNum个通道作为Y轴，通道需要按类似滑条的顺序排列--需要用户指定
    uint8_t yChNum;             //Y轴通道个数，T_SiObject中的keyMap前xChNum个通道作为X轴，之后的yChNum个通道作为Y轴，通道需要按类似滑条的顺序排列--需要用户指定
    uint8_t xChGrowDirect;      //X轴信号增长方向--需要用户指定
    uint8_t yChGrowDirect;      //Y轴信号增长方向--需要用户指定
    uint8_t percent;            //各通道分辨率--需要用户指定
    uint8_t groupSeed;          //5-15%，滤波系数，建议10 --需要用户指定
    uint8_t holdStepMargin;     //10-20，步偏移消抖 --需要用户指定
    uint8_t pressEliminate;     //按压消抖次数--需要用户指定
    uint16_t releaseTimeMs;     //释放消抖时间，单位ms--需要用户指定
    uint8_t releaseThreshold;   //释放阈值，范围3-9，表示：按键阈值的0.3-0.9 --需要用户指定
    T_SiData xChPressThreshold; //X轴通道按压阈值 --需要用户指定
    T_SiData yChPressThreshold; //Y轴通道按压阈值 --需要用户指定
    uint16_t forceReleaseTimeS; //强制释放时间，单位为秒，设为0时表示永不释放 --需要用户指定
} T_SiArbiterParaBitmap;

//判决器描述符
typedef struct
{
#define BITMAP_XY_LEN       2
    T_SiArbiterBase base;                       //判决器基类，必须放在开头
    T_SiArbiterParaBitmap para;                 //判决器算法参数
    uint8_t status;                             //状态位
    uint8_t eliminateCnt;                       //消抖计数器
    uint8_t mayNotOnce;                         //点阵图可能不是一笔完成
    uint8_t dataLen;                            //判决器数据缓冲区个数
    T_SiArbiterDataBitmap *pdata;               //判决器数据
    struct
    {
        int8_t curStep;                         //当前步位置
        int8_t lastPressStep;                   //上次按压步位置
        int8_t beginPressStep;                  //首次按下时步位置
        int8_t releaseStep;                     //松开时步位置
        T_SiData curPosition;                   //当前坐标位置
        T_SiData beginPressPosition;            //首次按压坐标位置
    } xy[BITMAP_XY_LEN];                        //0元素表示X，1元素表示Y
    uint32_t releaseMs;                         //释放时长计数器
    uint32_t pressingMs;                        //按压时长计数器
    T_SiBitmap bitmap;                          //点阵图缓冲区
    T_SiArbiterBitmapCallback callback;         //点阵图回调接口
} T_SiArbiterBitmap;

//点阵图判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterBitmapNodeInit(T_SiArbiterBitmap *nd, const T_SiArbiterParaBitmap *para, uint8_t dataLen, T_SiArbiterDataBitmap *pdata, T_SiArbiterBitmapCallback callback);

//************************************************************************************
//带定位的阅读灯判决器，每个灯需要三个电容检测通道，上面两个分别组成矩形或圆形的一半，下面一个组成矩形或圆形

//******判决器原理：通过各通道diff值的组合，可以定位出手指按压区域
//******注意：算法会用到math库和浮点运算，需要将栈空间设置大一些
//******支持操作：短按

#define ARBITER_LAMP_LOCATION_CHANNEL_NUM        3 //阅读灯支持的通道数

typedef void (*T_SiArbiterLampLocationChangedCallback)(T_SiKeyStatus status);

//判决器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint32_t pressingMs;            //按压时长计数器
} T_SiArbiterDataLampLocation;
//判决器算法参数
typedef struct
{
    uint8_t pressEliminate;                         //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;                       //释放消抖次数--需要用户指定
    uint8_t releaseThresholds[ARBITER_LAMP_LOCATION_CHANNEL_NUM]; //释放阈值（通道依次为topLeft,topRight,bottom），范围3-9，表示：按键阈值的0.3-0.9，当diff值小于本值认为释放--需要用户指定
    T_SiData pressThresholds[ARBITER_LAMP_LOCATION_CHANNEL_NUM];  //按压阈值（通道依次为topLeft,topRight,bottom）--需要用户指定
    uint8_t chTable[ARBITER_LAMP_LOCATION_CHANNEL_NUM];           //通道表，描述三个电容检测通道位置（通道依次为topLeft,topRight,bottom），值是转换后的keyMap编号，不是物理通道编号--需要用户指定
    uint16_t releaseLockBaselineMs;                 //释放后锁定baseline一段时间不更新，单位Ms--需要用户指定
    uint16_t rlScaleRatio;                          //上右和上左通道缩放比例，单位1/100--需要用户指定
    uint16_t rbScaleRatio;                          //上右和下通道缩放比例，单位1/100--需要用户指定
    uint16_t lbScaleRatio;                          //上左和下通道缩放比例，单位1/100--需要用户指定
    uint16_t clickScaleRatio;                       //手指敲击缩放比例，单位1/100--需要用户指定
    float handCoverScaleRatio;                      //手掌覆盖缩放比例，单位1--需要用户指定
    float validAreaThreshold;                       //有效触控区域阈值，rlPosValue低于该阈值有效--需要用户指定
    float clickThreshold;                           //手指敲击阈值，clickPosValue低于该阈值认为有效，若同时满足validAreaThreshold有效则认为按压有效--需要用户指定
    float handCoverThreshold;                       //手掌覆盖阈值，handCoverPosValue低于该阈值认为有效，若同时满足validAreaThreshold有效则认为按压有效--需要用户指定
    uint16_t forceReleaseTimeS;                     //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
} T_SiArbiterParaLampLocation;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;                   //判决器基类，必须放在开头
    T_SiArbiterParaLampLocation para;       //判决器算法参数
    uint8_t dataLen;                        //判决器数据缓冲区个数
    T_SiArbiterDataLampLocation *pdata;     //判决器数据
    uint8_t status;                         //状态
    uint8_t eliminateCnt;                   //消抖计数器
    uint8_t lockBaseline;                   //是否锁定基线
    uint32_t lockBaselineBeginTimeMs;       //锁定基线超时时间
    float rlPosValue;                       //上右和上左通道转换后的值
    float rbPosValue;                       //上右和下通道转换后的值
    float lbPosValue;                       //上左和下通道转换后的值
    float clickPosValue;                    //手指敲击转换后的值，小于clickThreshold时，认为手指敲击有效
    float handCoverPosValue;                //手掌覆盖转换后的值，小于handCoverThreshold时，认为手掌覆盖有效
    T_SiArbiterLampLocationChangedCallback valueChangedCallback; //值改变回调接口，status按键状态
} T_SiArbiterLampLocation;

//带定位的阅读灯判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterLampLocationNodeInit(T_SiArbiterLampLocation *nd, const T_SiArbiterParaLampLocation *para, uint8_t dataLen, T_SiArbiterDataLampLocation *pdata, T_SiArbiterLampLocationChangedCallback valueChangedCallback);

//************************************************************************************
//带定位的阅读灯判决器，每个灯需要两个电容检测通道，双pad布局采用大圆套小圆方式

//******判决器原理：通过各通道diff值的组合，可以定位出手指按压区域
//******注意：算法会用到math库和浮点运算，需要将栈空间设置大一些
//******支持操作：短按

#define ARBITER_LAMP_LOCATION2_CHANNEL_NUM        2 //阅读灯支持的通道数

//判决器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint32_t pressingMs;            //按压时长计数器
} T_SiArbiterDataLampLocation2;
//判决器算法参数
typedef struct
{
    uint8_t pressEliminate;                         //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;                       //释放消抖次数--需要用户指定
    uint8_t releaseThresholds[ARBITER_LAMP_LOCATION2_CHANNEL_NUM]; //释放阈值（通道依次为left/out,right/in），范围3-9，表示：按键阈值的0.3-0.9，当diff值小于本值认为释放--需要用户指定
    T_SiData pressThresholds[ARBITER_LAMP_LOCATION2_CHANNEL_NUM];  //按压阈值（通道依次为left/out,right/in）--需要用户指定
    uint8_t chTable[ARBITER_LAMP_LOCATION2_CHANNEL_NUM];           //通道表，描述三个电容检测通道位置（通道依次为left/out,right/in），值是转换后的keyMap编号，不是物理通道编号--需要用户指定
    uint16_t releaseLockBaselineMs;                 //释放后锁定baseline一段时间不更新，单位Ms--需要用户指定
    uint16_t outScaleRatio;                         //外通道缩放比例，单位1/100--需要用户指定
    uint16_t inScaleRatio;                          //内通道缩放比例，单位1/100--需要用户指定
    uint16_t clickScaleRatio;                       //手指敲击缩放比例，单位1--需要用户指定
    float upperBoundThreshold;                      //触控阈值上边界，clickPosValue在lowerBoundThreshold和upperBoundThreshold之间，且满足pressDistanceThreshold认为按压有效--需要用户指定
    float lowerBoundThreshold;                      //触控阈值下边界，clickPosValue在lowerBoundThreshold和upperBoundThreshold之间，且满足pressDistanceThreshold认为按压有效--需要用户指定
    float pressDistanceThreshold;                   //触控距离阈值，pressStrengthValue大于该阈值认为触控有效--需要用户指定
    float handCoverUpperBoundThreshold;             //手掌覆盖阈值上边界，pressStrengthValue在handCoverLowerBoundThreshold和handCoverUpperBoundThreshold之间认为按压有效--需要用户指定
    float handCoverLowerBoundThreshold;             //手掌覆盖阈值下边界，pressStrengthValue在handCoverLowerBoundThreshold和handCoverUpperBoundThreshold之间认为按压有效--需要用户指定
    float invalidNoiseThreshold;                    //当检测到噪音，且pressStrengthValue在invalidNoiseThreshold之下，认为按压无效--需要用户指定
    uint16_t forceReleaseTimeS;                     //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
} T_SiArbiterParaLampLocation2;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;                   //判决器基类，必须放在开头
    T_SiArbiterParaLampLocation2 para;      //判决器算法参数
    uint8_t dataLen;                        //判决器数据缓冲区个数
    T_SiArbiterDataLampLocation2 *pdata;    //判决器数据
    uint8_t status;                         //状态
    uint8_t eliminateCnt;                   //消抖计数器
    uint8_t lockBaseline;                   //是否锁定基线
    uint32_t lockBaselineBeginTimeMs;       //锁定基线超时时间
    float outDiffValue;                     //外pad缩放后diff值
    float inDiffValue;                      //内pad缩放后diff值
    float clickPosValue;                    //手指敲击转换后的值，在lowerBoundThreshold和upperBoundThreshold之间，认为手指敲击有效
    float pressStrengthValue;               //按压强度转换后的值
    T_SiArbiterLampLocationChangedCallback valueChangedCallback; //值改变回调接口，status按键状态
} T_SiArbiterLampLocation2;

//带定位的阅读灯判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterLampLocation2NodeInit(T_SiArbiterLampLocation2 *nd, const T_SiArbiterParaLampLocation2 *para, uint8_t dataLen, T_SiArbiterDataLampLocation2 *pdata, T_SiArbiterLampLocationChangedCallback valueChangedCallback);

//! @endcond       //doxygen中隐藏

/**
 * @defgroup ArbiterLamp 阅读灯判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

#define SI_NOISE_DATA_LAMP_WINDOWSIZE      20    /*!< lamp噪音检测器窗口缓冲区最大长度 */
#define SI_LAMP_MAX_CHANNEL_NUM            2     /*!< lamp最大检测通道数 */

/**
* @brief        阅读灯判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压阈值最短持续时间，单位是采样个数--需要用户指定 */

        uint16_t noisePreDummyTime;                  /*!< 在采集噪音前等待的时间，单位是采样个数，目的是等待手指按压信号稳定后测量噪音--需要用户指定 */
        uint16_t noiseSampNum;                       /*!< 噪音采样个数，0表示不测量噪音，最大支持SI_NOISE_DATA_DOORCTRL_WINDOWSIZE--需要用户指定 */
        uint8_t noiseUseRawdata;                     /*!< 1表示用rawdata测噪音，否则用filterdata测噪音--需要用户指定 */
        T_SiNoiseData noiseThreshold;                /*!< 噪音阈值，低于本阈值认为信号稳定--需要用户指定 */
        uint16_t noiseKeepTimeThreshold;             /*!< 噪音持续时间阈值，单位是采样个数，当噪音>noiseThreshold状态持续的时间小于noiseKeepTimeThreshold时认为按压有效--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */
    } pressDetect[SI_LAMP_MAX_CHANNEL_NUM];          /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.噪音满足时间阈值，4.满足最大阈值范围，都满足后才认为按压有效 */
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaLamp;

/**
* @brief        阅读灯判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    uint8_t keyPressed;             /*!< 是否被按下 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
    struct
    {
        uint16_t threshold12TimeCnt;  /*!< 从1阶阈值到2阶阈值时间计数器 */
        uint16_t threshold2KeepEnable; /*!< 允许2阶计数 */
        uint16_t threshold2KeepTimeCnt; /*!< 2阶阈值持续时间 */
        T_SiData maxDiff;           /*!< 最大diff值 */
        uint16_t noisePreDummyCnt;    /*!< 在采集噪音前等待的时间 */
        uint16_t noiseSampCount;    /*!< 噪音采样个数 */
        T_SiData noiseWindow[SI_NOISE_DATA_LAMP_WINDOWSIZE];   /*!< 噪音数据缓冲区 */
        T_SiNoiseData noiseValue;   /*!< 噪音值 */
        uint16_t noiseKeepTimeCnt;  /*!< 噪音保持时间 */
    } pressDetect;                  /*!< 按压检测 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataLamp;

/**
* @brief        阅读灯判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaLamp para;        /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataLamp *pdata;  /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterLamp;

/**
* @brief    阅读灯判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterLampNodeInit(T_SiArbiterLamp *nd, const T_SiArbiterParaLamp *para, uint8_t dataLen, T_SiArbiterDataLamp *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterDoorctrl 门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

#define SI_NOISE_DATA_DOORCTRL_WINDOWSIZE      20    /*!< doorctrl噪音检测器窗口缓冲区最大长度 */
#define SI_DOORCTRL_MAX_CHANNEL_NUM            4     /*!< doorctrl最大检测通道数 */

/**
* @brief        门把手判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    uint8_t mergeRainDetect;                         /*!< 合并淋雨检测，置1时当有一个通道满足淋雨状态时，会将所有通道都进入淋雨状态，置0时每通道执行独立的淋雨检测逻辑--需要用户指定 */
    uint8_t rainExitResetAlgo;                       /*!< 当从淋雨状态退出时，是否复位算法，1表示复位，0表示不复位--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压阈值最短持续时间，单位是采样个数--需要用户指定 */

        uint16_t noisePreDummyTime;                  /*!< 在采集噪音前等待的时间，单位是采样个数，目的是等待手指按压信号稳定后测量噪音--需要用户指定 */
        uint16_t noiseSampNum;                       /*!< 噪音采样个数，0表示不测量噪音，最大支持SI_NOISE_DATA_DOORCTRL_WINDOWSIZE--需要用户指定 */
        uint8_t noiseUseRawdata;                     /*!< 1表示用rawdata测噪音，否则用filterdata测噪音--需要用户指定 */
        T_SiNoiseData noiseThreshold;                /*!< 噪音阈值，低于本阈值认为信号稳定--需要用户指定 */
        uint16_t noiseKeepTimeThreshold;             /*!< 噪音持续时间阈值，单位是采样个数，当噪音>noiseThreshold状态持续的时间小于noiseKeepTimeThreshold时认为按压有效--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */
    } pressDetect[SI_DOORCTRL_MAX_CHANNEL_NUM];      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.噪音满足时间阈值，4.满足最大阈值范围，都满足后才认为按压有效 */
    struct
    {
        uint8_t rainNoisePressedStop;                /*!< 设为1时，当检测到按压时，停止雨检测--需要用户指定 */
        uint8_t rainNoiseSampNum;                    /*!< 雨噪音采样个数，0表示不测量噪音，最大支持SI_NOISE_DATA_DOORCTRL_WINDOWSIZE--需要用户指定 */
        uint8_t rainNoiseUseRawdata;                 /*!< 1表示用rawdata测噪音，否则用filterdata测噪音--需要用户指定 */
        uint8_t rainNoiseSampIntval;                 /*!< 雨噪音采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定 */
        uint16_t rainNoisePreDummyTime;              /*!< 在采集噪音前等待的时间，单位是采样个数，目的是等待手指按压信号稳定后测量噪音--需要用户指定 */
        uint16_t rainNoiseDetectEliminate;           /*!< 雨噪音检测消抖次数，当达到阈值且持续一段时间后认为噪音有效，单位为采样个数（以rainNoiseSampIntval采样）--需要用户指定 */
        T_SiNoiseData rainNoiseThreshold;            /*!< 雨噪音阈值--需要用户指定 */
        uint16_t rainNoiseExitTimeMs;                /*!< 雨噪音退出时间，当满足rainExit持续时间超过本阈值后退出淋雨状态--需要用户指定 */

        uint8_t pressedMaskEnable;                   /*!< 设为1时，使能按压掩码检测--需要用户指定 */
        uint16_t pressedNoiseMask;                   /*!< 当检测到pressedNoiseMask包含在allPressedMask中时进入淋雨状态--需要用户指定 */
        uint16_t pressedNoiseExitTimeMs;                /*!< 按压噪音退出时间，当满足rainExit持续时间超过本阈值后退出淋雨状态--需要用户指定 */

        uint8_t tailNoiseEnable;                     /*!< 尾噪音检测使能--需要用户指定 */
        uint16_t tailNoiseThresholdMs;               /*!< 尾噪音检测阈值，当出现2阶阈值释放后，在本时间范围内又出现了一次1阶但未达到2阶，认为触发尾噪音--需要用户指定 */
        uint16_t tailNoiseExitTimeMs;                /*!< 尾噪音退出时间，当满足rainExit持续时间超过本阈值后退出淋雨状态--需要用户指定 */
    } rainDetect[SI_DOORCTRL_MAX_CHANNEL_NUM];       /*!< 雨检测，1.达到发雨噪音阈值，2.达到diff噪音阈值，3.达到尾噪音阈值，满足任一条件进入淋雨状态，同时触发多个条件时,退出时间取最大值 */
    struct
    {
        uint8_t exitNoiseSampNum;                    /*!< 退出噪音采样个数，0表示不测量噪音，最大支持SI_NOISE_DATA_DOORCTRL_WINDOWSIZE--需要用户指定 */
        uint8_t exitNoiseUseRawdata;                 /*!< 1表示用rawdata测噪音，否则用filterdata测噪音--需要用户指定 */
        uint8_t exitNoiseSampIntval;                 /*!< 退出噪音采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定 */
        T_SiNoiseData exitNoiseThreshold;            /*!< 退出噪音阈值，低于本阈值认为满足退出条件--需要用户指定 */
        uint16_t forceExitNoiseTimeS;                /*!< 强制退出淋雨状态时间，单位秒，0表示永不强制退出--需要用户指定 */
    } rainExit[SI_DOORCTRL_MAX_CHANNEL_NUM];         /*!< 雨退出条件 */
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaDoorctrl;

/**
* @brief        门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    uint8_t keyPressed;             /*!< 是否被按下 */
    uint8_t keyDetected;            /*!< 是否已检测到按键 */
    uint8_t rainFlag;               /*!< 是否进入淋雨状态 */
    uint16_t allPressedMask;        /*!< 按压状态掩码，达到一阶阈值就置位 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
    struct
    {
        uint16_t threshold12TimeCnt;  /*!< 从1阶阈值到2阶阈值时间计数器 */
        uint16_t threshold2KeepEnable; /*!< 允许2阶计数 */
        uint16_t threshold2KeepTimeCnt; /*!< 2阶阈值持续时间 */
        T_SiData maxDiff;           /*!< 最大diff值 */
        uint16_t noisePreDummyCnt;    /*!< 在采集噪音前等待的时间 */
        uint16_t noiseSampCount;    /*!< 噪音采样个数 */
        T_SiData noiseWindow[SI_NOISE_DATA_DOORCTRL_WINDOWSIZE];   /*!< 噪音数据缓冲区 */
        T_SiNoiseData noiseValue;   /*!< 噪音值 */
        uint16_t noiseKeepTimeCnt;  /*!< 噪音保持时间 */
    } pressDetect;                  /*!< 按压检测 */
    struct
    {
        uint16_t rainNoiseEliminateCnt;   /*!< 消抖计数器 */
        uint16_t rainNoiseSampIndex;
        uint16_t rainNoisePreDummyCnt;    /*!< 在采集噪音前等待的时间 */
        uint16_t rainNoiseSampCount;      /*!< 噪音采样个数 */
        T_SiData rainNoiseWindow[SI_NOISE_DATA_DOORCTRL_WINDOWSIZE];   /*!< 噪音数据缓冲区 */
        T_SiNoiseData rainNoiseValue;   /*!< 噪音值 */

        uint8_t tailNoiseFirstFlag;
        uint32_t tailNoise1PressTime;   /*!< 1阶触发时间 */
        uint32_t tailNoise2ReleaseTime; /*!< 2阶释放时间 */
    } rainDetect;                   /*!< 雨检测 */
    struct
    {
        uint16_t exitNoiseTimeMs;         /*!< 雨退出时间 */
        uint16_t exitNoiseSampIndex;
        uint16_t exitNoiseSampCount;      /*!< 噪音采样个数 */
        T_SiData exitNoiseWindow[SI_NOISE_DATA_DOORCTRL_WINDOWSIZE];   /*!< 噪音数据缓冲区 */
        T_SiNoiseData exitNoiseValue;     /*!< 噪音值 */
        uint32_t exitNoiseBeginTimeMs;    /*!< 退出起始时间 */
        uint32_t enterRainTimeMs;         /*!< 进入淋雨时间 */
    } rainExit;                  /*!< 雨退出 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataDoorctrl;

/**
* @brief        门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaDoorctrl para;    /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    uint16_t rainExitMask;           /*!< 雨退出位掩码 */
    uint16_t pressedMask;            /*!< 按压状态掩码，达到一阶阈值就置位 */
    T_SiArbiterDataDoorctrl *pdata;  /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterDoorctrl;

/**
* @brief    门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterDoorctrlNodeInit(T_SiArbiterDoorctrl *nd, const T_SiArbiterParaDoorctrl *para, uint8_t dataLen, T_SiArbiterDataDoorctrl *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterLiteDoorctrl 轻量门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        轻量门把手判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压阈值最短持续时间，单位是采样个数--需要用户指定 */

        uint16_t noisePreDummyTime;                  /*!< 在采集噪音前等待的时间，单位是采样个数，目的是等待手指按压信号稳定后测量噪音--需要用户指定 */
        uint16_t noiseSampNum;                       /*!< 噪音采样个数，0表示不测量噪音，最大支持SI_NOISE_DATA_DOORCTRL_WINDOWSIZE--需要用户指定 */
        uint8_t noiseUseRawdata;                     /*!< 1表示用rawdata测噪音，否则用filterdata测噪音--需要用户指定 */
        T_SiNoiseData noiseThreshold;                /*!< 噪音阈值，低于本阈值认为信号稳定--需要用户指定 */
        uint16_t noiseKeepTimeThreshold;             /*!< 噪音持续时间阈值，单位是采样个数，当噪音>noiseThreshold状态持续的时间小于noiseKeepTimeThreshold时认为按压有效--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */
    } pressDetect[SI_DOORCTRL_MAX_CHANNEL_NUM];      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.噪音满足时间阈值，4.满足最大阈值范围，都满足后才认为按压有效 */
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaLiteDoorctrl;

/**
* @brief        轻量门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    uint8_t keyPressed;             /*!< 是否被按下 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
    struct
    {
        uint16_t threshold12TimeCnt;  /*!< 从1阶阈值到2阶阈值时间计数器 */
        uint16_t threshold2KeepEnable; /*!< 允许2阶计数 */
        uint16_t threshold2KeepTimeCnt; /*!< 2阶阈值持续时间 */
        T_SiData maxDiff;           /*!< 最大diff值 */
        uint16_t noisePreDummyCnt;    /*!< 在采集噪音前等待的时间 */
        uint16_t noiseSampCount;    /*!< 噪音采样个数 */
        T_SiData noiseWindow[SI_NOISE_DATA_DOORCTRL_WINDOWSIZE];   /*!< 噪音数据缓冲区 */
        T_SiNoiseData noiseValue;   /*!< 噪音值 */
        uint16_t noiseKeepTimeCnt;  /*!< 噪音保持时间 */
    } pressDetect;                  /*!< 按压检测 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataLiteDoorctrl;

/**
* @brief        轻量门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaLiteDoorctrl para;    /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataLiteDoorctrl *pdata;  /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterLiteDoorctrl;

/**
* @brief    轻量门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterLiteDoorctrlNodeInit(T_SiArbiterLiteDoorctrl *nd, const T_SiArbiterParaLiteDoorctrl *para, uint8_t dataLen, T_SiArbiterDataLiteDoorctrl *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterLite2Doorctrl 轻量门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        轻量门把手判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压阈值最短持续时间，单位是采样个数--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */
    } pressDetect[SI_DOORCTRL_MAX_CHANNEL_NUM];      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.噪音满足时间阈值，4.满足最大阈值范围，都满足后才认为按压有效 */
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaLite2Doorctrl;

/**
* @brief        轻量门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    uint8_t keyPressed;             /*!< 是否被按下 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
    struct
    {
        uint16_t threshold12TimeCnt;  /*!< 从1阶阈值到2阶阈值时间计数器 */
        uint16_t threshold2KeepEnable; /*!< 允许2阶计数 */
        uint16_t threshold2KeepTimeCnt; /*!< 2阶阈值持续时间 */
        T_SiData maxDiff;           /*!< 最大diff值 */
    } pressDetect;                  /*!< 按压检测 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataLite2Doorctrl;

/**
* @brief        轻量门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaLite2Doorctrl para;    /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataLite2Doorctrl *pdata;  /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterLite2Doorctrl;

/**
* @brief    轻量门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterLite2DoorctrlNodeInit(T_SiArbiterLite2Doorctrl *nd, const T_SiArbiterParaLite2Doorctrl *para, uint8_t dataLen, T_SiArbiterDataLite2Doorctrl *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterLite3Doorctrl 轻量门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        轻量门把手判决器算法参数
*/
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    uint8_t mergeKeyEnable;                          /*!< 使能合并按键功能，1使能，0关闭--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压阈值最短持续时间，单位是采样个数--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */
    } pressDetect[SI_DOORCTRL_MAX_CHANNEL_NUM];      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.噪音满足时间阈值，4.满足最大阈值范围，都满足后才认为按压有效 */
    T_SiData mergeKeyThreshold;                      /*!< 合并按键阈值--需要用户指定 */
    uint8_t mergeKeyReleaseThreshold;                /*!< 合并按键释放阈值，范围3-9，表示：合并阈值的0.3-0.9--需要用户指定 */
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaLite3Doorctrl;

/**
* @brief        轻量门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    uint8_t keyPressed;             /*!< 是否被按下 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
    struct
    {
        uint16_t threshold12TimeCnt;  /*!< 从1阶阈值到2阶阈值时间计数器 */
        uint16_t threshold2KeepEnable; /*!< 允许2阶计数 */
        uint16_t threshold2KeepTimeCnt; /*!< 2阶阈值持续时间 */
        T_SiData maxDiff;           /*!< 最大diff值 */
    } pressDetect;                  /*!< 按压检测 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataLite3Doorctrl;

/**
* @brief        轻量门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaLite3Doorctrl para;    /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataLite3Doorctrl *pdata;  /*!< 判决器数据 */
    struct
    {
        uint8_t status;
        uint8_t keyPressed;
        T_SiData mergeDiff;
    } mergeKey;
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterLite3Doorctrl;

/**
* @brief    轻量门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterLite3DoorctrlNodeInit(T_SiArbiterLite3Doorctrl *nd, const T_SiArbiterParaLite3Doorctrl *para, uint8_t dataLen, T_SiArbiterDataLite3Doorctrl *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

/**
 * @defgroup ArbiterFreq7Doorctrl 跳频门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        跳频门把手判决器算法参数
*/
typedef struct
{
    uint8_t detectChannelNo;                         /*!< 检测通道编号--需要用户指定 */
    uint8_t freqChannelNo;                           /*!< 跳频通道编号--需要用户指定 */
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        T_SiData forceValidThreshold;
        uint16_t forceValidCalcTimeout;
        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压检测保持时间，在这段时间内检测到符合2阶按压条件，即认为按压有效--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */
    } detectChannel;      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.满足最大阈值范围，都满足后才认为按压有效 */
    struct
    {
        T_SiData lockBaselineThreshold;
        uint8_t releaseThreshold;
        uint16_t unlockBaselineLockKeyMs;
        uint32_t forceReleaseBaselineTimeMs;

        T_SiData lowThreshold;                       /*!< 2阶信号稳定时跳频通道的diff低阈值--需要用户指定 */
        T_SiData highThreshold;                      /*!< 2阶信号稳定时跳频通道的diff高阈值--需要用户指定 */
    } freqChannel;                                   /*!< 跳频通道参数--需要用户指定 */
    struct
    {
        T_SiData lowThreshold;
        T_SiData highThreshold;
        T_SiData forceValidLowThreshold;
        T_SiData forceValidHighThreshold;
    } mergeChannel;
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaFreq7Doorctrl;

/**
* @brief        跳频门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;
    uint8_t keyPressed;
    uint32_t pressingMs;
    uint32_t lockBaselineBeginTimeMs;
    struct
    {
        uint16_t threshold12TimeCnt;
        uint16_t threshold2KeepTimeCnt;
        uint16_t forceValidCalcCnt;
        T_SiData maxDiff;
    } detectChannel;
    struct
    {
        uint8_t status;
        uint8_t lockBaseline;
        uint8_t lockKeyFlag;
        uint32_t beginLockBaselineMs;
        uint32_t unlockBaselineBeginLockKeyMs;

        T_SiData diff2Stage;
    } freqChannel;
    struct
    {
        T_SiData value;
        T_SiData validValue;
        T_SiData forceValidValue;
    } mergeChannel;
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataFreq7Doorctrl;

/**
* @brief        跳频门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaFreq7Doorctrl para;    /*!< 判决器算法参数 */
    T_SiArbiterDataFreq7Doorctrl data;  /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterFreq7Doorctrl;

/**
* @brief    跳频门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterFreq7DoorctrlNodeInit(T_SiArbiterFreq7Doorctrl *nd, const T_SiArbiterParaFreq7Doorctrl *para, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

/**
 * @defgroup ArbiterFreq10Doorctrl 跳频门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        跳频门把手判决器算法参数
*/
typedef struct
{
    uint8_t detectChannelNo;                         /*!< 检测通道编号--需要用户指定 */
    uint8_t freqChannelNo;                           /*!< 跳频通道编号--需要用户指定 */
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        T_SiData forceValidThreshold;
        uint16_t forceValidCalcTimeout;
        uint16_t doubleForceValidTimeMs;

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压检测保持时间，在这段时间内检测到符合2阶按压条件，即认为按压有效--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */

        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */
    } detectChannel;      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.满足最大阈值范围，都满足后才认为按压有效 */
    struct
    {
        uint32_t forceReleaseBaselineTimeMs;
        T_SiData lockBaselineThreshold;
        uint8_t releaseThreshold;
        uint8_t alwaysUpdataUnlockValue;
        uint8_t unlockBaselineReInit;
        uint16_t unlockBaselineSampNum;
        T_SiNoiseData unlockBaselineThreshold;
        uint16_t unlockBaselineEliminateMs;
        uint16_t unlockBaselineLockKeyMs;

        T_SiData lowThreshold;                       /*!< 2阶信号稳定时跳频通道的diff低阈值--需要用户指定 */
        T_SiData highThreshold;                      /*!< 2阶信号稳定时跳频通道的diff高阈值--需要用户指定 */

        struct
        {
            uint8_t enable;
            uint16_t preDetectMs;
            uint16_t postDetectMs;
            uint16_t postSampNum;
            T_SiNoiseData postSampThreshold;
        } quickUnlockBaseline;
    } freqChannel;                                   /*!< 跳频通道参数--需要用户指定 */
    struct
    {
        T_SiData lowThreshold;
        T_SiData highThreshold;
        T_SiData forceValidLowThreshold;
        T_SiData forceValidHighThreshold;
    } mergeChannel;
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaFreq10Doorctrl;

/**
* @brief        跳频门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;
    uint8_t keyPressed;
    uint32_t pressingMs;
    uint32_t lockBaselineBeginTimeMs;
    struct
    {
        uint16_t threshold12TimeCnt;
        uint16_t threshold2KeepTimeCnt;
        uint16_t forceValidCalcCnt;
        T_SiData maxDiff;
        uint8_t lastForceValidFlag;
        uint32_t lastForceValidTime;
    } detectChannel;
    struct
    {
        uint8_t status;
        uint8_t lockBaseline;
        uint8_t lockKeyFlag;
        uint32_t beginLockBaselineMs;
        T_SiFastNoise fn;
        float unlockValue;
        uint32_t unlockBaselineBeginTimeMs;
        uint32_t unlockBaselineBeginLockKeyMs;

        struct
        {
            uint8_t status;
            uint8_t preValid;
            T_SiNoiseData postValue;
            uint32_t beginTimeMs;
            T_SiFastNoise fn;
        } quickUnlockBaseline;

        T_SiData diff2Stage;
    } freqChannel;
    struct
    {
        T_SiData value;
        T_SiData validValue;
        T_SiData forceValidValue;
    } mergeChannel;
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataFreq10Doorctrl;

/**
* @brief        跳频门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaFreq10Doorctrl para;    /*!< 判决器算法参数 */
    T_SiArbiterDataFreq10Doorctrl data;  /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterFreq10Doorctrl;

/**
* @brief    跳频门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterFreq10DoorctrlNodeInit(T_SiArbiterFreq10Doorctrl *nd, const T_SiArbiterParaFreq10Doorctrl *para, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

/**
 * @defgroup ArbiterFreqShapeDoorctrl 跳频门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        跳频门把手判决器算法参数
*/
typedef struct
{
    uint8_t detectChannelNo;                         /*!< 检测通道编号--需要用户指定 */
    uint8_t freqChannelNo;                           /*!< 跳频通道编号--需要用户指定 */
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        T_SiData forceValidThreshold;
        uint16_t forceValidCalcTimeout;
        uint16_t doubleForceValidTimeMs;

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压检测保持时间，在这段时间内检测到符合2阶按压条件，即认为按压有效--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */

        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */
    } detectChannel;      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.满足最大阈值范围，都满足后才认为按压有效 */
    struct
    {
        uint32_t forceReleaseBaselineTimeMs;
        T_SiData lockBaselineThreshold;
        uint8_t releaseThreshold;
        uint8_t alwaysUpdataUnlockValue;
        uint8_t unlockBaselineReInit;
        uint16_t unlockBaselineSampNum;
        T_SiNoiseData unlockBaselineThreshold;
        uint16_t unlockBaselineEliminateMs;
        uint16_t unlockBaselineLockKeyMs;

        T_SiData lowThreshold;                       /*!< 2阶信号稳定时跳频通道的diff低阈值--需要用户指定 */
        T_SiData highThreshold;                      /*!< 2阶信号稳定时跳频通道的diff高阈值--需要用户指定 */

        struct
        {
            uint8_t enable;
            uint16_t preDetectMs;
            uint16_t postDetectMs;
            uint16_t postSampNum;
            T_SiNoiseData postSampThreshold;
        } quickUnlockBaseline;
    } freqChannel;                                   /*!< 跳频通道参数--需要用户指定 */
    struct
    {
        T_SiData lowThreshold;
        T_SiData highThreshold;
        T_SiData forceValidLowThreshold;
        T_SiData forceValidHighThreshold;
    } mergeChannel;
    struct
    {
        uint16_t lowThreshold;
        uint16_t highThreshold;
        uint16_t cycleThreshold;
        uint8_t marginRatio;
        uint8_t enterCount;
        uint8_t exitCount;

        uint8_t staticExitEnable;
        uint8_t staticExitChannelNo;
        uint8_t staticExitReset;
        uint16_t staticExitWindowLen;
        uint16_t staticExitEliminateTimeMs;
        T_SiNoiseData staticExitThreshold;
    } shape;
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaFreqShapeDoorctrl;

/**
* @brief        跳频门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;
    uint8_t keyPressed;
    uint32_t pressingMs;
    uint32_t lockBaselineBeginTimeMs;
    struct
    {
        uint16_t threshold12TimeCnt;
        uint16_t threshold2KeepTimeCnt;
        uint16_t forceValidCalcCnt;
        T_SiData maxDiff;
        uint8_t lastForceValidFlag;
        uint32_t lastForceValidTime;
    } detectChannel;
    struct
    {
        uint8_t status;
        uint8_t lockBaseline;
        uint8_t lockKeyFlag;
        uint32_t beginLockBaselineMs;
        T_SiFastNoise fn;
        float unlockValue;
        uint32_t unlockBaselineBeginTimeMs;
        uint32_t unlockBaselineBeginLockKeyMs;

        struct
        {
            uint8_t status;
            uint8_t preValid;
            T_SiNoiseData postValue;
            uint32_t beginTimeMs;
            T_SiFastNoise fn;
        } quickUnlockBaseline;

        T_SiData diff2Stage;
    } freqChannel;
    struct
    {
        T_SiData value;
        T_SiData validValue;
        T_SiData forceValidValue;
    } mergeChannel;
    struct
    {
        uint8_t noiseStatus;
        uint8_t noiseDetect;
        T_SiNoiseData noiseValue;

        uint8_t valid;
        uint8_t status;
        uint8_t count;
        uint16_t detectTimeMs;
        uint16_t detectIntvalTimeMs;
        uint32_t beginDetectTimeMs;

        uint32_t noiseBeginExitMs;
        T_SiFastNoise fn;
    } shape;
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataFreqShapeDoorctrl;

/**
* @brief        跳频门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaFreqShapeDoorctrl para;    /*!< 判决器算法参数 */
    T_SiArbiterDataFreqShapeDoorctrl data;  /*!< 判决器数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterFreqShapeDoorctrl;

/**
* @brief    跳频门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterFreqShapeDoorctrlNodeInit(T_SiArbiterFreqShapeDoorctrl *nd, const T_SiArbiterParaFreqShapeDoorctrl *para, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

/**
 * @defgroup ArbiterWaterProofDoorctrl 防水门把手判决器
 * @ingroup arbiter
 * 判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间及其它一些条件，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
 * @{
 */
//******支持操作：单击

/**
* @brief        防水门把手判决器算法参数
*/
typedef enum
{
    SI_WATER_PROOF_COUNT = 0,       /*!< 计数模式，窗口内所有数据低于阈值 */
    SI_WATER_PROOF_AVG              /*!< 均值模式，窗口内均值数据低于阈值 */
} T_SiWaterProofCalc;
typedef struct
{
    uint8_t pressEliminate;                          /*!< 按压消抖次数--需要用户指定 */
    uint8_t releaseEliminate;                        /*!< 释放消抖次数--需要用户指定 */
    struct
    {
        T_SiData pressThreshold1;                    /*!< 1阶按压阈值--需要用户指定 */
        T_SiData pressThreshold2;                    /*!< 2阶按压阈值--需要用户指定 */
        uint8_t releaseThreshold;                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        uint16_t threshold12TimeLow;                 /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold12TimeHigh;                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        uint16_t threshold2KeepTime;                 /*!< 2阶按压阈值最短持续时间，单位是采样个数--需要用户指定 */

        uint16_t noisePreDummyTime;                  /*!< 在采集噪音前等待的时间，单位是采样个数，目的是等待手指按压信号稳定后测量噪音--需要用户指定 */
        uint16_t noiseSampNum;                       /*!< 噪音采样个数，0表示不测量噪音，最大支持SI_NOISE_DATA_DOORCTRL_WINDOWSIZE--需要用户指定 */
        uint8_t noiseUseRawdata;                     /*!< 1表示用rawdata测噪音，否则用filterdata测噪音--需要用户指定 */
        T_SiNoiseData noiseThreshold;                /*!< 噪音阈值，低于本阈值认为信号稳定--需要用户指定 */
        uint16_t noiseKeepTimeThreshold;             /*!< 噪音持续时间阈值，单位是采样个数，当噪音>noiseThreshold状态持续的时间小于noiseKeepTimeThreshold时认为按压有效--需要用户指定 */

        T_SiData maxDiffLowThreshold;                /*!< 最大diff阈值下边界--需要用户指定 */
        T_SiData maxDiffHighThreshold;               /*!< 最大diff阈值上边界--需要用户指定 */
    } pressDetect;      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.噪音满足时间阈值，4.满足最大阈值范围，都满足后才认为按压有效 */
    struct
    {
        uint8_t windowLen;                          /*!< 窗口长度，0表示关闭防水功能，最大支持SI_NOISE_DATA_DOORCTRL_WINDOWSIZE--需要用户指定 */
        T_SiWaterProofCalc calcType;                /*!< 防水算法类型--需要用户指定 */
        T_SiData waterThreshold;                    /*!< 防水阈值--需要用户指定 */
        uint16_t waterDetectTimeThreshold;          /*!< 防水检测持续时间阈值，单位是采样个数，若在一段时间内仍未检测到防水有效信号，则认为防水检测失败--需要用户指定 */
    } waterProof;   /*在waterDetectTimeThreshold时间内，检测到waterValue>waterThreshold时,认为按压有效*/
    uint16_t releaseLockBaselineMs;                  /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    uint16_t threshold1ForceReleaseTimeS;            /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    uint16_t threshold2ForceReleaseTimeS;            /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiArbiterParaWaterProofDoorctrl;                 /*!< 需要两个通道，1通道作为检测通道，2通道作为防水通道 */

/**
* @brief        防水门把手判决器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t dummy;
    //! @endcond       //doxygen中隐藏
} T_SiArbiterDataWaterProofDoorctrl;
typedef struct
{
    //! @cond
    uint8_t status;                 /*!< 按键状态 */
    uint8_t eliminateCnt;           /*!< 消抖计数器 */
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    uint8_t keyPressed;             /*!< 是否被按下 */
    uint32_t pressingMs;            /*!< 按压时长计数器 */
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
    struct
    {
        uint16_t threshold12TimeCnt;  /*!< 从1阶阈值到2阶阈值时间计数器 */
        uint16_t threshold2KeepEnable; /*!< 允许2阶计数 */
        uint16_t threshold2KeepTimeCnt; /*!< 2阶阈值持续时间 */
        T_SiData maxDiff;           /*!< 最大diff值 */
        uint16_t noisePreDummyCnt;    /*!< 在采集噪音前等待的时间 */
        uint8_t noiseEndFlag;  /*!< 噪音检测结束 */
        uint8_t noiseValid;    /*!< 噪音有效 */
        uint16_t noiseSampCount;    /*!< 噪音采样个数 */
        T_SiData noiseWindow[SI_NOISE_DATA_DOORCTRL_WINDOWSIZE];   /*!< 噪音数据缓冲区 */
        T_SiNoiseData noiseValue;   /*!< 噪音值 */
        uint16_t noiseKeepTimeCnt;  /*!< 噪音保持时间 */
    } pressDetect;                  /*!< 按压检测 */
    struct
    {
        uint8_t waterEndFlag;  /*!< 防水检测结束 */
        uint8_t waterValid;    /*!< 防水有效 */
        uint8_t windowCount;  /*!< 窗口长度 */
        uint16_t waterDetectTimeCnt;  /*!< 防水检测时间 */
        T_SiData waterValue;  /*!< 防水值，需要<waterThreshold */
        T_SiData waterWindow[SI_NOISE_DATA_DOORCTRL_WINDOWSIZE];    /*!< 防水窗口 */
    } waterProof;      /*!< 防水 */
    //! @endcond       //doxygen中隐藏
} T_SiArbiterRunDataWaterProofDoorctrl;

/**
* @brief        防水门把手判决器描述符
* @details      判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间，认为按键被松开，每个按键阈值独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;            /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaWaterProofDoorctrl para;    /*!< 判决器算法参数 */
    uint8_t dataLen;                 /*!< 判决器数据缓冲区个数 */
    T_SiArbiterDataWaterProofDoorctrl *pdata;  /*!< 判决器数据 */
    T_SiArbiterRunDataWaterProofDoorctrl runData; /*!< 运行时数据 */
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; /*!< 值改变回调接口，keyNo按键编号，status按键状态 */
} T_SiArbiterWaterProofDoorctrl;

/**
* @brief    防水门把手判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @param[in]   dataLen 判决器缓冲区长度
* @param[in]   pdata 判决器缓冲区
* @param[in]   valueChangedCallback 按键值改变回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterWaterProofDoorctrlNodeInit(T_SiArbiterWaterProofDoorctrl *nd, const T_SiArbiterParaWaterProofDoorctrl *para, uint8_t dataLen, T_SiArbiterDataWaterProofDoorctrl *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);
/** @} */

//************************************************************************************
//空判决器

/**
 * @defgroup ArbiterNone 空判决器
 * @ingroup arbiter
 * @{
 */

/**
* @brief        空判决器描述符
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;        /*!< 判决器基类，必须放在开头 */
} T_SiArbiterNone;

/**
* @brief    空判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterNoneNodeInit(T_SiArbiterNone *nd);
/** @} */

//************************************************************************************
//自定义判决器

/**
 * @defgroup ArbiterCustom 自定义判决器
 * @ingroup arbiter
 * 判决器原理：执行用户自定义的run函数判断信号是否有效
 * @{
 */

/**
* @brief        自定义判决器算法参数
*/
typedef struct
{
    int (*baselineLocked)(T_SiObject *obj, uint8_t keyNo);              /*!< 基线更新是否被判决器锁定，keyNo表示信号编号，返回值1表示被锁定（基线将不再更新），0表示未被锁定（基线正常更新） */
    void (*run)(T_SiObject *obj, uint8_t keyNum, const T_SiData *filterBuf, const T_SiData *baselineBuf);     /*!< 运行，keyNum按键个数，filterBuf滤波缓冲区，baselineBuf基线缓冲区 */
} T_SiArbiterParaCustom;

/**
* @brief        自定义判决器描述符
* @details      判决器原理：执行用户自定义的run函数判断信号是否有效
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiArbiterBase base;        /*!< 判决器基类，必须放在开头 */
    T_SiArbiterParaCustom para;  /*!< 判决器算法参数 */
} T_SiArbiterCustom;

/**
* @brief    自定义判决器节点描述符初始化
* @param[in]   nd 判决器节点
* @param[in]   para 判决器参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiArbiterCustomNodeInit(T_SiArbiterCustom *nd, const T_SiArbiterParaCustom *para);
/** @} */

#ifdef __cplusplus
}
#endif

#endif
