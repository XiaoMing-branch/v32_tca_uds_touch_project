/**
*****************************************************************************
* @brief  基线追踪器
* @file   si_baseline.h
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

#ifndef __SI_BASELINE_H__
#define __SI_BASELINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup baseline 基线追踪器
 */
/** @} */

/**
 * @defgroup BaselineInterface 公共接口
 * @ingroup baseline
 * 基线追踪器相关接口及数据结构
 * @{
 */

/**
* @brief        基线追踪器基类
* @details      定义基线追踪器内部实现方法
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiBitMask readyMask;                                                       /*!< 基线就绪位掩码 */
    uint8_t bufLen;                                                              /*!< 缓冲区长度 */
    T_SiData *baselineBuf;                                                       /*!< 基线缓冲区 */

    T_SiErrRt(*init)(T_SiObject *obj);                                           /*!< 安装基线追踪器 */
    T_SiErrRt(*exit)(T_SiObject *obj);                                           /*!< 卸载基线追踪器 */
    void (*hook)(T_SiObject *obj, uint8_t keyNum, const T_SiData *baselineBuf);  /*!< 基线追踪器钩子函数 */
    void (*set)(T_SiObject *obj, uint8_t keyNo, T_SiData baseline);              /*!< 手动设置基线值 */
    void (*reset)(T_SiObject *obj, uint8_t keyNo);                               /*!< 基线追踪器复位 */

    T_SiErrRt(*run)(T_SiObject *obj, uint8_t keyNum, const T_SiData *filterBuf); /*!< 运行基线追踪器，filterBuf滤波缓冲区 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineBase;

/**
* @brief    注册基线追踪器回调函数
* @details     调度器执行完基线追踪器后，紧接着会执行其回调函数
* @param[in]   nd 基线追踪器节点
* @param[in]   hookCallback 回调函数
* @retval      无
*/
#define SiBaselineRegisterHook(nd,hookCallback)      (((T_SiBaselineBase*)(nd))->hook = hookCallback)

/**
* @brief    复制基线追踪器
* @details     从srcobj中复制基线值到dstobj中
* @param[in]   dstobj 目的对象
* @param[in]   srcobj 源对象
* @retval      SI_RT_OK 复制成功
* @retval      other 复制失败，某些基线追踪器可能禁止拷贝
*/
T_SiErrRt SiBaselineCopy(T_SiObject *dstobj, T_SiObject *srcobj);

/**
* @brief    复位单个基线值
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      SI_RT_OK 复位成功
* @retval      other 复位失败
*/
T_SiErrRt SiBaselineReset(T_SiObject *obj, uint8_t keyNo);

/**
* @brief    复位所有基线
* @param[in]   obj 信号集对象
* @retval      SI_RT_OK 复位成功
* @retval      other 复位失败
*/
T_SiErrRt SiBaselineResetAll(T_SiObject *obj);

/**
* @brief    设置基线值
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @param[in]   baselinev 基线值
* @retval      SI_RT_OK 设置成功
* @retval      other 设置失败
*/
T_SiErrRt SiBaselineSet(T_SiObject *obj, uint8_t keyNo, T_SiData baselinev);

/**
* @brief    修改基线追踪器算法参数
* @param[in]   nd 基线追踪器节点
* @param[in]   newpara 新参数
* @retval      无
*/
#define SiBaselineSetPara(nd,newpara)       (memcpy(&(nd)->para,(newpara),sizeof((nd)->para)))

/**
* @brief    基线追踪器是否就绪
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      0 未就绪
* @retval      其它 就绪
*/
#define SiBaselineIsReady(obj,keyNo) ((((T_SiBaselineBase *)(obj)->baseline)->readyMask & (0x1<<(keyNo))) != 0)

/**
* @brief    设置基线追踪器就绪位
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiBaselineSetReady(obj,keyNo) (((T_SiBaselineBase *)(obj)->baseline)->readyMask |= (0x1<<(keyNo)))

/**
* @brief    清除基线追踪器就绪位
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiBaselineClrReady(obj,keyNo) (((T_SiBaselineBase *)(obj)->baseline)->readyMask &= ~(0x1<<(keyNo)))

/** @} */

//****************************低功耗唤醒平均值基线追踪器**************************
/**
 * @defgroup BaselineKeyWkup 低功耗唤醒平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线
 * @{
 */

