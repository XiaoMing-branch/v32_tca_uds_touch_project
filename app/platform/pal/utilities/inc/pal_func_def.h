/**
 *****************************************************************************
 * @brief   pal func def header file.
 * @file    pal_func_def.h
 * @author  AE/FAE team
 * @date    2024.01.01
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */

#ifndef __PAL_FUNC_DEF_H__
#define __PAL_FUNC_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CRC计算模式选择宏
 */
#define PAL_CRC_TYPE_HARDWARE                     (0) /**< 硬件CRC计算 */
#define PAL_CRC_TYPE_TABLE                        (1) /**< 软件查表CRC计算 */
#define PAL_CRC_TYPE_SOFTWARE                     (2) /**< 软件逐位CRC计算 */

#include "tcae10_ll_def.h"
/**
 * @brief 功能配置宏
 */
#define CFG_SUPPORT_USE_CRC_TABLE                 (PAL_CRC_TYPE_SOFTWARE) /**< CRC模式选择 */
#define CFG_SUPPROT_LINSNPD_EXT_RES               (1) /**< LIN SNP扩展电阻支持 */
#define CFG_SUPPORT_SERIAL_NUM_CHECK              (0) /**< LED灯珠串联数自动检测 */

//#ifdef __has_include
//// 使用编译器提供的宏判断头文件是否存在
//#if __has_include("app.h")
//#include "app.h"
//#else
//#define CFG_SUPPORT_LIN_SNPD                      (0)
//#ifndef CFG_SUPPORT_LOG
//#define CFG_SUPPORT_LOG                           (0)
//#endif
//#endif
//#endif
#include "app.h"

/**
  * @brief  LED通道编号枚举
  */
typedef enum
{
    LED_CHANNLE_0,    /**< LED通道0 */
    LED_CHANNLE_MAX   /**< 通道数量 */
} led_channel_e;

/**
  * @brief  RGB颜色通道枚举
  */
typedef enum
{
    RGB_RED = 0,     /**< 红色通道 */
    RGB_GREEN,        /**< 绿色通道 */
    RGB_BLUE,         /**< 蓝色通道 */
    RGB_TYPE_MAX,     /**< RGB通道数量 */
} rgb_type_e;


#ifdef __cplusplus
}
#endif
#endif  /* __PAL_FUNC_DEF_H__ */
