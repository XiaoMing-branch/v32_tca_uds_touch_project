/**
*****************************************************************************
* @brief  滤波器
* @file   si_filter.h
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

#ifndef __SI_FILTER_H__
#define __SI_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup filter 滤波器
 * 对原始数据或放大后的原始数据进行滤波
 */
/** @} */

/**
 * @defgroup FilterInterface 公共接口
 * @ingroup filter
 * 滤波器相关接口及数据结构
 * @{
 */

/**
* @brief        滤波器基类
* @details      定义滤波器内部实现方法
* @attention    不要直接操作本结构体内容
*/
typedef struct T_SiFilterBase
{
    //! @cond
    T_SiBitMask readyMask;                                                  /*!< 滤波器就绪位掩码 */
    uint8_t bufLen;                                                         /*!< 缓冲区长度 */
    T_SiData *filterBuf;                                                    /*!< 滤波缓冲区 */

    T_SiErrRt(*init)(T_SiObject *obj, struct T_SiFilterBase *self);         /*!< 安装滤波器 */
    T_SiErrRt(*exit)(T_SiObject *obj, struct T_SiFilterBase *self);                                        /*!< 卸载滤波器 */
    void (*hook)(T_SiObject *obj, struct T_SiFilterBase *self, uint8_t keyNum, const T_SiData *filterBuf); /*!< 滤波器钩子函数 */
    void (*set)(T_SiObject *obj, struct T_SiFilterBase *self, uint8_t keyNo, T_SiData filterValue);        /*!< 手动设置滤波值 */
    void (*reset)(T_SiObject *obj, struct T_SiFilterBase *self, uint8_t keyNo);                            /*!< 滤波器复位，清除对应按键的滤波数据 */

    T_SiErrRt(*run)(T_SiObject *obj, struct T_SiFilterBase *self, uint8_t keyNo, T_SiData rawData);        /*!< 运行滤波器，rawData表示原始数据或放大后的原始数据 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterBase;

/**
* @brief    注册滤波器回调函数
* @details     调度器执行完滤波器后，紧接着会执行其回调函数
* @param[in]   nd 滤波器节点
* @param[in]   hookCallback 回调函数
* @retval      无
*/
#define SiFilterRegisterHook(nd,hookCallback)      (((T_SiFilterBase*)(nd))->hook = hookCallback)

/**
* @brief    复制滤波器
* @details     从srcobj中复制滤波值到dstobj中
* @param[in]   dstobj 目的对象
* @param[in]   srcobj 源对象
* @retval      SI_RT_OK 复制成功
* @retval      other 复制失败，某些滤波器可能禁止拷贝
*/
T_SiErrRt SiFilterCopy(T_SiObject *dstobj, T_SiObject *srcobj);

/**
* @brief    复位滤波器
* @param[in]   obj 信号集对象
* @retval      SI_RT_OK 复位成功
* @retval      other 复位失败
*/
T_SiErrRt SiFilterResetAll(T_SiObject *obj);

/**
* @brief    复位滤波器
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      SI_RT_OK 复位成功
* @retval      other 复位失败
*/
T_SiErrRt SiFilterReset(T_SiObject *obj, uint8_t keyNo);

/**
* @brief    设置滤波器值
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @param[in]   filterValue 信号值
* @retval      SI_RT_OK 成功
* @retval      other 失败
*/
T_SiErrRt SiFilterSetValue(T_SiObject *obj, uint8_t keyNo, T_SiData filterValue);

/**
* @brief    修改滤波器算法参数
* @param[in]   nd 滤波器对象
* @param[in]   newpara 新参数
* @retval      无
*/
#define SiFilterSetPara(nd,newpara)     (memcpy(&(nd)->para,(newpara),sizeof((nd)->para)))

/**
* @brief    滤波器是否就绪
* @param[in]   obj 信号集对象
* @param[in]   self 滤波器对象
* @param[in]   keyNo 信号编号
* @retval      0 未就绪
* @retval      其它 就绪
*/
#define SiFilterIsReady(obj,self,keyNo) ((((T_SiFilterBase *)(self))->readyMask & (0x1U<<(keyNo))) != 0)

/**
* @brief    设置滤波器就绪位
* @param[in]   obj 信号集对象
* @param[in]   self 滤波器对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiFilterSetReady(obj,self,keyNo) (((T_SiFilterBase *)(self))->readyMask |= (0x1U<<(keyNo)))

/**
* @brief    清除滤波器就绪位
* @param[in]   obj 信号集对象
* @param[in]   self 滤波器对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiFilterClrReady(obj,self,keyNo) (((T_SiFilterBase *)(self))->readyMask &= ~(0x1U<<(keyNo)))

/** @} */

//************************************************************************************
//通用滤波器

//****************************窗口平均滤波器**************************
/**
 * @defgroup FilterWindowAvg 窗口平均滤波器
 * @ingroup filter
 * 滤波器原理：去除窗口中一个最大值和一个最小值，求平均，每次最大步进step
 * @{
 */

#define SI_FILTER_DATA_WINDOWAVG_WINDOWSIZE      12      /*!< 窗口平均滤波器窗口缓冲区最大长度 */

/**
* @brief        窗口平均滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t fillLen;                                              /*!< 窗口中已经填充数据长度 */
    T_SiData window[SI_FILTER_DATA_WINDOWAVG_WINDOWSIZE];         /*!< 窗口缓冲区 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataWindowAvg;

/**
* @brief        窗口平均滤波器算法参数
*/
typedef struct
{
    uint8_t windowLen;                        /*!< 窗口长度，--需要用户指定 */
    T_SiData step;                            /*!< 步进值，--需要用户指定 */
} T_SiFilterParaWindowAvg;

/**
* @brief        窗口平均滤波器描述符
* @details      滤波器原理：去除窗口中一个最大值和一个最小值，求平均，每次最大步进step
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                      /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaWindowAvg para;             /*!< 滤波器算法参数 */
    uint8_t dataLen;                          /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataWindowAvg *pdata;           /*!< 滤波器数据 */
} T_SiFilterWindowAvg;