/**
* @brief        低功耗唤醒平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sampCnt;                  /*!< 已经采样次数 */
    uint8_t skipDataReady;             /*!< 数据包已丢弃 */
    int32_t sampSumValue;              /*!< 采样值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineDataKeyWkup;

/**
* @brief        低功耗唤醒平均值基线追踪器算法参数
*/
typedef struct
{
    uint8_t cntThreshold;          /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
    uint8_t firstCntThreshold;     /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
    T_SiData step;                 /*!< 基线追踪器步进值--需要用户指定 */
    uint8_t firstSkipDataCount;    /*!< 首次计算基线前，丢弃几个数据--需要用户指定 */
} T_SiBaselineParaKeyWkup;

/**
* @brief        低功耗唤醒平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;           /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaKeyWkup para;    /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                 /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataKeyWkup *pdata;  /*!< 基线追踪器数据 */
} T_SiBaselineKeyWkup;

/**
* @brief    低功耗唤醒平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineKeyWkupNodeInit(T_SiBaselineKeyWkup *nd, const T_SiBaselineParaKeyWkup *para, uint8_t dataLen, T_SiBaselineDataKeyWkup *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

//****************************低功耗唤醒平均值带QuickDrop基线追踪器**************************
/**
 * @defgroup BaselineKeyWkupQuickdrop 低功耗唤醒平均值带QuickDrop基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，满足QuickDrop条件执行QuickDrop
 * @{
 */

/**
* @brief        低功耗唤醒平均值带QuickDrop基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sampCnt;                  /*!< 已经采样次数 */
    uint16_t quickDropCnt;             /*!< quickDrop计数器 */
    uint8_t skipDataReady;             /*!< 数据包已丢弃 */
    int32_t sampSumValue;              /*!< 采样值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineDataKeyWkupQuickdrop;

/**
* @brief        低功耗唤醒平均值带QuickDrop基线追踪器算法参数
*/
typedef struct
{
    uint8_t cntThreshold;          /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
    uint8_t firstCntThreshold;     /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
    uint16_t quickDropHoldCnt;      /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能 */
    T_SiData quickDropThreshold;   /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
    T_SiData step;                 /*!< 基线追踪器步进值--需要用户指定 */
    uint8_t firstSkipDataCount;    /*!< 首次计算基线前，丢弃几个数据--需要用户指定 */
} T_SiBaselineParaKeyWkupQuickdrop;

/**
* @brief        低功耗唤醒平均值带QuickDrop基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，满足QuickDrop条件执行QuickDrop
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;                    /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaKeyWkupQuickdrop para;    /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                          /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataKeyWkupQuickdrop *pdata;  /*!< 基线追踪器数据 */
} T_SiBaselineKeyWkupQuickdrop;

/**
* @brief    低功耗唤醒平均值带QuickDrop基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineKeyWkupQuickdropNodeInit(T_SiBaselineKeyWkupQuickdrop *nd, const T_SiBaselineParaKeyWkupQuickdrop *para, uint8_t dataLen, T_SiBaselineDataKeyWkupQuickdrop *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

//************************************************************************************
//按键专用基线追踪器

//****************************按键平均值基线追踪器**************************
/**
 * @defgroup BaselineKeyAvg 按键平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线
 * @{
 */

/**
* @brief        按键平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sampCnt;                  /*!< 已经采样次数 */
    uint8_t quickDropCnt;             /*!< quickDrop计数器 */
    int32_t sampSumValue;             /*!< 采样值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineDataKeyAvg;

/**
* @brief        按键平均值基线追踪器算法参数
*/
typedef struct
{
    uint8_t cntThreshold;          /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
    uint8_t quickDropHoldCnt;      /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能 */
    T_SiData quickDropThreshold;   /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
    T_SiData step;                 /*!< 基线追踪器步进值--需要用户指定 */
} T_SiBaselineParaKeyAvg;

/**
* @brief        按键平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;           /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaKeyAvg para;     /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                 /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataKeyAvg *pdata;   /*!< 基线追踪器数据 */
} T_SiBaselineKeyAvg;

/**
* @brief    按键平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineKeyAvgNodeInit(T_SiBaselineKeyAvg *nd, const T_SiBaselineParaKeyAvg *para, uint8_t dataLen, T_SiBaselineDataKeyAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

/**
 * @defgroup BaselineBoundKeyWkup 带边界的按键平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线
 * @{
 */

