/**
*****************************************************************************
* @brief  si noise exit header
* @file   si_noise_exit.h
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

#ifndef __SI_NOISE_EXIT_H__
#define __SI_NOISE_EXIT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file
* @brief        噪音检测退出条件
* @date       2023-12-18
* @version  v1.0
* @par Copyright(c):    tinychip corporation
* @par History:
*   version: author, date, desc\n
*/

/**
* @brief        噪音检测退出条件基类
* @details      定义噪音检测器退出实现方法
* @attention    不要直接操作本结构体内容
*/
struct T_SiNoiseExitConditionBase
{
    uint8_t exitFlag;           //退出标志，1可以退出
    T_SiNoiseData noiseValue;   //噪音值
    uint32_t lastScheduleCount; //上一次调度器计数器值
    void (*reset)(T_SiObject *obj, struct T_SiNoiseExitConditionBase *self);
    void (*run)(T_SiObject *obj, struct T_SiNoiseExitConditionBase *self);
};
typedef struct T_SiNoiseExitConditionBase T_SiNoiseExitConditionBase;

//**********************************************
//MinMax退出条件，当MinMax差值低于阈值，允许退出

#define SI_NOISE_EXIT_MINMAX_WINDOWSIZE      20                 /*!< MinMax噪音检测退出条件窗口缓冲区最大长度 */

/**
* @brief        MinMax噪音检测退出条件数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    uint8_t fillLen;                                              /*!< 窗口中已经填充数据长度 */
    uint8_t skipCnt;                                              /*!< 跳帧计数器 */
    T_SiData window[SI_NOISE_EXIT_MINMAX_WINDOWSIZE];             /*!< 窗口缓冲区 */
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseExitConditionDataMinMax;

/**
* @brief        MinMax噪音检测退出条件算法参数
*/
typedef struct
{
    uint8_t channelNo;                                /*!< 逻辑通道编号，--需要用户指定 */
    uint8_t windowLen;                                /*!< 窗口长度，--需要用户指定 */
    uint8_t sampIntval;                               /*!< 采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定 */
    uint16_t eliminateTimeMs;                         /*!< 消抖时间，持续一段时间低于阈值，认为可以退出--需要用户指定 */
    T_SiNoiseData threshold;                          /*!< 噪音检测阈值--需要用户指定 */
} T_SiNoiseExitConditionParaMinMax;

/**
* @brief        MinMax噪音检测退出条件描述符
* @details      噪音检测器原理：当窗口中最大值和最小值的差值小于阈值threshold时，且持续一段时间，允许退出
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseExitConditionBase base;                   /*!< 噪音检测退出条件基类，必须放在开头 */
    T_SiNoiseExitConditionParaMinMax para;             /*!< 噪音检测退出条件算法参数 */
    T_SiNoiseExitConditionDataMinMax data;             /*!< 噪音检测退出条件数据 */
} T_SiNoiseExitConditionMinMax;

/**
* @brief    MinMax噪音检测退出条件节点描述符初始化
* @param[in]   nd 噪音检测退出条件节点
* @param[in]   para 噪音检测退出条件参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseExitConditionMinMaxInit(T_SiNoiseExitConditionMinMax *nd, const T_SiNoiseExitConditionParaMinMax *para);

//**********************************************
//方差退出条件，当方差低于阈值，允许退出
/**
* @brief        方差噪音检测退出条件数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    uint8_t fillLen;                                              /*!< 窗口中已经填充数据长度 */
    uint8_t skipCnt;                                              /*!< 跳帧计数器 */
    T_SiData window[SI_NOISE_EXIT_MINMAX_WINDOWSIZE];             /*!< 窗口缓冲区 */
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseExitConditionDataVariance;

/**
* @brief        方差噪音检测退出条件算法参数
*/
typedef struct
{
    uint8_t channelNo;                                /*!< 逻辑通道编号，--需要用户指定 */
    uint8_t windowLen;                                /*!< 窗口长度，--需要用户指定 */
    uint8_t sampIntval;                               /*!< 采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定 */
    uint16_t eliminateTimeMs;                         /*!< 消抖时间，持续一段时间低于阈值，认为可以退出--需要用户指定 */
    T_SiNoiseData threshold;                          /*!< 噪音检测阈值--需要用户指定 */
} T_SiNoiseExitConditionParaVariance;

/**
* @brief        方差噪音检测退出条件描述符
* @details      噪音检测器原理：当窗口中最大值和最小值的差值小于阈值threshold时，且持续一段时间，允许退出
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseExitConditionBase base;                   /*!< 噪音检测退出条件基类，必须放在开头 */
    T_SiNoiseExitConditionParaVariance para;             /*!< 噪音检测退出条件算法参数 */
    T_SiNoiseExitConditionDataVariance data;             /*!< 噪音检测退出条件数据 */
} T_SiNoiseExitConditionVariance;

/**
* @brief    方差噪音检测退出条件节点描述符初始化
* @param[in]   nd 噪音检测退出条件节点
* @param[in]   para 噪音检测退出条件参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseExitConditionVarianceInit(T_SiNoiseExitConditionVariance *nd, const T_SiNoiseExitConditionParaVariance *para);

/**
* @brief        方差噪音检测退出条件数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    T_SiFastNoise fn;
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseExitConditionDataFastVariance;

/**
* @brief        方差噪音检测退出条件算法参数
*/
typedef struct
{
    uint8_t channelNo;                                /*!< 逻辑通道编号，--需要用户指定 */
    uint8_t windowLen;                                /*!< 窗口长度，--需要用户指定 */
    uint16_t eliminateTimeMs;                         /*!< 消抖时间，持续一段时间低于阈值，认为可以退出--需要用户指定 */
    T_SiNoiseData threshold;                          /*!< 噪音检测阈值--需要用户指定 */
} T_SiNoiseExitConditionParaFastVariance;

/**
* @brief        方差噪音检测退出条件描述符
* @details      噪音检测器原理：当窗口中最大值和最小值的差值小于阈值threshold时，且持续一段时间，允许退出
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseExitConditionBase base;                   /*!< 噪音检测退出条件基类，必须放在开头 */
    T_SiNoiseExitConditionParaFastVariance para;             /*!< 噪音检测退出条件算法参数 */
    T_SiNoiseExitConditionDataFastVariance data;             /*!< 噪音检测退出条件数据 */
} T_SiNoiseExitConditionFastVariance;

/**
* @brief    方差噪音检测退出条件节点描述符初始化
* @param[in]   nd 噪音检测退出条件节点
* @param[in]   para 噪音检测退出条件参数
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseExitConditionFastVarianceInit(T_SiNoiseExitConditionFastVariance *nd, const T_SiNoiseExitConditionParaFastVariance *para);

#ifdef __cplusplus
}
#endif

#endif
