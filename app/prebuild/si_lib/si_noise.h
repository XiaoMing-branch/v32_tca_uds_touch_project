/**
*****************************************************************************
* @brief  噪音检测器
* @file   si_noise.h
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

#ifndef __SI_NOISE_H__
#define __SI_NOISE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup noise 噪音检测器
 */
/** @} */

/**
 * @defgroup NoiseInterface 公共接口
 * @ingroup noise
 * 噪音检测器相关接口及数据结构
 * @{
 */

//噪音检测描述符
struct T_SiNoiseDetect;

//噪音检测退出条件基类
struct T_SiNoiseExitConditionBase;

/**
* @brief        噪音检测器基类
* @details      定义噪音检测器内部实现方法
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiBitMask noiseMask;                                                              /*!< 是否检测到噪音，位1为检测到噪音 */
    uint8_t typeId;                                                                     /*!< 类型Id，最大支持到7，每种噪音检测Id的类型需要不同 */
    uint8_t bufLen;                                                                     /*!< 缓冲区长度 */
    T_SiNoiseData *noiseBuf;                                                            /*!< 噪音检测缓冲区 */
    struct T_SiNoiseDetect *noiseDetectHeader;                                          /*!< 噪音检测信号集节点链表 */

    T_SiErrRt(*init)(T_SiNoiseObject *obj);                                             /*!< 安装噪音检测器 */
    T_SiErrRt(*exit)(T_SiNoiseObject *obj);                                             /*!< 卸载噪音检测器 */
    void (*hook)(T_SiNoiseObject *obj, uint8_t keyNum, const T_SiNoiseData *noiseBuf);  /*!< 噪音检测器钩子函数 */
    void (*reset)(T_SiNoiseObject *obj, uint8_t keyNo);                                 /*!< 噪音检测器复位，清除对应按键的噪音缓冲区数据 */

    T_SiErrRt(*run)(T_SiNoiseObject *obj, uint8_t keyNo, T_SiData rawData);             /*!< 运行噪音检测器，rawData表示原始数据 */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseBase;

/**
* @brief  检测到噪音时执行动作类型
*/
typedef enum
{
    SI_NOISE_ACTION_LOCK = 0,       /*!< 锁定对应的object，被锁定的object不会被调度器调度 */
    SI_NOISE_ACTION_CUSTOM,         /*!< 用户自定义动作，被关联的object会被标记检测到噪音，具体执行动作由用户自己实现 */
} T_SiNoiseAction;

/**
* @brief  噪音检测器退出匹配类型
*/
typedef enum
{
    SI_NOISE_EXIT_MATCH_ALL = 0,       /*!< 所有退出条件都匹配时退出 */
    SI_NOISE_EXIT_MATCH_ANY,         /*!< 任一退出条件匹配退出 */
} T_SiNoiseExitMatchType;

/**
* @brief        噪音检测描述符
* @details      描述噪音检测器如何和信号集对象关联，以及执行的动作类型等信息
*/
struct T_SiNoiseDetect
{
#ifdef SI_PC_DEBUG
#define SI_NOISE_DETECT_NAME_LEN  100
    char name[SI_NOISE_DETECT_NAME_LEN];
#endif
    T_SiNoiseDetectMatchType matchType;             /*!< 匹配类型，所有或任意--需要用户指定 */
    T_SiBitMask mask;                               /*!< 待检测噪声通道掩码--需要用户指定 */
    T_SiNoiseAction action;                         /*!< 检测到噪音时执行的动作类型--需要用户指定 */
    uint8_t siObjIsArray;                           /*!< 信号集对象是否是数组，0表示不是，1表示是数组--需要用户指定 */
    uint8_t siObjArrayNum;                          /*!< 信号集对象是数组时，数组长度--需要用户指定 */
    T_SiObject *siObj;                             /*!< 信号集对象--需要用户指定 */
    T_SiObject **siObjs;                            /*!< 信号集对象数组--需要用户指定 */
    void (*statusChanged)(uint8_t status);          /*!< 噪音状态发生改变，status非0表示检测到噪音，0表示噪音消失，可以为NULL--需要用户指定 */
    void (*statusChanged2)(T_SiObject *obj, struct T_SiNoiseDetect *noiseDetect, uint8_t status); /*!< 噪音状态发生改变，status非0表示检测到噪音，0表示噪音消失，可以为NULL--用户可选配置 */
    void (*hook)(struct T_SiNoiseDetect *noiseDetect, T_SiNoiseObject *noiseObj);   /*!< 回调接口，可以为NULL--用户可选配置 */