/**
* @brief        带边界的按键平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    uint8_t invalidBound;              /*!< 是否处在边界外 */
    uint16_t sampCnt;                  /*!< 已经采样次数 */
    uint16_t quickDropCnt;             /*!< quickDrop计数器 */
    int32_t sampSumValue;              /*!< 采样值总和 */
} T_SiBaselineDataBoundKeyAvg;

/**
* @brief        带边界的按键平均值基线追踪器算法参数
*/
typedef struct
{
    uint16_t boundThreshold;            /*!< 边界阈值--需要用户指定 */
    uint16_t cntThreshold;              /*!< 当滤波值处在边界内时，采样计数器阈值，到达后更新基线--需要用户指定 */
    uint16_t invalidBoundCntThreshold;  /*!< 当滤波值处在边界外时，采样计数器阈值，到达后更新基线--需要用户指定 */
    uint16_t quickDropHoldCnt;          /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能 */
    T_SiData quickDropThreshold;        /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
    T_SiData step;                      /*!< 基线追踪器步进值--需要用户指定 */
} T_SiBaselineParaBoundKeyAvg;

/**
* @brief        带边界的按键平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量(测量多少次依赖信号波动在边界内还是边界外)取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;                /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaBoundKeyAvg para;     /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                      /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataBoundKeyAvg *pdata;   /*!< 基线追踪器数据 */
} T_SiBaselineBoundKeyAvg;

/**
* @brief    带边界的按键平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineBoundKeyAvgNodeInit(T_SiBaselineBoundKeyAvg *nd, const T_SiBaselineParaBoundKeyAvg *para, uint8_t dataLen, T_SiBaselineDataBoundKeyAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

//! @cond

//******基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickfit条件时，则立刻更新基线，quickfit条件：满足quickDropFitThreshold或quickRaiseFitThreshold任意一个

//基线追踪器数据
typedef struct
{
    uint16_t sampCnt;                  //已经采样次数
    uint8_t quickFitCnt;              //quickFit计数器
    int32_t sampSumValue;             //采样值总和
} T_SiBaselineDataKeyAvg2;
//基线追踪器算法参数
typedef struct
{
    uint8_t cntThreshold;                   //采样计数器阈值，到达后更新基线--需要用户指定
    uint8_t quickFitHoldCnt;                //当满足quickFit阈值一定次数后，启用quickFit功能
    T_SiData quickDropFitThreshold;         //quickFit拉低阈值，当基线-采样值超过本阈值时且小于quickDropHoldThreshold（不是绝对值，是单方向值），会快速更新基线--需要用户指定
    T_SiData quickDropHoldThreshold;        //quickFit拉低抑制阈值，当基线-采样值超过本阈值时关闭quickFit功能（不是绝对值，是单方向值）--需要用户指定
    T_SiData quickRaiseFitThreshold;        //quickFit拉升阈值，采样-基线值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定
    T_SiData step;                          //基线追踪器步进值--需要用户指定
} T_SiBaselineParaKeyAvg2;
//基线追踪器描述符
typedef struct
{
    T_SiBaselineBase base;           //基线追踪器基类，必须放在开头
    T_SiBaselineParaKeyAvg2 para;    //基线追踪器算法参数
    uint8_t dataLen;                 //基线追踪器数据缓冲区个数
    T_SiBaselineDataKeyAvg2 *pdata;  //基线追踪器数据
} T_SiBaselineKeyAvg2;

//平均值基线追踪器节点描述符初始化，SI_RT_OK表示成功
T_SiErrRt SiBaselineKeyAvg2NodeInit(T_SiBaselineKeyAvg2 *nd, const T_SiBaselineParaKeyAvg2 *para, uint8_t dataLen, T_SiBaselineDataKeyAvg2 *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);

//******基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickfit条件时，则立刻更新基线，quickfit条件：满足quickDropFitThreshold或quickRaiseFitThreshold任意一个

//基线追踪器数据
typedef struct
{
    uint16_t sampCnt;                  //已经采样次数
    uint8_t quickFitCnt;              //quickFit计数器
    int32_t sampSumValue;             //采样值总和
} T_SiBaselineDataKeyAvg3;
//基线追踪器算法参数
typedef struct
{
    uint8_t cntThreshold;                                 //采样计数器阈值，到达后更新基线--需要用户指定
    uint8_t quickFitHoldCnt;                              //当满足quickFit阈值一定次数后，启用quickFit功能
    T_SiData quickDropFitThreshold[SI_PNODE_CH_MAXNUM];         //quickFit拉低阈值，当基线-采样值超过本阈值时且小于quickDropHoldThreshold（不是绝对值，是单方向值），会快速更新基线--需要用户指定
    T_SiData quickDropHoldThreshold[SI_PNODE_CH_MAXNUM];        //quickFit拉低抑制阈值，当基线-采样值超过本阈值时关闭quickFit功能（不是绝对值，是单方向值）--需要用户指定
    T_SiData quickRaiseFitThreshold[SI_PNODE_CH_MAXNUM];        //quickFit拉升阈值，采样-基线值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定
    T_SiData step;                                        //基线追踪器步进值--需要用户指定
} T_SiBaselineParaKeyAvg3;
//基线追踪器描述符
typedef struct
{
    T_SiBaselineBase base;           //基线追踪器基类，必须放在开头
    T_SiBaselineParaKeyAvg3 para;    //基线追踪器算法参数
    uint8_t dataLen;                 //基线追踪器数据缓冲区个数
    T_SiBaselineDataKeyAvg3 *pdata;  //基线追踪器数据
} T_SiBaselineKeyAvg3;

//平均值基线追踪器节点描述符初始化，SI_RT_OK表示成功
T_SiErrRt SiBaselineKeyAvg3NodeInit(T_SiBaselineKeyAvg3 *nd, const T_SiBaselineParaKeyAvg3 *para, uint8_t dataLen, T_SiBaselineDataKeyAvg3 *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);

//! @endcond       //doxygen中隐藏

//************************************************************************************
//滑条专用基线追踪器

//****************************滑条平均值基线追踪器**************************
/**
 * @defgroup BaselineSliderAvg 滑条平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线
 * @{
 */

