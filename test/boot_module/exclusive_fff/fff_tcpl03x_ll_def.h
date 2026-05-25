#ifndef __FFF_TCPL03X_LL_DEF_H__
#define __FFF_TCPL03X_LL_DEF_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef ENABLE_TEST_MODE
#include "fff_tcpl03x.h"
#else
#include "tcpl03x.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef  USE_FULL_ASSERT
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif

/**
 * @defgroup CRG LOCK Configuratoins_Definitions
 */

#if 1
#define CRG_CONFIG_LOCK()           (void*)0
#define CRG_CONFIG_UNLOCK()         (void*)0
#else
#define CRG_CONFIG_LOCK()
#define CRG_CONFIG_UNLOCK()
#endif

/**
 * @defgroup SYSCFG LOCK Configuratoins_Definitions
 */
#if 0
#define SYSCFG_CONFIG_LOCK()        (SYSCFG->SYSCFG_LOCK=0X12345678)
#define SYSCFG_CONFIG_UNLOCK()      (SYSCFG->SYSCFG_LOCK=0XAA55AA55)
#else
#define SYSCFG_CONFIG_LOCK()
#define SYSCFG_CONFIG_UNLOCK()
#endif

/**
 * @defgroup ASYSCFG CONFIG Definitions
 */
#if 0
#define ASYSCFG_CONFIG_LOCK()       (ASYSCFG->ASYSCFG_LOCK=0x12345678)
#define ASYSCFG_CONFIG_UNLOCK()     (ASYSCFG->ASYSCFG_LOCK=0xAA55AA55)
#else
#define ASYSCFG_CONFIG_LOCK()
#define ASYSCFG_CONFIG_UNLOCK()
#endif

/**
 * @defgroup Test write lock definitions
 */
#if 0
#define TEST_CONFIG_LOCK()          (TEST->TEST_LOCK=0xFEDCBA98)
#define TEST_CONFIG_UNLOCK()        (TEST->TEST_LOCK=0x76543210)
#else
#define TEST_CONFIG_LOCK()
#define TEST_CONFIG_UNLOCK()
#endif

/**
 * @brief Performes a software reset
 */
#define LL_SOFTWARE_RESET()              (void*)0              
        

/**
 * @brief Performes a software reset
 */
#define LL_SOFTWARE_RESET_FLAG_CLEAR()            (void*)0

/**
 * @brief  fclk_src_e enumeration
 */
typedef enum
{
    FCLK_SRC_48M,
    FCLK_SRC_32K,
    FCLK_SRC_MAX,
} fclk_src_e;

/**
 * @brief  lin aa type enumeration
 */
typedef enum
{
    LIN_AA_STYPE_STEPS_4 = 0,
    LIN_AA_STYPE_STEPS_3,
    LIN_AA_STYPE_STEPS_2,
} lin_aa_type_e;

/**
 * @brief  ll status enumeration
 */
typedef enum
{
    LL_OK       = 0x00U,
    LL_ERROR    = 0x01U,
    LL_BUSY     = 0x02U,
    LL_TIMEOUT  = 0x03U,
    LL_COMM_ERROR  = 0x04U,
    LL_PARAM_INVALID  = 0x05U
} ll_status_e;

/**
 * @brief CALLBACK FUNC
 */
typedef void (*ISR_FUNC_CALLBACK)(uint32_t);

/**
 * @brief  ll clk struct
 */
typedef struct
{
    fclk_src_e clk_source;
    uint8_t  fclk_div;
} ll_clk_config_t;

/**
 * @brief  ll clk struct
 */
typedef struct
{
    uint32_t isr;
    uint8_t priority;
    bool isr_enable;
} ll_isr_config_t;

#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_cortex.h"
    #include "fff_tcpl03x_ll_sys.h"
    #include "fff_tcpl03x_ll_adc.h"
    #include "fff_tcpl03x_ll_flash.h"
    #include "fff_tcpl03x_ll_gpio.h"
    #include "fff_tcpl03x_ll_lpm.h"
    #include "fff_tcpl03x_ll_pwm.h"
    #include "fff_tcpl03x_ll_sci.h"
    #include "fff_tcpl03x_ll_timer.h"
    #include "fff_tcpl03x_ll_wdg.h"
#else
    #include "tcpl03x_ll_cortex.h"
    #include "tcpl03x_ll_sys.h"
    #include "tcpl03x_ll_adc.h"
    #include "tcpl03x_ll_flash.h"
    #include "tcpl03x_ll_gpio.h"
    #include "tcpl03x_ll_lpm.h"
    #include "tcpl03x_ll_pwm.h"
    #include "tcpl03x_ll_sci.h"
    #include "tcpl03x_ll_timer.h"
    #include "tcpl03x_ll_wdg.h"
#endif


#if defined(__cplusplus)
}
#endif
#endif /*__TCPL03X_LL_DEF_H__*/
