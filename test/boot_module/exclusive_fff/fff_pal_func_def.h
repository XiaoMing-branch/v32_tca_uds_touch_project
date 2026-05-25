#ifndef __FFF_PAL_FUNC_DEF_H__
#define __FFF_PAL_FUNC_DEF_H__

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

#ifdef ENABLE_TEST_MODE
  #include "fff_tcpl03x_ll_def.h"
#else
  #include "tcpl03x_ll_def.h"
#endif

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
#define interrupt_enable()    (void*)0
/*  mask all interrupt but NMI and HardFault */
#define interrupt_disable()   (void*)0


#ifndef CFG_SUPPORT_LED_NUM
#define CFG_SUPPORT_LED_NUM                       (1)
#endif /* CFG_SUPPORT_LED_NUM */
#define LED_CHANNEL_MAX                           (CFG_SUPPORT_LED_NUM)
#define LED_CHANNEL_0                             (0)
#define LED_CHANNEL_1                             (1)
#define LED_CHANNEL_2                             (2)

/**
  * @brief  typedef led channel enumeration
  */
typedef uint8_t led_channel_e;

/**
  * @brief  rgb type enumeration
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