/**
* @brief        滑条平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sampCnt;                  /*!< 已经采样次数 */
    uint8_t quickDropCnt;             /*!< quickDrop计数器 */
    int32_t sampSumValue;             /*!< 采样值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineDataSliderAvg;

/**
* @brief        滑条平均值基线追踪器算法参数
*/
typedef struct
{
    uint8_t cntThreshold;          /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
    uint8_t quickDropHoldCnt;      /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能--需要用户指定 */
    T_SiData quickDropThreshold;   /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
    T_SiData step;                 /*!< 基线追踪器步进值--需要用户指定 */
} T_SiBaselineParaSliderAvg;

/**
* @brief        滑条平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;              /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaSliderAvg para;     /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                    /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataSliderAvg *pdata;   /*!< 基线追踪器数据 */
} T_SiBaselineSliderAvg;

/**
* @brief    滑条平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineSliderAvgNodeInit(T_SiBaselineSliderAvg *nd, const T_SiBaselineParaSliderAvg *para, uint8_t dataLen, T_SiBaselineDataSliderAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

/**
 * @defgroup BaselineStrongAvg 增强型平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
 * @{
 */

/**
* @brief        增强型平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t invalidBound;              /*!< 是否处在边界外 */
    uint16_t sampCnt;                  /*!< 已经采样次数 */
    uint16_t quickDropCnt;             /*!< quickDrop计数器 */
    int32_t sampSumValue;              /*!< 采样值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineDataStrongAvg;

/**
* @brief        增强型平均值基线追踪器算法参数
*/
typedef struct
{
    uint16_t cntThreshold;             /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
    uint16_t firstCntThreshold;        /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
    T_SiData step;                     /*!< 基线追踪器步--需要用户指定 */
    struct
    {
        uint8_t enable;                /*!< quickDrop开关--需要用户指定 */
        uint16_t quickDropHoldCnt;     /*!< quickDrop消抖，用于quickDropThreshold和quickDropOffSafeThreshold消抖--需要用户指定 */
        T_SiData quickDropThreshold;   /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
        T_SiData quickDropOffSafeThreshold; /*!< quickDrop关闭时，当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死，0表示关闭--需要用户指定 */
    } quickDrop;                       //quickDrop功能
    struct
    {
        uint8_t enable;                /*!< 边界阈值开关--需要用户指定 */
        uint16_t boundThreshold;       /*!< 边界阈值，boundThreshold需要大于quickDropThreshold--需要用户指定 */
        uint16_t boundCntThreshold;    /*!< 当滤波值处在边界外时，采样计数器阈值，到达后更新基线--需要用户指定 */
    } bound;
} T_SiBaselineParaStrongAvg;

