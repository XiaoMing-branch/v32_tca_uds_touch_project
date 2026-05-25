/**
*****************************************************************************
* @brief  si arbiter interp header
* @file   si_arbiter_interp.h
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

#ifndef __SI_ARBITER_INTERP_H__
#define __SI_ARBITER_INTERP_H__

#ifdef __cplusplus
extern "C" {
#endif

//************************************************************************************
//按键解释判决器

//******判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间(如果配置按压条件，需满足)，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间（如果配置释放条件，需满足），认为按键被松开
//******支持操作：根据解释匹配器可选（如单击、双击、长按等）

//判决器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint8_t eliminateCnt;           //消抖计数器
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    T_SiKeyStatus matcherResult;    //匹配器结果
    uint32_t pressingMs;            //按压时长计数器
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
} T_SiArbiterDataInterpKey;
//判决器算法参数
typedef struct
{
    uint8_t pressEliminate;     //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;   //释放消抖次数--需要用户指定
    uint8_t releaseThreshold;   //释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定
    T_SiData pressThreshold;    //按压阈值--需要用户指定
    uint16_t forceReleaseTimeS; //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
    uint16_t releaseLockBaselineMs;   /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    struct T_SiArbiterInterpKeyMatcher *keyMatcher;  //按键匹配器，用来判断单双击等操作--需要用户指定

    uint8_t conditionArrayLen;                                   //条件列表长度--需要用户指定
    struct T_SiArbiterInterpKeyConditionBase **conditionArray;   //条件列表--需要用户指定
    const char *pressCondition;                                  //扩展的按压条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    const char *releaseCondition;                                //扩展的释放条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    struct T_SiArbiterInterpKeyEvent *prevEvent;                 //前置事件处理器--需要用户指定
    struct T_SiArbiterInterpKeyEvent *postEvent;                 //后置事件处理器--需要用户指定
} T_SiArbiterParaInterpKey;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;        //判决器基类，必须放在开头
    T_SiArbiterParaInterpKey para;     //判决器算法参数
    uint8_t dataLen;                //判决器数据缓冲区个数
    T_SiArbiterDataInterpKey *pdata;   //判决器数据
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; //值改变回调接口，keyNo按键编号，status按键状态
} T_SiArbiterInterpKey;

//判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyNodeInit(T_SiArbiterInterpKey *nd, const T_SiArbiterParaInterpKey *para, uint8_t dataLen, T_SiArbiterDataInterpKey *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

//******判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间(如果配置按压条件，需满足)，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间（如果配置释放条件，需满足），认为按键被松开
//******支持操作：根据解释匹配器可选（如单击、双击、长按等）

//判决器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint8_t eliminateCnt;           //消抖计数器
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    T_SiKeyStatus matcherResult;    //匹配器结果
    uint32_t pressingMs;            //按压时长计数器
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
} T_SiArbiterDataInterpKeyMt;
//判决器算法参数
typedef struct
{
    uint8_t pressEliminate;     //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;   //释放消抖次数--需要用户指定
    uint8_t releaseThreshold[SI_PNODE_CH_MAXNUM];   //释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定
    T_SiData pressThreshold[SI_PNODE_CH_MAXNUM];    //按压阈值--需要用户指定
    uint16_t forceReleaseTimeS; //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
    uint16_t releaseLockBaselineMs;   /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    struct T_SiArbiterInterpKeyMatcher *keyMatcher;  //按键匹配器，用来判断单双击等操作--需要用户指定

    uint8_t conditionArrayLen;                                         //条件列表长度--需要用户指定
    struct T_SiArbiterInterpKeyConditionBase **conditionArray;   //条件列表--需要用户指定
    const char *pressCondition;                                        //扩展的按压条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    const char *releaseCondition;                                      //扩展的释放条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    struct T_SiArbiterInterpKeyEvent *prevEvent;                 //前置事件处理器--需要用户指定
    struct T_SiArbiterInterpKeyEvent *postEvent;                 //后置事件处理器--需要用户指定
} T_SiArbiterParaInterpKeyMt;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;        //判决器基类，必须放在开头
    T_SiArbiterParaInterpKeyMt para;     //判决器算法参数
    uint8_t dataLen;                //判决器数据缓冲区个数
    T_SiArbiterDataInterpKeyMt *pdata;   //判决器数据
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; //值改变回调接口，keyNo按键编号，status按键状态
} T_SiArbiterInterpKeyMt;

//判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyMtNodeInit(T_SiArbiterInterpKeyMt *nd, const T_SiArbiterParaInterpKeyMt *para, uint8_t dataLen, T_SiArbiterDataInterpKeyMt *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

//************************************************************************************
//集合解释判决器

#define SET_INIT_STATUS             0           //集合初始状态
#define SET_PRESS_STATUS            1           //集合按下状态
#define SET_WAITRELEASE_STATUS      2           //刚检测到集合低于阈值，需要等待一会才会最终进入SET_RELEASE_STATUS
#define SET_RELEASE_STATUS          3           //集合松开状态，松开的集合，第二次扫描时会被设置成INIT状态

//******判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间(如果配置按压条件，需满足)，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间（如果配置释放条件，需满足），认为按键被松开
//******支持操作：根据解释匹配器可选（如单击、双击、长按、滑动等操作）

//判决器数据
typedef struct       //每通道数据
{
    uint8_t isPress;                //是否按压
} T_SiArbiterDataInterpSet;
typedef struct      //全局数据
{
    uint8_t status;                 //集合状态
    uint8_t eliminateCnt;           //消抖计数器
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    T_SiKeyStatus matcherResult;    //匹配器结果
    uint32_t pressingMs;            //按压时长计数器
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
} T_SiArbiterGbDataInterpSet;
//判决器算法参数
typedef struct
{
    uint8_t matchAll;           //1表示所有通道满足阈值认为有效，0表示任一通道满足阈值认为有效--需要用户指定

    uint8_t pressEliminate;     //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;   //释放消抖次数--需要用户指定
    uint8_t releaseThreshold;   //释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定
    T_SiData pressThreshold;    //按压阈值--需要用户指定
    uint8_t enableMergeDiffValue; //1使能合并diff按键值功能，0关闭，仅在matchAll为0时有效，使能后所有按键的diff值累加后和pressThreshold比较 --需要用户指定
    uint16_t forceReleaseTimeS; //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
    uint16_t releaseLockBaselineMs;   /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    struct T_SiArbiterInterpSetMatcher *keyMatcher;  //按键匹配器，用来判断单双击等操作--需要用户指定

    uint8_t conditionArrayLen;                                   //条件列表长度--需要用户指定
    struct T_SiArbiterInterpSetConditionBase **conditionArray;   //条件列表--需要用户指定
    const char *pressCondition;                                  //扩展的按压条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    const char *releaseCondition;                                //扩展的释放条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    struct T_SiArbiterInterpSetEvent *prevEvent;                 //前置事件处理器--需要用户指定
    struct T_SiArbiterInterpSetEvent *postEvent;                 //后置事件处理器--需要用户指定
} T_SiArbiterParaInterpSet;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;                //判决器基类，必须放在开头
    T_SiArbiterParaInterpSet para;       //判决器算法参数
    uint8_t dataLen;                     //判决器数据缓冲区个数
    T_SiArbiterDataInterpSet *pdata;     //判决器数据
    T_SiArbiterGbDataInterpSet gbdata;   //判决器全局数据
    T_SiArbiterSetValueChangedCallback valueChangedCallback; //值改变回调接口
} T_SiArbiterInterpSet;

//判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetNodeInit(T_SiArbiterInterpSet *nd, const T_SiArbiterParaInterpSet *para, uint8_t dataLen, T_SiArbiterDataInterpSet *pdata, T_SiArbiterSetValueChangedCallback valueChangedCallback);

//******判决器原理：当滤波值和基线值的差值超过设置的按压阈值一定的消抖时间(如果配置按压条件，需满足)，认为按键被按下，当差值低于设置的释放阈值一定的消抖时间（如果配置释放条件，需满足），认为按键被松开
//******支持操作：根据解释匹配器可选（如单击、双击、长按、滑动等操作）

//判决器数据
typedef struct       //每通道数据
{
    uint8_t isPress;                //是否按压
} T_SiArbiterDataInterpSetMt;
typedef struct      //全局数据
{
    uint8_t status;                 //集合状态
    uint8_t eliminateCnt;           //消抖计数器
    uint8_t lockBaseline;           /*!< 是否锁定基线 */
    T_SiKeyStatus matcherResult;    //匹配器结果
    uint32_t pressingMs;            //按压时长计数器
    uint32_t lockBaselineBeginTimeMs;    /*!< 锁定基线超时时间 */
} T_SiArbiterGbDataInterpSetMt;
//判决器算法参数
typedef struct
{
    uint8_t matchAll;           //1表示所有通道满足阈值认为有效，0表示任一通道满足阈值认为有效--需要用户指定

    uint8_t pressEliminate;     //按压消抖次数--需要用户指定
    uint8_t releaseEliminate;   //释放消抖次数--需要用户指定
    uint8_t releaseThreshold[SI_PNODE_CH_MAXNUM];   //释放阈值，范围3-9，表示：按键阈值的0.3-0.9--需要用户指定
    T_SiData pressThreshold[SI_PNODE_CH_MAXNUM];    //按压阈值--需要用户指定
    uint16_t forceReleaseTimeS; //强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定
    uint16_t releaseLockBaselineMs;   /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    struct T_SiArbiterInterpSetMatcher *keyMatcher;  //按键匹配器，用来判断单双击等操作--需要用户指定

    uint8_t conditionArrayLen;                                   //条件列表长度--需要用户指定
    struct T_SiArbiterInterpSetConditionBase **conditionArray;   //条件列表--需要用户指定
    const char *pressCondition;                                  //扩展的按压条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    const char *releaseCondition;                                //扩展的释放条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|--需要用户指定
    struct T_SiArbiterInterpSetEvent *prevEvent;                 //前置事件处理器--需要用户指定
    struct T_SiArbiterInterpSetEvent *postEvent;                 //后置事件处理器--需要用户指定
} T_SiArbiterParaInterpSetMt;
//判决器描述符
typedef struct
{
    T_SiArbiterBase base;                //判决器基类，必须放在开头
    T_SiArbiterParaInterpSetMt para;       //判决器算法参数
    uint8_t dataLen;                     //判决器数据缓冲区个数
    T_SiArbiterDataInterpSetMt *pdata;     //判决器数据
    T_SiArbiterGbDataInterpSetMt gbdata;   //判决器全局数据
    T_SiArbiterSetValueChangedCallback valueChangedCallback; //值改变回调接口
} T_SiArbiterInterpSetMt;

//判决器节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetMtNodeInit(T_SiArbiterInterpSetMt *nd, const T_SiArbiterParaInterpSetMt *para, uint8_t dataLen, T_SiArbiterDataInterpSetMt *pdata, T_SiArbiterSetValueChangedCallback valueChangedCallback);

#ifdef __cplusplus
}
#endif

#endif
