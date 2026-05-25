/**
*****************************************************************************
* @brief  放大器
* @file   si_amp.h
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

#ifndef __SI_AMP_H__
#define __SI_AMP_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup amp 放大器
 * 对原始数据进行放大
 */
 /** @} */

/**
 * @defgroup AmpInterface 公共接口
 * @ingroup amp
 * 放大器相关接口及数据结构
 * @{
 */

/**
* @brief        放大器基类
* @details      定义放大器内部实现方法
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiBitMask readyMask;                                                  /*!< 放大器就绪位掩码 */
    uint8_t bufLen;                                                         /*!< 缓冲区长度 */
    T_SiData *ampBuf;                                                       /*!< 放大器缓冲区 */

    T_SiErrRt(*init)(T_SiObject *obj);                                      /*!< 安装放大器 */
    T_SiErrRt(*exit)(T_SiObject *obj);                                      /*!< 卸载放大器 */
    void (*set)(T_SiObject *obj, uint8_t keyNo, T_SiData ampValue);         /*!< 手动设置放大器输出值 */
    void (*reset)(T_SiObject *obj, uint8_t keyNo);                          /*!< 放大器复位，清除对应按键的放大数据 */

    T_SiErrRt(*run)(T_SiObject *obj, uint8_t keyNo, T_SiData rawData);      /*!< 运行放大器，rawData表示原始数据 */
    //! @endcond       //doxygen中隐藏
} T_SiAmpBase;

/**
* @brief    复制放大器
* @details     从srcobj中复制放大器值到dstobj中
* @param[in]   dstobj 目的对象
* @param[in]   srcobj 源对象
* @retval      SI_RT_OK 复制成功
* @retval      other 复制失败，某些放大器可能禁止拷贝
*/
T_SiErrRt SiAmpCopy(T_SiObject *dstobj, T_SiObject *srcobj);

/**
* @brief    复位放大器
* @param[in]   obj 信号集对象
* @retval      SI_RT_OK 复位成功
* @retval      other 复位失败
*/
T_SiErrRt SiAmpResetAll(T_SiObject *obj);

/**
* @brief    修改放大器算法参数
* @param[in]   nd 放大器对象
* @param[in]   newpara 新参数
* @retval      无
*/
#define SiAmpSetPara(nd,newpara)     (memcpy(&(nd)->para,(newpara),sizeof((nd)->para)))

/**
* @brief    放大器是否就绪
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      0 未就绪
* @retval      其它 就绪
*/
#define SiAmpIsReady(obj,keyNo) (((T_SiAmpBase *)(obj)->amp)->readyMask & (0x1<<(keyNo)))

/**
* @brief    设置放大器就绪位
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiAmpSetReady(obj,keyNo) (((T_SiAmpBase *)(obj)->amp)->readyMask |= (0x1<<(keyNo)))

/**
* @brief    清除放大器就绪位
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiAmpClrReady(obj,keyNo) (((T_SiAmpBase *)(obj)->amp)->readyMask &= ~(0x1<<(keyNo)))

/** @} */

//****************************倍数放大器***************************************
/**
 * @defgroup AmpSimpMul 倍数放大器
 * @ingroup amp
 * 放大器原理：将原始数据直接乘以一个放大倍数
 * @{
 */

/**
* @brief        倍数放大器算法参数
*/
typedef struct
{
    uint16_t ratio;                           /*!< 放大比例，--需要用户指定 */
    T_SiData offset;                          /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiAmpParaSimpMul;

/**
* @brief        倍数放大器描述符
* @details      放大器原理：将原始数据直接乘以一个放大倍数
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiAmpBase base;                         /*!< 放大器基类，必须放在开头 */
    T_SiAmpParaSimpMul para;                  /*!< 放大器算法参数 */
    //! @endcond       //doxygen中隐藏
} T_SiAmpSimpMul;

/**
* @brief    倍数放大器节点描述符初始化
* @param[in]   nd 倍数放大器节点
* @param[in]   para 放大器参数
* @param[in]   ampBufLen 放大器结果缓冲区长度
* @param[in]   ampBuf 放大器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiAmpSimpMulNodeInit(T_SiAmpSimpMul *nd, const T_SiAmpParaSimpMul *para, uint8_t ampBufLen, T_SiData *ampBuf);
/** @} */

//****************************多通道倍数放大器*********************************
/**
 * @defgroup AmpSimpMulMt 多通道倍数放大器
 * @ingroup amp
 * 放大器原理：将原始数据直接乘以一个放大倍数
 * @{
 */