/**
* @brief        增强型平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;           /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaStrongAvg para;  /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                 /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataStrongAvg *pdata;/*!< 基线追踪器数据 */
} T_SiBaselineStrongAvg;

/**
* @brief    增强型平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineStrongAvgNodeInit(T_SiBaselineStrongAvg *nd, const T_SiBaselineParaStrongAvg *para, uint8_t dataLen, T_SiBaselineDataStrongAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

/**
 * @defgroup BaselineStrongAvgMt 每通道独立参数增强型平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
 * @{
 */

/**
* @brief        每通道独立参数增强型平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef T_SiBaselineDataStrongAvg T_SiBaselineDataMtStrongAvg;

/**
* @brief        每通道独立参数增强型平均值基线追踪器算法参数
*/
typedef T_SiBaselineParaStrongAvg T_SiBaselineParaMtStrongAvg[SI_PNODE_CH_MAXNUM];

/**
* @brief        每通道独立参数增强型平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;              /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaMtStrongAvg para;   /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                    /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataMtStrongAvg *pdata; /*!< 基线追踪器数据 */
} T_SiBaselineMtStrongAvg;

/**
* @brief    每通道独立参数增强型平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineMtStrongAvgNodeInit(T_SiBaselineMtStrongAvg *nd, const T_SiBaselineParaMtStrongAvg *para, uint8_t dataLen, T_SiBaselineDataMtStrongAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

/**
 * @defgroup BaselineStrongAvgMt 每通道独立参数增强型平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
 * @{
 */

#define SI_LITE_BASELINE_CHNUM     (2)     /*!< lite 基线通道数 */

/**
* @brief        每通道独立参数增强型平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef T_SiBaselineDataStrongAvg T_SiBaselineDataLiteMtStrongAvg;

/**
* @brief        每通道独立参数增强型平均值基线追踪器算法参数
*/
typedef T_SiBaselineParaStrongAvg T_SiBaselineParaLiteMtStrongAvg[SI_LITE_BASELINE_CHNUM];

/**
* @brief        每通道独立参数增强型平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;              /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaLiteMtStrongAvg para;   /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                    /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataLiteMtStrongAvg *pdata; /*!< 基线追踪器数据 */
} T_SiBaselineLiteMtStrongAvg;

/**
* @brief    每通道独立参数增强型平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineLiteMtStrongAvgNodeInit(T_SiBaselineLiteMtStrongAvg *nd, const T_SiBaselineParaLiteMtStrongAvg *para, uint8_t dataLen, T_SiBaselineDataLiteMtStrongAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

/**
 * @defgroup BaselineStrongAvgMt 每通道独立参数增强型平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
 * @{
 */

/**
* @brief        每通道独立参数增强型平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t invalidBound;              /*!< 是否处在边界外 */
    uint8_t forceEnableFsm;            /*!< forceEnable状态 */
    uint16_t sampCnt;                  /*!< 已经采样次数 */
    uint16_t quickDropCnt;             /*!< quickDrop计数器 */
    int32_t sampSumValue;              /*!< 采样值总和 */
    uint32_t forceEnableBeginTime;     /*!< forceEnable开始时间 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineDataDoorctrlMtStrongAvg;

/**
* @brief        每通道独立参数增强型平均值基线追踪器算法参数
*/
typedef struct
{
    uint16_t cntThreshold;             /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
    uint16_t firstCntThreshold;        /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
    T_SiData step;                     /*!< 基线追踪器步--需要用户指定 */
    struct
    {
        uint8_t enable;                /*!< quickDrop开关--需要用户指定 */
        uint16_t quickDropHoldCnt;     /*!< quickDrop消抖，on和off都会持续消抖时间后才生效--需要用户指定 */
        T_SiData quickDropThreshold;   /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
        struct
        {
            uint8_t forceEnableOnBoot; /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
            uint16_t forceEnableTimeoutMs; /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
            T_SiData safeThreshold1; /*!< 当基线偏差绝对值在safeThreshold1和safeThreshold2之间时，会被拉到safeThreshold1，防止基线长时间锁死--需要用户指定 */
            T_SiData safeThreshold2; /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
        } quickDropOff;
    } quickDrop;                       //quickDrop功能
    struct
    {
        uint8_t enable;                /*!< 边界阈值开关--需要用户指定 */
        uint8_t boundDirect;           /*!< 边界方向，0表示绝对值，1表示正值，2表示负值--需要用户指定 */
        uint16_t boundThreshold;       /*!< 边界阈值，boundThreshold需要大于quickDropThreshold--需要用户指定 */
        uint16_t boundCntThreshold;    /*!< 当滤波值处在边界外时），采样计数器阈值，到达后更新基线--需要用户指定 */
    } bound;
} T_SiBaselineParaDoorctrlMtStrongAvg[SI_LITE_BASELINE_CHNUM];

