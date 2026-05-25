/**
 *****************************************************************************
 * @brief   driver def header.
 *
 * @file    tcae10_ll_def.h
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

#ifndef __TCAE10_LL_DEF_H__
#define __TCAE10_LL_DEF_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tcae10.h"

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
#define CRG_CONFIG_LOCK()           (CRG->CRG_LOCK = 0X12345678)
#define CRG_CONFIG_UNLOCK()         (CRG->CRG_LOCK = 0X5A5A5A5A)
#else
#define CRG_CONFIG_LOCK()
#define CRG_CONFIG_UNLOCK()
#endif


/** @defgroup SYSCFG LOCK Configuratoins_Definitions
  * @{
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

/*!
   \brief Performes a software reset
*/
/**
 * @brief  执行软件复位
 */
#define LL_SOFTWARE_RESET()                             \
        do {                                            \
            CRG_CONFIG_UNLOCK();                        \
            CRG->M0_CLKRST_CTRL_F.RST_M0=1;             \
            CRG_CONFIG_LOCK();                          \
        } while(0)


/*!
   \brief Performes a software reset
*/
/**
 * @brief  清除软件复位标志
 */
#define LL_SOFTWARE_RESET_FLAG_CLEAR()                  \
        do {                                            \
            CRG_CONFIG_UNLOCK();                        \
            CRG->RST_CTRL_F.CLR_RST = 1;                \
            CRG_CONFIG_LOCK();                          \
        } while(0)


/**
  * @brief  fclk_src_e enumeration
  */
typedef enum
{
    FCLK_SRC_48M,
    FCLK_SRC_32K,
    FCLK_SRC_MAX,
} fclk_src_e;

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



#include "tcae10_ll_cortex.h"
#include "tcae10_ll_sys.h"
#include "tcae10_ll_adc.h"
#include "tcae10_ll_flash.h"
#include "tcae10_ll_gpio.h"
#include "tcae10_ll_lpm.h"
#include "tcae10_ll_pwm.h"
#include "tcae10_ll_sci.h"
#include "tcae10_ll_timer.h"
#include "tcae10_ll_wdg.h"
#include "tcae10_ll_delay.h"
#include "tcae10_ll_spi.h"
#include "tcae10_ll_uart.h"
#include "tcae10_ll_captouch.h"

#if defined(__cplusplus)
}
#endif
#endif /*__TCPL01X_LL_DEF_H__*/