    uint8_t exitResetSiObject;                      /*!< 退出时Reset信号集描述符--需要用户指定 */
    uint8_t exitConditionNum;                       /*!< 退出条件个数，为0表示立即退出--需要用户指定 */
    T_SiNoiseExitConditionBase **exitConditions;    /*!< 退出条件--需要用户指定 */
    T_SiNoiseExitMatchType exitMatchType;           /*!< 退出条件匹配类型--需要用户指定 */
    uint32_t exitTimeoutMs;                         /*!< 退出条件强制超时时间，0表示永不强制超时--需要用户指定 */

    //! @cond
    struct
    {
        uint8_t status;
        uint32_t beginExitT;
    } exitData;                                     /*!< 退出条件算法临时数据 */
    struct T_SiNoiseDetect *link;                   /*!< 单向链表 */
    //! @endcond       //doxygen中隐藏
};
typedef struct T_SiNoiseDetect T_SiNoiseDetect;

/**
* @brief    注册噪音检测器回调函数
* @details     调度器执行完噪音检测器后，紧接着会执行其回调函数
* @param[in]   nd 噪音检测器节点
* @param[in]   hookCallback 回调函数
* @retval      无
*/
#define SiNoiseRegisterHook(nd,hookCallback)      (((T_SiNoiseBase*)(nd))->hook = hookCallback)

/**
* @brief    复位噪音检测器
* @param[in]   obj 噪音检测对象
* @retval      SI_RT_OK 复位成功
* @retval      other 复位失败
*/
T_SiErrRt SiNoiseResetAll(T_SiNoiseObject *obj);

/**
* @brief    修改噪音检测器算法参数
* @param[in]   nd 噪音检测器节点
* @param[in]   newpara 新参数
* @retval      无
*/
#define SiNoiseSetPara(nd,newpara)     (memcpy(&(nd)->para,(newpara),sizeof((nd)->para)))

/**
* @brief    是否检测到噪音
* @param[in]   obj 描述符对象
* @param[in]   keyNo 信号编号
* @retval      0 未检测到噪音
* @retval      其它 检测到噪音
*/
#define SiNoiseIsDetect(obj,keyNo) (((T_SiNoiseBase *)(obj)->noise)->noiseMask & (0x1<<(keyNo)))

/**
* @brief    是否检测到噪音
* @param[in]   noise 噪音检测器节点
* @param[in]   keyNo 信号编号
* @retval      0 未检测到噪音
* @retval      其它 检测到噪音
*/
#define SiNoiseIsDetect2(noise,keyNo) (((T_SiNoiseBase *)noise)->noiseMask & (0x1<<(keyNo)))

/**
* @brief    是否检测到噪音
* @param[in]   obj 信号集对象
* @retval      0 未检测到噪音
* @retval      其它 检测到噪音
*/
#define SiNoiseIsDetect3(obj) ((obj)->noiseDetected)

/**
* @brief    是否检测到噪音
* @param[in]   algo 算法描述符
* @retval      0 未检测到噪音
* @retval      其它 检测到噪音
*/
int SiNoiseIsDetect4(T_SiAlgoObject *algo);

/**
* @brief    设置噪音标志位
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiNoiseSet(obj,keyNo) (((T_SiNoiseBase *)(obj)->noise)->noiseMask |= (0x1<<(keyNo)))

/**
* @brief    清除噪音标志位
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @retval      无
*/
#define SiNoiseClr(obj,keyNo) (((T_SiNoiseBase *)(obj)->noise)->noiseMask &= ~(0x1<<(keyNo)))

