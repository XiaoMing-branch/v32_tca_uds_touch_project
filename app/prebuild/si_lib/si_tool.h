/**
*****************************************************************************
* @brief  算法工具
* @file   si_tool.h
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

#ifndef __SI_TOOL_H__
#define __SI_TOOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup tool 算法工具
 * @{
 */

/**
* @brief       计算diff值
* @param[in]   direct 信号增长方向，当direct==SI_GROW_DIRECT_DOWN时，表示向下增长，差值为baselinev-filterv，否则表示向上增长，差值为filterv-baselinev
* @param[in]   filterv 滤波值
* @param[in]   baselinev 基线值
* @retval      diff值
*/
#define DiffSiValue(direct,filterv,baselinev) ((direct)==SI_GROW_DIRECT_DOWN ? ((baselinev)-(filterv)) : (filterv)-(baselinev))

/**
* @brief       根据obj的growDirect自动判断是否用growDirectMt计算diff值
* @param[in]   obj 信号集对象
* @param[in]   keyNo 信号编号
* @param[in]   filterv 滤波值
* @param[in]   baselinev 基线值
* @retval      diff值
*/
T_SiData SmartDiffSiValue(const struct T_SiObject *obj, uint8_t keyNo, T_SiData filterv, T_SiData baselinev);

/**
* @brief       冒泡排序
* @param[in]   array 数组缓冲区
* @param[in]   sz 数组长度
* @retval      无
*/
void SiBubbleSort(T_SiData array[], int sz);

/**
* @brief       int取绝对值
* @param[in]   v 待计算数值
* @retval      v的绝对值
*/
int SiIAbs(int v);

/**
* @brief       short取绝对值
* @param[in]   v 待计算数值
* @retval      v的绝对值
*/
int16_t SiSAbs(int16_t v);

/**
* @brief       取最大值
* @param[in]   a 数a
* @param[in]   b 数b
* @retval      ab中的最大值
*/
#define SiIMax(a,b)     ((a)>=(b) ? (a) : (b))

/**
* @brief       取最小值
* @param[in]   a 数a
* @param[in]   b 数b
* @retval      ab中的最小值
*/
#define SiIMin(a,b)     ((a)<=(b) ? (a) : (b))

extern const uint8_t SiFastDivTable[257];   //快速除法表
static __INLINE int SiRightShift(int v, int offset)
{
    return v >> offset;
}
/**
* @brief       快速求除数
* @param[in]   um 分子
* @param[in]   den 分母
* @retval      um/den
*/
#define SiIFastDiv(um,den)  (((den)<=256 && (den)>0 && SiFastDivTable[(den)]) ? SiRightShift((um),SiFastDivTable[(den)]): ((um)/(den)))

/**
* @brief       查表法求sin值
* @param[in]   d 范围0-180，精度5°
* @retval      sin值
*/
float SiSin(uint8_t d);

/**
* @brief       查表法求cos值
* @param[in]   d 范围0-180，精度5°
* @retval      cos值
*/
float SiCos(uint8_t d);

void SiInitFastNoise(T_SiFastNoise *pdata);
void SiCalcFastNoise(T_SiFastNoise *pdata, float x);
float SiGetFastNoise(T_SiFastNoise *pdata);

#define CRAMMER_BUF_LEN                         3               //解三元一次方程组缓冲区大小
/**
* @brief        Crammer法则解三元一次方程组
* @details      a[0][0] * x + a[0][1] * y + a[0][2]*z = b[0]   \n
*               a[1][0] * x + a[1][1] * y + a[1][2]*z = b[1]   \n
*               a[2][0] * x + a[2][1] * y + a[2][2]*z = b[2]   \n
* @param[in]   a 方程系数
* @param[in]   b 方程结果
* @param[out]   x 解x
* @param[out]   y 解y
* @param[out]   z 解z
* @retval      0 无解
* @retval      其它 有解
*/
int SiCrammerParseXYZ(float a[3][CRAMMER_BUF_LEN], float b[3], float *x, float *y, float *z);

/**
* @brief       返回错误码描述字符串
* @param[in]   rt 错误码
* @retval      描述字符串
*/
const char *SiErrRtDesp(T_SiErrRt rt);

/**
* @brief       获取所有算法对象的para成员，模仿模板编程
* @param[in]   nd 算法对象
* @retval      para成员
*/
#define SiGetPara(nd)       ((nd).para)

#define SI_EXPRESSION_STACK_LEN     32  /*!< 表达式栈长度 */

/**
* @brief        表达式栈
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    uint8_t pos;                           /*!< 栈顶指针 */
    uint8_t buf[SI_EXPRESSION_STACK_LEN];  /*!< 缓冲区 */
} T_SiExpressionStack;

/**
* @brief       表达式栈初始化
* @param[in]   stack 表达式栈
* @retval      无
*/
void ExpressionStackInit(T_SiExpressionStack *stack);    //栈初始化

/**
* @brief       入栈
* @param[in]   stack 表达式栈
* @param[in]   v 入栈值
* @retval      0 失败
* @retval      1 成功
*/
uint8_t ExpressionStackPush(T_SiExpressionStack *stack, uint8_t v);  //入栈，成功返回1

/**
* @brief       出栈
* @param[in]   stack 表达式栈
* @param[out]   pv 出栈值
* @retval      0 失败
* @retval      1 成功
*/
uint8_t ExpressionStackPop(T_SiExpressionStack *stack, uint8_t *pv); //出栈，成功反回1

/**
* @brief       获取栈顶数据
* @param[in]   stack 表达式栈
* @param[out]   pv 栈顶值
* @retval      0 失败
* @retval      1 成功
*/
uint8_t ExpressionStackGetTop(T_SiExpressionStack *stack, uint8_t *pv); //获取头部数据，成功返回1

/** @} */

#ifdef __cplusplus
}
#endif

#endif
