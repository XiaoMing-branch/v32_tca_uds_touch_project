/**
*****************************************************************************
* @brief  类型定义
* @file   si_type.h
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

#ifndef __SI_TYPE_H__
#define __SI_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SI_PC_DEBUG
#define __INLINE       inline
#else
#ifndef   __INLINE
#define __INLINE
#endif
#endif

#define SiUnused(x)     ((void)(x))

/**
 * @defgroup type 类型定义
 * 定义了算法中常用的数据类型
 * @{
 */

/** 算法调度器类型 */
typedef enum
{
    SI_SCHEDULER_BALANCE = 0,            /*!< 均衡调度器，对当前的T_SiObject的滤波器、arbiter、baseline都扫描完后，再扫描下一个T_SiObject */
    SI_SCHEDULER_LOWPOWER,               /*!< 低功耗调度器，只有本调度器可以从低功耗或慢扫模式唤醒 */
    SI_SCHEDULER_SLICE,                  /*!< 切片调度器，会先扫描完所有T_SiObject的滤波器，再扫描完所有T_SiObject的arbiter，再扫描完所有T_SiObject的baseline */
} T_SiScheduler;

/**
* 信号类型
* @attention 算法中暂未使用
*/
typedef enum
{
    SI_TYPE_KEY = 0,             /*!< 按键 */
    SI_TYPE_SLIDER,              /*!< 滑条 */
    SI_TYPE_INEAR                /*!< 入耳 */
} T_SiType;

/**
* 信号增长方向
* @brief 信号被触发时，数值是变大还是变小
*/
typedef enum
{
    SI_GROW_DIRECT_UP = 0,              /*!< 向上增长，检测到有效信号时，数值变大 */
    SI_GROW_DIRECT_DOWN,                /*!< 向下增长，检测到有效信号时，数值变小 */
    SI_GROW_DIRECT_MT,                  /*!< 多通道增长，设置为本值后growDirectMt有效，依据growDirectMt配置判断每个通道的增长方向 */
} T_SiGrowDirect;

/**
* 信号对象状态
* @brief 设置不同状态，可控制对象是否会被调度器调用
*/
typedef enum
{
    SI_OBJECT_STATUS_RUN = 0,           /*!< 运行状态，会被调度器调用 */
    SI_OBJECT_STATUS_LOCK,              /*!< 锁定状态，不会被调度器调用 */
} T_SiObjectStatus;

/**
* 噪音检测器状态
* @brief 设置不同状态，可控制噪音检测器是否会被调度器调用
*/
typedef enum
{
    SI_NOISE_OBJECT_STATUS_RUN = 0,           /*!< 运行状态，会被调度器调用 */
    SI_NOISE_OBJECT_STATUS_LOCK,              /*!< 锁定状态，不会被调度器调用 */
} T_SiNoiseObjectStatus;

/** 噪音检测器匹配类型 */
typedef enum
{
    SI_NOISE_DETECT_MATCH_ALL = 0,          /*!< 全部噪声检测通道都检测到，才认为有噪声 */
    SI_NOISE_DETECT_MATCH_ANY               /*!< 任意噪声检测通道检测到，就认为有噪声 */
} T_SiNoiseDetectMatchType;

/** 返回值编码 */
typedef enum
{
    SI_RT_OK = 0,        /*!< 正常 */
    SI_RT_WKUP,          /*!< 从低功耗中唤醒 */
    SI_RTERR_NULL,       /*!< 空指针异常 */
    SI_RTERR_OVF,        /*!< 溢出错误 */
    SI_RTERR_BUFLEN,     /*!< 缓冲区大小配置异常 */
    SI_RTERR_PARA,       /*!< 参数配置错误 */
    SI_RTERR_NOKEY,      /*!< 未找到对应的按键 */
    SI_RTERR_NOTEXIST,   /*!< 不存在 */
    SI_RTERR_DISMATCH,   /*!< 不匹配 */
    SI_RTERR_NOTSUPPORT, /*!< 不支持 */
    SI_RTERR_SYNTAX,     /*!< 语法错误 */
} T_SiErrRt;

/** 按键状态 */
typedef enum
{
    SI_KEY_NONE = 0,             /*!< 无按键 */
    SI_KEY_INVALID,              /*!< 无效按键 */
    SI_KEY_RAW_PRESS,            /*!< 按键按下原始值 */
    SI_KEY_RAW_RELEASE,          /*!< 按键释放原始值 */
    SI_KEY_PRESS,                /*!< 按键按下 */
    SI_KEY_RELEASE,              /*!< 按键释放 */
    SI_KEY_CLICK,                /*!< 按键单击 */
    SI_KEY_DBLCLICK,             /*!< 按键双击 */
    SI_KEY_LONGPRESS,            /*!< 按键长按 */
    SI_KEY_SLIDER,               /*!< 按键滑动 */
} T_SiKeyStatus;

/** 滑条状态 */
typedef enum
{
    SI_SLIDER_START = 0,        /*!< 滑条开始滑动 */
    SI_SLIDER_RUN,              /*!< 滑条滑动中 */
    SI_SLIDER_STOP              /*!< 滑条滑动结束 */
} T_SiSliderStatus;

/** 滑条方向 */
typedef enum
{
    SI_SLIDER_HORIZONTAL,       /*!< 滑条水平方向 */
    SI_SLIDER_VERTICAL,         /*!< 滑条垂直方向 */
} T_SiSliderDirect;

typedef int16_t T_SiData;        /*!< 信号数据类型定义 */

typedef uint32_t T_SiBitMask;    /*!< 信号通道位掩码类型定义 */

typedef int16_t T_SiNoiseData;  /*!< 噪音数据类型定义 */

typedef struct
{
    uint32_t count;
    float mean;
    float m2;
} T_SiFastNoise;                 /*!< 噪音数据类型定义 */

#define SI_MAX_NOISE_DATA_VALUE     (0x7FFF)    /*!< 噪音数据最大值 */

/**
* @brief    滑条值改变回调函数类型
* @param[in]   status 滑条状态
* @param[in]   curpos 滑条当前坐标位置
* @param[in]   value 滑动距离，正值表示正向滑动，负值表示反向滑动
* @retval      无
*/
typedef void (*T_SiArbiterSliderValueChangedCallback)(T_SiSliderStatus status, int curpos, int value);

/**
* @brief    按键状态改变回调函数类型
* @param[in]   keyNo 按键编号
* @param[in]   status 按键状态
* @retval      无
*/
typedef void (*T_SiArbiterKeyValueChangedCallback)(uint8_t keyNo, T_SiKeyStatus status);

/**
* @brief    按键状态改变回调函数类型
* @param[in]   keyNo 按键编号
* @param[in]   status 按键状态
* @param[in]   forceRelease 1表示强制释放，0表示非强制释放
* @retval      无
*/
typedef void (*T_SiArbiterKeyForceValueChangedCallback)(uint8_t keyNo, T_SiKeyStatus status, int forceRelease);

/**
* @brief    集合状态改变回调函数类型
* @param[in]   status 集合状态
* @param[in]   value 值有可能是指针，也有可能是有符号数，具体依据status而定
* @retval      无
*/
typedef void (*T_SiArbiterSetValueChangedCallback)(T_SiKeyStatus status, uint32_t value);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
