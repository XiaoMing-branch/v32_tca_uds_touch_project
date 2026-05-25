/**
*****************************************************************************
* @brief  装饰滤波器
* @file   si_filter_dctor.h
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

#ifndef __SI_FILTER_DCTOR_H__
#define __SI_FILTER_DCTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "si_filter.h"

/**
 * @defgroup filterDctor 装饰滤波器
 * 本文件中所有滤波器都提供装饰器支持，可实现滤波器堆叠
 */
/** @} */

//****************************带装饰器支持的MinMax滤波器**************************
/**
 * @defgroup FilterDctorMinMax 带装饰器支持的MinMax滤波器
 * @ingroup filterDctor
 * 滤波器原理：去除窗口中n个最大值和m个最小值，求平均，每次最大步进step
 * @{
 */

/**
* @brief        带装饰器支持的MinMax滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t fillLen;                                           /*!< 窗口中已经填充数据长度 */
    T_SiData window[SI_FILTER_DATA_MINMAX_WINDOWSIZE];         /*!< 窗口缓冲区 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataDctorMinMax;

/**
* @brief        带装饰器支持的MinMax滤波器算法参数
*/
typedef struct
{
    uint8_t windowLen;                    /*!< 窗口长度，discardMinvNum+discardMaxvNum需要小于窗口长度--需要用户指定 */
    uint8_t discardMinvNum;               /*!< 丢弃最小值个数--需要用户指定 */
    uint8_t discardMaxvNum;               /*!< 丢弃最大值个数--需要用户指定 */
    T_SiData step;                        /*!< 步进值--需要用户指定 */
} T_SiFilterParaDctorMinMax;

/**
* @brief        带装饰器支持的MinMax滤波器描述符
* @details      滤波器原理：去除窗口中n个最大值和m个最小值，求平均，每次最大步进step
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMinMax para;        /*!< 滤波器算法参数 */
    uint8_t dataLen;                       /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorMinMax *pdata;      /*!< 滤波器数据 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMinMax;

/**
* @brief    带装饰器支持的MinMax滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMinMaxNodeInit(T_SiFilterDctorMinMax *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMinMax *para, uint8_t dataLen, T_SiFilterDataDctorMinMax *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的Sum累加放大器**************************
/**
 * @defgroup FilterDctorSum 带装饰器支持的Sum累加放大器
 * @ingroup filterDctor
 * 滤波器原理：将数值多次累加放大
 * @{
 */