/**
* @brief    窗口平均滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterWindowAvgNodeInit(T_SiFilterWindowAvg *nd, const T_SiFilterParaWindowAvg *para, uint8_t dataLen, T_SiFilterDataWindowAvg *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************MinMax滤波器**************************
/**
 * @defgroup FilterMinMax MinMax滤波器
 * @ingroup filter
 * 滤波器原理：去除窗口中n个最大值和m个最小值，求平均，每次最大步进step
 * @{
 */

#define SI_FILTER_DATA_MINMAX_WINDOWSIZE      12      /*!< MinMax滤波器窗口缓冲区最大长度 */

/**
* @brief        MinMax滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t fillLen;                                           /*!< 窗口中已经填充数据长度 */
    T_SiData window[SI_FILTER_DATA_MINMAX_WINDOWSIZE];         /*!< 窗口缓冲区 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataMinMax;

/**
* @brief        MinMax滤波器算法参数
*/
typedef struct
{
    uint8_t windowLen;                        /*!< 窗口长度，discardMinvNum+discardMaxvNum需要小于窗口长度--需要用户指定 */
    uint8_t discardMinvNum;                   /*!< 丢弃最小值个数--需要用户指定 */
    uint8_t discardMaxvNum;                   /*!< 丢弃最大值个数--需要用户指定 */
    T_SiData step;                            /*!< 步进值--需要用户指定 */
} T_SiFilterParaMinMax;

/**
* @brief        MinMax滤波器描述符
* @details      滤波器原理：去除窗口中n个最大值和m个最小值，求平均，每次最大步进step
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                   /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaMinMax para;             /*!< 滤波器算法参数 */
    uint8_t dataLen;                       /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataMinMax *pdata;           /*!< 滤波器数据 */
} T_SiFilterMinMax;

/**
* @brief    MinMax滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterMinMaxNodeInit(T_SiFilterMinMax *nd, const T_SiFilterParaMinMax *para, uint8_t dataLen, T_SiFilterDataMinMax *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************MinMaxSum滤波器**************************
/**
 * @defgroup FilterMinMaxSum MinMaxSum滤波器
 * @ingroup filter
 * 滤波器原理：去除窗口中n个最大值和m个最小值，剩下值求和（类似于带了简单放大功能），每次最大步进step
 * @{
 */