/**
* @brief    检查噪音是否匹配
* @param[in]   noise 噪音检测器节点
* @param[in]   detect 噪音检测描述符节点
* @retval      0 未检测到噪音
* @retval      1 检测到噪音
*/
int SiNoiseIsMatch(T_SiNoiseBase *noise, T_SiNoiseDetect *detect);

/**
* @brief    注册噪音检测描述符
* @param[in]   noise 噪音检测器节点
* @param[in]   noiseDetect 噪音检测描述符节点
* @retval      SI_RT_OK 注册成功
* @retval      other 注册失败
*/
T_SiErrRt SiNoiseDetectRegister(T_SiNoiseBase *noise, T_SiNoiseDetect *noiseDetect);

/**
* @brief    运行噪声检测器
* @param[in]   noiseObj 噪音检测对象
* @param[in]   siRawData 原始数据
* @retval      SI_RT_OK 成功
* @retval      other 失败
*/
T_SiErrRt NoiseDetectRun(T_SiNoiseObject *noiseObj, T_SiData *siRawData);

/** @} */

//****************************通用噪音检测对象*******************************************************

//****************************MinMax噪音检测器******************************************
/**
 * @defgroup NoiseMinMax MinMax噪音检测器
 * @ingroup noise
 * 噪音检测器原理：当窗口中最大值和最小值的差值大于阈值threshold时，认为有噪音，当差值小于threshold且持续releaseDelayS后认为噪音消失
 * @{
 */
//******噪音检测器优缺点：消耗资源少，噪音分辨率不高

#define SI_NOISE_DATA_MINMAX_WINDOWSIZE      20                 /*!< MinMax噪音检测器窗口缓冲区最大长度 */
#define SI_NOISE_MINMAX_TYPEID          0                       /*!< 噪音检测器typeId */

/**
* @brief        MinMax噪音检测器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    uint8_t fillLen;                                              /*!< 窗口中已经填充数据长度 */
    uint8_t skipCnt;                                              /*!< 跳帧计数器 */
    uint16_t eliminate;                                           /*!< 消抖计数器 */
    T_SiData window[SI_NOISE_DATA_MINMAX_WINDOWSIZE];             /*!< 窗口缓冲区 */
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    uint32_t lastRunT;                                            /*!< 上次运行时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseDataMinMax;

/**
* @brief        MinMax噪音检测器算法参数
*/
typedef struct
{
    uint8_t windowLen;                                /*!< 窗口长度，--需要用户指定 */
    uint8_t sampIntval;                               /*!< 采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定 */
    uint16_t detectEliminate;                         /*!< 检测消抖次数，当达到阈值且持续一段时间后认为噪音有效，单位为采样个数（以sampIntval采样）--需要用户指定 */
    uint8_t releaseDelayS[SI_PNODE_CH_MAXNUM];        /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位S--需要用户指定 */
    uint8_t releaseDelayDeciS[SI_PNODE_CH_MAXNUM];    /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位0.1S--需要用户指定 */
    T_SiNoiseData threshold[SI_PNODE_CH_MAXNUM];      /*!< 噪音检测阈值--需要用户指定 */
    uint16_t validIntvalMs;                           /*!< 两次处理间的有效间隔时间，超过本阈值认为无效，0表示不检测阈值是否超时--需要用户指定 */
} T_SiNoiseParaMinMax;

/**
* @brief        MinMax噪音检测器描述符
* @details      噪音检测器原理：当窗口中最大值和最小值的差值大于阈值threshold时，认为有噪音，当差值小于threshold且持续releaseDelayS后认为噪音消失
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseBase base;                   /*!< 噪音检测器基类，必须放在开头 */
    T_SiNoiseParaMinMax para;             /*!< 噪音检测器算法参数 */
    uint8_t dataLen;                      /*!< 噪音检测器数据缓冲区个数 */
    T_SiNoiseDataMinMax *pdata;           /*!< 噪音检测器数据 */
} T_SiNoiseMinMax;