/**
* @brief        带装饰器支持的Sum累加放大器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sumCnt;              /*!< 已经累加次数 */
    T_SiData sumValue;            /*!< 累加值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataDctorSum;

/**
* @brief        带装饰器支持的Sum累加放大器算法参数
*/
typedef struct
{
    uint16_t ratio;                       /*!< 放大比例，--需要用户指定 */
    T_SiData offset;                      /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiFilterParaDctorSum;

/**
* @brief        带装饰器支持的Sum累加放大器描述符
* @details      滤波器原理：将数值多次累加放大
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorSum para;           /*!< 滤波器算法参数 */
    uint8_t dataLen;                       /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorSum *pdata;         /*!< 滤波器数据 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorSum;

/**
* @brief    带装饰器支持的Sum累加放大器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorSumNodeInit(T_SiFilterDctorSum *nd, T_SiFilterBase *component, const T_SiFilterParaDctorSum *para, uint8_t dataLen, T_SiFilterDataDctorSum *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的每通道独立参数Sum累加放大器**************************
/**
 * @defgroup FilterDctorSum 带装饰器支持的每通道独立参数Sum累加放大器
 * @ingroup filterDctor
 * 滤波器原理：将数值多次累加放大
 * @{
 */

/**
* @brief        带装饰器支持的每通道独立参数Sum累加放大器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef T_SiFilterDataDctorSum T_SiFilterDataDctorMtSum;

/**
* @brief        带装饰器支持的每通道独立参数Sum累加放大器算法参数
*/
typedef T_SiFilterParaDctorSum T_SiFilterParaDctorMtSum[SI_PNODE_CH_MAXNUM];

/**
* @brief        带装饰器支持的每通道独立参数Sum累加放大器描述符
* @details      滤波器原理：将数值多次累加放大
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMtSum para;         /*!< 滤波器算法参数 */
    uint8_t dataLen;                       /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorMtSum *pdata;       /*!< 滤波器数据 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMtSum;

/**
* @brief    带装饰器支持的每通道独立参数Sum累加放大器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMtSumNodeInit(T_SiFilterDctorMtSum *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtSum *para, uint8_t dataLen, T_SiFilterDataDctorMtSum *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的窗口Sum累加放大器**************************
/**
 * @defgroup FilterDctorWinSum 带装饰器支持的窗口Sum累加放大器
 * @ingroup filterDctor
 * 滤波器原理：将数值多次累加放大
 * @{
 */

/**
* @brief        带装饰器支持的窗口Sum累加放大器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t fillLen;                                           /*!< 窗口中已经填充数据长度 */
    T_SiData window[SI_FILTER_DATA_MINMAX_WINDOWSIZE];         /*!< 窗口缓冲区 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataDctorWinSum;

/**
* @brief        带装饰器支持的窗口Sum累加放大器算法参数
*/
typedef struct
{
    uint16_t ratio;                    /*!< 放大倍数，最大12--需要用户指定 */
    T_SiData offset;                   /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiFilterParaDctorWinSum;

/**
* @brief        带装饰器支持的窗口Sum累加放大器描述符
* @details      滤波器原理：将数值多次累加放大
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorWinSum para;        /*!< 滤波器算法参数 */
    uint8_t dataLen;                       /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorWinSum *pdata;      /*!< 滤波器数据 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorWinSum;

/**
* @brief    带装饰器支持的窗口Sum累加放大器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorWinSumNodeInit(T_SiFilterDctorWinSum *nd, T_SiFilterBase *component, const T_SiFilterParaDctorWinSum *para, uint8_t dataLen, T_SiFilterDataDctorWinSum *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的每通道独立参数窗口Sum累加放大器**************************
/**
 * @defgroup FilterDctorMtWinSum 带装饰器支持的每通道独立参数窗口Sum累加放大器
 * @ingroup filterDctor
 * 滤波器原理：将数值多次累加放大
 * @{
 */

/**
* @brief        带装饰器支持的每通道独立参数窗口Sum累加放大器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef T_SiFilterDataDctorWinSum T_SiFilterDataDctorMtWinSum;

/**
* @brief        带装饰器支持的每通道独立参数窗口Sum累加放大器算法参数
*/
typedef T_SiFilterParaDctorWinSum T_SiFilterParaDctorMtWinSum[SI_PNODE_CH_MAXNUM];

/**
* @brief        带装饰器支持的每通道独立参数窗口Sum累加放大器描述符
* @details      滤波器原理：将数值多次累加放大
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMtWinSum para;        /*!< 滤波器算法参数 */
    uint8_t dataLen;                       /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorMtWinSum *pdata;      /*!< 滤波器数据 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMtWinSum;

/**
* @brief    带装饰器支持的每通道独立参数窗口Sum累加放大器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMtWinSumNodeInit(T_SiFilterDctorMtWinSum *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtWinSum *para, uint8_t dataLen, T_SiFilterDataDctorMtWinSum *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的每通道独立参数窗口Sum累加放大器**************************
/**
 * @defgroup FilterDctorMtWinSum 带装饰器支持的每通道独立参数窗口Sum累加放大器
 * @ingroup filterDctor
 * 滤波器原理：将数值多次累加放大
 * @{
 */

#define SI_FILTER_DATA_LITE_MINMAX_WINDOWSIZE              3

/**
* @brief        带装饰器支持的窗口Sum累加放大器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t fillLen;                                           /*!< 窗口中已经填充数据长度 */
    T_SiData window[SI_FILTER_DATA_LITE_MINMAX_WINDOWSIZE];         /*!< 窗口缓冲区 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataDctorMtLiteWinSum;

/**
* @brief        带装饰器支持的每通道独立参数窗口Sum累加放大器算法参数
*/
typedef T_SiFilterParaDctorWinSum T_SiFilterParaDctorMtLiteWinSum[SI_PNODE_CH_MAXNUM];

/**
* @brief        带装饰器支持的每通道独立参数窗口Sum累加放大器描述符
* @details      滤波器原理：将数值多次累加放大
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMtLiteWinSum para;        /*!< 滤波器算法参数 */
    uint8_t dataLen;                       /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorMtLiteWinSum *pdata;      /*!< 滤波器数据 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMtLiteWinSum;

/**
* @brief    带装饰器支持的每通道独立参数窗口Sum累加放大器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMtLiteWinSumNodeInit(T_SiFilterDctorMtLiteWinSum *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtLiteWinSum *para, uint8_t dataLen, T_SiFilterDataDctorMtLiteWinSum *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的Mul倍数放大器**************************
/**
 * @defgroup FilterDctorMul 带装饰器支持的Mul倍数放大器
 * @ingroup filterDctor
 * 滤波器原理：将数据直接乘以一个放大倍数
 * @{
 */

/**
* @brief        带装饰器支持的Mul倍数放大器算法参数
*/
typedef struct
{
    uint16_t ratio;                       /*!< 放大比例，--需要用户指定 */
    T_SiData offset;                      /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiFilterParaDctorMul;

/**
* @brief        带装饰器支持的Mul倍数放大器描述符
* @details      滤波器原理：将数据直接乘以一个放大倍数
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMul para;           /*!< 滤波器算法参数 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMul;

/**
* @brief    带装饰器支持的Mul倍数放大器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMulNodeInit(T_SiFilterDctorMul *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMul *para, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的每通道独立参数Mul倍数放大器**************************
/**
 * @defgroup FilterDctorMulMt 带装饰器支持的每通道独立参数Mul倍数放大器
 * @ingroup filterDctor
 * 滤波器原理：将数据直接乘以一个放大倍数
 * @{
 */

/**
* @brief        带装饰器支持的每通道独立参数Mul倍数放大器算法参数
*/
typedef T_SiFilterParaDctorMul T_SiFilterParaDctorMtMul[SI_PNODE_CH_MAXNUM];

/**
* @brief        带装饰器支持的每通道独立参数Mul倍数放大器描述符
* @details      滤波器原理：将数据直接乘以一个放大倍数
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMtMul para;         /*!< 滤波器算法参数 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMtMul;

/**
* @brief    带装饰器支持的每通道独立参数Mul倍数放大器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMtMulNodeInit(T_SiFilterDctorMtMul *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtMul *para, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的Avg滤波器**************************
/**
 * @defgroup FilterDctorAvg 带装饰器支持的Avg滤波器
 * @ingroup filterDctor
 * 滤波器原理：多次采样求平均，每次最大步进step
 * @{
 */

/**
* @brief        带装饰器支持的Avg滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sampCnt;                 /*!< 已经采样次数 */
    int32_t sampSumValue;            /*!< 采样值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataDctorAvg;

/**
* @brief        带装饰器支持的Avg滤波器算法参数
*/
typedef struct
{
    uint16_t cntThreshold;            /*!< 采样计数器阈值，到达后更新滤波值--需要用户指定 */
    T_SiData step;                    /*!< 步进值，--需要用户指定 */
} T_SiFilterParaDctorAvg;

/**
* @brief        带装饰器支持的Avg滤波器描述符
* @details      滤波器原理：多次采样求平均，每次最大步进step
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorAvg para;        /*!< 滤波器算法参数 */
    uint8_t dataLen;                    /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorAvg *pdata;      /*!< 滤波器数据 */
    T_SiFilterBase *component;          /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorAvg;

/**
* @brief    带装饰器支持的Avg滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorAvgNodeInit(T_SiFilterDctorAvg *nd, T_SiFilterBase *component, const T_SiFilterParaDctorAvg *para, uint8_t dataLen, T_SiFilterDataDctorAvg *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的巴特沃斯滤波器**************************

//! @cond

//******滤波器原理：巴特沃斯滤波器

//滤波器数据类型
typedef T_ButterworthData T_SiFilterDataDctorButterworth;
//滤波器算法参数
typedef T_ButterworthPara T_SiFilterParaDctorButterworth;
//滤波器描述符
typedef struct
{
    T_SiFilterBase base;                   //滤波器基类，必须放在开头
    T_SiFilterParaDctorButterworth para;   //滤波器算法参数
    uint8_t dataLen;                       //滤波器数据缓冲区个数
    T_SiFilterDataDctorButterworth *pdata; //滤波器数据
    T_SiFilterBase *component;             //被装饰的滤波器，可以为NULL
} T_SiFilterDctorButterworth;

//Butterworth滤波器节点描述符初始化，component为被装饰的滤波器，为NULL表示不装饰，SI_RT_OK表示成功
T_SiErrRt SiFilterDctorButterworthNodeInit(T_SiFilterDctorButterworth *nd, T_SiFilterBase *component, const T_SiFilterParaDctorButterworth *para, uint8_t dataLen, T_SiFilterDataDctorButterworth *pdata, uint8_t filterBufLen, T_SiData *filterBuf);

//! @endcond       //doxygen中隐藏

//****************************带装饰器支持的每通道独立参数巴特沃斯滤波器**************************

//! @cond

//******滤波器原理：巴特沃斯滤波器

//滤波器数据类型
typedef T_ButterworthData T_SiFilterDataDctorMtButterworth;
//滤波器算法参数
typedef T_ButterworthPara T_SiFilterParaDctorMtButterworth[SI_PNODE_CH_MAXNUM];
//滤波器描述符
typedef struct
{
    T_SiFilterBase base;                     //滤波器基类，必须放在开头
    T_SiFilterParaDctorMtButterworth para;   //滤波器算法参数
    uint8_t dataLen;                         //滤波器数据缓冲区个数
    T_SiFilterDataDctorMtButterworth *pdata; //滤波器数据
    T_SiFilterBase *component;               //被装饰的滤波器，可以为NULL
} T_SiFilterDctorMtButterworth;

//MtButterworth滤波器节点描述符初始化，component为被装饰的滤波器，为NULL表示不装饰，SI_RT_OK表示成功
T_SiErrRt SiFilterDctorMtButterworthNodeInit(T_SiFilterDctorMtButterworth *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtButterworth *para, uint8_t dataLen, T_SiFilterDataDctorMtButterworth *pdata, uint8_t filterBufLen, T_SiData *filterBuf);

//! @endcond       //doxygen中隐藏

//****************************带装饰器支持的IIR滤波器**************************
/**
 * @defgroup FilterDctorIIR 带装饰器支持的IIR滤波器
 * @ingroup filterDctor
 * 滤波器原理：IIR滤波器
 * @{
 */

/**
* @brief        带装饰器支持的IIR滤波器算法参数
*/
typedef struct
{
    uint8_t gain;                          /*!< 增益，阶数--需要用户指定 */
} T_SiFilterParaDctorIIR;

/**
* @brief        带装饰器支持的IIR滤波器描述符
* @details      滤波器原理：IIR低通滤波器
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorIIR para;           /*!< 滤波器算法参数 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorIIR;

/**
* @brief    带装饰器支持的IIR滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorIIRNodeInit(T_SiFilterDctorIIR *nd, T_SiFilterBase *component, const T_SiFilterParaDctorIIR *para, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的每通道独立参数IIR滤波器**************************
/**
 * @defgroup FilterDctorIIRMt 带装饰器支持的每通道独立参数IIR滤波器
 * @ingroup filterDctor
 * 滤波器原理：IIR滤波器
 * @{
 */

/**
* @brief        带装饰器支持的每通道独立参数IIR滤波器算法参数
*/
typedef T_SiFilterParaDctorIIR T_SiFilterParaDctorMtIIR[SI_PNODE_CH_MAXNUM];

/**
* @brief        带装饰器支持的每通道独立参数IIR滤波器描述符
* @details      滤波器原理：IIR低通滤波器
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMtIIR para;         /*!< 滤波器算法参数 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMtIIR;

/**
* @brief    带装饰器支持的每通道独立参数IIR滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMtIIRNodeInit(T_SiFilterDctorMtIIR *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtIIR *para, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的NoiseAvoid滤波器**************************
/**
 * @defgroup FilterDctorNoiseAvoid 带装饰器支持的NoiseAvoid滤波器
 * @ingroup filterDctor
 * 滤波器原理：小于微调值时保持上一次值，在微调和粗调范围之间时按微调步进调节，大于粗调时直接采用最新值
 * @{
 */

/**
* @brief        带装饰器支持的NoiseAvoid滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef T_NoiseAvoidData T_SiFilterDataDctorNoiseAvoid;

/**
* @brief        带装饰器支持的NoiseAvoid滤波器算法参数
*/
typedef T_NoiseAvoidPara T_SiFilterParaDctorNoiseAvoid;

/**
* @brief        带装饰器支持的NoiseAvoid滤波器描述符
* @details      滤波器原理：小于微调值时保持上一次值，在微调和粗调范围之间时按微调步进调节，大于粗调时直接采用最新值
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                  /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorNoiseAvoid para;   /*!< 滤波器算法参数 */
    uint8_t dataLen;                      /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorNoiseAvoid *pdata; /*!< 滤波器数据 */
    T_SiFilterBase *component;            /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorNoiseAvoid;

/**
* @brief    带装饰器支持的NoiseAvoid滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorNoiseAvoidNodeInit(T_SiFilterDctorNoiseAvoid *nd, T_SiFilterBase *component, const T_SiFilterParaDctorNoiseAvoid *para, uint8_t dataLen, T_SiFilterDataDctorNoiseAvoid *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的每通道独立参数NoiseAvoid滤波器**************************
/**
 * @defgroup FilterDctorNoiseAvoidMt 带装饰器支持的每通道独立参数NoiseAvoid滤波器
 * @ingroup filterDctor
 * 滤波器原理：小于微调值时保持上一次值，在微调和粗调范围之间时按微调步进调节，大于粗调时直接采用最新值
 * @{
 */

/**
* @brief        带装饰器支持的每通道独立参数NoiseAvoid滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef T_NoiseAvoidData T_SiFilterDataDctorMtNoiseAvoid;

/**
* @brief        带装饰器支持的每通道独立参数NoiseAvoid滤波器算法参数
*/
typedef T_NoiseAvoidPara T_SiFilterParaDctorMtNoiseAvoid[SI_PNODE_CH_MAXNUM];

/**
* @brief        带装饰器支持的每通道独立参数NoiseAvoid滤波器描述符
* @details      滤波器原理：小于微调值时保持上一次值，在微调和粗调范围之间时按微调步进调节，大于粗调时直接采用最新值
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                    /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMtNoiseAvoid para;   /*!< 滤波器算法参数 */
    uint8_t dataLen;                        /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorMtNoiseAvoid *pdata; /*!< 滤波器数据 */
    T_SiFilterBase *component;              /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMtNoiseAvoid;

/**
* @brief    带装饰器支持的每通道独立参数NoiseAvoid滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMtNoiseAvoidNodeInit(T_SiFilterDctorMtNoiseAvoid *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtNoiseAvoid *para, uint8_t dataLen, T_SiFilterDataDctorMtNoiseAvoid *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************DoubleSamp滤波器******************************************************
/**
 * @defgroup FilterDctorDoubleSamp DoubleSamp滤波器
 * @ingroup filterDctor
 * 滤波器原理：保证输出采样率不变的情况下，将输入值放大一倍
 * @{
 */

/**
* @brief        DoubleSamp滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiData lastValue;                   /*!< 上一次值 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataDctorDoubleSamp;

/**
* @brief        DoubleSamp滤波器算法参数
*/
typedef struct
{
    T_SiData offset;                      /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiFilterParaDctorDoubleSamp;

/**
* @brief        DoubleSamp滤波器描述符
* @details      滤波器原理：保证输出采样率不变的情况下，将输入值放大一倍
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                          /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorDoubleSamp para;           /*!< 滤波器算法参数 */
    uint8_t dataLen;                              /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorDoubleSamp *pdata;         /*!< 滤波器数据 */
    T_SiFilterBase *component;                    /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorDoubleSamp;

/**
* @brief    DoubleSamp滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorDoubleSampNodeInit(T_SiFilterDctorDoubleSamp *nd, T_SiFilterBase *component, const T_SiFilterParaDctorDoubleSamp *para, uint8_t dataLen, T_SiFilterDataDctorDoubleSamp *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************每通道独立参数DoubleSamp滤波器******************************************************
/**
 * @defgroup FilterDctorDoubleSampMt 每通道独立参数DoubleSamp滤波器
 * @ingroup filterDctor
 * 滤波器原理：保证输出采样率不变的情况下，将输入值放大一倍
 * @{
 */

/**
* @brief        每通道独立参数DoubleSamp滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef T_SiFilterDataDctorDoubleSamp T_SiFilterDataDctorMtDoubleSamp;

/**
* @brief        每通道独立参数DoubleSamp滤波器算法参数
*/
typedef T_SiFilterParaDctorDoubleSamp T_SiFilterParaDctorMtDoubleSamp[SI_PNODE_CH_MAXNUM];

/**
* @brief        每通道独立参数DoubleSamp滤波器描述符
* @details      滤波器原理：保证输出采样率不变的情况下，将输入值放大一倍
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                          /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMtDoubleSamp para;         /*!< 滤波器算法参数 */
    uint8_t dataLen;                              /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataDctorMtDoubleSamp *pdata;       /*!< 滤波器数据 */
    T_SiFilterBase *component;                    /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMtDoubleSamp;

/**
* @brief    每通道独立参数DoubleSamp滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，为NULL表示不装饰
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMtDoubleSampNodeInit(T_SiFilterDctorMtDoubleSamp *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMtDoubleSamp *para, uint8_t dataLen, T_SiFilterDataDctorMtDoubleSamp *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************带装饰器支持的虚拟按键滤波器**************************
/**
 * @defgroup FilterDctorMergeKey 带装饰器支持的虚拟按键滤波器
 * @ingroup filterDctor
 * 滤波器原理：多个物理按键进行op运算，生成新的虚拟通道
 * @{
 */

/**
* @brief        带装饰器支持的虚拟按键滤波器算法参数
*/
typedef struct
{
    uint16_t key_mask;           //置1掩码的按键raw值累加组合生成新的按键
    uint16_t sign_mask;          //符号位掩码，如果为0表示正加，如果为1表示负加
    uint16_t new_key_channelno;  //新生成的按键通道号，从0开始计数，如果和掩码按键冲突会覆盖掩码按键值
    int raw_offset;              //按键raw值累加后-raw_offset为新虚拟按键raw值
} T_SiFilterParaMergeKeyItem;
typedef struct
{
    uint8_t keyNum;                          /*!< 合并按键个数--需要用户指定 */
    const T_SiFilterParaMergeKeyItem *keys;        /*!< 按键参数--需要用户指定 */
} T_SiFilterParaDctorMergeKey;

/**
* @brief        带装饰器支持的虚拟按键滤波器描述符
* @details      滤波器原理：多个物理按键进行op运算，生成新的虚拟通道
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDctorMergeKey para;      /*!< 滤波器算法参数 */
    T_SiFilterBase *component;             /*!< 被装饰的滤波器，可以为NULL */
} T_SiFilterDctorMergeKey;

/**
* @brief    带装饰器支持的虚拟按键滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   component 被装饰的滤波器，不可以为NULL
* @param[in]   para 滤波器参数
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDctorMergeKeyNodeInit(T_SiFilterDctorMergeKey *nd, T_SiFilterBase *component, const T_SiFilterParaDctorMergeKey *para, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

#ifdef __cplusplus
}
#endif

#endif