//******备注：不支持set操作

/**
* @brief        MinMaxSum滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t fillLen;                                            /*!< 窗口中已经填充数据长度 */
    T_SiData window[SI_FILTER_DATA_MINMAX_WINDOWSIZE];          /*!< 窗口缓冲区 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataMinMaxSum;

/**
* @brief        MinMaxSum滤波器算法参数
*/
typedef struct
{
    uint8_t windowLen;                         /*!< 窗口长度，discardMinvNum+discardMaxvNum需要小于窗口长度--需要用户指定 */
    uint8_t discardMinvNum;                    /*!< 丢弃最小值个数--需要用户指定 */
    uint8_t discardMaxvNum;                    /*!< 丢弃最大值个数--需要用户指定 */
    T_SiData step;                             /*!< 步进值--需要用户指定 */
} T_SiFilterParaMinMaxSum;

/**
* @brief        MinMaxSum滤波器描述符
* @details      滤波器原理：去除窗口中n个最大值和m个最小值，剩下值求和（类似于带了简单放大功能），每次最大步进step
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                      /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaMinMaxSum para;             /*!< 滤波器算法参数 */
    uint8_t dataLen;                          /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataMinMaxSum *pdata;           /*!< 滤波器数据 */
} T_SiFilterMinMaxSum;

/**
* @brief    MinMaxSum滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterMinMaxSumNodeInit(T_SiFilterMinMaxSum *nd, const T_SiFilterParaMinMaxSum *para, uint8_t dataLen, T_SiFilterDataMinMaxSum *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************平均值滤波器**************************
/**
 * @defgroup FilterAvg 平均值滤波器
 * @ingroup filter
 * 滤波器原理：多次采样求平均，每次最大步进step
 * @{
 */

/**
* @brief        平均值滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t sampCnt;                  /*!< 已经采样次数 */
    int32_t sampSumValue;            /*!< 采样值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataAvg;

/**
* @brief        平均值滤波器算法参数
*/
typedef struct
{
    uint8_t cntThreshold;          /*!< 采样计数器阈值，到达后更新滤波值--需要用户指定 */
    T_SiData step;                 /*!< 步进值，--需要用户指定 */
} T_SiFilterParaAvg;

/**
* @brief        平均值滤波器描述符
* @details      滤波器原理：多次采样求平均，每次最大步进step
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaAvg para;             /*!< 滤波器算法参数 */
    uint8_t dataLen;                    /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataAvg *pdata;           /*!< 滤波器数据 */
} T_SiFilterAvg;