/**
* @brief    MinMax噪音检测器节点描述符初始化
* @param[in]   nd 噪音检测器节点
* @param[in]   para 噪音检测器参数
* @param[in]   dataLen 噪音检测器缓冲区长度
* @param[in]   pdata 噪音检测器缓冲区
* @param[in]   noiseBufLen 噪音检测器结果缓冲区长度
* @param[in]   noiseBuf 噪音检测器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseMinMaxNodeInit(T_SiNoiseMinMax *nd, const T_SiNoiseParaMinMax *para, uint8_t dataLen, T_SiNoiseDataMinMax *pdata, uint8_t noiseBufLen, T_SiNoiseData *noiseBuf);
/** @} */

//****************************方差噪音检测器******************************************
/**
 * @defgroup NoiseVariance 方差噪音检测器
 * @ingroup noise
 * 噪音检测器原理：计算窗口中的方差值，不用标准差是因为标准差会用到浮点运算，方差值大于阈值threshold时，认为有噪音，当方差值小于threshold且持续releaseDelayS后认为噪音消失
 * @{
 */
//******噪音检测器优缺点：噪音分辨率高，消耗资源多

#define SI_NOISE_VARIANCE_TYPEID            1               /*!< 噪音检测器typeId */

/**
* @brief        方差噪音检测器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    uint8_t fillLen;                                              /*!< 窗口中已经填充数据长度 */
    uint8_t skipCnt;                                              /*!< 跳帧计数器 */
    uint16_t eliminate;                                           /*!< 消抖计数器 */
    T_SiData window[SI_NOISE_DATA_MINMAX_WINDOWSIZE];             /*!< 窗口缓冲区 */
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    uint32_t lastRunT;                                            /*!< 上次运行时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseDataVariance;

/**
* @brief        方差噪音检测器算法参数
*/
typedef struct
{
    uint8_t windowLen;                                /*!< 窗口长度--需要用户指定 */
    uint8_t sampIntval;                               /*!< 采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定 */
    uint16_t detectEliminate;                         /*!< 检测消抖次数，当达到阈值且持续一段时间后认为噪音有效，单位为采样个数（以sampIntval采样）--需要用户指定 */
    uint8_t releaseDelayS[SI_PNODE_CH_MAXNUM];        /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位S--需要用户指定 */
    uint8_t releaseDelayDeciS[SI_PNODE_CH_MAXNUM];    /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位0.1S--需要用户指定 */
    T_SiNoiseData threshold[SI_PNODE_CH_MAXNUM];      /*!< 噪音检测阈值--需要用户指定 */
    uint16_t validIntvalMs;                           /*!< 两次处理间的有效间隔时间，超过本阈值认为无效，0表示不检测阈值是否超时--需要用户指定 */
} T_SiNoiseParaVariance;

/**
* @brief        方差噪音检测器描述符
* @details      噪音检测器原理：计算窗口中的方差值，不用标准差是因为标准差会用到浮点运算，方差值大于阈值threshold时，认为有噪音，当方差值小于threshold且持续releaseDelayS后认为噪音消失
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseBase base;                   /*!< 噪音检测器基类，必须放在开头 */
    T_SiNoiseParaVariance para;           /*!< 噪音检测器算法参数 */
    uint8_t dataLen;                      /*!< 噪音检测器数据缓冲区个数 */
    T_SiNoiseDataVariance *pdata;         /*!< 噪音检测器数据 */
} T_SiNoiseVariance;

/**
* @brief    方差噪音检测器节点描述符初始化
* @param[in]   nd 噪音检测器节点
* @param[in]   para 噪音检测器参数
* @param[in]   dataLen 噪音检测器缓冲区长度
* @param[in]   pdata 噪音检测器缓冲区
* @param[in]   noiseBufLen 噪音检测器结果缓冲区长度
* @param[in]   noiseBuf 噪音检测器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseVarianceNodeInit(T_SiNoiseVariance *nd, const T_SiNoiseParaVariance *para, uint8_t dataLen, T_SiNoiseDataVariance *pdata, uint8_t noiseBufLen, T_SiNoiseData *noiseBuf);
/** @} */

