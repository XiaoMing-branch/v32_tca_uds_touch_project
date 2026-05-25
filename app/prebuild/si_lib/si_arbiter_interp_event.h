/**
*****************************************************************************
* @brief  si arbiter interp event header
* @file   si_arbiter_interp_event.h
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

#ifndef __SI_ARBITER_INTERP_EVENT_H__
#define __SI_ARBITER_INTERP_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

//表达式数值类型
typedef enum
{
    SI_EXPRESSION_VTYPE_ERROR = 0,       //无效类型
    SI_EXPRESSION_VTYPE_VALUE,           //操作数类型
    SI_EXPRESSION_VTYPE_OPERATOR,        //操作符类型
} T_SiExpressionVType;

extern T_SiExpressionStack expressionOptr;      //操作符栈
extern T_SiExpressionStack expressionOpnd;      //操作数栈

//传入的op1和op2必须是合法的运算符，本函数不做运算符合法性检查
char ExpressionPrecede(uint8_t op1, uint8_t op2);   //比较op1和op2的算符优先级，返回：<>=，无效比较返回0

//解析表达式字符串，返回剩余字符串指针，pv为返回数据
const char *ExpressionParse(const char *conditionString, uint8_t *pv, T_SiExpressionVType *vtype);

//条件解释字符串语法分析，失败返回0
uint8_t SiArbiterInterpConditionSyntaxCheck(uint8_t conditionLen, const char *conditionString);

//****************************************************************************************************************************************
//解释判决器按键事件

//************************************************************************************
//按键事件处理器节点
struct T_SiArbiterInterpKeyEventNode
{
    const char *condition;                                 //条件
    struct T_SiArbiterInterpKeyActionBase *actionHeader;   //动作，支持执行多个动作

    struct T_SiArbiterInterpKeyEventNode *link;    //单向链表
};
typedef struct T_SiArbiterInterpKeyEventNode T_SiArbiterInterpKeyEventNode;

//按键事件处理器
struct T_SiArbiterInterpKeyEvent
{
    struct T_SiArbiterInterpKeyEventNode *header;
};
typedef struct T_SiArbiterInterpKeyEvent T_SiArbiterInterpKeyEvent;

//事件节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyEventNodeInit(T_SiArbiterInterpKeyEventNode *nd, const char *conditionString);
//事件描述符添加动作
void SiArbiterInterpKeyEventNodeAddAction(T_SiArbiterInterpKeyEventNode *nd, T_SiArbiterInterpKeyActionBase *action);
//事件描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpKeyEventInit(T_SiArbiterInterpKeyEvent *event);
//事件添加节点
void SiArbiterInterpKeyEventAddNode(T_SiArbiterInterpKeyEvent *event, T_SiArbiterInterpKeyEventNode *nd);

void SiArbiterInterpKeyEventRun(T_SiObject *obj, struct T_SiArbiterInterpKeyEvent *event, struct T_SiArbiterInterpKeyConditionBase **conditionArray, uint8_t keyNo);   //运行事件处理器
uint8_t SiArbiterInterpKeyEventSyntaxCheck(struct T_SiArbiterInterpKeyEvent *event, uint8_t conditionLen);   //条件解释字符串语法分析，失败返回0

//****************************************************************************************************************************************
//解释判决器集合事件

//************************************************************************************
//集合事件处理器基类
struct T_SiArbiterInterpSetEventNode
{
    const char *condition;                                       //条件，支持"(!&|)"逻辑运算符，优先级()>!>&>|
    struct T_SiArbiterInterpSetActionBase *actionHeader;   //动作，支持执行多个动作

    struct T_SiArbiterInterpSetEventNode *link;    //单向链表
};
typedef struct T_SiArbiterInterpSetEventNode T_SiArbiterInterpSetEventNode;

//集合事件处理器
struct T_SiArbiterInterpSetEvent
{
    struct T_SiArbiterInterpSetEventNode *header;
};
typedef struct T_SiArbiterInterpSetEvent T_SiArbiterInterpSetEvent;

//事件节点描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetEventNodeInit(T_SiArbiterInterpSetEventNode *nd, const char *conditionString);
//事件描述符添加动作
void SiArbiterInterpSetEventNodeAddAction(T_SiArbiterInterpSetEventNode *nd, T_SiArbiterInterpSetActionBase *action);
//事件描述符初始化，TOUCH_RT_OK表示成功
T_SiErrRt SiArbiterInterpSetEventInit(T_SiArbiterInterpSetEvent *event);
//事件添加节点
void SiArbiterInterpSetEventAddNode(T_SiArbiterInterpSetEvent *event, T_SiArbiterInterpSetEventNode *nd);

void SiArbiterInterpSetEventRun(T_SiObject *obj, struct T_SiArbiterInterpSetEvent *event, struct T_SiArbiterInterpSetConditionBase **conditionArray);   //运行事件处理器
uint8_t SiArbiterInterpSetEventSyntaxCheck(struct T_SiArbiterInterpSetEvent *event, uint8_t conditionLen);   //条件解释字符串语法分析，失败返回0

#ifdef __cplusplus
}
#endif

#endif