/**
* @brief    平均值滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterAvgNodeInit(T_SiFilterAvg *nd, const T_SiFilterParaAvg *para, uint8_t dataLen, T_SiFilterDataAvg *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************MBNAIBAs滤波器**************************

//! @cond

//******滤波器原理：依次执行：MinMax滤波器+巴特沃斯滤波器+NoiseAvoid滤波器+放大器+IIR滤波器+巴特沃斯滤波器 + 累加放大器
//******备注：不支持set操作

#define BUTTERWORTH_FILTER_ORDER                            5               //巴特沃斯滤波器阶数
#define SI_FILTER_MBNAIBAs_AMP_WINDOWSIZE                   5               //累加放大器窗口大小

//巴特沃斯滤波器运行时数据
typedef struct
{
    T_SiData y;
    T_SiData x;
    uint16_t cnt;
    T_SiData diff;

    uint16_t gain;
    uint16_t gainOffset;
} T_ButterworthData;

//巴特沃斯滤波器算法参数
typedef struct
{
    uint16_t gain;
    uint16_t fastLevel;
    uint16_t fastKeepLevel;
} T_ButterworthPara;

//噪声抑制滤波器运行时数据
typedef struct
{
    T_SiData data;
} T_NoiseAvoidData;

//噪声抑制滤波器算法参数
typedef struct
{
    uint16_t coarseTune;                      //粗调
    uint16_t fineTune;                        //微调
    uint16_t fineStep;                        //微调步进，建议为1
} T_NoiseAvoidPara;

//滤波器数据类型
typedef struct
{
    //MinMax滤波器
    struct
    {
        uint8_t fillLen;                                           //窗口中已经填充数据长度
        T_SiData window[SI_FILTER_DATA_MINMAX_WINDOWSIZE];         //窗口缓冲区
        T_SiData rtdata;                                           //滤波器结果
    } minMax;

    //巴特沃斯滤波器
    struct
    {
        T_ButterworthData bw;
        T_SiData rtdata;                                           //滤波器结果
    } bw1;

    //NoiseAvoid滤波器
    struct
    {
        T_NoiseAvoidData na;
        T_SiData rtdata;                                           //滤波器结果
    } noiseAvoid;

    //放大器
    struct
    {
        uint8_t scanCnt;                                           //扫描次数
        T_SiData sumData;                                          //放大器累加结果
        T_SiData rtdata;                                           //滤波器结果
    } amp;

    //IIR滤波器
    struct
    {
        T_SiData rtdata;                                           //滤波器结果
    } iir;

    //巴特沃斯滤波器
    struct
    {
        T_ButterworthData bw;
        T_SiData rtdata;                                           //滤波器结果
    } bw2;

    //放大器2
    struct
    {
        uint8_t fillLen;                                           //窗口中已经填充数据长度
        T_SiData window[SI_FILTER_MBNAIBAs_AMP_WINDOWSIZE];        //窗口缓冲区
        T_SiData rtdata;                                           //滤波器结果
    } amp2;

} T_SiFilterDataMBNAIBAs;
//滤波器算法参数
typedef struct
{
    //MinMax滤波器
    struct
    {
        uint8_t windowLen;                        //窗口长度，discardMinvNum+discardMaxvNum需要小于窗口长度--需要用户指定
        uint8_t discardMinvNum;                   //丢弃最小值个数--需要用户指定
        uint8_t discardMaxvNum;                   //丢弃最大值个数--需要用户指定
    } minMax;

    //巴特沃斯滤波器
    T_ButterworthPara bwOffset1;

    //NoiseAvoid滤波器
    T_NoiseAvoidPara noiseAvoid;

    //放大器
    struct
    {
        uint8_t ratio;                            //放大比例
        T_SiData offset;                          //信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定
    } amp[SI_PNODE_CH_MAXNUM];

    //IIR滤波器
    struct
    {
        uint8_t gain;                             //增益
    } iir[SI_PNODE_CH_MAXNUM];

    //巴特沃斯滤波器
    T_ButterworthPara bwOffset2;

    //放大器2
    struct
    {
        uint8_t ratio;                            //放大比例，最大SI_FILTER_MBNAIBAs_AMP_WINDOWSIZE
        T_SiData offset;                          //信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定
    } amp2[SI_PNODE_CH_MAXNUM];

} T_SiFilterParaMBNAIBAs;
//滤波器描述符
typedef struct
{
    T_SiFilterBase base;                  //滤波器基类，必须放在开头
    T_SiFilterParaMBNAIBAs para;          //滤波器算法参数
    uint8_t dataLen;                      //滤波器数据缓冲区个数
    T_SiFilterDataMBNAIBAs *pdata;        //滤波器数据
} T_SiFilterMBNAIBAs;

//滤波器节点描述符初始化，SI_RT_OK表示成功
T_SiErrRt SiFilterMBNAIBAsNodeInit(T_SiFilterMBNAIBAs *nd, const T_SiFilterParaMBNAIBAs *para, uint8_t dataLen, T_SiFilterDataMBNAIBAs *pdata, uint8_t filterBufLen, T_SiData *filterBuf);

//! @endcond       //doxygen中隐藏

//****************************MsAI滤波器**************************

//! @cond

//******滤波器原理：依次执行：MinMaxSum滤波器+倍数放大器+IIR滤波器
//******备注：不支持set操作

//滤波器数据类型
typedef struct
{
    //MinMaxSum滤波器
    struct
    {
        uint8_t fillLen;                                           //窗口中已经填充数据长度
        T_SiData window[SI_FILTER_DATA_MINMAX_WINDOWSIZE];         //窗口缓冲区
        T_SiData rtdata;                                           //滤波器结果
    } minMaxSum;

    //放大器
    struct
    {
        T_SiData rtdata;                                           //滤波器结果
    } amp;

    //IIR滤波器
    struct
    {
        T_SiData rtdata;                                           //滤波器结果
    } iir;

} T_SiFilterDataMsAI;
//滤波器算法参数
typedef struct
{
    //MinMaxSum滤波器
    struct
    {
        uint8_t windowLen;                        //窗口长度，discardMinvNum+discardMaxvNum需要小于窗口长度--需要用户指定
        uint8_t discardMinvNum;                   //丢弃最小值个数--需要用户指定
        uint8_t discardMaxvNum;                   //丢弃最大值个数--需要用户指定
    } minMaxSum;

    //放大器
    struct
    {
        uint8_t ratio;                            //放大比例
        T_SiData offset;                          //信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定
    } amp[SI_PNODE_CH_MAXNUM];

    //IIR滤波器
    struct
    {
        uint8_t gain;                             //增益
    } iir[SI_PNODE_CH_MAXNUM];

    T_SiData step;                            //步进值--需要用户指定

} T_SiFilterParaMsAI;
//滤波器描述符
typedef struct
{
    T_SiFilterBase base;                  //滤波器基类，必须放在开头
    T_SiFilterParaMsAI para;            //滤波器算法参数
    uint8_t dataLen;                      //滤波器数据缓冲区个数
    T_SiFilterDataMsAI *pdata;          //滤波器数据
} T_SiFilterMsAI;

//MsAI滤波器节点描述符初始化，SI_RT_OK表示成功
T_SiErrRt SiFilterMsAINodeInit(T_SiFilterMsAI *nd, const T_SiFilterParaMsAI *para, uint8_t dataLen, T_SiFilterDataMsAI *pdata, uint8_t filterBufLen, T_SiData *filterBuf);

//! @endcond       //doxygen中隐藏

//****************************MI滤波器****************************
/**
 * @defgroup FilterMI MI滤波器
 * @ingroup filter
 * 滤波器原理：依次执行：MinMax滤波器+IIR滤波器
 * @{
 */
