/**
 *****************************************************************************
 * @brief   pal func def header file.
 *
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

#define PAL_CRC_TYPE_HARDWARE                     (0)
#define PAL_CRC_TYPE_TABLE                        (1)
#define PAL_CRC_TYPE_SOFTWARE                     (2)

#if defined (__TCPL01X__)
#include "tcpl01x_ll_def.h"
#define CFG_SUPPORT_USE_CRC_TABLE                 (PAL_CRC_TYPE_SOFTWARE)
#define CFG_SUPPROT_LINSNPD_EXT_RES               (0)
#elif defined __TCPL03X__
#include "tcpl03x_ll_def.h"
#define CFG_SUPPORT_USE_CRC_TABLE                 (PAL_CRC_TYPE_SOFTWARE)
#define CFG_SUPPROT_LINSNPD_EXT_RES               (0)
#else
// #warning no define tinychip, use tcpl01x
#define __TCPL01X__
#include "tcpl01x_ll_def.h"
#define CFG_SUPPORT_USE_CRC_TABLE                 (PAL_CRC_TYPE_SOFTWARE)
#define CFG_SUPPROT_LINSNPD_EXT_RES               (0)
#endif

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 6000000)
#ifndef  __LL_DRIVER_EXAMPLE__
#include "app.h"
#endif  /* __LL_DRIVER_EXAMPLE__ */
#else
#ifdef __has_include
// 使用编译器提供的宏判断头文件是否存在
#if __has_include("app.h")
#include "app.h"
#else
#define CFG_SUPPORT_LIN_SNPD                      (0)
#ifndef CFG_SUPPORT_LOG
#define CFG_SUPPORT_LOG                           (0)
#endif  /* CFG_SUPPORT_LOG */
#endif  /* __has_include app.h */
#endif  /* __has_include */
#endif  /* __ARMCC_VERSION */

/* resume all interrupt enabled */
#define interrupt_enable()    __enable_irq()
/*  mask all interrupt but NMI and HardFault */
#define interrupt_disable()   __disable_irq()


#ifndef CFG_SUPPORT_LED_NUM
#define CFG_SUPPORT_LED_NUM                       (1)
#endif /* CFG_SUPPORT_LED_NUM */
#define LED_CHANNEL_MAX                           (CFG_SUPPORT_LED_NUM)
#define LED_CHANNEL_0                             (0)
#define LED_CHANNEL_1                             (1)
#define LED_CHANNEL_2                             (2)

/**
 * @brief  LED通道号类型定义
 * @note   使用uint8_t表示LED通道编号
 */
typedef uint8_t led_channel_e;

/**
 * @brief  RGB颜色类型枚举
 * @note   定义红/绿/蓝三色索引
 */
typedef enum
{
    LED_R = 0,
    LED_G,
    LED_B,
    LED_TYPE_MAX,
} led_type_e;

#ifdef __cplusplus
}
#endif
#endif /* __PAL_FUNC_DEF_H__ */
