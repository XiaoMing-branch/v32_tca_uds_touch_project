/**
*****************************************************************************
* @brief  参数调节器
* @file   si_para_adjuster.h
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

#ifndef __FFF_SI_PARA_ADJUSTER_H__
#define __FFF_SI_PARA_ADJUSTER_H__

#include "fff_si_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup paraAdjuster 参数调节器
 * 参数调节器采用组合设计模式实现，会在不同条件下配置不同的算法参数
 * @{
 */

/**
* @brief        参数调节器基类
* @details      定义调节器内部实现方法以及存储结构
* @attention    不要直接操作本结构体内容
*/
typedef struct T_SiParaAdjusterBase
{
    //! @cond
    struct T_SiParaAdjusterBase *child;             /*!< 孩子 */
    struct T_SiParaAdjusterBase *sibling;           /*!< 兄弟 */

    T_SiErrRt(*run)(struct T_SiParaAdjusterBase *self, int forceAdjust); /*!< 运行参数调节器forceAdjust，forceAdjust为1表示强制调节参数 */
    //! @endcond       //doxygen中隐藏
} T_SiParaAdjusterBase;

/**
* @brief        参数调节器增加一个孩子
* @details      孩子是有序的，先增加在前，后增加在后
* @param[in]   self 参数调节器
* @param[in]   child 孩子
* @retval      SI_RT_OK 添加成功
* @retval      other 添加失败
*/
T_SiErrRt SiParaAdjusterAddChild(T_SiParaAdjusterBase *self, T_SiParaAdjusterBase *child);

/**
* @brief        参数调节器增加一个兄弟
* @details      兄弟是有序的，先增加在前，后增加在后
* @param[in]   self 参数调节器
* @param[in]   sibling 兄弟
* @retval      SI_RT_OK 添加成功
* @retval      other 添加失败
*/
T_SiErrRt SiParaAdjusterAddBrother(T_SiParaAdjusterBase *self, T_SiParaAdjusterBase *sibling);

/**
* @brief        强制执行一次参数调节器
* @param[in]   self 参数调节器
* @retval      SI_RT_OK 成功
* @retval      other 失败
*/
T_SiErrRt SiParaAdjusterForceRun(T_SiParaAdjusterBase *self);

//************************************************************************************
//参数调节器组合

/**
* @brief        参数调节器组合回调接口
* @retval      编号 选择对应的孩子节点编号
*/
typedef uint32_t (*T_SiParaAdjusterCompositeSelectCallback)(void);           //回调接口，选择对应的参数编号

/**
* @brief        参数调节器组合
* @details      定义调节器组合内部实现方法
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiParaAdjusterBase base;          /*!< 参数调节器基类，必须放在开头 */

    int curPara;                        /*!< 当前参数，<0表示还未配置参数，其它值表示child的编号，如0表示第0个孩子，1表示第1个孩子 */
    T_SiParaAdjusterCompositeSelectCallback selectCallback;        /*!< 回调接口，返回对应的参数编号 */
    //! @endcond       //doxygen中隐藏
} T_SiParaAdjusterComposite;

/**
* @brief        参数调节器组合节点初始化
* @param[in]   nd 参数调节器组合节点
* @param[in]   selectCallback 参数调节器组合回调接口
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiParaAdjusterCompositeNodeInit(T_SiParaAdjusterComposite *nd, T_SiParaAdjusterCompositeSelectCallback selectCallback);

//************************************************************************************
//参数调节器叶节点

/**
* @brief        参数调节器叶节点
* @details      定义调节器叶节点内部实现方法
* @attention    不要直接操作本结构体内容
*/
typedef struct
{
    //! @cond
    T_SiParaAdjusterBase base;          /*!< 参数调节器基类，必须放在开头 */

    const void *srcAddr;                /*!< 参数源地址 */
    void *dstAddr;                      /*!< 参数目标地址 */
    size_t len;                         /*!< 参数长度 */
    //! @endcond       //doxygen中隐藏
} T_SiParaAdjusterLeaf;

/**
* @brief       参数调节器叶子节点初始化
* @param[in]   nd 参数调节器叶子节点
* @param[in]   srcAddr 参数源地址
* @param[in]   dstAddr 参数目标地址
* @param[in]   len 参数长度
* @retval      SI_RT_OK 初始化成功
* @retval      other 初始化失败
*/
T_SiErrRt SiParaAdjusterLeafNodeInit(T_SiParaAdjusterLeaf *nd, const void *srcAddr, void *dstAddr, size_t len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