//******备注：不支持set操作

/**
* @brief        MI滤波器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    //MinMax滤波器
    struct
    {
        uint8_t fillLen;                                           /*!< 窗口中已经填充数据长度 */
        T_SiData window[SI_FILTER_DATA_MINMAX_WINDOWSIZE];         /*!< 窗口缓冲区 */
        T_SiData rtdata;                                           /*!< 滤波器结果 */
    } minMax;

    //IIR滤波器
    struct
    {
        T_SiData rtdata;                                           /*!< 滤波器结果 */
    } iir;
    //! @endcond       //doxygen中隐藏
} T_SiFilterDataMI;

/**
* @brief        MI滤波器算法参数
*/
typedef struct
{
    //MinMax滤波器
    struct
    {
        uint8_t windowLen;                        /*!< 窗口长度，discardMinvNum+discardMaxvNum需要小于窗口长度--需要用户指定 */
        uint8_t discardMinvNum;                   /*!< 丢弃最小值个数--需要用户指定 */
        uint8_t discardMaxvNum;                   /*!< 丢弃最大值个数--需要用户指定 */
    } minMax;

    //IIR滤波器
    struct
    {
        uint8_t gain;                             /*!< 增益，IIR阶数 */
    } iir[SI_PNODE_CH_MAXNUM];

    T_SiData step;                                /*!< 步进值--需要用户指定 */

} T_SiFilterParaMI;

/**
* @brief        MI滤波器描述符
* @details      滤波器原理：依次执行：MinMax滤波器+IIR滤波器
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;              /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaMI para;            /*!< 滤波器算法参数 */
    uint8_t dataLen;                  /*!< 滤波器数据缓冲区个数 */
    T_SiFilterDataMI *pdata;          /*!< 滤波器数据 */
} T_SiFilterMI;