/**
* @brief        每通道独立参数增强型平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;              /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaDoorctrlMtStrongAvg para;   /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                    /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataDoorctrlMtStrongAvg *pdata; /*!< 基线追踪器数据 */
} T_SiBaselineDoorctrlMtStrongAvg;

/**
* @brief    每通道独立参数增强型平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineDoorctrlMtStrongAvgNodeInit(T_SiBaselineDoorctrlMtStrongAvg *nd, const T_SiBaselineParaDoorctrlMtStrongAvg *para, uint8_t dataLen, T_SiBaselineDataDoorctrlMtStrongAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

/**
 * @defgroup BaselineStrongAvgMt 每通道独立参数增强型平均值基线追踪器
 * @ingroup baseline
 * 基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
 * @{
 */

/**
* @brief        每通道独立参数增强型平均值基线追踪器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t forceEnableFsm;            /*!< forceEnable状态 */
    uint8_t sampCnt;                   /*!< 已经采样次数 */
    uint16_t quickDropCnt;             /*!< quickDrop计数器 */
    int32_t sampSumValue;              /*!< 采样值总和 */
    uint32_t forceEnableBeginTime;     /*!< forceEnable开始时间 */
    //! @endcond       //doxygen中隐藏
} T_SiBaselineDataDoorctrl2MtStrongAvg;

/**
* @brief        每通道独立参数增强型平均值基线追踪器算法参数
*/
typedef struct
{
    uint8_t cntThreshold;             /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
    uint8_t firstCntThreshold;        /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
    T_SiData step;                     /*!< 基线追踪器步--需要用户指定 */
    struct
    {
        uint8_t enable;                /*!< quickDrop开关--需要用户指定 */
        uint16_t quickDropHoldCnt;     /*!< quickDrop消抖，on和off都会持续消抖时间后才生效--需要用户指定 */
        T_SiData quickDropThreshold;   /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
        struct
        {
            uint8_t forceEnableOnBoot; /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
            uint16_t forceEnableTimeoutMs; /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
            T_SiData safeThreshold;    /*!< 当基线偏差绝对值超过本阈值时，会被拉到本阈值范围，防止基线长时间锁死--需要用户指定 */
        } quickDropOff;
    } quickDrop;                       //quickDrop功能
} T_SiBaselineParaDoorctrl2MtStrongAvg[SI_LITE_BASELINE_CHNUM];

/**
* @brief        每通道独立参数增强型平均值基线追踪器描述符
* @details      基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线，若超过bound阈值，则以bound中的速度更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;              /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaDoorctrl2MtStrongAvg para;   /*!< 基线追踪器算法参数 */
    uint8_t dataLen;                    /*!< 基线追踪器数据缓冲区个数 */
    T_SiBaselineDataDoorctrl2MtStrongAvg *pdata; /*!< 基线追踪器数据 */
} T_SiBaselineDoorctrl2MtStrongAvg;

