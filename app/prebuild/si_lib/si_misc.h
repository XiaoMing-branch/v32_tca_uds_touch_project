/**
*****************************************************************************
* @brief  杂项
* @file   si_misc.h
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

#ifndef __SI_MISC_H__
#define __SI_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

//******************************************************************************************
//原始数据预处理器

//************************************防水通道预处理器**************************************

/*
    防水算法有两种方式：
        1.把防水通道的初始rawdata记下来，最终的rawdata=检测通道的rawdata-(防水通道的当前rawdata+compensation-防水通道的初始rawdata)*tenRatio
        2.最终的diffvalue=检测值的diffvalue-防水通道的diffvalue*系数，防水通道的baseline更新要慢一点，小一点，否则可能导致防水通道失效，若出现长时间积水，本算法不适用
        方法1编程指导：SiRawDataHandleGuardFunc + 其它常规组件（不可使用带防水的判决器）
        方法2编程指导：其它常规组件 + T_SiArbiterKeyGuard
*/

/**
* @brief        防水通道预处理器算法参数
* @details      防水原理：把防水通道的初始rawdata记下来，最终的rawdata=检测通道的rawdata-(防水通道的当前rawdata+compensation-防水通道的初始rawdata)*tenRatio
*/
typedef struct
{
    uint8_t tenRatio;                   /*!< 防水系数，单位十分之几，比如设置成3，表示十分之3，即0.3 */
    T_SiData compensation;              /*!< 防水补偿 */
} T_SiRawDataHandleGuardPara;

/**
* @brief       防水通道预处理器执行函数
* @param[in]   obj 信号集对象
* @param[in]   para 防水通道预处理器算法参数
* @retval      无
*/
void SiRawDataHandleGuardFunc(struct T_SiObject *obj, const void *para);

#ifdef __cplusplus
}
#endif

#endif
