/**
 *****************************************************************************
 * @brief   Device Peripheral Access Layer Header File.
 *          This file contains all the peripheral register's definitions, bits
 *          definitions and memory mapping for TCAE32A chip.
 *
 * @file   tcae10.h
 * @author AE/FAE team
 * @date
 * @Version V1.0.0
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */
#ifndef __FFF_TCAE10_H__
#define __FFF_TCAE10_H__

#include "fff.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "fff_base_types.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define M32(adr)               (*((volatile unsigned long  *) (adr)))

/**
* @brief 32 bits memory read macro.
*/
#if !defined(REG_READ32)
#define REG_READ32(address)               (*(volatile uint32_t*)(address))
#endif

/**
* @brief 32 bits memory write macro.
*/
#if !defined(REG_WRITE32)
#define REG_WRITE32(address, value)       ((*(volatile uint32_t*)(address))= (uint32_t)(value))
#endif

/**
* @brief 32 bits bits setting macro.
*/
#if !defined(REG_BIT_SET32)
#define REG_BIT_SET32(address, mask)      ((*(volatile uint32_t*)(address))|= (uint32_t)(mask))
#endif

/**
* @brief 32 bits bits clearing macro.
*/
#if !defined(REG_BIT_CLEAR32)
#define REG_BIT_CLEAR32(address, mask)    ((*(volatile uint32_t*)(address))&= ((uint32_t)~((uint32_t)(mask))))
#endif

/**
* @brief 32 bit clear bits and set with new value
* @note It is user's responsability to make sure that value has only "mask" bits set - (value&~mask)==0
*/
#if !defined(REG_RMW32)
#define REG_RMW32(address, mask, value)   (REG_WRITE32((address), ((REG_READ32(address)& ((uint32_t)~((uint32_t)(mask))))| ((uint32_t)(value)))))
#endif

#if !defined(REG_TRIM_VERSION)
#define REG_TRIM_VERSION()                 ((*(volatile uint32_t*)(0X00800000UL)) & 0x00003FFFUL)
#endif

typedef enum IRQn
{
    /******  Cortex-M# Processor Exceptions Numbers ***************************************************/

    /* ToDo: use this Cortex interrupt numbers if your device is a CORTEX-M4 device                   */
    NonMaskableInt_IRQn           = -14,      /*!<  2 Non Maskable Interrupt                        */
    HardFault_IRQn                = -13,      /*!<  3 Hard Fault Interrupt                          */
    SVCall_IRQn                   = -5,       /*!< 11 SV Call Interrupt                             */
    PendSV_IRQn                   = -2,       /*!< 14 Pend SV Interrupt                             */
    SysTick_IRQn                  = -1,       /*!< 15 System Tick Interrupt                         */

    /******  Device Specific Interrupt Numbers ********************************************************/
    /* ToDo: add here your device specific external interrupt numbers
             according the interrupt handlers defined in startup_Device.s
             eg.: Interrupt for Timer#1       TIM1_IRQHandler   ->   TIM1_IRQn                        */

    FLASH_IRQn         = 0,
    ADC_IRQn            = 1,
    PWM_IRQn            = 2,
    TIMER_IRQn          = 3,
    IWDG_IRQn           = 4,
    LINSCI_IRQn         = 5,
    AON_IRQn            = 6,
    GPIO_IRQn           = 7,
    SPI_IRQn            = 8,
    TOUCH_IRQn          = 9,
    LINSCI_UART_IRQn    = 10,

} IRQn_Type;

/************************************************************************/
/*  Processor and Core Peripheral Section                               */
/************************************************************************/
/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined (__ICCARM__)
#pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc11-extensions"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
/* anonymous unions are enabled by default */
#elif defined (__TMS470__)
/* anonymous unions are enabled by default */
#elif defined (__TASKING__)
#pragma warning 586
#elif defined (__CSMC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif
/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM0_REV                 0x0000U   /* Core revision r0p0 */
#define __MPU_PRESENT             0U        /* no MPU present */
#define __VTOR_PRESENT            0U        /* no VTOR present */
#define __NVIC_PRIO_BITS          2U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */

#include "fff_core_cm0.h"

#ifndef DEFAULT_SYSTEM_CLOCK
#define DEFAULT_SYSTEM_CLOCK     (48000000UL)
#endif
#define DEFAULT_FCLK_CLOCK       (48000000UL)

/* ************************************************************************** */
/**  SOFTWARE PERIPHERAL API DEFINITION FOR TCAE31A */
/* ************************************************************************** */
/** \defgroup TCAE31A_api Peripheral Software API */

// Register Structure Definition
#include "fff_register.h"

/************************************************************************/
/*  APB address config                                                  */
/************************************************************************/
//#include "register_map.h"



/* --------  End of section using anonymous unions and disabling warnings  -------- */
#if   defined (__CC_ARM)
#pragma pop
#elif defined (__ICCARM__)
/* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
#pragma clang diagnostic pop
#elif defined (__GNUC__)
/* anonymous unions are enabled by default */
#elif defined (__TMS470__)
/* anonymous unions are enabled by default */
#elif defined (__TASKING__)
#pragma warning restore
#elif defined (__CSMC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif



#ifdef __cplusplus
}
#endif
#endif /* __FFF_TCAE10_H__ */
