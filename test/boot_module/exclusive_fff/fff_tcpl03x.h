#ifndef __FFF_TCPL03X_H__
#define __FFF_TCPL03X_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief 32 bits memory read macro.
*/
#if !defined(REG_READ32)
#define REG_READ32(address)               (void*)0
#endif

/**
* @brief 32 bits memory write macro.
*/
#if !defined(REG_WRITE32)
#define REG_WRITE32(address, value)       (void*)0
#endif

/**
* @brief 32 bits bits setting macro.
*/
#if !defined(REG_BIT_SET32)
#define REG_BIT_SET32(address, mask)      (void*)0
#endif

/**
* @brief 32 bits bits clearing macro.
*/
#if !defined(REG_BIT_CLEAR32)
#define REG_BIT_CLEAR32(address, mask)    (void*)0
#endif

/**
* @brief 32 bit clear bits and set with new value
* @note It is user's responsability to make sure that value has only "mask" bits set - (value&~mask)==0
*/
#if !defined(REG_RMW32)
#define REG_RMW32(address, mask, value)   (void*)0
#endif

#if !defined(REG_TRIM_VERSION)
#define REG_TRIM_VERSION()                 (void*)0
#endif

typedef enum IRQn
{
    /******  Cortex-M# Processor Exceptions Numbers ***************************************************/

    /* ToDo: use this Cortex interrupt numbers if your device is a CORTEX-M4 device                   */
    NonMaskableInt_IRQn           = -14,      /*!<  2 Non Maskable Interrupt                          */
    HardFault_IRQn                = -13,      /*!<  3 Hard Fault Interrupt                            */
    SVCall_IRQn                   = -5,       /*!< 11 SV Call Interrupt                               */
    PendSV_IRQn                   = -2,       /*!< 14 Pend SV Interrupt                               */
    SysTick_IRQn                  = -1,       /*!< 15 System Tick Interrupt                           */

    /******  Device Specific Interrupt Numbers ********************************************************/
    /* ToDo: add here your device specific external interrupt numbers
             according the interrupt handlers defined in startup_Device.s
             eg.: Interrupt for Timer#1       TIM1_IRQHandler   ->   TIM1_IRQn                        */

    FLASH_IRQn          = 0,
    ADC_IRQn            = 1,
    PWM_IRQn            = 2,
    TIMER_IRQn          = 3,
    IWDG_IRQn           = 4,
    LINSCI_IRQn         = 5,
    AON_IRQn            = 6,
    GPIO_IRQn           = 7,
    RESERVE_S_IRQn      = 8,
    RESERVE_T_IRQn      = 9,
    LINSCI_UART_IRQn    = 10,

} IRQn_Type;


/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM0_REV                 0x0000U   /* Core revision r0p0 */
#define __MPU_PRESENT             0U        /* no MPU present */
#define __VTOR_PRESENT            0U        /* no VTOR present */
#define __NVIC_PRIO_BITS          2U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */

// #include "core_cm0.h"

/* Register Structure Definition */
// #include "./reg/register.h"

/* Default System clock value */
#ifndef DEFAULT_SYSTEM_CLOCK
#define DEFAULT_SYSTEM_CLOCK     (48000000UL)
#endif


#ifdef __cplusplus
}
#endif
#endif /* __TCPL03X_H__ */