/**
* @brief    每通道独立参数增强型平均值基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   dataLen 基线追踪器缓冲区长度
* @param[in]   pdata 基线追踪器缓冲区
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineDoorctrl2MtStrongAvgNodeInit(T_SiBaselineDoorctrl2MtStrongAvg *nd, const T_SiBaselineParaDoorctrl2MtStrongAvg *para, uint8_t dataLen, T_SiBaselineDataDoorctrl2MtStrongAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

//************************************************************************************
//入耳检测专用基线追踪器

//************************************************************************************
//BITMAP专用基线追踪器

//****************************BITMAP平均值基线追踪器**************************

//! @cond

//******基线追踪原理：当仲裁器允许更新基线时，采用多次测量取平均值，并按step长度更新基线，若满足quickdrop条件时，则立刻更新基线

//基线追踪器数据
typedef struct
{
    uint16_t sampCnt;                  //已经采样次数
    uint8_t quickDropCnt;             //quickDrop计数器
    int32_t sampSumValue;             //采样值总和
} T_SiBaselineDataBitmapAvg;
//基线追踪器算法参数
typedef struct
{
    uint8_t cntThreshold;          //采样计数器阈值，到达后更新基线--需要用户指定
    uint8_t quickDropHoldCnt;      //当满足quickDrop阈值一定次数后，启用quickDrop功能
    uint8_t xChNum;                //X轴通道个数，T_SiObject中的keyMap前xChNum个通道作为X轴，之后的yChNum个通道作为Y轴，通道需要按类似滑条的顺序排列--需要用户指定
    uint8_t yChNum;                //Y轴通道个数，T_SiObject中的keyMap前xChNum个通道作为X轴，之后的yChNum个通道作为Y轴，通道需要按类似滑条的顺序排列--需要用户指定
    uint8_t xChGrowDirect;         //X轴信号增长方向--需要用户指定
    uint8_t yChGrowDirect;         //Y轴信号增长方向--需要用户指定
    T_SiData xChQuickDropThreshold;//X轴通道quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定
    T_SiData yChQuickDropThreshold;//Y轴通道quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定
    T_SiData xChStep;              //X轴基线追踪器步进值--需要用户指定
    T_SiData yChStep;              //Y轴基线追踪器步进值--需要用户指定
} T_SiBaselineParaBitmapAvg;
//基线追踪器描述符
typedef struct
{
    T_SiBaselineBase base;              //基线追踪器基类，必须放在开头
    T_SiBaselineParaBitmapAvg para;     //基线追踪器算法参数
    uint8_t dataLen;                    //基线追踪器数据缓冲区个数
    T_SiBaselineDataBitmapAvg *pdata;   //基线追踪器数据
} T_SiBaselineBitmapAvg;

//平均值基线追踪器节点描述符初始化，SI_RT_OK表示成功
T_SiErrRt SiBaselineBitmapAvgNodeInit(T_SiBaselineBitmapAvg *nd, const T_SiBaselineParaBitmapAvg *para, uint8_t dataLen, T_SiBaselineDataBitmapAvg *pdata, uint8_t baselineBufLen, T_SiData *baselineBuf);

//! @endcond       //doxygen中隐藏

//****************************空基线追踪器**************************
/**
 * @defgroup BaselineNone 空基线追踪器
 * @ingroup baseline
 * @{
 */

/**
* @brief        空基线追踪器描述符
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;              /*!< 基线追踪器基类，必须放在开头 */
} T_SiBaselineNone;

/**
* @brief    空基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineNoneNodeInit(T_SiBaselineNone *nd, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

//****************************自定义基线追踪器**************************
/**
 * @defgroup BaselineCustom 自定义基线追踪器
 * @ingroup baseline
 * 基线追踪原理：执行用户自定义的run函数更新基线
 * @{
 */

/**
* @brief        自定义基线追踪器算法参数
*/
typedef struct
{
    void (*run)(T_SiObject *obj, uint8_t keyNum, const T_SiData *filterBuf);     /*!< 基线更新执行函数，keyNum按键个数，filterBuf滤波缓冲区 */
} T_SiBaselineParaCustom;

/**
* @brief        自定义基线追踪器描述符
* @details      基线追踪原理：执行用户自定义的run函数更新基线
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiBaselineBase base;           /*!< 基线追踪器基类，必须放在开头 */
    T_SiBaselineParaCustom para;     /*!< 基线追踪器算法参数 */
} T_SiBaselineCustom;

/**
* @brief    自定义基线追踪器节点描述符初始化
* @param[in]   nd 基线追踪器节点
* @param[in]   para 基线追踪器参数
* @param[in]   baselineBufLen 基线追踪器结果缓冲区长度
* @param[in]   baselineBuf 基线追踪器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiBaselineCustomNodeInit(T_SiBaselineCustom *nd, const T_SiBaselineParaCustom *para, uint8_t baselineBufLen, T_SiData *baselineBuf);
/** @} */

#ifdef __cplusplus
}
#endif

#endif