/**
* @brief    MI滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   para 滤波器参数
* @param[in]   dataLen 滤波器缓冲区长度
* @param[in]   pdata 滤波器缓冲区
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterMINodeInit(T_SiFilterMI *nd, const T_SiFilterParaMI *para, uint8_t dataLen, T_SiFilterDataMI *pdata, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************门把手滤波器**************************
/**
 * @defgroup FilterDoorctrl 门把手滤波器
 * @ingroup filterDctor
 * 滤波器原理：本滤波器只支持两个通道，通道1是检测通道，通道2是跳频通道
 * @{
 */

#define SI_FILTER_DOORCTRL_CHNUM     (2)     /*!<门把手滤波器通道数 */

/**
* @brief        门把手滤波器算法数据
*/
typedef struct
{
    struct
    {
        T_SiData lastValue;
    } gainers[SI_FILTER_DOORCTRL_CHNUM];
    struct
    {
        T_SiData value;
    } gainerRaws[SI_FILTER_DOORCTRL_CHNUM];
    struct
    {
        uint8_t status;
        uint8_t phaseCount;
    } mixer;
    struct
    {
        T_SiData value;
    } mixerRaws[SI_FILTER_DOORCTRL_CHNUM];
} T_SiFilterDataDoorctrl;

/**
* @brief        门把手滤波器算法参数
*/
typedef struct
{
    struct
    {
        uint8_t enable;                    /*!< 增益器使能，1使能，0关闭--需要用户指定 */
        T_SiData offset;                   /*!< 信号偏置值--需要用户指定 */
    } gainers[SI_FILTER_DOORCTRL_CHNUM];/*!< 增益器 */
} T_SiFilterParaDoorctrl;

/**
* @brief        门把手滤波器描述符
* @details      滤波器原理：本滤波器只支持两个通道，通道1是检测通道，通道2是跳频通道
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                 /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaDoorctrl para;         /*!< 滤波器算法参数 */
    T_SiFilterDataDoorctrl data;         /*!< 滤波器数据 */
} T_SiFilterDoorctrl;

/**
* @brief    门把手滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   para 滤波器参数
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterDoorctrlNodeInit(T_SiFilterDoorctrl *nd, const T_SiFilterParaDoorctrl *para, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************门把手滤波器**************************
/**
 * @defgroup FilterExtDoorctrl 门把手滤波器
 * @ingroup filterDctor
 * 滤波器原理：本滤波器只支持两个通道，通道1是检测通道，通道2是跳频通道
 * @{
 */

/**
* @brief        门把手滤波器算法数据
*/
typedef struct
{
    struct
    {
        T_SiData lastValue;
    } gainers[SI_FILTER_DOORCTRL_CHNUM];
    struct
    {
        T_SiData value;
    } gainerRaws[SI_FILTER_DOORCTRL_CHNUM];
    struct
    {
        uint8_t status;
        uint8_t phaseCount;
    } mixer;
    struct
    {
        T_SiData value;
    } mixerRaws[SI_FILTER_DOORCTRL_CHNUM];
} T_SiFilterDataExtDoorctrl;

/**
* @brief        门把手滤波器算法参数
*/
typedef struct
{
    struct
    {
        uint8_t enable;                    /*!< 增益器使能，1使能，0关闭--需要用户指定 */
        T_SiData offset;                   /*!< 信号偏置值--需要用户指定 */
    } gainers[SI_FILTER_DOORCTRL_CHNUM];/*!< 增益器 */
    T_SiNoiseData kfr;                     /*!< 跳频通道方差值，单位0.01--需要用户指定 */
} T_SiFilterParaExtDoorctrl;

/**
* @brief        门把手滤波器描述符
* @details      滤波器原理：本滤波器只支持两个通道，通道1是检测通道，通道2是跳频通道
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                 /*!< 滤波器基类，必须放在开头 */
    T_SiFilterParaExtDoorctrl para;         /*!< 滤波器算法参数 */
    T_SiFilterDataExtDoorctrl data;         /*!< 滤波器数据 */
    struct
    {
        uint8_t reset;
        T_SiData rawv;
        float x;
        float p;
        float q;
    } kf;
} T_SiFilterExtDoorctrl;

