/**
*****************************************************************************
* @brief  si arbiter interp condition key header
* @file   si_arbiter_interp_condition_key.h
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

#ifndef __SI_ARBITER_INTERP_CONDITION_KEY_H__
#define __SI_ARBITER_INTERP_CONDITION_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

//************************************************************************************
//按键条件
typedef enum
{
    SI_ARBITER_INTERP_KEY_CONDITION_ONCE = 0,       //条件仅被执行一次
    SI_ARBITER_INTERP_KEY_CONDITION_REALTIME        //条件实时执行，该类型的条件需要是无状态的
} T_SiArbiterInterpKeyConditionType;

typedef enum
{
    SI_ARBITER_INTERP_KEY_CONDITION_STATUS_INIT = 0,        //初始状态
    SI_ARBITER_INTERP_KEY_CONDITION_STATUS_PRESS,           //按压状态
    SI_ARBITER_INTERP_KEY_CONDITION_STATUS_RELEASE          //释放状态
} T_SiArbiterInterpKeyConditionStatusType;

//按键条件基类
struct T_SiArbiterInterpKeyConditionBase
{
    T_SiArbiterInterpKeyConditionType type;
    uint8_t (*isok)(T_SiObject *obj, struct T_SiArbiterInterpKeyConditionBase *self, uint8_t keyNo);       //条件是否满足
    void (*statusChanged)(T_SiObject *obj, struct T_SiArbiterInterpKeyConditionBase *self, uint8_t keyNo, T_SiArbiterInterpKeyConditionStatusType status); //状态改变
    void (*reset)(T_SiObject *obj, struct T_SiArbiterInterpKeyConditionBase *self, uint8_t keyNo);         //每次释放后会调用reset方法
    void (*run)(T_SiObject *obj, struct T_SiArbiterInterpKeyConditionBase *self, uint8_t keyNo);
};
typedef struct T_SiArbiterInterpKeyConditionBase T_SiArbiterInterpKeyConditionBase;

void SiArbiterInterpKeyConditionStatusChanged(T_SiObject *obj, struct T_SiArbiterInterpKeyConditionBase **conditionArray, uint8_t conditionLen, uint8_t keyNo, T_SiArbiterInterpKeyConditionStatusType status);     //运行状态改变
void SiArbiterInterpKeyConditionReset(T_SiObject *obj, struct T_SiArbiterInterpKeyConditionBase **conditionArray, uint8_t conditionLen, uint8_t keyNo);     //复位

uint8_t SiArbiterInterpKeyConditionRun(T_SiObject *obj, struct T_SiArbiterInterpKeyConditionBase **conditionArray, const char *conditionString, uint8_t keyNo);   //运行条件，返回条件是否满足

//**********************************************
//测试条件，测试用，始终返回参数v的值
//条件算法参数
typedef struct
{
    uint8_t v;
} T_SiArbiterInterpKeyConditionParaTest;
//条件描述符
typedef struct
{
    T_SiArbiterInterpKeyConditionBase base;            //条件基类，必须放在开头
    T_SiArbiterInterpKeyConditionParaTest para;        //条件参数
} T_SiArbiterInterpKeyConditionTest;

//条件描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyConditionTestInit(T_SiArbiterInterpKeyConditionTest *nd, const T_SiArbiterInterpKeyConditionParaTest *para);

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
} T_SiArbiterInterpKeyConditionDataGradient;
//条件算法参数
typedef struct
{
    uint8_t sampIntval;                                //采样间隔，单位为帧个数，表示隔几帧采一条数据--需要用户指定
    uint8_t gradientIntval;                            //斜率间隔，单位为帧个数，范围[0，SI_NOISE_DATA_GRADIENT_WINDOWSIZE-1)--需要用户指定
    T_SiNoiseData upperThreshold[SI_PNODE_CH_MAXNUM];  //斜率上边界,[lowerThreshold,upperThreshold认为有效)
    T_SiNoiseData lowerThreshold[SI_PNODE_CH_MAXNUM];  //斜率下边界,[lowerThreshold,upperThreshold认为有效)
    int gradientSign;                                  //斜率符号，取-1，0，1，0表示取绝对值，否则与斜率值相乘
} T_SiArbiterInterpKeyConditionParaGradient;
//条件描述符
typedef struct
{
    T_SiArbiterInterpKeyConditionBase base;            //条件基类，必须放在开头
    T_SiArbiterInterpKeyConditionParaGradient para;    //条件参数
    uint8_t dataLen;                                   //数据缓冲区个数
    T_SiArbiterInterpKeyConditionDataGradient *pdata;  //数据
} T_SiArbiterInterpKeyConditionGradient;

//条件描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyConditionGradientInit(T_SiArbiterInterpKeyConditionGradient *nd, const T_SiArbiterInterpKeyConditionParaGradient *para, uint8_t dataLen, T_SiArbiterInterpKeyConditionDataGradient *pdata);

#ifdef __cplusplus
}
#endif

#endif