/**
* @brief        方差噪音检测器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    uint16_t eliminate;                                           /*!< 消抖计数器 */
    T_SiFastNoise fn;
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    uint32_t lastRunT;                                            /*!< 上次运行时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseDataFastVariance;

/**
* @brief        方差噪音检测器算法参数
*/
typedef struct
{
    uint16_t windowLen;                               /*!< 窗口长度--需要用户指定 */
    uint16_t detectEliminate;                         /*!< 检测消抖次数，当达到阈值且持续一段时间后认为噪音有效，单位为一窗口数据--需要用户指定 */
    uint8_t releaseDelayS;                            /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位S--需要用户指定 */
    uint8_t releaseDelayDeciS;                        /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位0.1S--需要用户指定 */
    T_SiNoiseData threshold;                          /*!< 噪音检测阈值--需要用户指定 */
    uint16_t validIntvalMs;                           /*!< 两次处理间的有效间隔时间，超过本阈值认为无效，0表示不检测阈值是否超时--需要用户指定 */
} T_SiNoiseParaFastVariance;

/**
* @brief        方差噪音检测器描述符
* @details      噪音检测器原理：计算窗口中的方差值，不用标准差是因为标准差会用到浮点运算，方差值大于阈值threshold时，认为有噪音，当方差值小于threshold且持续releaseDelayS后认为噪音消失
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseBase base;                   /*!< 噪音检测器基类，必须放在开头 */
    T_SiNoiseParaFastVariance para;           /*!< 噪音检测器算法参数 */
    uint8_t dataLen;                      /*!< 噪音检测器数据缓冲区个数 */
    T_SiNoiseDataFastVariance *pdata;         /*!< 噪音检测器数据 */
} T_SiNoiseFastVariance;

/**
* @brief    方差噪音检测器节点描述符初始化
* @param[in]   nd 噪音检测器节点
* @param[in]   para 噪音检测器参数
* @param[in]   dataLen 噪音检测器缓冲区长度
* @param[in]   pdata 噪音检测器缓冲区
* @param[in]   noiseBufLen 噪音检测器结果缓冲区长度
* @param[in]   noiseBuf 噪音检测器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseFastVarianceNodeInit(T_SiNoiseFastVariance *nd, const T_SiNoiseParaFastVariance *para, uint8_t dataLen, T_SiNoiseDataFastVariance *pdata, uint8_t noiseBufLen, T_SiNoiseData *noiseBuf);
/** @} */

//****************************斜率噪音检测器******************************************
/**
 * @defgroup NoiseGradient 斜率噪音检测器
 * @ingroup noise
 * 噪音检测器原理：当斜率绝对值大于阈值threshold时，认为有噪音，当斜率绝对值小于threshold且持续releaseDelayS后认为噪音消失
 * @{
 */

#define SI_NOISE_DATA_GRADIENT_WINDOWSIZE      10                 /*!< 斜率噪音检测器窗口缓冲区最大长度 */
#define SI_NOISE_GRADIENT_TYPEID            2                     /*!< 噪音检测器typeId */

/**
* @brief        斜率噪音检测器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    uint8_t fillLen;                                              /*!< 窗口中已经填充数据长度 */
    uint8_t skipCnt;                                              /*!< 跳帧计数器 */
    T_SiData window[SI_NOISE_DATA_GRADIENT_WINDOWSIZE];           /*!< 窗口缓冲区 */
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    uint32_t lastRunT;                                            /*!< 上次运行时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseDataGradient;

/**
* @brief        斜率噪音检测器算法参数
*/
typedef struct
{
    uint8_t sampIntval;                               /*!< 采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定 */
    uint8_t gradientIntval;                           /*!< 斜率间隔，单位为帧个数，范围[0，SI_NOISE_DATA_GRADIENT_WINDOWSIZE-1)--需要用户指定 */
    uint8_t releaseDelayS[SI_PNODE_CH_MAXNUM];        /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位S--需要用户指定 */
    uint8_t releaseDelayDeciS[SI_PNODE_CH_MAXNUM];    /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位0.1S--需要用户指定 */
    T_SiNoiseData threshold[SI_PNODE_CH_MAXNUM];      /*!< 噪音检测阈值--需要用户指定 */
    uint16_t validIntvalMs;                           /*!< 两次处理间的有效间隔时间，超过本阈值认为无效，0表示不检测阈值是否超时--需要用户指定 */
} T_SiNoiseParaGradient;