/**
* @brief    门把手滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   para 滤波器参数
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterExtDoorctrlNodeInit(T_SiFilterExtDoorctrl *nd, const T_SiFilterParaExtDoorctrl *para, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//****************************空滤波器**************************
/**
 * @defgroup FilterNone 空滤波器
 * @ingroup filter
 * 滤波器原理：无滤波，直接用原始值作为滤波值
 * @{
 */

/**
* @brief        空滤波器描述符
* @details      滤波器原理：无滤波，直接用原始值作为滤波值
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiFilterBase base;                /*!< 滤波器基类，必须放在开头 */
} T_SiFilterNone;

/**
* @brief    空滤波器节点描述符初始化
* @param[in]   nd 滤波器节点
* @param[in]   filterBufLen 滤波器结果缓冲区长度
* @param[in]   filterBuf 滤波器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiFilterNoneNodeInit(T_SiFilterNone *nd, uint8_t filterBufLen, T_SiData *filterBuf);
/** @} */

//************************************************************************************
//按键专用滤波器

//************************************************************************************
//滑条专用滤波器

//************************************************************************************
//入耳检测专用滤波器

//************************************************************************************
//滤波器工具

//! @cond

/**
* @brief    计算minmax平均值
* @details     丢掉discardMaxvNum个最大值，discardMinvNum个最小值，取平均
* @param[in]   wdLen 缓冲区长度
* @param[in]   wdBuf 缓冲区
* @param[in]   discardMinvNum 丢弃最小值个数
* @param[in]   discardMaxvNum 丢弃最大值个数
* @retval      平均值
*/
T_SiData CalcMinMaxAvg(uint8_t wdLen, T_SiData *wdBuf, uint8_t discardMinvNum, uint8_t discardMaxvNum);

/**
* @brief    计算平均值
* @details     求平均
* @param[in]   wdLen 缓冲区长度
* @param[in]   wdBuf 缓冲区
* @retval      平均值
*/
T_SiData CalcAvg(uint8_t wdLen, T_SiData *wdBuf);

/**
* @brief    计算minmax累加值
* @details     丢掉discardMaxvNum个最大值，discardMinvNum个最小值，剩下值累加
* @param[in]   wdLen 缓冲区长度
* @param[in]   wdBuf 缓冲区
* @param[in]   discardMinvNum 丢弃最小值个数
* @param[in]   discardMaxvNum 丢弃最大值个数
* @retval      累加值
*/
T_SiData CalcMinMaxSum(uint8_t wdLen, T_SiData *wdBuf, uint8_t discardMinvNum, uint8_t discardMaxvNum);

/**
* @brief    计算累加值
* @details     求累加
* @param[in]   wdLen 缓冲区长度
* @param[in]   wdBuf 缓冲区
* @retval      累加值
*/
T_SiData CalcSum(uint8_t wdLen, T_SiData *wdBuf);

/**
* @brief    计算IIR滤波值
* @param[in]   PRE 上次数值
* @param[in]   CURR 当前值
* @param[in]   OFFSET 增益
* @retval      滤波值
*/
#define SiIIR(PRE, CURR, OFFSET) ((PRE) = ((PRE)==0) ? (CURR) : (SiIFastDiv(((int)(PRE)*(OFFSET)+(CURR)),((OFFSET)+1))))

/**
* @brief    计算巴特沃斯滤波值
* @retval      滤波值
*/
T_SiData CalcButterworth(const T_ButterworthPara *offset, T_ButterworthData *bw, uint8_t order, T_SiData preData, T_SiData curData);

/**
* @brief    计算噪声抑制滤波值
* @details     信号波动不小于粗调值时放行，否则大于微调值时，将上次值微调后返回数据，否则直接返回上次值
* @param[in]   para 算法参数
* @param[in]   data 数据缓冲区
* @param[in]   curData 当前值
* @retval      滤波值
*/
T_SiData CalcNoiseAvoid(const T_NoiseAvoidPara *para, T_NoiseAvoidData *data, T_SiData curData);

//! @endcond       //doxygen中隐藏

#ifdef __cplusplus
}
#endif

#endif
