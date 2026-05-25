/**
*****************************************************************************
* @brief  si arbiter interp action key header
* @file   si_arbiter_interp_action_key.h
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

#ifndef __SI_ARBITER_INTERP_ACTION_KEY_H__
#define __SI_ARBITER_INTERP_ACTION_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

//************************************************************************************
//按键动作基类
struct T_SiArbiterInterpKeyActionBase
{
    void (*run)(T_SiObject *obj, struct T_SiArbiterInterpKeyActionBase *self, uint8_t keyNo);

    struct T_SiArbiterInterpKeyActionBase *link;        //单向链表
};
typedef struct T_SiArbiterInterpKeyActionBase T_SiArbiterInterpKeyActionBase;

void SiArbiterInterpKeyActionRun(T_SiObject *obj, struct T_SiArbiterInterpKeyActionBase *header, uint8_t keyNo);  //执行动作

#ifdef __cplusplus
}
#endif

#endif