/**
* @brief        斜率噪音检测器描述符
* @details      噪音检测器原理：当斜率绝对值大于阈值threshold时，认为有噪音，当斜率绝对值小于threshold且持续releaseDelayS后认为噪音消失
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseBase base;                   /*!< 噪音检测器基类，必须放在开头 */
    T_SiNoiseParaGradient para;           /*!< 噪音检测器算法参数 */
    uint8_t dataLen;                      /*!< 噪音检测器数据缓冲区个数 */
    T_SiNoiseDataGradient *pdata;         /*!< 噪音检测器数据 */
} T_SiNoiseGradient;

/**
* @brief    斜率噪音检测器节点描述符初始化
* @param[in]   nd 噪音检测器节点
* @param[in]   para 噪音检测器参数
* @param[in]   dataLen 噪音检测器缓冲区长度
* @param[in]   pdata 噪音检测器缓冲区
* @param[in]   noiseBufLen 噪音检测器结果缓冲区长度
* @param[in]   noiseBuf 噪音检测器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseGradientNodeInit(T_SiNoiseGradient *nd, const T_SiNoiseParaGradient *para, uint8_t dataLen, T_SiNoiseDataGradient *pdata, uint8_t noiseBufLen, T_SiNoiseData *noiseBuf);
/** @} */

//****************************Diff噪音检测器******************************************
/**
 * @defgroup NoiseDiff Diff噪音检测器
 * @ingroup noise
 * 噪音检测器原理：当diff绝对值大于阈值threshold时，认为有噪音，当diff绝对值小于释放阈值且持续releaseDelayS后认为噪音消失
 * @{
 */

#define SI_NOISE_DIFF_TYPEID            3                     /*!< 噪音检测器typeId */

/**
* @brief        Diff噪音检测器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    struct
    {
        uint8_t ready;                                            /*!< 基线是否就绪 */
        uint8_t locked;                                           /*!< 基线是否被锁定 */
        uint16_t sampCnt;                                         /*!< 已经采样次数 */
        T_SiData value;                                           /*!< 基线值 */
        int32_t sampSumValue;                                     /*!< 采样值总和 */
    } baseline;
    uint16_t eliminateCnt;                                        /*!< 消抖计数器 */
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    uint32_t beginDetectT;                                        /*!< 记录开始检测时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseDataDiff;

/**
* @brief        Diff噪音检测器算法参数
*/
typedef struct
{
    int16_t thresholdSign;                   /*!< 阈值的符号，取-1，0，1，当为0时取diff的绝对值，否则diff值乘以thresholdSign后再与detectThreshold比较--需要用户指定 */
    struct
    {
        uint16_t cntThreshold;               /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        uint16_t firstCntThreshold;          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        T_SiData steps[SI_PNODE_CH_MAXNUM];  /*!< 基线追踪器步进--需要用户指定 */
    } baseline;       //基线
    struct
    {
        uint16_t detectEliminate;           /*!< 检测消抖次数--需要用户指定 */
        T_SiNoiseData detectThreshold;      /*!< 噪音检测阈值--需要用户指定 */
        uint8_t releaseThreshold;           /*!< 释放阈值，范围3-9，表示：检测阈值的0.3-0.9--需要用户指定 */
        uint8_t releaseDelayS;              /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位S--需要用户指定 */
        uint8_t releaseDelayDeciS;          /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位0.1S--需要用户指定 */
    } arbiters[SI_PNODE_CH_MAXNUM];
    uint16_t forceReleaseTimeS;             /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiNoiseParaDiff;

/**
* @brief        Diff噪音检测器描述符
* @details      噪音检测器原理：当diff绝对值大于阈值threshold时，认为有噪音，当diff绝对值小于释放阈值且持续releaseDelayS后认为噪音消失
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseBase base;               /*!< 噪音检测器基类，必须放在开头 */
    T_SiNoiseParaDiff para;           /*!< 噪音检测器算法参数 */
    uint8_t dataLen;                  /*!< 噪音检测器数据缓冲区个数 */
    T_SiNoiseDataDiff *pdata;         /*!< 噪音检测器数据 */
} T_SiNoiseDiff;

