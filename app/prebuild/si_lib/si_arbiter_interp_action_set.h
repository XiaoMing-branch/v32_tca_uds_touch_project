/**
*****************************************************************************
* @brief  si arbiter interp action set header
* @file   si_arbiter_interp_action_set.h
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

#ifndef __SI_ARBITER_INTERP_ACTION_SET_H__
#define __SI_ARBITER_INTERP_ACTION_SET_H__

#ifdef __cplusplus
extern "C" {
#endif

//************************************************************************************
//集合动作基类
struct T_SiArbiterInterpSetActionBase
{
    void (*run)(T_SiObject *obj, struct T_SiArbiterInterpSetActionBase *self);

    struct T_SiArbiterInterpSetActionBase *link;        //单向链表
};
typedef struct T_SiArbiterInterpSetActionBase T_SiArbiterInterpSetActionBase;

void SiArbiterInterpSetActionRun(T_SiObject *obj, struct T_SiArbiterInterpSetActionBase *header);  //执行动作

#ifdef __cplusplus
}
#endif

#endif
