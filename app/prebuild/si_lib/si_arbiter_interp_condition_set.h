/**
*****************************************************************************
* @brief  si arbiter interp condition set header
* @file   si_arbiter_interp_condition_set.h
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

#ifndef __SI_ARBITER_INTERP_CONDITION_SET_H__
#define __SI_ARBITER_INTERP_CONDITION_SET_H__

#ifdef __cplusplus
extern "C" {
#endif

//************************************************************************************
//集合条件
typedef enum
{
    SI_ARBITER_INTERP_SET_CONDITION_ONCE = 0,       //条件仅被执行一次
    SI_ARBITER_INTERP_SET_CONDITION_REALTIME        //条件实时执行，该类型的条件需要是无状态的
} T_SiArbiterInterpSetConditionType;

typedef enum
{
    SI_ARBITER_INTERP_SET_CONDITION_STATUS_INIT = 0,        //初始状态
    SI_ARBITER_INTERP_SET_CONDITION_STATUS_PRESS,           //按压状态
    SI_ARBITER_INTERP_SET_CONDITION_STATUS_RELEASE          //释放状态
} T_SiArbiterInterpSetConditionStatusType;

//集合条件基类
struct T_SiArbiterInterpSetConditionBase
{
    T_SiArbiterInterpSetConditionType type;
    uint8_t (*isok)(T_SiObject *obj, struct T_SiArbiterInterpSetConditionBase *self);       //条件是否满足
    void (*statusChanged)(T_SiObject *obj, struct T_SiArbiterInterpSetConditionBase *self, T_SiArbiterInterpSetConditionStatusType status); //状态改变
    void (*reset)(T_SiObject *obj, struct T_SiArbiterInterpSetConditionBase *self);         //每次释放后会调用reset方法
    void (*run)(T_SiObject *obj, struct T_SiArbiterInterpSetConditionBase *self);
};
typedef struct T_SiArbiterInterpSetConditionBase T_SiArbiterInterpSetConditionBase;

void SiArbiterInterpSetConditionStatusChanged(T_SiObject *obj, struct T_SiArbiterInterpSetConditionBase **conditionArray, uint8_t conditionLen, T_SiArbiterInterpSetConditionStatusType status);     //运行状态改变
void SiArbiterInterpSetConditionReset(T_SiObject *obj, struct T_SiArbiterInterpSetConditionBase **conditionArray, uint8_t conditionLen);     //复位

uint8_t SiArbiterInterpSetConditionRun(T_SiObject *obj, struct T_SiArbiterInterpSetConditionBase **conditionArray, const char *conditionString);   //运行条件，返回条件是否满足

//**********************************************
//测试条件，测试用，始终返回参数v的值
//条件算法参数
typedef struct
{
    uint8_t v;
} T_SiArbiterInterpSetConditionParaTest;
//条件描述符
typedef struct
{
    T_SiArbiterInterpSetConditionBase base;            //条件基类，必须放在开头
    T_SiArbiterInterpSetConditionParaTest para;        //条件参数
} T_SiArbiterInterpSetConditionTest;

//条件描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetConditionTestInit(T_SiArbiterInterpSetConditionTest *nd, const T_SiArbiterInterpSetConditionParaTest *para);

//**********************************************
//斜率，判断在SI_KEY_RAW_PRESS到SI_KEY_RAW_RELEASE之间，斜率值是否达到过斜率阈值
//数据
typedef struct
{
    uint8_t status;                                               //检测器状态
    uint8_t fillLen;                                              //窗口中已经填充数据长度
    uint8_t skipCnt;                                              //跳帧计数器
    T_SiNoiseData gradientValue;                                  //当前斜率值
    T_SiData window[SI_NOISE_DATA_GRADIENT_WINDOWSIZE];           //窗口缓冲区
} T_SiArbiterInterpSetConditionDataGradient;
//条件算法参数
typedef struct
{
    uint8_t chanelNo;                           //通道编号
    uint8_t sampIntval;                         //采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定
    uint8_t gradientIntval;                     //斜率间隔，单位为帧个数，范围[0，SI_NOISE_DATA_GRADIENT_WINDOWSIZE-1)--需要用户指定
    T_SiNoiseData upperThreshold;               //斜率上边界,[lowerThreshold,upperThreshold认为有效)
    T_SiNoiseData lowerThreshold;               //斜率下边界,[lowerThreshold,upperThreshold认为有效)
    int gradientSign;                           //斜率符号，取-1，0，1，0表示取绝对值，否则与斜率值相乘
} T_SiArbiterInterpSetConditionParaGradient;
//条件描述符
typedef struct
{
    T_SiArbiterInterpSetConditionBase base;            //条件基类，必须放在开头
    T_SiArbiterInterpSetConditionParaGradient para;    //条件参数
    T_SiArbiterInterpSetConditionDataGradient data;    //数据
} T_SiArbiterInterpSetConditionGradient;

//条件描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetConditionGradientInit(T_SiArbiterInterpSetConditionGradient *nd, const T_SiArbiterInterpSetConditionParaGradient *para);

#ifdef __cplusplus
}
#endif

#endif
