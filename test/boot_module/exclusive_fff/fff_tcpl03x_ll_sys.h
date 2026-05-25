#ifndef __FFF_TCPL03X_LL_SYS_H__
#define __FFF_TCPL03X_LL_SYS_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x.h"
#else
    #include "tcpl03x.h"
#endif

#include "fff.h"


#if defined(__cplusplus)
extern "C" {
#endif

/**
  * @defgroup ASYSCFG_OTP_RST_Definitions
  */
#define ASYSCFG_OTP_RST_ENABLE()    (void*)0
#define ASYSCFG_OTP_RST_DISABLE()   (void*)0

/**
  * @defgroup ASYSCFG_HRC_CLOCK_Config_Macro
  */
/*Enable software HRC, when it enable the PMU high power mode ready*/
#define ASYSCFG_SW_HRC_ENABLE()                     (void*)0
/*Disable software HRC, */
#define ASYSCFG_SW_HRC_DISABLE()                    (void*)0
/*Disable software HRC clock in sleepwalk mode*/
#define ASYSCFG_HRC_CLOCK_DISABLE_ON_SLEEPWALK()    (void*)0
/*Enable software HRC clock in sleepwalk mode*/
#define ASYSCFG_HRC_CLOCK_ENABLE_ON_SLEEPWALK()     (void*)0

/** ]
  * @defgroup ASYSCFG_RESET_CAUSE_Definitions
  */
#define ASYSCFG_RST_CAUSE_SW_POR_REQ        0X01
#define ASYSCFG_RST_CAUSE_IO4_PAD_RST       0X02
#define ASYSCFG_RST_CAUSE_IWDG              0X04
#define ASYSCFG_RST_CAUSE_OTP               0X08
#define ASYSCFG_RST_CAUSE_VS_ALT            0X10
#define ASYSCFG_RST_CAUSE_CM0_RST           0X20    //CM0 Reset or CM0 lockup

/**
  * @defgroup ASYSCFG_RESET_CONTROL_Definitions
  */
#define ASYSCFG_CM0_LOCKUP_RST_ENABLE()     (void*)0
#define ASYSCFG_CM0_LOCKUP_RST_DISABLE()    (void*)0
/*Request a software reset, this will reset all logic*/
#define ASYSCFG_RST_REQUEST()               (void*)0
/*Get the reset cuase*/
#define ASYSCFG_RST_CAUSE_GET()             (void*)0
/*Clear the reset cuase*/
#define ASYSCFG_RST_CAUSE_CLEAR()           (void*)0

/**
  * @defgroup SYSCFG_MISC_CONFIGS_Definitions
  */
#define SYSCFG_PWM_DISABLE_WHEN_OTP(status)   (void*)0
    

#define SYSCFG_PWM_DISABLE_WHEN_VS_ALTER(status)  (void*)0
  

#define SYSCFG_PWM_DISABLE_WHEN_VS_LVD(status)  (void*)0
    
typedef enum
{
    WAKEUP_SOUERCE_LIN  = 0,
    WAKEUP_SOURCE_IO0,
    WAKEUP_SOURCE_MAX
} wakeup_source_e;

/**
 * @brief  wakeup time enumeration, 1clk unit cycle = 1/32KHz = 31.25 us
 */
typedef enum
{
    WAKEUP_TIME_1  = 0,
    WAKEUP_TIME_2,
    WAKEUP_TIME_3,
    WAKEUP_TIME_4,
    WAKEUP_TIME_5,
    WAKEUP_TIME_6,
    WAKEUP_TIME_7,
    WAKEUP_TIME_8,
    WAKEUP_TIME_9,
    WAKEUP_TIME_10,
    WAKEUP_TIME_11,
    WAKEUP_TIME_12,
    WAKEUP_TIME_13,
    WAKEUP_TIME_14,
    WAKEUP_TIME_15,
    WAKEUP_TIME_16,
    WAKEUP_TIME_MAX
} wakeup_time_e;

/**
 * @brief  wakeup filter enumeration, 1clk unit cycle = 1/32KHz = 31.25 us
 */
typedef enum
{
    WAKEUP_FILTER_1  = 0,
    WAKEUP_FILTER_2,
    WAKEUP_FILTER_3,
    WAKEUP_FILTER_4,
    WAKEUP_FILTER_MAX
} wakeup_filter_e;

/**
 * @brief  syscfg Interrupt enumeration
 */
typedef enum
{
    ASYSCFG_INT_WAKEUP        = 1,
    ASYSCFG_INT_VSLVD         = 2,
    ASYSCFG_INT_OTP           = 4,
    ASYSCFG_INT_RESERVED12_OCP = 8,
} asyscfg_isr_type_e;


/**
 * @brief  syscfg Interrupt triger flag enumeration
 */
typedef enum
{
    ASYSCFG_TRIGGER_NULL        = 0x00,  /* null */
    ASYSCFG_TRIGGER_POSEDGE     = 0x01,  /* posedge */
    ASYSCFG_TRIGGER_NEGEDGE     = 0x02,  /* negedge */
    ASYSCFG_TRIGGER_HIGH_LEVEL  = 0x04,  /* high level */
} asyscfg_trigger_flag_e;


/**
 * @brief  system backup zone enumeration
 */
typedef enum
{
    SYSTEM_BACKUP_ZONE_0,
    SYSTEM_BACKUP_ZONE_1,
    SYSTEM_BACKUP_ZONE_MAX

} sys_backup_zone_e;

DECLARE_FAKE_VOID_FUNC(ll_syscfg_info_get,uint8_t *,uint16_t *);
DECLARE_FAKE_VOID_FUNC(ll_wakeup_init,wakeup_source_e,wakeup_time_e,wakeup_filter_e);
DECLARE_FAKE_VOID_FUNC(ll_syscfg_isr_enable,asyscfg_isr_type_e,bool);
DECLARE_FAKE_VOID_FUNC(ll_syscfg_otp_enable,bool);
DECLARE_FAKE_VALUE_FUNC(bool,ll_syscfg_otp_status);
DECLARE_FAKE_VOID_FUNC(ll_syscfg_isr_clear,asyscfg_isr_type_e);
DECLARE_FAKE_VALUE_FUNC(uint8_t,ll_syscfg_isr_get,asyscfg_isr_type_e);
DECLARE_FAKE_VOID_FUNC(ll_syscfg_backup_reg_write,sys_backup_zone_e,uint32_t);
DECLARE_FAKE_VOID_FUNC(ll_syscfg_backup_reg_read,sys_backup_zone_e,uint32_t *);


#if defined(__cplusplus)
}
#endif
#endif /* __TCPL03X_LL_SYS_H__ */