/**
* @brief        多通道倍数放大器算法参数
*/
typedef struct
{
    uint16_t deciRatios[SI_PNODE_CH_MAXNUM];                       /*!< 放大比例，单位1/10，--需要用户指定 */
    T_SiData offsets[SI_PNODE_CH_MAXNUM];                          /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiAmpParaSimpMulMt;

/**
* @brief        多通道倍数放大器描述符
* @details      放大器原理：将原始数据直接乘以一个放大倍数，每个通道放大倍数独立可调
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiAmpBase base;                         /*!< 放大器基类，必须放在开头 */
    T_SiAmpParaSimpMulMt para;                /*!< 放大器算法参数 */
    //! @endcond       //doxygen中隐藏
} T_SiAmpSimpMulMt;

/**
* @brief    多通道倍数放大器节点描述符初始化
* @param[in]   nd 多通道倍数放大器节点
* @param[in]   para 放大器参数
* @param[in]   ampBufLen 放大器结果缓冲区长度
* @param[in]   ampBuf 放大器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiAmpSimpMulMtNodeInit(T_SiAmpSimpMulMt *nd, const T_SiAmpParaSimpMulMt *para, uint8_t ampBufLen, T_SiData *ampBuf);
/** @} */

//****************************累加放大器***************************************
/**
 * @defgroup AmpSum 累加放大器
 * @ingroup amp
 * 放大器原理：将原始数值多次累加放大
 * @{
 */

/**
* @brief        累加放大器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sumCnt;              /*!< 已经累加次数 */
    T_SiData sumValue;            /*!< 累加值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiAmpDataSum;

/**
* @brief        累加放大器算法参数
*/
typedef struct
{
    uint16_t ratio;                       /*!< 放大比例，--需要用户指定 */
    T_SiData offset;                      /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiAmpParaSum;

/**
* @brief        累加放大器描述符
* @details      放大器原理：将原始数值多次累加放大
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiAmpBase base;                     /*!< 放大器基类，必须放在开头 */
    T_SiAmpParaSum para;                  /*!< 放大器算法参数 */
    uint8_t dataLen;                      /*!< 放大器数据缓冲区个数 */
    T_SiAmpDataSum *pdata;                /*!< 放大器数据 */
    //! @endcond       //doxygen中隐藏
} T_SiAmpSum;

/**
* @brief    累加放大器节点描述符初始化
* @param[in]   nd 累加放大器节点
* @param[in]   para 放大器参数
* @param[in]   dataLen 放大器缓冲区长度
* @param[in]   pdata 放大器缓冲区
* @param[in]   ampBufLen 放大器结果缓冲区长度
* @param[in]   ampBuf 放大器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiAmpSumNodeInit(T_SiAmpSum *nd, const T_SiAmpParaSum *para, uint8_t dataLen, T_SiAmpDataSum *pdata, uint8_t ampBufLen, T_SiData *ampBuf);
/** @} */

//****************************多通道累加放大器*********************************
/**
 * @defgroup AmpSumMt 多通道累加放大器
 * @ingroup amp
 * 放大器原理：将原始数值多次累加放大
 * @{
 */

/**
* @brief        多通道累加放大器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint16_t sumCnt;              /*!< 已经累加次数 */
    T_SiData sumValue;            /*!< 累加值总和 */
    //! @endcond       //doxygen中隐藏
} T_SiAmpDataSumMt;

/**
* @brief        多通道累加放大器算法参数
*/
typedef struct
{
    uint16_t ratios[SI_PNODE_CH_MAXNUM];        /*!< 放大比例，--需要用户指定 */
    T_SiData offsets[SI_PNODE_CH_MAXNUM];       /*!< 信号偏置值，原始数据会减去信号偏置值后再放大，--需要用户指定 */
} T_SiAmpParaSumMt;

/**
* @brief        多通道累加放大器描述符
* @details      放大器原理：将原始数值多次累加放大
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiAmpBase base;                     /*!< 放大器基类，必须放在开头 */
    T_SiAmpParaSumMt para;                /*!< 放大器算法参数 */
    uint8_t dataLen;                      /*!< 放大器数据缓冲区个数 */
    T_SiAmpDataSumMt *pdata;              /*!< 放大器数据 */
    //! @endcond       //doxygen中隐藏
} T_SiAmpSumMt;

/**
* @brief    多通道累加放大器节点描述符初始化
* @param[in]   nd 多通道累加放大器节点
* @param[in]   para 放大器参数
* @param[in]   dataLen 放大器缓冲区长度
* @param[in]   pdata 放大器缓冲区
* @param[in]   ampBufLen 放大器结果缓冲区长度
* @param[in]   ampBuf 放大器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiAmpSumMtNodeInit(T_SiAmpSumMt *nd, const T_SiAmpParaSumMt *para, uint8_t dataLen, T_SiAmpDataSumMt *pdata, uint8_t ampBufLen, T_SiData *ampBuf);
/** @} */

#ifdef __cplusplus
}
#endif

#endif