/**
* @brief    Diff噪音检测器节点描述符初始化
* @param[in]   nd 噪音检测器节点
* @param[in]   para 噪音检测器参数
* @param[in]   dataLen 噪音检测器缓冲区长度
* @param[in]   pdata 噪音检测器缓冲区
* @param[in]   noiseBufLen 噪音检测器结果缓冲区长度
* @param[in]   noiseBuf 噪音检测器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseDiffNodeInit(T_SiNoiseDiff *nd, const T_SiNoiseParaDiff *para, uint8_t dataLen, T_SiNoiseDataDiff *pdata, uint8_t noiseBufLen, T_SiNoiseData *noiseBuf);
/** @} */

/**
* @brief        LiteDiff噪音检测器数据类型
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    uint8_t status;                                               /*!< 检测器状态 */
    struct
    {
        uint8_t ready;                                            /*!< 基线是否就绪 */
        uint8_t locked;                                           /*!< 基线是否被锁定 */
        uint16_t sampCnt;                                         /*!< 已经采样次数 */
        T_SiData value;                                           /*!< 基线值 */
        int32_t sampSumValue;                                     /*!< 采样值总和 */
    } baseline;
    uint16_t eliminateCnt;                                        /*!< 消抖计数器 */
    uint32_t beginReleaseT;                                       /*!< 记录开始释放时间，单位ms */
    uint32_t beginDetectT;                                        /*!< 记录开始检测时间，单位ms */
    //! @endcond       //doxygen中隐藏
} T_SiNoiseDataLiteDiff;

/**
* @brief        Diff噪音检测器算法参数
*/
typedef struct
{
    int16_t thresholdSign;                   /*!< 阈值的符号，取-1，0，1，当为0时取diff的绝对值，否则diff值乘以thresholdSign后再与detectThreshold比较--需要用户指定 */
    struct
    {
        uint16_t cntThreshold;               /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        uint16_t firstCntThreshold;          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        T_SiData step;                       /*!< 基线追踪器步进--需要用户指定 */
    } baseline;       //基线
    struct
    {
        uint16_t detectEliminate;           /*!< 检测消抖次数--需要用户指定 */
        T_SiNoiseData detectThreshold;      /*!< 噪音检测阈值--需要用户指定 */
        uint8_t releaseThreshold;           /*!< 释放阈值，范围3-9，表示：检测阈值的0.3-0.9--需要用户指定 */
        uint8_t releaseDelayS;              /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位S--需要用户指定 */
        uint8_t releaseDelayDeciS;          /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位0.1S--需要用户指定 */
    } arbiter;
    uint16_t forceReleaseTimeS;             /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
} T_SiNoiseParaLiteDiff;

/**
* @brief        Diff噪音检测器描述符
* @details      噪音检测器原理：当diff绝对值大于阈值threshold时，认为有噪音，当diff绝对值小于释放阈值且持续releaseDelayS后认为噪音消失
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    T_SiNoiseBase base;               /*!< 噪音检测器基类，必须放在开头 */
    T_SiNoiseParaLiteDiff para;           /*!< 噪音检测器算法参数 */
    uint8_t dataLen;                  /*!< 噪音检测器数据缓冲区个数 */
    T_SiNoiseDataLiteDiff *pdata;         /*!< 噪音检测器数据 */
} T_SiNoiseLiteDiff;

/**
* @brief    Diff噪音检测器节点描述符初始化
* @param[in]   nd 噪音检测器节点
* @param[in]   para 噪音检测器参数
* @param[in]   dataLen 噪音检测器缓冲区长度
* @param[in]   pdata 噪音检测器缓冲区
* @param[in]   noiseBufLen 噪音检测器结果缓冲区长度
* @param[in]   noiseBuf 噪音检测器结果缓冲区
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiNoiseLiteDiffNodeInit(T_SiNoiseLiteDiff *nd, const T_SiNoiseParaLiteDiff *para, uint8_t dataLen, T_SiNoiseDataLiteDiff *pdata, uint8_t noiseBufLen, T_SiNoiseData *noiseBuf);
/** @} */

#ifdef __cplusplus
}
#endif

#endif
