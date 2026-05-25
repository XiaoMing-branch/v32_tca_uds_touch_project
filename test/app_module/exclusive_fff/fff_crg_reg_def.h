/**
 *****************************************************************************
 * @brief   crg_reg_def header file.
 *
 * @file    crg_reg_def.h
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

#ifndef   FFF_CRG_DEF_H__
#define   FFF_CRG_DEF_H__

// Register Offset Macro Definition
#define CRG_HCLK_CTRL_OFFSET                                    0x0000
#define CRG_PCLK_CTRL_OFFSET                                    0x0004
#define CRG_M0_CLKRST_CTRL_OFFSET                               0x0008
#define CRG_TIM_LITE_CLKRST_CTRL_OFFSET                         0x000C
#define CRG_PRINT_UART_CLKRST_CTRL_OFFSET                       0x0010
#define CRG_IWDG_CLKRST_CTRL_OFFSET                             0x0014
#define CRG_PWM_CLKRST_CTRL_OFFSET                              0x0018
#define CRG_ADC_CLKRST_CTRL_OFFSET                              0x001C
#define CRG_LIN_SCI_CLKRST_CTRL_OFFSET                         0x0020
#define CRG_GPIO_CLKRST_CTRL_OFFSET                             0x0024
#define CRG_PINMUX_CLKRST_CTRL_OFFSET                           0x0028
#define CRG_CAPTOUCH_CLKRST_CTRL_OFFSET                            0x002C
#define CRG_SPI_CLKRST_CTRL_OFFSET                              0x0030
#define CRG_LIN_SCI1_CLKRST_CTRL_OFFSET                         0x0034
#define CRG_HRC_SSG_CTRL_OFFSET                                 0x0038
#define CRG_CRG_LOCK_OFFSET                                     0x00FC

// Register Address Macro Definition
#define CRG_HCLK_CTRL_ADDR                                      ( CRG_BASE_ADDR + CRG_HCLK_CTRL_OFFSET)
#define CRG_PCLK_CTRL_ADDR                                      ( CRG_BASE_ADDR + CRG_PCLK_CTRL_OFFSET)
#define CRG_M0_CLKRST_CTRL_ADDR                                 ( CRG_BASE_ADDR + CRG_M0_CLKRST_CTRL_OFFSET)
#define CRG_TIM_LITE_CLKRST_CTRL_ADDR                           ( CRG_BASE_ADDR + CRG_TIM_LITE_CLKRST_CTRL_OFFSET)
#define CRG_PRINT_UART_CLKRST_CTRL_ADDR                         ( CRG_BASE_ADDR + CRG_PRINT_UART_CLKRST_CTRL_OFFSET)
#define CRG_IWDG_CLKRST_CTRL_ADDR                               ( CRG_BASE_ADDR + CRG_IWDG_CLKRST_CTRL_OFFSET)
#define CRG_PWM_CLKRST_CTRL_ADDR                                ( CRG_BASE_ADDR + CRG_PWM_CLKRST_CTRL_OFFSET)
#define CRG_ADC_CLKRST_CTRL_ADDR                                ( CRG_BASE_ADDR + CRG_ADC_CLKRST_CTRL_OFFSET)
#define CRG_LIN_SCI_CLKRST_CTRL_ADDR                           ( CRG_BASE_ADDR + CRG_LIN_SCI_CLKRST_CTRL_OFFSET)
#define CRG_GPIO_CLKRST_CTRL_ADDR                               ( CRG_BASE_ADDR + CRG_GPIO_CLKRST_CTRL_OFFSET)
#define CRG_PINMUX_CLKRST_CTRL_ADDR                             ( CRG_BASE_ADDR + CRG_PINMUX_CLKRST_CTRL_OFFSET)
#define CRG_CAPTOUCH_CLKRST_CTRL_ADDR                              ( CRG_BASE_ADDR + CRG_CAPTOUCH_CLKRST_CTRL_OFFSET)
#define CRG_SPI_CLKRST_CTRL_ADDR                                ( CRG_BASE_ADDR + CRG_SPI_CLKRST_CTRL_OFFSET)
#define CRG_LIN_SCI1_CLKRST_CTRL_ADDR                           ( CRG_BASE_ADDR + CRG_LIN_SCI1_CLKRST_CTRL_OFFSET)
#define CRG_HRC_SSG_CTRL_ADDR                                   ( CRG_BASE_ADDR + CRG_HRC_SSG_CTRL_OFFSET)
#define CRG_CRG_LOCK_ADDR                                       ( CRG_BASE_ADDR + CRG_CRG_LOCK_OFFSET)

// Register Field Macro Definition
#define CRG_HCLK_CTRL_AUTO_HCLK_FORCE_EN_SHIFT                                8
#define CRG_HCLK_CTRL_AUTO_HCLK_FORCE_EN_MASK                                 0x00000100
#define CRG_HCLK_CTRL_AUTO_HCLK_FORCE_EN_SET(n)                               (((uint32_t)(n) << 8  ) & 0x00000100)
#define CRG_HCLK_CTRL_AUTO_HCLK_FORCE_EN_CLR                                  0xFFFFFEFF

#define CRG_HCLK_CTRL_HCLK_DIV_SHIFT                                          4
#define CRG_HCLK_CTRL_HCLK_DIV_MASK                                           0x000000F0
#define CRG_HCLK_CTRL_HCLK_DIV_SET(n)                                         (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_HCLK_CTRL_HCLK_DIV_CLR                                            0xFFFFFF0F

#define CRG_HCLK_CTRL_HCLK_DIV_LOAD_SHIFT                                     3
#define CRG_HCLK_CTRL_HCLK_DIV_LOAD_MASK                                      0x00000008
#define CRG_HCLK_CTRL_HCLK_DIV_LOAD_SET(n)                                    (((uint32_t)(n) << 3  ) & 0x00000008)
#define CRG_HCLK_CTRL_HCLK_DIV_LOAD_CLR                                       0xFFFFFFF7

#define CRG_HCLK_CTRL_HCLK_SEL_SHIFT                                          0
#define CRG_HCLK_CTRL_HCLK_SEL_MASK                                           0x00000001
#define CRG_HCLK_CTRL_HCLK_SEL_SET(n)                                         (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_HCLK_CTRL_HCLK_SEL_CLR                                            0xFFFFFFFE

#define CRG_PCLK_CTRL_PCLK_EN_ASYSCFG_SHIFT                                   9
#define CRG_PCLK_CTRL_PCLK_EN_ASYSCFG_MASK                                    0x00000200
#define CRG_PCLK_CTRL_PCLK_EN_ASYSCFG_SET(n)                                  (((uint32_t)(n) << 9  ) & 0x00000200)
#define CRG_PCLK_CTRL_PCLK_EN_ASYSCFG_CLR                                     0xFFFFFDFF

#define CRG_PCLK_CTRL_PCLK_EN_TEST_SHIFT                                      8
#define CRG_PCLK_CTRL_PCLK_EN_TEST_MASK                                       0x00000100
#define CRG_PCLK_CTRL_PCLK_EN_TEST_SET(n)                                     (((uint32_t)(n) << 8  ) & 0x00000100)
#define CRG_PCLK_CTRL_PCLK_EN_TEST_CLR                                        0xFFFFFEFF

#define CRG_PCLK_CTRL_PCLK_DIV_LOAD_SHIFT                                     3
#define CRG_PCLK_CTRL_PCLK_DIV_LOAD_MASK                                      0x00000008
#define CRG_PCLK_CTRL_PCLK_DIV_LOAD_SET(n)                                    (((uint32_t)(n) << 3  ) & 0x00000008)
#define CRG_PCLK_CTRL_PCLK_DIV_LOAD_CLR                                       0xFFFFFFF7

#define CRG_PCLK_CTRL_PCLK_DIV_SHIFT                                          0
#define CRG_PCLK_CTRL_PCLK_DIV_MASK                                           0x00000007
#define CRG_PCLK_CTRL_PCLK_DIV_SET(n)                                         (((uint32_t)(n) << 0  ) & 0x00000007)
#define CRG_PCLK_CTRL_PCLK_DIV_CLR                                            0xFFFFFFF8

#define CRG_M0_CLKRST_CTRL_FCLK_DIV_SYSTICK_SHIFT                             4
#define CRG_M0_CLKRST_CTRL_FCLK_DIV_SYSTICK_MASK                              0x000000F0
#define CRG_M0_CLKRST_CTRL_FCLK_DIV_SYSTICK_SET(n)                            (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_M0_CLKRST_CTRL_FCLK_DIV_SYSTICK_CLR                               0xFFFFFF0F

#define CRG_M0_CLKRST_CTRL_FCLK_EN_SYSTICK_SHIFT                              2
#define CRG_M0_CLKRST_CTRL_FCLK_EN_SYSTICK_MASK                               0x00000004
#define CRG_M0_CLKRST_CTRL_FCLK_EN_SYSTICK_SET(n)                             (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_M0_CLKRST_CTRL_FCLK_EN_SYSTICK_CLR                                0xFFFFFFFB

#define CRG_M0_CLKRST_CTRL_RST_M0_SHIFT                                       0
#define CRG_M0_CLKRST_CTRL_RST_M0_MASK                                        0x00000001
#define CRG_M0_CLKRST_CTRL_RST_M0_SET(n)                                      (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_M0_CLKRST_CTRL_RST_M0_CLR                                         0xFFFFFFFE

#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_DIV_TIM_LITE_SHIFT                      4
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_DIV_TIM_LITE_MASK                       0x00000FF0
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_DIV_TIM_LITE_SET(n)                     (((uint32_t)(n) << 4  ) & 0x00000FF0)
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_DIV_TIM_LITE_CLR                        0xFFFFF00F

#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_SEL_TIM_LITE_SHIFT                      3
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_SEL_TIM_LITE_MASK                       0x00000008
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_SEL_TIM_LITE_SET(n)                     (((uint32_t)(n) << 3  ) & 0x00000008)
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_SEL_TIM_LITE_CLR                        0xFFFFFFF7

#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_EN_TIM_LITE_SHIFT                       2
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_EN_TIM_LITE_MASK                        0x00000004
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_EN_TIM_LITE_SET(n)                      (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_TIM_LITE_CLKRST_CTRL_FCLK_EN_TIM_LITE_CLR                         0xFFFFFFFB

#define CRG_TIM_LITE_CLKRST_CTRL_PCLK_EN_TIM_LITE_SHIFT                       1
#define CRG_TIM_LITE_CLKRST_CTRL_PCLK_EN_TIM_LITE_MASK                        0x00000002
#define CRG_TIM_LITE_CLKRST_CTRL_PCLK_EN_TIM_LITE_SET(n)                      (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_TIM_LITE_CLKRST_CTRL_PCLK_EN_TIM_LITE_CLR                         0xFFFFFFFD

#define CRG_TIM_LITE_CLKRST_CTRL_RST_TIM_LITE_SHIFT                           0
#define CRG_TIM_LITE_CLKRST_CTRL_RST_TIM_LITE_MASK                            0x00000001
#define CRG_TIM_LITE_CLKRST_CTRL_RST_TIM_LITE_SET(n)                          (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_TIM_LITE_CLKRST_CTRL_RST_TIM_LITE_CLR                             0xFFFFFFFE

#define CRG_PRINT_UART_CLKRST_CTRL_PCLK_EN_PRINT_UART_SHIFT                   1
#define CRG_PRINT_UART_CLKRST_CTRL_PCLK_EN_PRINT_UART_MASK                    0x00000002
#define CRG_PRINT_UART_CLKRST_CTRL_PCLK_EN_PRINT_UART_SET(n)                  (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_PRINT_UART_CLKRST_CTRL_PCLK_EN_PRINT_UART_CLR                     0xFFFFFFFD

#define CRG_PRINT_UART_CLKRST_CTRL_RST_PRINT_UART_SHIFT                       0
#define CRG_PRINT_UART_CLKRST_CTRL_RST_PRINT_UART_MASK                        0x00000001
#define CRG_PRINT_UART_CLKRST_CTRL_RST_PRINT_UART_SET(n)                      (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_PRINT_UART_CLKRST_CTRL_RST_PRINT_UART_CLR                         0xFFFFFFFE

#define CRG_IWDG_CLKRST_CTRL_PCLK_EN_IWDG_SHIFT                               1
#define CRG_IWDG_CLKRST_CTRL_PCLK_EN_IWDG_MASK                                0x00000002
#define CRG_IWDG_CLKRST_CTRL_PCLK_EN_IWDG_SET(n)                              (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_IWDG_CLKRST_CTRL_PCLK_EN_IWDG_CLR                                 0xFFFFFFFD

#define CRG_PWM_CLKRST_CTRL_FCLK_DIV_PWM_SHIFT                                4
#define CRG_PWM_CLKRST_CTRL_FCLK_DIV_PWM_MASK                                 0x000000F0
#define CRG_PWM_CLKRST_CTRL_FCLK_DIV_PWM_SET(n)                               (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_PWM_CLKRST_CTRL_FCLK_DIV_PWM_CLR                                  0xFFFFFF0F

#define CRG_PWM_CLKRST_CTRL_FCLK_SEL_PWM_SHIFT                                3
#define CRG_PWM_CLKRST_CTRL_FCLK_SEL_PWM_MASK                                 0x00000008
#define CRG_PWM_CLKRST_CTRL_FCLK_SEL_PWM_SET(n)                               (((uint32_t)(n) << 3  ) & 0x00000008)
#define CRG_PWM_CLKRST_CTRL_FCLK_SEL_PWM_CLR                                  0xFFFFFFF7

#define CRG_PWM_CLKRST_CTRL_FCLK_EN_PWM_SHIFT                                 2
#define CRG_PWM_CLKRST_CTRL_FCLK_EN_PWM_MASK                                  0x00000004
#define CRG_PWM_CLKRST_CTRL_FCLK_EN_PWM_SET(n)                                (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_PWM_CLKRST_CTRL_FCLK_EN_PWM_CLR                                   0xFFFFFFFB

#define CRG_PWM_CLKRST_CTRL_PCLK_EN_PWM_SHIFT                                 1
#define CRG_PWM_CLKRST_CTRL_PCLK_EN_PWM_MASK                                  0x00000002
#define CRG_PWM_CLKRST_CTRL_PCLK_EN_PWM_SET(n)                                (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_PWM_CLKRST_CTRL_PCLK_EN_PWM_CLR                                   0xFFFFFFFD

#define CRG_PWM_CLKRST_CTRL_RST_PWM_SHIFT                                     0
#define CRG_PWM_CLKRST_CTRL_RST_PWM_MASK                                      0x00000001
#define CRG_PWM_CLKRST_CTRL_RST_PWM_SET(n)                                    (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_PWM_CLKRST_CTRL_RST_PWM_CLR                                       0xFFFFFFFE

#define CRG_ADC_CLKRST_CTRL_FCLK_DIV_ADC_SHIFT                                4
#define CRG_ADC_CLKRST_CTRL_FCLK_DIV_ADC_MASK                                 0x000000F0
#define CRG_ADC_CLKRST_CTRL_FCLK_DIV_ADC_SET(n)                               (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_ADC_CLKRST_CTRL_FCLK_DIV_ADC_CLR                                  0xFFFFFF0F

#define CRG_ADC_CLKRST_CTRL_FCLK_EN_ADC_SHIFT                                 2
#define CRG_ADC_CLKRST_CTRL_FCLK_EN_ADC_MASK                                  0x00000004
#define CRG_ADC_CLKRST_CTRL_FCLK_EN_ADC_SET(n)                                (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_ADC_CLKRST_CTRL_FCLK_EN_ADC_CLR                                   0xFFFFFFFB

#define CRG_ADC_CLKRST_CTRL_PCLK_EN_ADC_SHIFT                                 1
#define CRG_ADC_CLKRST_CTRL_PCLK_EN_ADC_MASK                                  0x00000002
#define CRG_ADC_CLKRST_CTRL_PCLK_EN_ADC_SET(n)                                (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_ADC_CLKRST_CTRL_PCLK_EN_ADC_CLR                                   0xFFFFFFFD

#define CRG_ADC_CLKRST_CTRL_RST_ADC_SHIFT                                     0
#define CRG_ADC_CLKRST_CTRL_RST_ADC_MASK                                      0x00000001
#define CRG_ADC_CLKRST_CTRL_RST_ADC_SET(n)                                    (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_ADC_CLKRST_CTRL_RST_ADC_CLR                                       0xFFFFFFFE

#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_DIV_LIN_SCI_SHIFT                      4
#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_DIV_LIN_SCI_MASK                       0x000000F0
#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_DIV_LIN_SCI_SET(n)                     (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_DIV_LIN_SCI_CLR                        0xFFFFFF0F

#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_EN_LIN_SCI_SHIFT                       2
#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_EN_LIN_SCI_MASK                        0x00000004
#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_EN_LIN_SCI_SET(n)                      (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_LIN_SCI_CLKRST_CTRL_FCLK_EN_LIN_SCI_CLR                         0xFFFFFFFB

#define CRG_LIN_SCI_CLKRST_CTRL_PCLK_EN_LIN_SCI_SHIFT                       1
#define CRG_LIN_SCI_CLKRST_CTRL_PCLK_EN_LIN_SCI_MASK                        0x00000002
#define CRG_LIN_SCI_CLKRST_CTRL_PCLK_EN_LIN_SCI_SET(n)                      (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_LIN_SCI_CLKRST_CTRL_PCLK_EN_LIN_SCI_CLR                         0xFFFFFFFD

#define CRG_LIN_SCI_CLKRST_CTRL_RST_LIN_SCI_SHIFT                           0
#define CRG_LIN_SCI_CLKRST_CTRL_RST_LIN_SCI_MASK                            0x00000001
#define CRG_LIN_SCI_CLKRST_CTRL_RST_LIN_SCI_SET(n)                          (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_LIN_SCI_CLKRST_CTRL_RST_LIN_SCI_CLR                             0xFFFFFFFE

#define CRG_GPIO_CLKRST_CTRL_FCLK_DIV_GPIO_SHIFT                              4
#define CRG_GPIO_CLKRST_CTRL_FCLK_DIV_GPIO_MASK                               0x000000F0
#define CRG_GPIO_CLKRST_CTRL_FCLK_DIV_GPIO_SET(n)                             (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_GPIO_CLKRST_CTRL_FCLK_DIV_GPIO_CLR                                0xFFFFFF0F

#define CRG_GPIO_CLKRST_CTRL_FCLK_SEL_GPIO_SHIFT                              3
#define CRG_GPIO_CLKRST_CTRL_FCLK_SEL_GPIO_MASK                               0x00000008
#define CRG_GPIO_CLKRST_CTRL_FCLK_SEL_GPIO_SET(n)                             (((uint32_t)(n) << 3  ) & 0x00000008)
#define CRG_GPIO_CLKRST_CTRL_FCLK_SEL_GPIO_CLR                                0xFFFFFFF7

#define CRG_GPIO_CLKRST_CTRL_FCLK_EN_GPIO_SHIFT                               2
#define CRG_GPIO_CLKRST_CTRL_FCLK_EN_GPIO_MASK                                0x00000004
#define CRG_GPIO_CLKRST_CTRL_FCLK_EN_GPIO_SET(n)                              (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_GPIO_CLKRST_CTRL_FCLK_EN_GPIO_CLR                                 0xFFFFFFFB

#define CRG_GPIO_CLKRST_CTRL_PCLK_EN_GPIO_SHIFT                               1
#define CRG_GPIO_CLKRST_CTRL_PCLK_EN_GPIO_MASK                                0x00000002
#define CRG_GPIO_CLKRST_CTRL_PCLK_EN_GPIO_SET(n)                              (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_GPIO_CLKRST_CTRL_PCLK_EN_GPIO_CLR                                 0xFFFFFFFD

#define CRG_GPIO_CLKRST_CTRL_RST_GPIO_SHIFT                                   0
#define CRG_GPIO_CLKRST_CTRL_RST_GPIO_MASK                                    0x00000001
#define CRG_GPIO_CLKRST_CTRL_RST_GPIO_SET(n)                                  (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_GPIO_CLKRST_CTRL_RST_GPIO_CLR                                     0xFFFFFFFE

#define CRG_PINMUX_CLKRST_CTRL_PCLK_EN_PINMUX_SHIFT                           1
#define CRG_PINMUX_CLKRST_CTRL_PCLK_EN_PINMUX_MASK                            0x00000002
#define CRG_PINMUX_CLKRST_CTRL_PCLK_EN_PINMUX_SET(n)                          (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_PINMUX_CLKRST_CTRL_PCLK_EN_PINMUX_CLR                             0xFFFFFFFD

#define CRG_PINMUX_CLKRST_CTRL_RST_PINMUX_SHIFT                               0
#define CRG_PINMUX_CLKRST_CTRL_RST_PINMUX_MASK                                0x00000001
#define CRG_PINMUX_CLKRST_CTRL_RST_PINMUX_SET(n)                              (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_PINMUX_CLKRST_CTRL_RST_PINMUX_CLR                                 0xFFFFFFFE

#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_DIV_CAPTOUCH_SHIFT                            4
#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_DIV_CAPTOUCH_MASK                             0x000000F0
#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_DIV_CAPTOUCH_SET(n)                           (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_DIV_CAPTOUCH_CLR                              0xFFFFFF0F

#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_EN_CAPTOUCH_SHIFT                             2
#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_EN_CAPTOUCH_MASK                              0x00000004
#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_EN_CAPTOUCH_SET(n)                            (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_CAPTOUCH_CLKRST_CTRL_FCLK_EN_CAPTOUCH_CLR                               0xFFFFFFFB

#define CRG_CAPTOUCH_CLKRST_CTRL_PCLK_EN_CAPTOUCH_SHIFT                             1
#define CRG_CAPTOUCH_CLKRST_CTRL_PCLK_EN_CAPTOUCH_MASK                              0x00000002
#define CRG_CAPTOUCH_CLKRST_CTRL_PCLK_EN_CAPTOUCH_SET(n)                            (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_CAPTOUCH_CLKRST_CTRL_PCLK_EN_CAPTOUCH_CLR                               0xFFFFFFFD

#define CRG_CAPTOUCH_CLKRST_CTRL_RST_CAPTOUCH_SHIFT                                 0
#define CRG_CAPTOUCH_CLKRST_CTRL_RST_CAPTOUCH_MASK                                  0x00000001
#define CRG_CAPTOUCH_CLKRST_CTRL_RST_CAPTOUCH_SET(n)                                (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_CAPTOUCH_CLKRST_CTRL_RST_CAPTOUCH_CLR                                   0xFFFFFFFE

#define CRG_SPI_CLKRST_CTRL_FCLK_DIV_SPI_SHIFT                                4
#define CRG_SPI_CLKRST_CTRL_FCLK_DIV_SPI_MASK                                 0x000000F0
#define CRG_SPI_CLKRST_CTRL_FCLK_DIV_SPI_SET(n)                               (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_SPI_CLKRST_CTRL_FCLK_DIV_SPI_CLR                                  0xFFFFFF0F

#define CRG_SPI_CLKRST_CTRL_FCLK_SEL_SPI_SHIFT                                3
#define CRG_SPI_CLKRST_CTRL_FCLK_SEL_SPI_MASK                                 0x00000008
#define CRG_SPI_CLKRST_CTRL_FCLK_SEL_SPI_SET(n)                               (((uint32_t)(n) << 3  ) & 0x00000008)
#define CRG_SPI_CLKRST_CTRL_FCLK_SEL_SPI_CLR                                  0xFFFFFFF7

#define CRG_SPI_CLKRST_CTRL_FCLK_EN_SPI_SHIFT                                 2
#define CRG_SPI_CLKRST_CTRL_FCLK_EN_SPI_MASK                                  0x00000004
#define CRG_SPI_CLKRST_CTRL_FCLK_EN_SPI_SET(n)                                (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_SPI_CLKRST_CTRL_FCLK_EN_SPI_CLR                                   0xFFFFFFFB

#define CRG_SPI_CLKRST_CTRL_PCLK_EN_SPI_SHIFT                                 1
#define CRG_SPI_CLKRST_CTRL_PCLK_EN_SPI_MASK                                  0x00000002
#define CRG_SPI_CLKRST_CTRL_PCLK_EN_SPI_SET(n)                                (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_SPI_CLKRST_CTRL_PCLK_EN_SPI_CLR                                   0xFFFFFFFD

#define CRG_SPI_CLKRST_CTRL_RST_SPI_SHIFT                                     0
#define CRG_SPI_CLKRST_CTRL_RST_SPI_MASK                                      0x00000001
#define CRG_SPI_CLKRST_CTRL_RST_SPI_SET(n)                                    (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_SPI_CLKRST_CTRL_RST_SPI_CLR                                       0xFFFFFFFE

#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_SHIFT                      4
#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_MASK                       0x000000F0
#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_SET(n)                     (((uint32_t)(n) << 4  ) & 0x000000F0)
#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_CLR                        0xFFFFFF0F

#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_EN_LIN_SCI1_SHIFT                       2
#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_EN_LIN_SCI1_MASK                        0x00000004
#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_EN_LIN_SCI1_SET(n)                      (((uint32_t)(n) << 2  ) & 0x00000004)
#define CRG_LIN_SCI1_CLKRST_CTRL_FCLK_EN_LIN_SCI1_CLR                         0xFFFFFFFB

#define CRG_LIN_SCI1_CLKRST_CTRL_PCLK_EN_LIN_SCI1_SHIFT                       1
#define CRG_LIN_SCI1_CLKRST_CTRL_PCLK_EN_LIN_SCI1_MASK                        0x00000002
#define CRG_LIN_SCI1_CLKRST_CTRL_PCLK_EN_LIN_SCI1_SET(n)                      (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_LIN_SCI1_CLKRST_CTRL_PCLK_EN_LIN_SCI1_CLR                         0xFFFFFFFD

#define CRG_LIN_SCI1_CLKRST_CTRL_RST_LIN_SCI1_SHIFT                           0
#define CRG_LIN_SCI1_CLKRST_CTRL_RST_LIN_SCI1_MASK                            0x00000001
#define CRG_LIN_SCI1_CLKRST_CTRL_RST_LIN_SCI1_SET(n)                          (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_LIN_SCI1_CLKRST_CTRL_RST_LIN_SCI1_CLR                             0xFFFFFFFE

#define CRG_HRC_SSG_CTRL_SSG_UPDT_BUSY_SHIFT                                  30
#define CRG_HRC_SSG_CTRL_SSG_UPDT_BUSY_MASK                                   0x40000000
#define CRG_HRC_SSG_CTRL_SSG_UPDT_BUSY_SET(n)                                 (((uint32_t)(n) << 30 ) & 0x40000000)
#define CRG_HRC_SSG_CTRL_SSG_UPDT_BUSY_CLR                                    0xBFFFFFFF

#define CRG_HRC_SSG_CTRL_SSG_SOFT_OUT_SHIFT                                   24
#define CRG_HRC_SSG_CTRL_SSG_SOFT_OUT_MASK                                    0x3F000000
#define CRG_HRC_SSG_CTRL_SSG_SOFT_OUT_SET(n)                                  (((uint32_t)(n) << 24 ) & 0x3F000000)
#define CRG_HRC_SSG_CTRL_SSG_SOFT_OUT_CLR                                     0xC0FFFFFF

#define CRG_HRC_SSG_CTRL_SSG_BETA_SHIFT                                       16
#define CRG_HRC_SSG_CTRL_SSG_BETA_MASK                                        0x001F0000
#define CRG_HRC_SSG_CTRL_SSG_BETA_SET(n)                                      (((uint32_t)(n) << 16 ) & 0x001F0000)
#define CRG_HRC_SSG_CTRL_SSG_BETA_CLR                                         0xFFE0FFFF

#define CRG_HRC_SSG_CTRL_SSG_ALPHA_SHIFT                                      8
#define CRG_HRC_SSG_CTRL_SSG_ALPHA_MASK                                       0x00001F00
#define CRG_HRC_SSG_CTRL_SSG_ALPHA_SET(n)                                     (((uint32_t)(n) << 8  ) & 0x00001F00)
#define CRG_HRC_SSG_CTRL_SSG_ALPHA_CLR                                        0xFFFFE0FF

#define CRG_HRC_SSG_CTRL_SSG_MOD_FREQ_SEL_SHIFT                               4
#define CRG_HRC_SSG_CTRL_SSG_MOD_FREQ_SEL_MASK                                0x00000070
#define CRG_HRC_SSG_CTRL_SSG_MOD_FREQ_SEL_SET(n)                              (((uint32_t)(n) << 4  ) & 0x00000070)
#define CRG_HRC_SSG_CTRL_SSG_MOD_FREQ_SEL_CLR                                 0xFFFFFF8F

#define CRG_HRC_SSG_CTRL_SSG_AMP_SHIFT                                        2
#define CRG_HRC_SSG_CTRL_SSG_AMP_MASK                                         0x0000000C
#define CRG_HRC_SSG_CTRL_SSG_AMP_SET(n)                                       (((uint32_t)(n) << 2  ) & 0x0000000C)
#define CRG_HRC_SSG_CTRL_SSG_AMP_CLR                                          0xFFFFFFF3

#define CRG_HRC_SSG_CTRL_SSG_SOFT_EN_SHIFT                                    1
#define CRG_HRC_SSG_CTRL_SSG_SOFT_EN_MASK                                     0x00000002
#define CRG_HRC_SSG_CTRL_SSG_SOFT_EN_SET(n)                                   (((uint32_t)(n) << 1  ) & 0x00000002)
#define CRG_HRC_SSG_CTRL_SSG_SOFT_EN_CLR                                      0xFFFFFFFD

#define CRG_HRC_SSG_CTRL_SSG_EN_SHIFT                                         0
#define CRG_HRC_SSG_CTRL_SSG_EN_MASK                                          0x00000001
#define CRG_HRC_SSG_CTRL_SSG_EN_SET(n)                                        (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_HRC_SSG_CTRL_SSG_EN_CLR                                           0xFFFFFFFE

#define CRG_CRG_LOCK_CRG_LOCK_SHIFT                                           0
#define CRG_CRG_LOCK_CRG_LOCK_MASK                                            0x00000001
#define CRG_CRG_LOCK_CRG_LOCK_SET(n)                                          (((uint32_t)(n) << 0  ) & 0x00000001)
#define CRG_CRG_LOCK_CRG_LOCK_CLR                                             0xFFFFFFFE

// Register Structure Definition
typedef struct
{
    __IO uint32_t HCLK_SEL           : 1  ; // 0  : 0
    __IO uint32_t RESERVED1          : 2  ; // 2  : 1
    __IO uint32_t HCLK_DIV_LOAD      : 1  ; // 3  : 3
    __IO uint32_t HCLK_DIV           : 4  ; // 7  : 4
    __IO uint32_t AUTO_HCLK_FORCE_EN : 1  ; // 8  : 8
    __IO uint32_t RESERVED0          : 23 ; // 31 : 9
} CRG_HCLK_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t PCLK_DIV           : 3  ; // 2  : 0
    __IO uint32_t PCLK_DIV_LOAD      : 1  ; // 3  : 3
    __IO uint32_t RESERVED1          : 4  ; // 7  : 4
    __IO uint32_t PCLK_EN_TEST       : 1  ; // 8  : 8
    __IO uint32_t PCLK_EN_ASYSCFG    : 1  ; // 9  : 9
    __IO uint32_t RESERVED0          : 22 ; // 31 : 10
} CRG_PCLK_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_M0             : 1  ; // 0  : 0
    __IO uint32_t RESERVED2          : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_SYSTICK    : 1  ; // 2  : 2
    __IO uint32_t RESERVED1          : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_SYSTICK   : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_M0_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_TIM_LITE       : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_TIM_LITE   : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_TIM_LITE   : 1  ; // 2  : 2
    __IO uint32_t FCLK_SEL_TIM_LITE  : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_TIM_LITE  : 8  ; // 11 : 4
    __IO uint32_t RESERVED0          : 20 ; // 31 : 12
} CRG_TIM_LITE_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_PRINT_UART     : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_PRINT_UART : 1  ; // 1  : 1
    __IO uint32_t RESERVED0          : 30 ; // 31 : 2
} CRG_PRINT_UART_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RESERVED1          : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_IWDG       : 1  ; // 1  : 1
    __IO uint32_t RESERVED0          : 30 ; // 31 : 2
} CRG_IWDG_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_PWM            : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_PWM        : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_PWM        : 1  ; // 2  : 2
    __IO uint32_t FCLK_SEL_PWM       : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_PWM       : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_PWM_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_ADC            : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_ADC        : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_ADC        : 1  ; // 2  : 2
    __IO uint32_t RESERVED1          : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_ADC       : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_ADC_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_LIN_SCI       : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_LIN_SCI   : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_LIN_SCI   : 1  ; // 2  : 2
    __IO uint32_t RESERVED1          : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_LIN_SCI  : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_LIN_SCI_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_GPIO           : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_GPIO       : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_GPIO       : 1  ; // 2  : 2
    __IO uint32_t FCLK_SEL_GPIO      : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_GPIO      : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_GPIO_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_PINMUX         : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_PINMUX     : 1  ; // 1  : 1
    __IO uint32_t RESERVED0          : 30 ; // 31 : 2
} CRG_PINMUX_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_CAPTOUCH          : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_CAPTOUCH      : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_CAPTOUCH      : 1  ; // 2  : 2
    __IO uint32_t RESERVED1          : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_CAPTOUCH     : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_CAPTOUCH_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_SPI            : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_SPI        : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_SPI        : 1  ; // 2  : 2
    __IO uint32_t FCLK_SEL_SPI       : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_SPI       : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_SPI_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t RST_LIN_SCI1       : 1  ; // 0  : 0
    __IO uint32_t PCLK_EN_LIN_SCI1   : 1  ; // 1  : 1
    __IO uint32_t FCLK_EN_LIN_SCI1   : 1  ; // 2  : 2
    __IO uint32_t RESERVED1          : 1  ; // 3  : 3
    __IO uint32_t FCLK_DIV_LIN_SCI1  : 4  ; // 7  : 4
    __IO uint32_t RESERVED0          : 24 ; // 31 : 8
} CRG_LIN_SCI1_CLKRST_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t SSG_EN             : 1  ; // 0  : 0
    __IO uint32_t SSG_SOFT_EN        : 1  ; // 1  : 1
    __IO uint32_t SSG_AMP            : 2  ; // 3  : 2
    __IO uint32_t SSG_MOD_FREQ_SEL   : 3  ; // 6  : 4
    __IO uint32_t RESERVED3          : 1  ; // 7  : 7
    __IO uint32_t SSG_ALPHA          : 5  ; // 12 : 8
    __IO uint32_t RESERVED2          : 3  ; // 15 : 13
    __IO uint32_t SSG_BETA           : 5  ; // 20 : 16
    __IO uint32_t RESERVED1          : 3  ; // 23 : 21
    __IO uint32_t SSG_SOFT_OUT       : 6  ; // 29 : 24
    __IO uint32_t SSG_UPDT_BUSY      : 1  ; // 30 : 30
    __IO uint32_t RESERVED0          : 1  ; // 31 : 31
} CRG_HRC_SSG_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t CRG_LOCK           : 1  ; // 0  : 0
    __IO uint32_t RESERVED0          : 31 ; // 31 : 1
} CRG_CRG_LOCK_FIELD_T;

// Register Map Structure Definition
typedef struct
{
    union
    {
        __IO  uint32_t HCLK_CTRL                                       ; // 0x0000
        CRG_HCLK_CTRL_FIELD_T HCLK_CTRL_F                              ;
    };

    union
    {
        __IO  uint32_t PCLK_CTRL                                       ; // 0x0004
        CRG_PCLK_CTRL_FIELD_T PCLK_CTRL_F                              ;
    };

    union
    {
        __IO  uint32_t M0_CLKRST_CTRL                                  ; // 0x0008
        CRG_M0_CLKRST_CTRL_FIELD_T M0_CLKRST_CTRL_F                    ;
    };

    union
    {
        __IO  uint32_t TIM_LITE_CLKRST_CTRL                            ; // 0x000C
        CRG_TIM_LITE_CLKRST_CTRL_FIELD_T TIM_LITE_CLKRST_CTRL_F        ;
    };

    union
    {
        __IO  uint32_t PRINT_UART_CLKRST_CTRL                          ; // 0x0010
        CRG_PRINT_UART_CLKRST_CTRL_FIELD_T PRINT_UART_CLKRST_CTRL_F    ;
    };

    union
    {
        __IO  uint32_t IWDG_CLKRST_CTRL                                ; // 0x0014
        CRG_IWDG_CLKRST_CTRL_FIELD_T IWDG_CLKRST_CTRL_F                ;
    };

    union
    {
        __IO  uint32_t PWM_CLKRST_CTRL                                 ; // 0x0018
        CRG_PWM_CLKRST_CTRL_FIELD_T PWM_CLKRST_CTRL_F                  ;
    };

    union
    {
        __IO  uint32_t ADC_CLKRST_CTRL                                 ; // 0x001C
        CRG_ADC_CLKRST_CTRL_FIELD_T ADC_CLKRST_CTRL_F                  ;
    };

    union
    {
        __IO  uint32_t LIN_SCI_CLKRST_CTRL                            ; // 0x0020
        CRG_LIN_SCI_CLKRST_CTRL_FIELD_T LIN_SCI_CLKRST_CTRL_F        ;
    };

    union
    {
        __IO  uint32_t GPIO_CLKRST_CTRL                                ; // 0x0024
        CRG_GPIO_CLKRST_CTRL_FIELD_T GPIO_CLKRST_CTRL_F                ;
    };

    union
    {
        __IO  uint32_t PINMUX_CLKRST_CTRL                              ; // 0x0028
        CRG_PINMUX_CLKRST_CTRL_FIELD_T PINMUX_CLKRST_CTRL_F            ;
    };

    union
    {
        __IO  uint32_t CAPTOUCH_CLKRST_CTRL                               ; // 0x002C
        CRG_CAPTOUCH_CLKRST_CTRL_FIELD_T CAPTOUCH_CLKRST_CTRL_F              ;
    };

    union
    {
        __IO  uint32_t SPI_CLKRST_CTRL                                 ; // 0x0030
        CRG_SPI_CLKRST_CTRL_FIELD_T SPI_CLKRST_CTRL_F                  ;
    };

    union
    {
        __IO  uint32_t LIN_SCI1_CLKRST_CTRL                            ; // 0x0034
        CRG_LIN_SCI1_CLKRST_CTRL_FIELD_T LIN_SCI1_CLKRST_CTRL_F        ;
    };

    union
    {
        __IO  uint32_t HRC_SSG_CTRL                                    ; // 0x0038
        CRG_HRC_SSG_CTRL_FIELD_T HRC_SSG_CTRL_F                        ;
    };

    uint32_t RESERVED0[48]                                              ;

    union
    {
        __IO  uint32_t CRG_LOCK                                        ; // 0x00FC
        CRG_CRG_LOCK_FIELD_T CRG_LOCK_F                                ;
    };

} CRG_REG_TypeDef;

#endif
