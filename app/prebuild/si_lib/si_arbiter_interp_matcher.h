/**
*****************************************************************************
* @brief  si arbiter interp matcher header
* @file   si_arbiter_interp_matcher.h
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

#ifndef __SI_ARBITER_INTERP_MATCHER_H__
#define __SI_ARBITER_INTERP_MATCHER_H__

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************************************************************************
//解释判决器按键匹配器

//************************************************************************************
//匹配器基类
struct T_SiArbiterInterpKeyMatcherBase
{
    uint8_t (*matchSuccess)(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcherBase *self, uint8_t keyNo);   //是否匹配成功
    uint8_t (*canRelease)(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcherBase *self, uint8_t keyNo);     //是否可以释放按键，如果可以释放表明已经存在匹配结果
    uint32_t (*getSuccessTime)(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcherBase *self, uint8_t keyNo); //获取匹配器匹配成功的时间
    T_SiKeyStatus(*getResult)(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcherBase *self, uint8_t keyNo);  //返回匹配结果
    void (*reset)(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcherBase *self, uint8_t keyNo);             //每次匹配完成后会调用reset方法
    void (*run)(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcherBase *self, uint8_t keyNo, uint8_t isPress, uint8_t isRelease);

    struct T_SiArbiterInterpKeyMatcherBase *link;        //单向链表
};
typedef struct T_SiArbiterInterpKeyMatcherBase T_SiArbiterInterpKeyMatcherBase;

//匹配器
struct T_SiArbiterInterpKeyMatcher
{
    uint8_t headFirst;    //为0表示最晚匹配最优，为1表示最早匹配最优
    struct T_SiArbiterInterpKeyMatcherBase *header;
};

//匹配器初始化，headFirst为0表示最晚匹配最优，为1表示最早匹配最优
T_SiErrRt SiArbiterInterpKeyMatcherInit(struct T_SiArbiterInterpKeyMatcher *matcher, uint8_t headFirst);
//注册匹配器
T_SiErrRt SiArbiterInterpKeyMatcherRegister(struct T_SiArbiterInterpKeyMatcher *matcher, struct T_SiArbiterInterpKeyMatcherBase *obj);

void SiArbiterInterpKeyMatcherRun(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcher *matcher, uint8_t keyNo, uint8_t isPress, uint8_t isRelease);     //运行匹配器
void SiArbiterInterpKeyMatcherReset(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcher *matcher, uint8_t keyNo);       //复位匹配器
uint8_t SiArbiterInterpKeyMatcherCanRelease(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcher *matcher, uint8_t keyNo);   //是否可以释放按键，如果可以释放表明已经存在匹配结果
T_SiKeyStatus SiArbiterInterpKeyMatcherGetResult(T_SiObject *obj, struct T_SiArbiterInterpKeyMatcher *matcher, uint8_t keyNo); //获取匹配结果

//******单击匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
} T_SiArbiterInterpKeyMatcherDataClick;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpKeyMatcherBase base;          //匹配器基类，必须放在开头
    uint8_t dataLen;                               //匹配器数据缓冲区个数
    T_SiArbiterInterpKeyMatcherDataClick *pdata;   //匹配器数据
} T_SiArbiterInterpKeyMatcherClick;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyMatcherClickInit(T_SiArbiterInterpKeyMatcherClick *nd, uint8_t dataLen, T_SiArbiterInterpKeyMatcherDataClick *pdata);

//******双击匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint8_t matchSuccess;           //匹配成功
    uint8_t invalidKey;             //无效按键标记位
    uint16_t successTime;           //匹配成功时间
    uint32_t beginClickTime;        //开始敲击时间
    uint32_t waitDblClkMs;          //等待双击时间
} T_SiArbiterInterpKeyMatcherDataDblClick;
//匹配器算法参数
typedef struct
{
    uint8_t maxPressTime;       //敲击时，手指按在pad上不释放的最大时间，若手指按压在pad上不拿开超过一段时间认为双击无效，单位0.1s，为0表示不检测--需要用户指定
    uint8_t dblClickIntval;     //判断按键双击时间间隔，单位0.1s--需要用户指定
    uint8_t multiClickIgnore;   //设置1表示敲击次数超过2时认为双击无效，否则认为双击有效
} T_SiArbiterInterpKeyMatcherParaDblClick;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpKeyMatcherBase base;             //匹配器基类，必须放在开头
    T_SiArbiterInterpKeyMatcherParaDblClick para;     //匹配器算法参数
    uint8_t dataLen;                                  //匹配器数据缓冲区个数
    T_SiArbiterInterpKeyMatcherDataDblClick *pdata;   //匹配器数据
} T_SiArbiterInterpKeyMatcherDblClick;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyMatcherDblClickInit(T_SiArbiterInterpKeyMatcherDblClick *nd, const T_SiArbiterInterpKeyMatcherParaDblClick *para, uint8_t dataLen, T_SiArbiterInterpKeyMatcherDataDblClick *pdata);

//******长按匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint16_t successTime;           //匹配成功时间
    uint32_t beginPressTime;        //开始按压时间
} T_SiArbiterInterpKeyMatcherDataLongPress;
//匹配器算法参数
typedef struct
{
    uint8_t longPressTime;     //判断按键长按时间，单位0.1s--需要用户指定
} T_SiArbiterInterpKeyMatcherParaLongPress;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpKeyMatcherBase base;             //匹配器基类，必须放在开头
    T_SiArbiterInterpKeyMatcherParaLongPress para;     //匹配器算法参数
    uint8_t dataLen;                                  //匹配器数据缓冲区个数
    T_SiArbiterInterpKeyMatcherDataLongPress *pdata;   //匹配器数据
} T_SiArbiterInterpKeyMatcherLongPress;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyMatcherLongPressInit(T_SiArbiterInterpKeyMatcherLongPress *nd, const T_SiArbiterInterpKeyMatcherParaLongPress *para, uint8_t dataLen, T_SiArbiterInterpKeyMatcherDataLongPress *pdata);

//******按压匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
} T_SiArbiterInterpKeyMatcherDataPress;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpKeyMatcherBase base;          //匹配器基类，必须放在开头
    uint8_t dataLen;                               //匹配器数据缓冲区个数
    T_SiArbiterInterpKeyMatcherDataPress *pdata;   //匹配器数据
    T_SiArbiterKeyValueChangedCallback valueChangedCallback; //值改变回调接口，keyNo按键编号，status按键状态
} T_SiArbiterInterpKeyMatcherPress;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyMatcherPressInit(T_SiArbiterInterpKeyMatcherPress *nd, uint8_t dataLen, T_SiArbiterInterpKeyMatcherDataPress *pdata, T_SiArbiterKeyValueChangedCallback valueChangedCallback);

//****************************************************************************************************************************************
//解释判决器集合匹配器

//************************************************************************************
//匹配器基类
struct T_SiArbiterInterpSetMatcherBase
{
    uint8_t (*matchSuccess)(T_SiObject *obj, struct T_SiArbiterInterpSetMatcherBase *self);   //是否匹配成功
    uint8_t (*canRelease)(T_SiObject *obj, struct T_SiArbiterInterpSetMatcherBase *self);     //是否可以释放按键，如果可以释放表明已经存在匹配结果
    uint32_t (*getSuccessTime)(T_SiObject *obj, struct T_SiArbiterInterpSetMatcherBase *self); //获取匹配器匹配成功的时间
    T_SiKeyStatus(*getResult)(T_SiObject *obj, struct T_SiArbiterInterpSetMatcherBase *self, uint32_t *rtvalue); //返回匹配结果
    void (*reset)(T_SiObject *obj, struct T_SiArbiterInterpSetMatcherBase *self);             //每次匹配完成后会调用reset方法
    void (*run)(T_SiObject *obj, struct T_SiArbiterInterpSetMatcherBase *self, uint8_t isPress, uint8_t isRelease);

    struct T_SiArbiterInterpSetMatcherBase *link;        //单向链表
};
typedef struct T_SiArbiterInterpSetMatcherBase T_SiArbiterInterpSetMatcherBase;

//匹配器
struct T_SiArbiterInterpSetMatcher
{
    uint8_t headFirst;    //为0表示最晚匹配最优，为1表示最早匹配最优
    struct T_SiArbiterInterpSetMatcherBase *header;
};

//匹配器初始化，headFirst为0表示最晚匹配最优，为1表示最早匹配最优
T_SiErrRt SiArbiterInterpSetMatcherInit(struct T_SiArbiterInterpSetMatcher *matcher, uint8_t headFirst);
//注册匹配器
T_SiErrRt SiArbiterInterpSetMatcherRegister(struct T_SiArbiterInterpSetMatcher *matcher, struct T_SiArbiterInterpSetMatcherBase *obj);

void SiArbiterInterpSetMatcherRun(T_SiObject *obj, struct T_SiArbiterInterpSetMatcher *matcher, uint8_t isPress, uint8_t isRelease);     //运行匹配器
void SiArbiterInterpSetMatcherReset(T_SiObject *obj, struct T_SiArbiterInterpSetMatcher *matcher);       //复位匹配器
uint8_t SiArbiterInterpSetMatcherCanRelease(T_SiObject *obj, struct T_SiArbiterInterpSetMatcher *matcher);   //是否可以释放按键，如果可以释放表明已经存在匹配结果
T_SiKeyStatus SiArbiterInterpSetMatcherGetResult(T_SiObject *obj, struct T_SiArbiterInterpSetMatcher *matcher, uint32_t *rtvalue); //获取匹配结果

//******单击匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
} T_SiArbiterInterpSetMatcherDataClick;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpSetMatcherBase base;          //匹配器基类，必须放在开头
    T_SiArbiterInterpSetMatcherDataClick *pdata;     //匹配器数据
} T_SiArbiterInterpSetMatcherClick;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetMatcherClickInit(T_SiArbiterInterpSetMatcherClick *nd, T_SiArbiterInterpSetMatcherDataClick *pdata);

//******双击匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint8_t matchSuccess;           //匹配成功
    uint8_t invalidKey;             //无效按键标记位
    uint16_t successTime;           //匹配成功时间
    uint32_t beginClickTime;        //开始敲击时间
    uint32_t waitDblClkMs;          //等待双击时间
} T_SiArbiterInterpSetMatcherDataDblClick;
//匹配器算法参数
typedef struct
{
    uint8_t maxPressTime;       //敲击时，手指按在pad上不释放的最大时间，若手指按压在pad上不拿开超过一段时间认为双击无效，单位0.1s，为0表示不检测--需要用户指定
    uint8_t dblClickIntval;     //判断按键双击时间间隔，单位0.1s--需要用户指定
    uint8_t multiClickIgnore;   //设置1表示敲击次数超过2时认为双击无效，否则认为双击有效
} T_SiArbiterInterpSetMatcherParaDblClick;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpSetMatcherBase base;             //匹配器基类，必须放在开头
    T_SiArbiterInterpSetMatcherParaDblClick para;     //匹配器算法参数
    T_SiArbiterInterpSetMatcherDataDblClick *pdata;   //匹配器数据
} T_SiArbiterInterpSetMatcherDblClick;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetMatcherDblClickInit(T_SiArbiterInterpSetMatcherDblClick *nd, const T_SiArbiterInterpSetMatcherParaDblClick *para, T_SiArbiterInterpSetMatcherDataDblClick *pdata);

//******长按匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
    uint16_t successTime;           //匹配成功时间
    uint32_t beginPressTime;        //开始按压时间
} T_SiArbiterInterpSetMatcherDataLongPress;
//匹配器算法参数
typedef struct
{
    uint8_t longPressTime;     //判断按键长按时间，单位0.1s--需要用户指定
} T_SiArbiterInterpSetMatcherParaLongPress;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpSetMatcherBase base;              //匹配器基类，必须放在开头
    T_SiArbiterInterpSetMatcherParaLongPress para;     //匹配器算法参数
    T_SiArbiterInterpSetMatcherDataLongPress *pdata;   //匹配器数据
} T_SiArbiterInterpSetMatcherLongPress;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetMatcherLongPressInit(T_SiArbiterInterpSetMatcherLongPress *nd, const T_SiArbiterInterpSetMatcherParaLongPress *para, T_SiArbiterInterpSetMatcherDataLongPress *pdata);

//******滑条匹配器************************************************************************************

//匹配器数据
typedef struct
{
    T_SiData curData;               //当前数据
    T_SiData curCoordinate;         //当前坐标
    int32_t chperSum;               //累加进位
} T_SiArbiterInterpSetMatcherDataSlider;
typedef struct
{
    uint8_t status;                     //滑条状态
    int8_t curStep;                     //当前步位置
    int8_t lastPressStep;               //上次按压步位置
    int8_t beginPressStep;              //首次按下时步位置
    int8_t releaseStep;                 //松开时步位置
    uint8_t dataLen;                    //判决器数据缓冲区个数
    uint8_t eliminateCnt;               //消抖计数器
    T_SiData curPosition;               //当前坐标位置
    T_SiData beginPressPosition;        //首次按压坐标位置
    int32_t slideDistance;              //滑动距离
    uint8_t invalidSlider;              //无效滑动标记位
    uint16_t successTime;               //匹配成功时间
    uint32_t beginPressTime;            //开始按压时间
} T_SiArbiterInterpSetMatcherGbDataSlider;
//匹配器算法参数
typedef struct
{
    uint16_t resolution;        //分辨率，512 - 1024，建议512 --需要用户指定
    uint8_t resolutionStep;     //几级分辨率，MIN_SLIDER_STEP - MAX_SLIDER_STEP --需要用户指定
    uint8_t groupSeed;          //5-15%，滤波系数，建议10 --需要用户指定
    uint8_t holdStepMargin;     //10-20，步偏移消抖 --需要用户指定
    uint8_t minSlideDistance;   //最小滑动距离，小于本值认为滑动无效 --需要用户指定
    uint8_t maxPressTime;       //滑动时，手指按在pad上不释放的最大时间，若手指按压在pad上不拿开超过一段时间认为滑动无效，单位0.1s，为0表示不检测--需要用户指定
    uint8_t releaseEliminate;   //释放消抖次数--需要用户指定
    T_SiData pressThreshold;    //按压阈值，消抖用 --需要用户指定
} T_SiArbiterInterpSetMatcherParaSlider;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpSetMatcherBase base;           //匹配器基类，必须放在开头
    T_SiArbiterInterpSetMatcherParaSlider para;     //匹配器算法参数
    uint8_t dataLen;                                //匹配器数据缓冲区个数
    T_SiArbiterInterpSetMatcherDataSlider *pdata;   //匹配器数据
    T_SiArbiterInterpSetMatcherGbDataSlider gbdata; //匹配器全局数据
    T_SiArbiterSliderValueChangedCallback valueChangedCallback;     //滑条值改变回调函数
} T_SiArbiterInterpSetMatcherSlider;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetMatcherSliderInit(T_SiArbiterInterpSetMatcherSlider *nd, const T_SiArbiterInterpSetMatcherParaSlider *para, uint8_t dataLen,
        T_SiArbiterInterpSetMatcherDataSlider *pdata, T_SiArbiterSliderValueChangedCallback callback);

//******按压匹配器************************************************************************************

//匹配器数据
typedef struct
{
    uint8_t status;                 //按键状态
} T_SiArbiterInterpSetMatcherDataPress;
//匹配器描述符
typedef struct
{
    T_SiArbiterInterpSetMatcherBase base;          //匹配器基类，必须放在开头
    T_SiArbiterInterpSetMatcherDataPress *pdata;   //匹配器数据
    T_SiArbiterSetValueChangedCallback valueChangedCallback; //值改变回调接口
} T_SiArbiterInterpSetMatcherPress;

//匹配器描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetMatcherPressInit(T_SiArbiterInterpSetMatcherPress *nd, T_SiArbiterInterpSetMatcherDataPress *pdata, T_SiArbiterSetValueChangedCallback valueChangedCallback);

#ifdef __cplusplus
}
#endif

#endif
