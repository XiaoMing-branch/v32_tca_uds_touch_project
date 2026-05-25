/**
 *****************************************************************************
 * @brief   register_map header file.
 *
 * @file    register_map.h
 * @author  AE/FAE team
 * @date    2024.09.01
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

#ifndef __FFF_REGISTER_MAP_H__
#define __FFF_REGISTER_MAP_H__

// ---------------------------------------------------------------------
// Address
// ---------------------------------------------------------------------
// Memory
// #define  SRAM_BASE_ADDR             0x10000000
// #define  SRAM_SIZE_OF_BYTES         0x800 // 2K bytes
// #define  SRAM_END_ADDR              (SRAM_BASE_ADDR   + 0x700) // actual sram end is + 0x7FF, used to test sram and avoid to corrupt stack.

// #define  SRAM_BASE_ADDR             0x10000000
// #define  SRAM_SIZE_OF_BYTES         0xC00 // 3K bytes
// #define  SRAM_END_ADDR              (SRAM_BASE_ADDR   + 0xB00) // actual sram end is + 0xBFF, used to test sram and avoid to corrupt stack.

#define  SRAM_BASE_ADDR             0x10000000
#define  SRAM_SIZE_OF_BYTES         0x1000 // 4K bytes
#define  SRAM_END_ADDR              (SRAM_BASE_ADDR   + 0xF00) // actual sram end is + 0xFFF, used to test sram and avoid to corrupt stack.

#define  __EFLASH_BASE_ADDR         0x00000000
#define  EFLASH_NVM_BASE_ADDR       (__EFLASH_BASE_ADDR + 0x000000)
#define  EFLASH_NVR_BASE_ADDR       (__EFLASH_BASE_ADDR + 0x800000)
#define  EFLASH_CFG_BASE_ADDR       (__EFLASH_BASE_ADDR + 0x880000)
#define  EFLASH_BASE_ADDR           (__EFLASH_BASE_ADDR + 0xFF0000) // Used in eflash_reg_def.h

// AHB Peripheral

// APB Peripheral
#define  APB_BASE_ADDRESS           0x40000000
#define  SYSCFG_BASE_ADDR           (APB_BASE_ADDRESS + 0x0000)
#define  CRG_BASE_ADDR              (APB_BASE_ADDRESS + 0x1000)
#define  PINMUX_BASE_ADDR           (APB_BASE_ADDRESS + 0x2000)
#define  TIM_LITE_BASE_ADDR         (APB_BASE_ADDRESS + 0x3000)
#define  PRINT_UART_BASE_ADDR       (APB_BASE_ADDRESS + 0x4000)
#define  PWM_BASE_ADDR              (APB_BASE_ADDRESS + 0x5000)
#define  LIN_SCI_BASE_ADDR          (APB_BASE_ADDRESS + 0x6000)
#define  ADC_BASE_ADDR              (APB_BASE_ADDRESS + 0x7000)
#define  GPIO_BASE_ADDR             (APB_BASE_ADDRESS + 0x8000)
#define  CAPTOUCH_BASE_ADDR         (APB_BASE_ADDRESS + 0x9000)
#define  IWDG_BASE_ADDR             (APB_BASE_ADDRESS + 0xA000)
#define  LIN_SCI1_BASE_ADDR         (APB_BASE_ADDRESS + 0xB000)
#define  ASYSCFG_BASE_ADDR          (APB_BASE_ADDRESS + 0xC000)
#define  SPI_BASE_ADDR              (APB_BASE_ADDRESS + 0xD000)
// #define  RESERVED                   (APB_BASE_ADDRESS + 0xE000)
#define  TEST_BASE_ADDR             (APB_BASE_ADDRESS + 0xF000)

// --------------------------------------------------------------------
// Reg Map
// --------------------------------------------------------------------
// Memory
#define  EFLASH                     ((EFLASH_REG_TypeDef*)          EFLASH_BASE_ADDR)
#define  SRAM                       ((SRAM_REG_TypeDef*)            SRAM_BASE_ADDR)

// AHB Peripheral

// APB Peripheral
#define  SYSCFG                     ((SYSCFG_REG_TypeDef*)          SYSCFG_BASE_ADDR)
#define  CRG                        ((CRG_REG_TypeDef*)             CRG_BASE_ADDR)
#define  PINMUX                     ((PINMUX_REG_TypeDef*)          PINMUX_BASE_ADDR)
#define  TIM_LITE                   ((TIM_LITE_REG_TypeDef*)        TIM_LITE_BASE_ADDR)
#define  PRINT_UART                 ((PRINT_UART_REG_TypeDef*)      PRINT_UART_BASE_ADDR)
#define  PWM                        ((PWM_REG_TypeDef*)             PWM_BASE_ADDR)
#define  LIN_SCI                    ((LIN_SCI_REG_TypeDef*)         LIN_SCI_BASE_ADDR)
#define  ADC                        ((ADC_REG_TypeDef*)             ADC_BASE_ADDR)
#define  GPIO                       ((GPIO_REG_TypeDef*)            GPIO_BASE_ADDR)
#define  CAPTOUCH                   ((CAPTOUCH_REG_TypeDef*)        CAPTOUCH_BASE_ADDR)
#define  IWDG                       ((IWDG_REG_TypeDef*)            IWDG_BASE_ADDR)
#define  LIN_SCI1                   ((LIN_SCI1_REG_TypeDef*)        LIN_SCI1_BASE_ADDR)
#define  ASYSCFG                    ((ASYSCFG_REG_TypeDef*)         ASYSCFG_BASE_ADDR)
#define  SPI                        ((SPI_REG_TypeDef*)             SPI_BASE_ADDR)
#define  TEST                       ((TEST_REG_TypeDef*)            TEST_BASE_ADDR)

#endif

