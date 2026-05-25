/**
 *****************************************************************************
 * @brief   pwm_reg_def header file.
 *
 * @file    pwm_reg_def.h
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

#ifndef   FFF_PWM_DEF_H__
#define   FFF_PWM_DEF_H__

// Register Offset Macro Definition
#define PWM_CTRL_OFFSET                                         0x0000
#define PWM_CNT_CTRL_OFFSET                                     0x0004
#define PWM_CNT_CFG_OFFSET                                      0x0008
#define PWM_CH_CTRL_OFFSET                                      0x000C
#define PWM_CH0_PWM_CFG_OFFSET                                  0x0010
#define PWM_CH1_PWM_CFG_OFFSET                                  0x0014
#define PWM_CH2_PWM_CFG_OFFSET                                  0x0018
#define PWM_CH3_PWM_CFG_OFFSET                                  0x001C
#define PWM_CNT_VAL_OFFSET                                      0x0020
#define PWM_STATUS_OFFSET                                       0x0024
#define PWM_IMR_OFFSET                                          0x0028
#define PWM_ICR_OFFSET                                          0x002C
#define PWM_ISR_OFFSET                                          0x0030
#define PWM_IRSR_OFFSET                                         0x0034
#define PWM_LED_CTRL_OFFSET                                     0x0038
#define PWM_LED_LC0_CTRL_OFFSET                                 0x003C
#define PWM_LED_LC1_CTRL_OFFSET                                 0x0040
#define PWM_LED_LC2_CTRL_OFFSET                                 0x0044
#define PWM_WR_LOCK_OFFSET                                      0x00FC

// Register Address Macro Definition
#define PWM_CTRL_ADDR                                           ( PWM_BASE_ADDR + PWM_CTRL_OFFSET)
#define PWM_CNT_CTRL_ADDR                                       ( PWM_BASE_ADDR + PWM_CNT_CTRL_OFFSET)
#define PWM_CNT_CFG_ADDR                                        ( PWM_BASE_ADDR + PWM_CNT_CFG_OFFSET)
#define PWM_CH_CTRL_ADDR                                        ( PWM_BASE_ADDR + PWM_CH_CTRL_OFFSET)
#define PWM_CH0_PWM_CFG_ADDR                                    ( PWM_BASE_ADDR + PWM_CH0_PWM_CFG_OFFSET)
#define PWM_CH1_PWM_CFG_ADDR                                    ( PWM_BASE_ADDR + PWM_CH1_PWM_CFG_OFFSET)
#define PWM_CH2_PWM_CFG_ADDR                                    ( PWM_BASE_ADDR + PWM_CH2_PWM_CFG_OFFSET)
#define PWM_CH3_PWM_CFG_ADDR                                    ( PWM_BASE_ADDR + PWM_CH3_PWM_CFG_OFFSET)
#define PWM_CNT_VAL_ADDR                                        ( PWM_BASE_ADDR + PWM_CNT_VAL_OFFSET)
#define PWM_STATUS_ADDR                                         ( PWM_BASE_ADDR + PWM_STATUS_OFFSET)
#define PWM_IMR_ADDR                                            ( PWM_BASE_ADDR + PWM_IMR_OFFSET)
#define PWM_ICR_ADDR                                            ( PWM_BASE_ADDR + PWM_ICR_OFFSET)
#define PWM_ISR_ADDR                                            ( PWM_BASE_ADDR + PWM_ISR_OFFSET)
#define PWM_IRSR_ADDR                                           ( PWM_BASE_ADDR + PWM_IRSR_OFFSET)
#define PWM_LED_CTRL_ADDR                                       ( PWM_BASE_ADDR + PWM_LED_CTRL_OFFSET)
#define PWM_LED_LC0_CTRL_ADDR                                   ( PWM_BASE_ADDR + PWM_LED_LC0_CTRL_OFFSET)
#define PWM_LED_LC1_CTRL_ADDR                                   ( PWM_BASE_ADDR + PWM_LED_LC1_CTRL_OFFSET)
#define PWM_LED_LC2_CTRL_ADDR                                   ( PWM_BASE_ADDR + PWM_LED_LC2_CTRL_OFFSET)
#define PWM_WR_LOCK_ADDR                                        ( PWM_BASE_ADDR + PWM_WR_LOCK_OFFSET)

// Register Field Macro Definition
#define PWM_CTRL_PWM_STP_SHIFT                                               16
#define PWM_CTRL_PWM_STP_MASK                                                0x00010000
#define PWM_CTRL_PWM_STP_SET(n)                                              (((uint32_t)(n) << 16 ) & 0x00010000)
#define PWM_CTRL_PWM_STP_CLR                                                 0xFFFEFFFF

#define PWM_CTRL_CH_SYNC_SEL_SHIFT                                           13
#define PWM_CTRL_CH_SYNC_SEL_MASK                                            0x00006000
#define PWM_CTRL_CH_SYNC_SEL_SET(n)                                          (((uint32_t)(n) << 13 ) & 0x00006000)
#define PWM_CTRL_CH_SYNC_SEL_CLR                                             0xFFFF9FFF

#define PWM_CTRL_CH_SYNC_EN_SHIFT                                            12
#define PWM_CTRL_CH_SYNC_EN_MASK                                             0x00001000
#define PWM_CTRL_CH_SYNC_EN_SET(n)                                           (((uint32_t)(n) << 12 ) & 0x00001000)
#define PWM_CTRL_CH_SYNC_EN_CLR                                              0xFFFFEFFF

#define PWM_CTRL_BRK_EN_SHIFT                                                8
#define PWM_CTRL_BRK_EN_MASK                                                 0x00000100
#define PWM_CTRL_BRK_EN_SET(n)                                               (((uint32_t)(n) << 8  ) & 0x00000100)
#define PWM_CTRL_BRK_EN_CLR                                                  0xFFFFFEFF

#define PWM_CTRL_LD_TRIG_SHIFT                                               4
#define PWM_CTRL_LD_TRIG_MASK                                                0x00000010
#define PWM_CTRL_LD_TRIG_SET(n)                                              (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_CTRL_LD_TRIG_CLR                                                 0xFFFFFFEF

#define PWM_CTRL_AUTO_LD_EN_SHIFT                                            2
#define PWM_CTRL_AUTO_LD_EN_MASK                                             0x00000004
#define PWM_CTRL_AUTO_LD_EN_SET(n)                                           (((uint32_t)(n) << 2  ) & 0x00000004)
#define PWM_CTRL_AUTO_LD_EN_CLR                                              0xFFFFFFFB

#define PWM_CTRL_PWM_SW_EN_SHIFT                                             1
#define PWM_CTRL_PWM_SW_EN_MASK                                              0x00000002
#define PWM_CTRL_PWM_SW_EN_SET(n)                                            (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_CTRL_PWM_SW_EN_CLR                                               0xFFFFFFFD

#define PWM_CTRL_SYNC_EN_SHIFT                                               0
#define PWM_CTRL_SYNC_EN_MASK                                                0x00000001
#define PWM_CTRL_SYNC_EN_SET(n)                                              (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_CTRL_SYNC_EN_CLR                                                 0xFFFFFFFE

#define PWM_CNT_CTRL_CNT1_EN_MODE_SHIFT                                      13
#define PWM_CNT_CTRL_CNT1_EN_MODE_MASK                                       0x00002000
#define PWM_CNT_CTRL_CNT1_EN_MODE_SET(n)                                     (((uint32_t)(n) << 13 ) & 0x00002000)
#define PWM_CNT_CTRL_CNT1_EN_MODE_CLR                                        0xFFFFDFFF

#define PWM_CNT_CTRL_CNT0_EN_MODE_SHIFT                                      12
#define PWM_CNT_CTRL_CNT0_EN_MODE_MASK                                       0x00001000
#define PWM_CNT_CTRL_CNT0_EN_MODE_SET(n)                                     (((uint32_t)(n) << 12 ) & 0x00001000)
#define PWM_CNT_CTRL_CNT0_EN_MODE_CLR                                        0xFFFFEFFF

#define PWM_CNT_CTRL_CNT1_ONE_SHOT_TRIG_SHIFT                                9
#define PWM_CNT_CTRL_CNT1_ONE_SHOT_TRIG_MASK                                 0x00000200
#define PWM_CNT_CTRL_CNT1_ONE_SHOT_TRIG_SET(n)                               (((uint32_t)(n) << 9  ) & 0x00000200)
#define PWM_CNT_CTRL_CNT1_ONE_SHOT_TRIG_CLR                                  0xFFFFFDFF

#define PWM_CNT_CTRL_CNT0_ONE_SHOT_TRIG_SHIFT                                8
#define PWM_CNT_CTRL_CNT0_ONE_SHOT_TRIG_MASK                                 0x00000100
#define PWM_CNT_CTRL_CNT0_ONE_SHOT_TRIG_SET(n)                               (((uint32_t)(n) << 8  ) & 0x00000100)
#define PWM_CNT_CTRL_CNT0_ONE_SHOT_TRIG_CLR                                  0xFFFFFEFF

#define PWM_CNT_CTRL_CNT1_ONE_SHOT_EN_SHIFT                                  7
#define PWM_CNT_CTRL_CNT1_ONE_SHOT_EN_MASK                                   0x00000080
#define PWM_CNT_CTRL_CNT1_ONE_SHOT_EN_SET(n)                                 (((uint32_t)(n) << 7  ) & 0x00000080)
#define PWM_CNT_CTRL_CNT1_ONE_SHOT_EN_CLR                                    0xFFFFFF7F

#define PWM_CNT_CTRL_CNT0_ONE_SHOT_EN_SHIFT                                  6
#define PWM_CNT_CTRL_CNT0_ONE_SHOT_EN_MASK                                   0x00000040
#define PWM_CNT_CTRL_CNT0_ONE_SHOT_EN_SET(n)                                 (((uint32_t)(n) << 6  ) & 0x00000040)
#define PWM_CNT_CTRL_CNT0_ONE_SHOT_EN_CLR                                    0xFFFFFFBF

#define PWM_CNT_CTRL_CNT1_MODE_SHIFT                                         5
#define PWM_CNT_CTRL_CNT1_MODE_MASK                                          0x00000020
#define PWM_CNT_CTRL_CNT1_MODE_SET(n)                                        (((uint32_t)(n) << 5  ) & 0x00000020)
#define PWM_CNT_CTRL_CNT1_MODE_CLR                                           0xFFFFFFDF

#define PWM_CNT_CTRL_CNT0_MODE_SHIFT                                         4
#define PWM_CNT_CTRL_CNT0_MODE_MASK                                          0x00000010
#define PWM_CNT_CTRL_CNT0_MODE_SET(n)                                        (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_CNT_CTRL_CNT0_MODE_CLR                                           0xFFFFFFEF

#define PWM_CNT_CTRL_CNT1_EN_SHIFT                                           1
#define PWM_CNT_CTRL_CNT1_EN_MASK                                            0x00000002
#define PWM_CNT_CTRL_CNT1_EN_SET(n)                                          (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_CNT_CTRL_CNT1_EN_CLR                                             0xFFFFFFFD

#define PWM_CNT_CTRL_CNT0_EN_SHIFT                                           0
#define PWM_CNT_CTRL_CNT0_EN_MASK                                            0x00000001
#define PWM_CNT_CTRL_CNT0_EN_SET(n)                                          (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_CNT_CTRL_CNT0_EN_CLR                                             0xFFFFFFFE

#define PWM_CNT_CFG_PERIOD1_SHIFT                                            16
#define PWM_CNT_CFG_PERIOD1_MASK                                             0xFFFF0000
#define PWM_CNT_CFG_PERIOD1_SET(n)                                           (((uint32_t)(n) << 16 ) & 0xFFFF0000)
#define PWM_CNT_CFG_PERIOD1_CLR                                              0x0000FFFF

#define PWM_CNT_CFG_PERIOD0_SHIFT                                            0
#define PWM_CNT_CFG_PERIOD0_MASK                                             0x0000FFFF
#define PWM_CNT_CFG_PERIOD0_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x0000FFFF)
#define PWM_CNT_CFG_PERIOD0_CLR                                              0xFFFF0000

#define PWM_CH_CTRL_CH3_CNT_SEL_SHIFT                                        19
#define PWM_CH_CTRL_CH3_CNT_SEL_MASK                                         0x00080000
#define PWM_CH_CTRL_CH3_CNT_SEL_SET(n)                                       (((uint32_t)(n) << 19 ) & 0x00080000)
#define PWM_CH_CTRL_CH3_CNT_SEL_CLR                                          0xFFF7FFFF

#define PWM_CH_CTRL_CH2_CNT_SEL_SHIFT                                        18
#define PWM_CH_CTRL_CH2_CNT_SEL_MASK                                         0x00040000
#define PWM_CH_CTRL_CH2_CNT_SEL_SET(n)                                       (((uint32_t)(n) << 18 ) & 0x00040000)
#define PWM_CH_CTRL_CH2_CNT_SEL_CLR                                          0xFFFBFFFF

#define PWM_CH_CTRL_CH1_CNT_SEL_SHIFT                                        17
#define PWM_CH_CTRL_CH1_CNT_SEL_MASK                                         0x00020000
#define PWM_CH_CTRL_CH1_CNT_SEL_SET(n)                                       (((uint32_t)(n) << 17 ) & 0x00020000)
#define PWM_CH_CTRL_CH1_CNT_SEL_CLR                                          0xFFFDFFFF

#define PWM_CH_CTRL_CH0_CNT_SEL_SHIFT                                        16
#define PWM_CH_CTRL_CH0_CNT_SEL_MASK                                         0x00010000
#define PWM_CH_CTRL_CH0_CNT_SEL_SET(n)                                       (((uint32_t)(n) << 16 ) & 0x00010000)
#define PWM_CH_CTRL_CH0_CNT_SEL_CLR                                          0xFFFEFFFF

#define PWM_CH_CTRL_CH3_MODE_SHIFT                                           14
#define PWM_CH_CTRL_CH3_MODE_MASK                                            0x0000C000
#define PWM_CH_CTRL_CH3_MODE_SET(n)                                          (((uint32_t)(n) << 14 ) & 0x0000C000)
#define PWM_CH_CTRL_CH3_MODE_CLR                                             0xFFFF3FFF

#define PWM_CH_CTRL_CH2_MODE_SHIFT                                           12
#define PWM_CH_CTRL_CH2_MODE_MASK                                            0x00003000
#define PWM_CH_CTRL_CH2_MODE_SET(n)                                          (((uint32_t)(n) << 12 ) & 0x00003000)
#define PWM_CH_CTRL_CH2_MODE_CLR                                             0xFFFFCFFF

#define PWM_CH_CTRL_CH1_MODE_SHIFT                                           10
#define PWM_CH_CTRL_CH1_MODE_MASK                                            0x00000C00
#define PWM_CH_CTRL_CH1_MODE_SET(n)                                          (((uint32_t)(n) << 10 ) & 0x00000C00)
#define PWM_CH_CTRL_CH1_MODE_CLR                                             0xFFFFF3FF

#define PWM_CH_CTRL_CH0_MODE_SHIFT                                           8
#define PWM_CH_CTRL_CH0_MODE_MASK                                            0x00000300
#define PWM_CH_CTRL_CH0_MODE_SET(n)                                          (((uint32_t)(n) << 8  ) & 0x00000300)
#define PWM_CH_CTRL_CH0_MODE_CLR                                             0xFFFFFCFF

#define PWM_CH_CTRL_CH3_PTY_SHIFT                                            7
#define PWM_CH_CTRL_CH3_PTY_MASK                                             0x00000080
#define PWM_CH_CTRL_CH3_PTY_SET(n)                                           (((uint32_t)(n) << 7  ) & 0x00000080)
#define PWM_CH_CTRL_CH3_PTY_CLR                                              0xFFFFFF7F

#define PWM_CH_CTRL_CH2_PTY_SHIFT                                            6
#define PWM_CH_CTRL_CH2_PTY_MASK                                             0x00000040
#define PWM_CH_CTRL_CH2_PTY_SET(n)                                           (((uint32_t)(n) << 6  ) & 0x00000040)
#define PWM_CH_CTRL_CH2_PTY_CLR                                              0xFFFFFFBF

#define PWM_CH_CTRL_CH1_PTY_SHIFT                                            5
#define PWM_CH_CTRL_CH1_PTY_MASK                                             0x00000020
#define PWM_CH_CTRL_CH1_PTY_SET(n)                                           (((uint32_t)(n) << 5  ) & 0x00000020)
#define PWM_CH_CTRL_CH1_PTY_CLR                                              0xFFFFFFDF

#define PWM_CH_CTRL_CH0_PTY_SHIFT                                            4
#define PWM_CH_CTRL_CH0_PTY_MASK                                             0x00000010
#define PWM_CH_CTRL_CH0_PTY_SET(n)                                           (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_CH_CTRL_CH0_PTY_CLR                                              0xFFFFFFEF

#define PWM_CH_CTRL_CH3_EN_SHIFT                                             3
#define PWM_CH_CTRL_CH3_EN_MASK                                              0x00000008
#define PWM_CH_CTRL_CH3_EN_SET(n)                                            (((uint32_t)(n) << 3  ) & 0x00000008)
#define PWM_CH_CTRL_CH3_EN_CLR                                               0xFFFFFFF7

#define PWM_CH_CTRL_CH2_EN_SHIFT                                             2
#define PWM_CH_CTRL_CH2_EN_MASK                                              0x00000004
#define PWM_CH_CTRL_CH2_EN_SET(n)                                            (((uint32_t)(n) << 2  ) & 0x00000004)
#define PWM_CH_CTRL_CH2_EN_CLR                                               0xFFFFFFFB

#define PWM_CH_CTRL_CH1_EN_SHIFT                                             1
#define PWM_CH_CTRL_CH1_EN_MASK                                              0x00000002
#define PWM_CH_CTRL_CH1_EN_SET(n)                                            (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_CH_CTRL_CH1_EN_CLR                                               0xFFFFFFFD

#define PWM_CH_CTRL_CH0_EN_SHIFT                                             0
#define PWM_CH_CTRL_CH0_EN_MASK                                              0x00000001
#define PWM_CH_CTRL_CH0_EN_SET(n)                                            (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_CH_CTRL_CH0_EN_CLR                                               0xFFFFFFFE

#define PWM_CH0_PWM_CFG_HT0_SHIFT                                            16
#define PWM_CH0_PWM_CFG_HT0_MASK                                             0xFFFF0000
#define PWM_CH0_PWM_CFG_HT0_SET(n)                                           (((uint32_t)(n) << 16 ) & 0xFFFF0000)
#define PWM_CH0_PWM_CFG_HT0_CLR                                              0x0000FFFF

#define PWM_CH0_PWM_CFG_LT0_SHIFT                                            0
#define PWM_CH0_PWM_CFG_LT0_MASK                                             0x0000FFFF
#define PWM_CH0_PWM_CFG_LT0_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x0000FFFF)
#define PWM_CH0_PWM_CFG_LT0_CLR                                              0xFFFF0000

#define PWM_CH1_PWM_CFG_HT1_SHIFT                                            16
#define PWM_CH1_PWM_CFG_HT1_MASK                                             0xFFFF0000
#define PWM_CH1_PWM_CFG_HT1_SET(n)                                           (((uint32_t)(n) << 16 ) & 0xFFFF0000)
#define PWM_CH1_PWM_CFG_HT1_CLR                                              0x0000FFFF

#define PWM_CH1_PWM_CFG_LT1_SHIFT                                            0
#define PWM_CH1_PWM_CFG_LT1_MASK                                             0x0000FFFF
#define PWM_CH1_PWM_CFG_LT1_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x0000FFFF)
#define PWM_CH1_PWM_CFG_LT1_CLR                                              0xFFFF0000

#define PWM_CH2_PWM_CFG_HT2_SHIFT                                            16
#define PWM_CH2_PWM_CFG_HT2_MASK                                             0xFFFF0000
#define PWM_CH2_PWM_CFG_HT2_SET(n)                                           (((uint32_t)(n) << 16 ) & 0xFFFF0000)
#define PWM_CH2_PWM_CFG_HT2_CLR                                              0x0000FFFF

#define PWM_CH2_PWM_CFG_LT2_SHIFT                                            0
#define PWM_CH2_PWM_CFG_LT2_MASK                                             0x0000FFFF
#define PWM_CH2_PWM_CFG_LT2_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x0000FFFF)
#define PWM_CH2_PWM_CFG_LT2_CLR                                              0xFFFF0000

#define PWM_CH3_PWM_CFG_HT3_SHIFT                                            16
#define PWM_CH3_PWM_CFG_HT3_MASK                                             0xFFFF0000
#define PWM_CH3_PWM_CFG_HT3_SET(n)                                           (((uint32_t)(n) << 16 ) & 0xFFFF0000)
#define PWM_CH3_PWM_CFG_HT3_CLR                                              0x0000FFFF

#define PWM_CH3_PWM_CFG_LT3_SHIFT                                            0
#define PWM_CH3_PWM_CFG_LT3_MASK                                             0x0000FFFF
#define PWM_CH3_PWM_CFG_LT3_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x0000FFFF)
#define PWM_CH3_PWM_CFG_LT3_CLR                                              0xFFFF0000

#define PWM_CNT_VAL_CNT1_VAL_SHIFT                                           16
#define PWM_CNT_VAL_CNT1_VAL_MASK                                            0xFFFF0000
#define PWM_CNT_VAL_CNT1_VAL_SET(n)                                          (((uint32_t)(n) << 16 ) & 0xFFFF0000)
#define PWM_CNT_VAL_CNT1_VAL_CLR                                             0x0000FFFF

#define PWM_CNT_VAL_CNT0_VAL_SHIFT                                           0
#define PWM_CNT_VAL_CNT0_VAL_MASK                                            0x0000FFFF
#define PWM_CNT_VAL_CNT0_VAL_SET(n)                                          (((uint32_t)(n) << 0  ) & 0x0000FFFF)
#define PWM_CNT_VAL_CNT0_VAL_CLR                                             0xFFFF0000

#define PWM_STATUS_LDO5V_OCP_SHIFT                                           5
#define PWM_STATUS_LDO5V_OCP_MASK                                            0x00000020
#define PWM_STATUS_LDO5V_OCP_SET(n)                                          (((uint32_t)(n) << 5  ) & 0x00000020)
#define PWM_STATUS_LDO5V_OCP_CLR                                             0xFFFFFFDF

#define PWM_STATUS_CH3_OUT_SHIFT                                             4
#define PWM_STATUS_CH3_OUT_MASK                                              0x00000010
#define PWM_STATUS_CH3_OUT_SET(n)                                            (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_STATUS_CH3_OUT_CLR                                               0xFFFFFFEF

#define PWM_STATUS_CH2_OUT_SHIFT                                             3
#define PWM_STATUS_CH2_OUT_MASK                                              0x00000008
#define PWM_STATUS_CH2_OUT_SET(n)                                            (((uint32_t)(n) << 3  ) & 0x00000008)
#define PWM_STATUS_CH2_OUT_CLR                                               0xFFFFFFF7

#define PWM_STATUS_CH1_OUT_SHIFT                                             2
#define PWM_STATUS_CH1_OUT_MASK                                              0x00000004
#define PWM_STATUS_CH1_OUT_SET(n)                                            (((uint32_t)(n) << 2  ) & 0x00000004)
#define PWM_STATUS_CH1_OUT_CLR                                               0xFFFFFFFB

#define PWM_STATUS_CH0_OUT_SHIFT                                             1
#define PWM_STATUS_CH0_OUT_MASK                                              0x00000002
#define PWM_STATUS_CH0_OUT_SET(n)                                            (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_STATUS_CH0_OUT_CLR                                               0xFFFFFFFD

#define PWM_STATUS_BRK_IN_VAL_SHIFT                                          0
#define PWM_STATUS_BRK_IN_VAL_MASK                                           0x00000001
#define PWM_STATUS_BRK_IN_VAL_SET(n)                                         (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_STATUS_BRK_IN_VAL_CLR                                            0xFFFFFFFE

#define PWM_IMR_LDO5V_OCP_FE_INT_MSK_SHIFT                                   15
#define PWM_IMR_LDO5V_OCP_FE_INT_MSK_MASK                                    0x00008000
#define PWM_IMR_LDO5V_OCP_FE_INT_MSK_SET(n)                                  (((uint32_t)(n) << 15 ) & 0x00008000)
#define PWM_IMR_LDO5V_OCP_FE_INT_MSK_CLR                                     0xFFFF7FFF

#define PWM_IMR_LDO5V_OCP_RE_INT_MSK_SHIFT                                   14
#define PWM_IMR_LDO5V_OCP_RE_INT_MSK_MASK                                    0x00004000
#define PWM_IMR_LDO5V_OCP_RE_INT_MSK_SET(n)                                  (((uint32_t)(n) << 14 ) & 0x00004000)
#define PWM_IMR_LDO5V_OCP_RE_INT_MSK_CLR                                     0xFFFFBFFF

#define PWM_IMR_LDO5V_RDY_INT_MSK_SHIFT                                      13
#define PWM_IMR_LDO5V_RDY_INT_MSK_MASK                                       0x00002000
#define PWM_IMR_LDO5V_RDY_INT_MSK_SET(n)                                     (((uint32_t)(n) << 13 ) & 0x00002000)
#define PWM_IMR_LDO5V_RDY_INT_MSK_CLR                                        0xFFFFDFFF

#define PWM_IMR_CH3_PWM_CLR_INT_MSK_SHIFT                                    12
#define PWM_IMR_CH3_PWM_CLR_INT_MSK_MASK                                     0x00001000
#define PWM_IMR_CH3_PWM_CLR_INT_MSK_SET(n)                                   (((uint32_t)(n) << 12 ) & 0x00001000)
#define PWM_IMR_CH3_PWM_CLR_INT_MSK_CLR                                      0xFFFFEFFF

#define PWM_IMR_CH3_PWM_SET_INT_MSK_SHIFT                                    11
#define PWM_IMR_CH3_PWM_SET_INT_MSK_MASK                                     0x00000800
#define PWM_IMR_CH3_PWM_SET_INT_MSK_SET(n)                                   (((uint32_t)(n) << 11 ) & 0x00000800)
#define PWM_IMR_CH3_PWM_SET_INT_MSK_CLR                                      0xFFFFF7FF

#define PWM_IMR_CH2_PWM_CLR_INT_MSK_SHIFT                                    10
#define PWM_IMR_CH2_PWM_CLR_INT_MSK_MASK                                     0x00000400
#define PWM_IMR_CH2_PWM_CLR_INT_MSK_SET(n)                                   (((uint32_t)(n) << 10 ) & 0x00000400)
#define PWM_IMR_CH2_PWM_CLR_INT_MSK_CLR                                      0xFFFFFBFF

#define PWM_IMR_CH2_PWM_SET_INT_MSK_SHIFT                                    9
#define PWM_IMR_CH2_PWM_SET_INT_MSK_MASK                                     0x00000200
#define PWM_IMR_CH2_PWM_SET_INT_MSK_SET(n)                                   (((uint32_t)(n) << 9  ) & 0x00000200)
#define PWM_IMR_CH2_PWM_SET_INT_MSK_CLR                                      0xFFFFFDFF

#define PWM_IMR_CH1_PWM_CLR_INT_MSK_SHIFT                                    8
#define PWM_IMR_CH1_PWM_CLR_INT_MSK_MASK                                     0x00000100
#define PWM_IMR_CH1_PWM_CLR_INT_MSK_SET(n)                                   (((uint32_t)(n) << 8  ) & 0x00000100)
#define PWM_IMR_CH1_PWM_CLR_INT_MSK_CLR                                      0xFFFFFEFF

#define PWM_IMR_CH1_PWM_SET_INT_MSK_SHIFT                                    7
#define PWM_IMR_CH1_PWM_SET_INT_MSK_MASK                                     0x00000080
#define PWM_IMR_CH1_PWM_SET_INT_MSK_SET(n)                                   (((uint32_t)(n) << 7  ) & 0x00000080)
#define PWM_IMR_CH1_PWM_SET_INT_MSK_CLR                                      0xFFFFFF7F

#define PWM_IMR_CH0_PWM_CLR_INT_MSK_SHIFT                                    6
#define PWM_IMR_CH0_PWM_CLR_INT_MSK_MASK                                     0x00000040
#define PWM_IMR_CH0_PWM_CLR_INT_MSK_SET(n)                                   (((uint32_t)(n) << 6  ) & 0x00000040)
#define PWM_IMR_CH0_PWM_CLR_INT_MSK_CLR                                      0xFFFFFFBF

#define PWM_IMR_CH0_PWM_SET_INT_MSK_SHIFT                                    5
#define PWM_IMR_CH0_PWM_SET_INT_MSK_MASK                                     0x00000020
#define PWM_IMR_CH0_PWM_SET_INT_MSK_SET(n)                                   (((uint32_t)(n) << 5  ) & 0x00000020)
#define PWM_IMR_CH0_PWM_SET_INT_MSK_CLR                                      0xFFFFFFDF

#define PWM_IMR_CNT1_UDF_INT_MSK_SHIFT                                       4
#define PWM_IMR_CNT1_UDF_INT_MSK_MASK                                        0x00000010
#define PWM_IMR_CNT1_UDF_INT_MSK_SET(n)                                      (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_IMR_CNT1_UDF_INT_MSK_CLR                                         0xFFFFFFEF

#define PWM_IMR_CNT1_OVF_INT_MSK_SHIFT                                       3
#define PWM_IMR_CNT1_OVF_INT_MSK_MASK                                        0x00000008
#define PWM_IMR_CNT1_OVF_INT_MSK_SET(n)                                      (((uint32_t)(n) << 3  ) & 0x00000008)
#define PWM_IMR_CNT1_OVF_INT_MSK_CLR                                         0xFFFFFFF7

#define PWM_IMR_CNT0_UDF_INT_MSK_SHIFT                                       2
#define PWM_IMR_CNT0_UDF_INT_MSK_MASK                                        0x00000004
#define PWM_IMR_CNT0_UDF_INT_MSK_SET(n)                                      (((uint32_t)(n) << 2  ) & 0x00000004)
#define PWM_IMR_CNT0_UDF_INT_MSK_CLR                                         0xFFFFFFFB

#define PWM_IMR_CNT0_OVF_INT_MSK_SHIFT                                       1
#define PWM_IMR_CNT0_OVF_INT_MSK_MASK                                        0x00000002
#define PWM_IMR_CNT0_OVF_INT_MSK_SET(n)                                      (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_IMR_CNT0_OVF_INT_MSK_CLR                                         0xFFFFFFFD

#define PWM_IMR_BRK_INT_MSK_SHIFT                                            0
#define PWM_IMR_BRK_INT_MSK_MASK                                             0x00000001
#define PWM_IMR_BRK_INT_MSK_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_IMR_BRK_INT_MSK_CLR                                              0xFFFFFFFE

#define PWM_ICR_LDO5V_OCP_FE_INT_CLR_SHIFT                                   15
#define PWM_ICR_LDO5V_OCP_FE_INT_CLR_MASK                                    0x00008000
#define PWM_ICR_LDO5V_OCP_FE_INT_CLR_SET(n)                                  (((uint32_t)(n) << 15 ) & 0x00008000)
#define PWM_ICR_LDO5V_OCP_FE_INT_CLR_CLR                                     0xFFFF7FFF

#define PWM_ICR_LDO5V_OCP_RE_INT_CLR_SHIFT                                   14
#define PWM_ICR_LDO5V_OCP_RE_INT_CLR_MASK                                    0x00004000
#define PWM_ICR_LDO5V_OCP_RE_INT_CLR_SET(n)                                  (((uint32_t)(n) << 14 ) & 0x00004000)
#define PWM_ICR_LDO5V_OCP_RE_INT_CLR_CLR                                     0xFFFFBFFF

#define PWM_ICR_LDO5V_RDY_INT_CLR_SHIFT                                      13
#define PWM_ICR_LDO5V_RDY_INT_CLR_MASK                                       0x00002000
#define PWM_ICR_LDO5V_RDY_INT_CLR_SET(n)                                     (((uint32_t)(n) << 13 ) & 0x00002000)
#define PWM_ICR_LDO5V_RDY_INT_CLR_CLR                                        0xFFFFDFFF

#define PWM_ICR_CH3_PWM_CLR_INT_CLR_SHIFT                                    12
#define PWM_ICR_CH3_PWM_CLR_INT_CLR_MASK                                     0x00001000
#define PWM_ICR_CH3_PWM_CLR_INT_CLR_SET(n)                                   (((uint32_t)(n) << 12 ) & 0x00001000)
#define PWM_ICR_CH3_PWM_CLR_INT_CLR_CLR                                      0xFFFFEFFF

#define PWM_ICR_CH3_PWM_SET_INT_CLR_SHIFT                                    11
#define PWM_ICR_CH3_PWM_SET_INT_CLR_MASK                                     0x00000800
#define PWM_ICR_CH3_PWM_SET_INT_CLR_SET(n)                                   (((uint32_t)(n) << 11 ) & 0x00000800)
#define PWM_ICR_CH3_PWM_SET_INT_CLR_CLR                                      0xFFFFF7FF

#define PWM_ICR_CH2_PWM_CLR_INT_CLR_SHIFT                                    10
#define PWM_ICR_CH2_PWM_CLR_INT_CLR_MASK                                     0x00000400
#define PWM_ICR_CH2_PWM_CLR_INT_CLR_SET(n)                                   (((uint32_t)(n) << 10 ) & 0x00000400)
#define PWM_ICR_CH2_PWM_CLR_INT_CLR_CLR                                      0xFFFFFBFF

#define PWM_ICR_CH2_PWM_SET_INT_CLR_SHIFT                                    9
#define PWM_ICR_CH2_PWM_SET_INT_CLR_MASK                                     0x00000200
#define PWM_ICR_CH2_PWM_SET_INT_CLR_SET(n)                                   (((uint32_t)(n) << 9  ) & 0x00000200)
#define PWM_ICR_CH2_PWM_SET_INT_CLR_CLR                                      0xFFFFFDFF

#define PWM_ICR_CH1_PWM_CLR_INT_CLR_SHIFT                                    8
#define PWM_ICR_CH1_PWM_CLR_INT_CLR_MASK                                     0x00000100
#define PWM_ICR_CH1_PWM_CLR_INT_CLR_SET(n)                                   (((uint32_t)(n) << 8  ) & 0x00000100)
#define PWM_ICR_CH1_PWM_CLR_INT_CLR_CLR                                      0xFFFFFEFF

#define PWM_ICR_CH1_PWM_SET_INT_CLR_SHIFT                                    7
#define PWM_ICR_CH1_PWM_SET_INT_CLR_MASK                                     0x00000080
#define PWM_ICR_CH1_PWM_SET_INT_CLR_SET(n)                                   (((uint32_t)(n) << 7  ) & 0x00000080)
#define PWM_ICR_CH1_PWM_SET_INT_CLR_CLR                                      0xFFFFFF7F

#define PWM_ICR_CH0_PWM_CLR_INT_CLR_SHIFT                                    6
#define PWM_ICR_CH0_PWM_CLR_INT_CLR_MASK                                     0x00000040
#define PWM_ICR_CH0_PWM_CLR_INT_CLR_SET(n)                                   (((uint32_t)(n) << 6  ) & 0x00000040)
#define PWM_ICR_CH0_PWM_CLR_INT_CLR_CLR                                      0xFFFFFFBF

#define PWM_ICR_CH0_PWM_SET_INT_CLR_SHIFT                                    5
#define PWM_ICR_CH0_PWM_SET_INT_CLR_MASK                                     0x00000020
#define PWM_ICR_CH0_PWM_SET_INT_CLR_SET(n)                                   (((uint32_t)(n) << 5  ) & 0x00000020)
#define PWM_ICR_CH0_PWM_SET_INT_CLR_CLR                                      0xFFFFFFDF

#define PWM_ICR_CNT1_UDF_INT_CLR_SHIFT                                       4
#define PWM_ICR_CNT1_UDF_INT_CLR_MASK                                        0x00000010
#define PWM_ICR_CNT1_UDF_INT_CLR_SET(n)                                      (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_ICR_CNT1_UDF_INT_CLR_CLR                                         0xFFFFFFEF

#define PWM_ICR_CNT1_OVF_INT_CLR_SHIFT                                       3
#define PWM_ICR_CNT1_OVF_INT_CLR_MASK                                        0x00000008
#define PWM_ICR_CNT1_OVF_INT_CLR_SET(n)                                      (((uint32_t)(n) << 3  ) & 0x00000008)
#define PWM_ICR_CNT1_OVF_INT_CLR_CLR                                         0xFFFFFFF7

#define PWM_ICR_CNT0_UDF_INT_CLR_SHIFT                                       2
#define PWM_ICR_CNT0_UDF_INT_CLR_MASK                                        0x00000004
#define PWM_ICR_CNT0_UDF_INT_CLR_SET(n)                                      (((uint32_t)(n) << 2  ) & 0x00000004)
#define PWM_ICR_CNT0_UDF_INT_CLR_CLR                                         0xFFFFFFFB

#define PWM_ICR_CNT0_OVF_INT_CLR_SHIFT                                       1
#define PWM_ICR_CNT0_OVF_INT_CLR_MASK                                        0x00000002
#define PWM_ICR_CNT0_OVF_INT_CLR_SET(n)                                      (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_ICR_CNT0_OVF_INT_CLR_CLR                                         0xFFFFFFFD

#define PWM_ICR_BRK_INT_CLR_SHIFT                                            0
#define PWM_ICR_BRK_INT_CLR_MASK                                             0x00000001
#define PWM_ICR_BRK_INT_CLR_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_ICR_BRK_INT_CLR_CLR                                              0xFFFFFFFE

#define PWM_ISR_LDO5V_OCP_FE_INT_STATUS_SHIFT                                15
#define PWM_ISR_LDO5V_OCP_FE_INT_STATUS_MASK                                 0x00008000
#define PWM_ISR_LDO5V_OCP_FE_INT_STATUS_SET(n)                               (((uint32_t)(n) << 15 ) & 0x00008000)
#define PWM_ISR_LDO5V_OCP_FE_INT_STATUS_CLR                                  0xFFFF7FFF

#define PWM_ISR_LDO5V_OCP_RE_INT_STATUS_SHIFT                                14
#define PWM_ISR_LDO5V_OCP_RE_INT_STATUS_MASK                                 0x00004000
#define PWM_ISR_LDO5V_OCP_RE_INT_STATUS_SET(n)                               (((uint32_t)(n) << 14 ) & 0x00004000)
#define PWM_ISR_LDO5V_OCP_RE_INT_STATUS_CLR                                  0xFFFFBFFF

#define PWM_ISR_LDO5V_RDY_INT_STATUS_SHIFT                                   13
#define PWM_ISR_LDO5V_RDY_INT_STATUS_MASK                                    0x00002000
#define PWM_ISR_LDO5V_RDY_INT_STATUS_SET(n)                                  (((uint32_t)(n) << 13 ) & 0x00002000)
#define PWM_ISR_LDO5V_RDY_INT_STATUS_CLR                                     0xFFFFDFFF

#define PWM_ISR_CH3_PWM_CLR_INT_STATUS_SHIFT                                 12
#define PWM_ISR_CH3_PWM_CLR_INT_STATUS_MASK                                  0x00001000
#define PWM_ISR_CH3_PWM_CLR_INT_STATUS_SET(n)                                (((uint32_t)(n) << 12 ) & 0x00001000)
#define PWM_ISR_CH3_PWM_CLR_INT_STATUS_CLR                                   0xFFFFEFFF

#define PWM_ISR_CH3_PWM_SET_INT_STATUS_SHIFT                                 11
#define PWM_ISR_CH3_PWM_SET_INT_STATUS_MASK                                  0x00000800
#define PWM_ISR_CH3_PWM_SET_INT_STATUS_SET(n)                                (((uint32_t)(n) << 11 ) & 0x00000800)
#define PWM_ISR_CH3_PWM_SET_INT_STATUS_CLR                                   0xFFFFF7FF

#define PWM_ISR_CH2_PWM_CLR_INT_STATUS_SHIFT                                 10
#define PWM_ISR_CH2_PWM_CLR_INT_STATUS_MASK                                  0x00000400
#define PWM_ISR_CH2_PWM_CLR_INT_STATUS_SET(n)                                (((uint32_t)(n) << 10 ) & 0x00000400)
#define PWM_ISR_CH2_PWM_CLR_INT_STATUS_CLR                                   0xFFFFFBFF

#define PWM_ISR_CH2_PWM_SET_INT_STATUS_SHIFT                                 9
#define PWM_ISR_CH2_PWM_SET_INT_STATUS_MASK                                  0x00000200
#define PWM_ISR_CH2_PWM_SET_INT_STATUS_SET(n)                                (((uint32_t)(n) << 9  ) & 0x00000200)
#define PWM_ISR_CH2_PWM_SET_INT_STATUS_CLR                                   0xFFFFFDFF

#define PWM_ISR_CH1_PWM_CLR_INT_STATUS_SHIFT                                 8
#define PWM_ISR_CH1_PWM_CLR_INT_STATUS_MASK                                  0x00000100
#define PWM_ISR_CH1_PWM_CLR_INT_STATUS_SET(n)                                (((uint32_t)(n) << 8  ) & 0x00000100)
#define PWM_ISR_CH1_PWM_CLR_INT_STATUS_CLR                                   0xFFFFFEFF

#define PWM_ISR_CH1_PWM_SET_INT_STATUS_SHIFT                                 7
#define PWM_ISR_CH1_PWM_SET_INT_STATUS_MASK                                  0x00000080
#define PWM_ISR_CH1_PWM_SET_INT_STATUS_SET(n)                                (((uint32_t)(n) << 7  ) & 0x00000080)
#define PWM_ISR_CH1_PWM_SET_INT_STATUS_CLR                                   0xFFFFFF7F

#define PWM_ISR_CH0_PWM_CLR_INT_STATUS_SHIFT                                 6
#define PWM_ISR_CH0_PWM_CLR_INT_STATUS_MASK                                  0x00000040
#define PWM_ISR_CH0_PWM_CLR_INT_STATUS_SET(n)                                (((uint32_t)(n) << 6  ) & 0x00000040)
#define PWM_ISR_CH0_PWM_CLR_INT_STATUS_CLR                                   0xFFFFFFBF

#define PWM_ISR_CH0_PWM_SET_INT_STATUS_SHIFT                                 5
#define PWM_ISR_CH0_PWM_SET_INT_STATUS_MASK                                  0x00000020
#define PWM_ISR_CH0_PWM_SET_INT_STATUS_SET(n)                                (((uint32_t)(n) << 5  ) & 0x00000020)
#define PWM_ISR_CH0_PWM_SET_INT_STATUS_CLR                                   0xFFFFFFDF

#define PWM_ISR_CNT1_UDF_INT_STATUS_SHIFT                                    4
#define PWM_ISR_CNT1_UDF_INT_STATUS_MASK                                     0x00000010
#define PWM_ISR_CNT1_UDF_INT_STATUS_SET(n)                                   (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_ISR_CNT1_UDF_INT_STATUS_CLR                                      0xFFFFFFEF

#define PWM_ISR_CNT1_OVF_INT_STATUS_SHIFT                                    3
#define PWM_ISR_CNT1_OVF_INT_STATUS_MASK                                     0x00000008
#define PWM_ISR_CNT1_OVF_INT_STATUS_SET(n)                                   (((uint32_t)(n) << 3  ) & 0x00000008)
#define PWM_ISR_CNT1_OVF_INT_STATUS_CLR                                      0xFFFFFFF7

#define PWM_ISR_CNT0_UDF_INT_STATUS_SHIFT                                    2
#define PWM_ISR_CNT0_UDF_INT_STATUS_MASK                                     0x00000004
#define PWM_ISR_CNT0_UDF_INT_STATUS_SET(n)                                   (((uint32_t)(n) << 2  ) & 0x00000004)
#define PWM_ISR_CNT0_UDF_INT_STATUS_CLR                                      0xFFFFFFFB

#define PWM_ISR_CNT0_OVF_INT_STATUS_SHIFT                                    1
#define PWM_ISR_CNT0_OVF_INT_STATUS_MASK                                     0x00000002
#define PWM_ISR_CNT0_OVF_INT_STATUS_SET(n)                                   (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_ISR_CNT0_OVF_INT_STATUS_CLR                                      0xFFFFFFFD

#define PWM_ISR_BRK_INT_STATUS_SHIFT                                         0
#define PWM_ISR_BRK_INT_STATUS_MASK                                          0x00000001
#define PWM_ISR_BRK_INT_STATUS_SET(n)                                        (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_ISR_BRK_INT_STATUS_CLR                                           0xFFFFFFFE

#define PWM_IRSR_LDO5V_OCP_FE_INT_RAW_STATUS_SHIFT                           15
#define PWM_IRSR_LDO5V_OCP_FE_INT_RAW_STATUS_MASK                            0x00008000
#define PWM_IRSR_LDO5V_OCP_FE_INT_RAW_STATUS_SET(n)                          (((uint32_t)(n) << 15 ) & 0x00008000)
#define PWM_IRSR_LDO5V_OCP_FE_INT_RAW_STATUS_CLR                             0xFFFF7FFF

#define PWM_IRSR_LDO5V_OCP_RE_INT_RAW_STATUS_SHIFT                           14
#define PWM_IRSR_LDO5V_OCP_RE_INT_RAW_STATUS_MASK                            0x00004000
#define PWM_IRSR_LDO5V_OCP_RE_INT_RAW_STATUS_SET(n)                          (((uint32_t)(n) << 14 ) & 0x00004000)
#define PWM_IRSR_LDO5V_OCP_RE_INT_RAW_STATUS_CLR                             0xFFFFBFFF

#define PWM_IRSR_LDO5V_RDY_INT_RAW_STATUS_SHIFT                              13
#define PWM_IRSR_LDO5V_RDY_INT_RAW_STATUS_MASK                               0x00002000
#define PWM_IRSR_LDO5V_RDY_INT_RAW_STATUS_SET(n)                             (((uint32_t)(n) << 13 ) & 0x00002000)
#define PWM_IRSR_LDO5V_RDY_INT_RAW_STATUS_CLR                                0xFFFFDFFF

#define PWM_IRSR_CH3_PWM_CLR_INT_RAW_STATUS_SHIFT                            12
#define PWM_IRSR_CH3_PWM_CLR_INT_RAW_STATUS_MASK                             0x00001000
#define PWM_IRSR_CH3_PWM_CLR_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 12 ) & 0x00001000)
#define PWM_IRSR_CH3_PWM_CLR_INT_RAW_STATUS_CLR                              0xFFFFEFFF

#define PWM_IRSR_CH3_PWM_SET_INT_RAW_STATUS_SHIFT                            11
#define PWM_IRSR_CH3_PWM_SET_INT_RAW_STATUS_MASK                             0x00000800
#define PWM_IRSR_CH3_PWM_SET_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 11 ) & 0x00000800)
#define PWM_IRSR_CH3_PWM_SET_INT_RAW_STATUS_CLR                              0xFFFFF7FF

#define PWM_IRSR_CH2_PWM_CLR_INT_RAW_STATUS_SHIFT                            10
#define PWM_IRSR_CH2_PWM_CLR_INT_RAW_STATUS_MASK                             0x00000400
#define PWM_IRSR_CH2_PWM_CLR_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 10 ) & 0x00000400)
#define PWM_IRSR_CH2_PWM_CLR_INT_RAW_STATUS_CLR                              0xFFFFFBFF

#define PWM_IRSR_CH2_PWM_SET_INT_RAW_STATUS_SHIFT                            9
#define PWM_IRSR_CH2_PWM_SET_INT_RAW_STATUS_MASK                             0x00000200
#define PWM_IRSR_CH2_PWM_SET_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 9  ) & 0x00000200)
#define PWM_IRSR_CH2_PWM_SET_INT_RAW_STATUS_CLR                              0xFFFFFDFF

#define PWM_IRSR_CH1_PWM_CLR_INT_RAW_STATUS_SHIFT                            8
#define PWM_IRSR_CH1_PWM_CLR_INT_RAW_STATUS_MASK                             0x00000100
#define PWM_IRSR_CH1_PWM_CLR_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 8  ) & 0x00000100)
#define PWM_IRSR_CH1_PWM_CLR_INT_RAW_STATUS_CLR                              0xFFFFFEFF

#define PWM_IRSR_CH1_PWM_SET_INT_RAW_STATUS_SHIFT                            7
#define PWM_IRSR_CH1_PWM_SET_INT_RAW_STATUS_MASK                             0x00000080
#define PWM_IRSR_CH1_PWM_SET_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 7  ) & 0x00000080)
#define PWM_IRSR_CH1_PWM_SET_INT_RAW_STATUS_CLR                              0xFFFFFF7F

#define PWM_IRSR_CH0_PWM_CLR_INT_RAW_STATUS_SHIFT                            6
#define PWM_IRSR_CH0_PWM_CLR_INT_RAW_STATUS_MASK                             0x00000040
#define PWM_IRSR_CH0_PWM_CLR_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 6  ) & 0x00000040)
#define PWM_IRSR_CH0_PWM_CLR_INT_RAW_STATUS_CLR                              0xFFFFFFBF

#define PWM_IRSR_CH0_PWM_SET_INT_RAW_STATUS_SHIFT                            5
#define PWM_IRSR_CH0_PWM_SET_INT_RAW_STATUS_MASK                             0x00000020
#define PWM_IRSR_CH0_PWM_SET_INT_RAW_STATUS_SET(n)                           (((uint32_t)(n) << 5  ) & 0x00000020)
#define PWM_IRSR_CH0_PWM_SET_INT_RAW_STATUS_CLR                              0xFFFFFFDF

#define PWM_IRSR_CNT1_UDF_INT_RAW_STATUS_SHIFT                               4
#define PWM_IRSR_CNT1_UDF_INT_RAW_STATUS_MASK                                0x00000010
#define PWM_IRSR_CNT1_UDF_INT_RAW_STATUS_SET(n)                              (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_IRSR_CNT1_UDF_INT_RAW_STATUS_CLR                                 0xFFFFFFEF

#define PWM_IRSR_CNT1_OVF_INT_RAW_STATUS_SHIFT                               3
#define PWM_IRSR_CNT1_OVF_INT_RAW_STATUS_MASK                                0x00000008
#define PWM_IRSR_CNT1_OVF_INT_RAW_STATUS_SET(n)                              (((uint32_t)(n) << 3  ) & 0x00000008)
#define PWM_IRSR_CNT1_OVF_INT_RAW_STATUS_CLR                                 0xFFFFFFF7

#define PWM_IRSR_CNT0_UDF_INT_RAW_STATUS_SHIFT                               2
#define PWM_IRSR_CNT0_UDF_INT_RAW_STATUS_MASK                                0x00000004
#define PWM_IRSR_CNT0_UDF_INT_RAW_STATUS_SET(n)                              (((uint32_t)(n) << 2  ) & 0x00000004)
#define PWM_IRSR_CNT0_UDF_INT_RAW_STATUS_CLR                                 0xFFFFFFFB

#define PWM_IRSR_CNT0_OVF_INT_RAW_STATUS_SHIFT                               1
#define PWM_IRSR_CNT0_OVF_INT_RAW_STATUS_MASK                                0x00000002
#define PWM_IRSR_CNT0_OVF_INT_RAW_STATUS_SET(n)                              (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_IRSR_CNT0_OVF_INT_RAW_STATUS_CLR                                 0xFFFFFFFD

#define PWM_IRSR_BRK_INT_RAW_STATUS_SHIFT                                    0
#define PWM_IRSR_BRK_INT_RAW_STATUS_MASK                                     0x00000001
#define PWM_IRSR_BRK_INT_RAW_STATUS_SET(n)                                   (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_IRSR_BRK_INT_RAW_STATUS_CLR                                      0xFFFFFFFE

#define PWM_LED_CTRL_LED_LDO_RDY_SHIFT                                       20
#define PWM_LED_CTRL_LED_LDO_RDY_MASK                                        0x00100000
#define PWM_LED_CTRL_LED_LDO_RDY_SET(n)                                      (((uint32_t)(n) << 20 ) & 0x00100000)
#define PWM_LED_CTRL_LED_LDO_RDY_CLR                                         0xFFEFFFFF

#define PWM_LED_CTRL_LED_RESERVE_SHIFT                                       8
#define PWM_LED_CTRL_LED_RESERVE_MASK                                        0x0001FF00
#define PWM_LED_CTRL_LED_RESERVE_SET(n)                                      (((uint32_t)(n) << 8  ) & 0x0001FF00)
#define PWM_LED_CTRL_LED_RESERVE_CLR                                         0xFFFE00FF

#define PWM_LED_CTRL_LED_LDO5V_OUT_SEL_SHIFT                                 4
#define PWM_LED_CTRL_LED_LDO5V_OUT_SEL_MASK                                  0x00000010
#define PWM_LED_CTRL_LED_LDO5V_OUT_SEL_SET(n)                                (((uint32_t)(n) << 4  ) & 0x00000010)
#define PWM_LED_CTRL_LED_LDO5V_OUT_SEL_CLR                                   0xFFFFFFEF

#define PWM_LED_CTRL_LED_LDO5V_EN_SHIFT                                      1
#define PWM_LED_CTRL_LED_LDO5V_EN_MASK                                       0x00000002
#define PWM_LED_CTRL_LED_LDO5V_EN_SET(n)                                     (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_LED_CTRL_LED_LDO5V_EN_CLR                                        0xFFFFFFFD

#define PWM_LED_CTRL_LED_EN_SHIFT                                            0
#define PWM_LED_CTRL_LED_EN_MASK                                             0x00000001
#define PWM_LED_CTRL_LED_EN_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_LED_CTRL_LED_EN_CLR                                              0xFFFFFFFE

#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TF_SHIFT                                10
#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TF_MASK                                 0x00000C00
#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TF_SET(n)                               (((uint32_t)(n) << 10 ) & 0x00000C00)
#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TF_CLR                                  0xFFFFF3FF

#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TR_SHIFT                                8
#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TR_MASK                                 0x00000300
#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TR_SET(n)                               (((uint32_t)(n) << 8  ) & 0x00000300)
#define PWM_LED_LC0_CTRL_LED_LC0_SEL_TR_CLR                                  0xFFFFFCFF

#define PWM_LED_LC0_CTRL_LED_LC0_IOUT_SEL_SHIFT                              4
#define PWM_LED_LC0_CTRL_LED_LC0_IOUT_SEL_MASK                               0x000000F0
#define PWM_LED_LC0_CTRL_LED_LC0_IOUT_SEL_SET(n)                             (((uint32_t)(n) << 4  ) & 0x000000F0)
#define PWM_LED_LC0_CTRL_LED_LC0_IOUT_SEL_CLR                                0xFFFFFF0F

#define PWM_LED_LC0_CTRL_LED_LC0_DISPU_EN_SHIFT                              1
#define PWM_LED_LC0_CTRL_LED_LC0_DISPU_EN_MASK                               0x00000002
#define PWM_LED_LC0_CTRL_LED_LC0_DISPU_EN_SET(n)                             (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_LED_LC0_CTRL_LED_LC0_DISPU_EN_CLR                                0xFFFFFFFD

#define PWM_LED_LC0_CTRL_LED_LC0_DIAG_EN_SHIFT                               0
#define PWM_LED_LC0_CTRL_LED_LC0_DIAG_EN_MASK                                0x00000001
#define PWM_LED_LC0_CTRL_LED_LC0_DIAG_EN_SET(n)                              (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_LED_LC0_CTRL_LED_LC0_DIAG_EN_CLR                                 0xFFFFFFFE

#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TF_SHIFT                                10
#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TF_MASK                                 0x00000C00
#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TF_SET(n)                               (((uint32_t)(n) << 10 ) & 0x00000C00)
#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TF_CLR                                  0xFFFFF3FF

#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TR_SHIFT                                8
#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TR_MASK                                 0x00000300
#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TR_SET(n)                               (((uint32_t)(n) << 8  ) & 0x00000300)
#define PWM_LED_LC1_CTRL_LED_LC1_SEL_TR_CLR                                  0xFFFFFCFF

#define PWM_LED_LC1_CTRL_LED_LC1_IOUT_SEL_SHIFT                              4
#define PWM_LED_LC1_CTRL_LED_LC1_IOUT_SEL_MASK                               0x000000F0
#define PWM_LED_LC1_CTRL_LED_LC1_IOUT_SEL_SET(n)                             (((uint32_t)(n) << 4  ) & 0x000000F0)
#define PWM_LED_LC1_CTRL_LED_LC1_IOUT_SEL_CLR                                0xFFFFFF0F

#define PWM_LED_LC1_CTRL_LED_LC1_DISPU_EN_SHIFT                              1
#define PWM_LED_LC1_CTRL_LED_LC1_DISPU_EN_MASK                               0x00000002
#define PWM_LED_LC1_CTRL_LED_LC1_DISPU_EN_SET(n)                             (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_LED_LC1_CTRL_LED_LC1_DISPU_EN_CLR                                0xFFFFFFFD

#define PWM_LED_LC1_CTRL_LED_LC1_DIAG_EN_SHIFT                               0
#define PWM_LED_LC1_CTRL_LED_LC1_DIAG_EN_MASK                                0x00000001
#define PWM_LED_LC1_CTRL_LED_LC1_DIAG_EN_SET(n)                              (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_LED_LC1_CTRL_LED_LC1_DIAG_EN_CLR                                 0xFFFFFFFE

#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TF_SHIFT                                10
#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TF_MASK                                 0x00000C00
#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TF_SET(n)                               (((uint32_t)(n) << 10 ) & 0x00000C00)
#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TF_CLR                                  0xFFFFF3FF

#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TR_SHIFT                                8
#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TR_MASK                                 0x00000300
#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TR_SET(n)                               (((uint32_t)(n) << 8  ) & 0x00000300)
#define PWM_LED_LC2_CTRL_LED_LC2_SEL_TR_CLR                                  0xFFFFFCFF

#define PWM_LED_LC2_CTRL_LED_LC2_IOUT_SEL_SHIFT                              4
#define PWM_LED_LC2_CTRL_LED_LC2_IOUT_SEL_MASK                               0x000000F0
#define PWM_LED_LC2_CTRL_LED_LC2_IOUT_SEL_SET(n)                             (((uint32_t)(n) << 4  ) & 0x000000F0)
#define PWM_LED_LC2_CTRL_LED_LC2_IOUT_SEL_CLR                                0xFFFFFF0F

#define PWM_LED_LC2_CTRL_LED_LC2_DISPU_EN_SHIFT                              1
#define PWM_LED_LC2_CTRL_LED_LC2_DISPU_EN_MASK                               0x00000002
#define PWM_LED_LC2_CTRL_LED_LC2_DISPU_EN_SET(n)                             (((uint32_t)(n) << 1  ) & 0x00000002)
#define PWM_LED_LC2_CTRL_LED_LC2_DISPU_EN_CLR                                0xFFFFFFFD

#define PWM_LED_LC2_CTRL_LED_LC2_DIAG_EN_SHIFT                               0
#define PWM_LED_LC2_CTRL_LED_LC2_DIAG_EN_MASK                                0x00000001
#define PWM_LED_LC2_CTRL_LED_LC2_DIAG_EN_SET(n)                              (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_LED_LC2_CTRL_LED_LC2_DIAG_EN_CLR                                 0xFFFFFFFE

#define PWM_WR_LOCK_WR_LOCK_SHIFT                                            0
#define PWM_WR_LOCK_WR_LOCK_MASK                                             0x00000001
#define PWM_WR_LOCK_WR_LOCK_SET(n)                                           (((uint32_t)(n) << 0  ) & 0x00000001)
#define PWM_WR_LOCK_WR_LOCK_CLR                                              0xFFFFFFFE

// Register Structure Definition
typedef struct
{
    __IO uint32_t SYNC_EN                     : 1  ; // 0  : 0
    __IO uint32_t PWM_SW_EN                   : 1  ; // 1  : 1
    __IO uint32_t AUTO_LD_EN                  : 1  ; // 2  : 2
    __IO uint32_t RESERVED4                   : 1  ; // 3  : 3
    __IO uint32_t LD_TRIG                     : 1  ; // 4  : 4
    __IO uint32_t RESERVED3                   : 3  ; // 7  : 5
    __IO uint32_t BRK_EN                      : 1  ; // 8  : 8
    __IO uint32_t RESERVED2                   : 3  ; // 11 : 9
    __IO uint32_t CH_SYNC_EN                  : 1  ; // 12 : 12
    __IO uint32_t CH_SYNC_SEL                 : 2  ; // 14 : 13
    __IO uint32_t RESERVED1                   : 1  ; // 15 : 15
    __IO uint32_t PWM_STP                     : 1  ; // 16 : 16
    __IO uint32_t RESERVED0                   : 15 ; // 31 : 17
} PWM_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t CNT0_EN                     : 1  ; // 0  : 0
    __IO uint32_t CNT1_EN                     : 1  ; // 1  : 1
    __IO uint32_t RESERVED2                   : 2  ; // 3  : 2
    __IO uint32_t CNT0_MODE                   : 1  ; // 4  : 4
    __IO uint32_t CNT1_MODE                   : 1  ; // 5  : 5
    __IO uint32_t CNT0_ONE_SHOT_EN            : 1  ; // 6  : 6
    __IO uint32_t CNT1_ONE_SHOT_EN            : 1  ; // 7  : 7
    __IO uint32_t CNT0_ONE_SHOT_TRIG          : 1  ; // 8  : 8
    __IO uint32_t CNT1_ONE_SHOT_TRIG          : 1  ; // 9  : 9
    __IO uint32_t RESERVED1                   : 2  ; // 11 : 10
    __IO uint32_t CNT0_EN_MODE                : 1  ; // 12 : 12
    __IO uint32_t CNT1_EN_MODE                : 1  ; // 13 : 13
    __IO uint32_t RESERVED0                   : 18 ; // 31 : 14
} PWM_CNT_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t PERIOD0                     : 16 ; // 15 : 0
    __IO uint32_t PERIOD1                     : 16 ; // 31 : 16
} PWM_CNT_CFG_FIELD_T;

typedef struct
{
    __IO uint32_t CH0_EN                      : 1  ; // 0  : 0
    __IO uint32_t CH1_EN                      : 1  ; // 1  : 1
    __IO uint32_t CH2_EN                      : 1  ; // 2  : 2
    __IO uint32_t CH3_EN                      : 1  ; // 3  : 3
    __IO uint32_t CH0_PTY                     : 1  ; // 4  : 4
    __IO uint32_t CH1_PTY                     : 1  ; // 5  : 5
    __IO uint32_t CH2_PTY                     : 1  ; // 6  : 6
    __IO uint32_t CH3_PTY                     : 1  ; // 7  : 7
    __IO uint32_t CH0_MODE                    : 2  ; // 9  : 8
    __IO uint32_t CH1_MODE                    : 2  ; // 11 : 10
    __IO uint32_t CH2_MODE                    : 2  ; // 13 : 12
    __IO uint32_t CH3_MODE                    : 2  ; // 15 : 14
    __IO uint32_t CH0_CNT_SEL                 : 1  ; // 16 : 16
    __IO uint32_t CH1_CNT_SEL                 : 1  ; // 17 : 17
    __IO uint32_t CH2_CNT_SEL                 : 1  ; // 18 : 18
    __IO uint32_t CH3_CNT_SEL                 : 1  ; // 19 : 19
    __IO uint32_t RESERVED0                   : 12 ; // 31 : 20
} PWM_CH_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t LT0                         : 16 ; // 15 : 0
    __IO uint32_t HT0                         : 16 ; // 31 : 16
} PWM_CH0_PWM_CFG_FIELD_T;

typedef struct
{
    __IO uint32_t LT1                         : 16 ; // 15 : 0
    __IO uint32_t HT1                         : 16 ; // 31 : 16
} PWM_CH1_PWM_CFG_FIELD_T;

typedef struct
{
    __IO uint32_t LT2                         : 16 ; // 15 : 0
    __IO uint32_t HT2                         : 16 ; // 31 : 16
} PWM_CH2_PWM_CFG_FIELD_T;

typedef struct
{
    __IO uint32_t LT3                         : 16 ; // 15 : 0
    __IO uint32_t HT3                         : 16 ; // 31 : 16
} PWM_CH3_PWM_CFG_FIELD_T;

typedef struct
{
    __IO uint32_t CNT0_VAL                    : 16 ; // 15 : 0
    __IO uint32_t CNT1_VAL                    : 16 ; // 31 : 16
} PWM_CNT_VAL_FIELD_T;

typedef struct
{
    __IO uint32_t BRK_IN_VAL                  : 1  ; // 0  : 0
    __IO uint32_t CH0_OUT                     : 1  ; // 1  : 1
    __IO uint32_t CH1_OUT                     : 1  ; // 2  : 2
    __IO uint32_t CH2_OUT                     : 1  ; // 3  : 3
    __IO uint32_t CH3_OUT                     : 1  ; // 4  : 4
    __IO uint32_t LDO5V_OCP                   : 1  ; // 5  : 5
    __IO uint32_t RESERVED0                   : 26 ; // 31 : 6
} PWM_STATUS_FIELD_T;

typedef struct
{
    __IO uint32_t BRK_INT_MSK                 : 1  ; // 0  : 0
    __IO uint32_t CNT0_OVF_INT_MSK            : 1  ; // 1  : 1
    __IO uint32_t CNT0_UDF_INT_MSK            : 1  ; // 2  : 2
    __IO uint32_t CNT1_OVF_INT_MSK            : 1  ; // 3  : 3
    __IO uint32_t CNT1_UDF_INT_MSK            : 1  ; // 4  : 4
    __IO uint32_t CH0_PWM_SET_INT_MSK         : 1  ; // 5  : 5
    __IO uint32_t CH0_PWM_CLR_INT_MSK         : 1  ; // 6  : 6
    __IO uint32_t CH1_PWM_SET_INT_MSK         : 1  ; // 7  : 7
    __IO uint32_t CH1_PWM_CLR_INT_MSK         : 1  ; // 8  : 8
    __IO uint32_t CH2_PWM_SET_INT_MSK         : 1  ; // 9  : 9
    __IO uint32_t CH2_PWM_CLR_INT_MSK         : 1  ; // 10 : 10
    __IO uint32_t CH3_PWM_SET_INT_MSK         : 1  ; // 11 : 11
    __IO uint32_t CH3_PWM_CLR_INT_MSK         : 1  ; // 12 : 12
    __IO uint32_t LDO5V_RDY_INT_MSK           : 1  ; // 13 : 13
    __IO uint32_t LDO5V_OCP_RE_INT_MSK        : 1  ; // 14 : 14
    __IO uint32_t LDO5V_OCP_FE_INT_MSK        : 1  ; // 15 : 15
    __IO uint32_t RESERVED0                   : 16 ; // 31 : 16
} PWM_IMR_FIELD_T;

typedef struct
{
    __IO uint32_t BRK_INT_CLR                 : 1  ; // 0  : 0
    __IO uint32_t CNT0_OVF_INT_CLR            : 1  ; // 1  : 1
    __IO uint32_t CNT0_UDF_INT_CLR            : 1  ; // 2  : 2
    __IO uint32_t CNT1_OVF_INT_CLR            : 1  ; // 3  : 3
    __IO uint32_t CNT1_UDF_INT_CLR            : 1  ; // 4  : 4
    __IO uint32_t CH0_PWM_SET_INT_CLR         : 1  ; // 5  : 5
    __IO uint32_t CH0_PWM_CLR_INT_CLR         : 1  ; // 6  : 6
    __IO uint32_t CH1_PWM_SET_INT_CLR         : 1  ; // 7  : 7
    __IO uint32_t CH1_PWM_CLR_INT_CLR         : 1  ; // 8  : 8
    __IO uint32_t CH2_PWM_SET_INT_CLR         : 1  ; // 9  : 9
    __IO uint32_t CH2_PWM_CLR_INT_CLR         : 1  ; // 10 : 10
    __IO uint32_t CH3_PWM_SET_INT_CLR         : 1  ; // 11 : 11
    __IO uint32_t CH3_PWM_CLR_INT_CLR         : 1  ; // 12 : 12
    __IO uint32_t LDO5V_RDY_INT_CLR           : 1  ; // 13 : 13
    __IO uint32_t LDO5V_OCP_RE_INT_CLR        : 1  ; // 14 : 14
    __IO uint32_t LDO5V_OCP_FE_INT_CLR        : 1  ; // 15 : 15
    __IO uint32_t RESERVED0                   : 16 ; // 31 : 16
} PWM_ICR_FIELD_T;

typedef struct
{
    __IO uint32_t BRK_INT_STATUS              : 1  ; // 0  : 0
    __IO uint32_t CNT0_OVF_INT_STATUS         : 1  ; // 1  : 1
    __IO uint32_t CNT0_UDF_INT_STATUS         : 1  ; // 2  : 2
    __IO uint32_t CNT1_OVF_INT_STATUS         : 1  ; // 3  : 3
    __IO uint32_t CNT1_UDF_INT_STATUS         : 1  ; // 4  : 4
    __IO uint32_t CH0_PWM_SET_INT_STATUS      : 1  ; // 5  : 5
    __IO uint32_t CH0_PWM_CLR_INT_STATUS      : 1  ; // 6  : 6
    __IO uint32_t CH1_PWM_SET_INT_STATUS      : 1  ; // 7  : 7
    __IO uint32_t CH1_PWM_CLR_INT_STATUS      : 1  ; // 8  : 8
    __IO uint32_t CH2_PWM_SET_INT_STATUS      : 1  ; // 9  : 9
    __IO uint32_t CH2_PWM_CLR_INT_STATUS      : 1  ; // 10 : 10
    __IO uint32_t CH3_PWM_SET_INT_STATUS      : 1  ; // 11 : 11
    __IO uint32_t CH3_PWM_CLR_INT_STATUS      : 1  ; // 12 : 12
    __IO uint32_t LDO5V_RDY_INT_STATUS        : 1  ; // 13 : 13
    __IO uint32_t LDO5V_OCP_RE_INT_STATUS     : 1  ; // 14 : 14
    __IO uint32_t LDO5V_OCP_FE_INT_STATUS     : 1  ; // 15 : 15
    __IO uint32_t RESERVED0                   : 16 ; // 31 : 16
} PWM_ISR_FIELD_T;

typedef struct
{
    __IO uint32_t BRK_INT_RAW_STATUS          : 1  ; // 0  : 0
    __IO uint32_t CNT0_OVF_INT_RAW_STATUS     : 1  ; // 1  : 1
    __IO uint32_t CNT0_UDF_INT_RAW_STATUS     : 1  ; // 2  : 2
    __IO uint32_t CNT1_OVF_INT_RAW_STATUS     : 1  ; // 3  : 3
    __IO uint32_t CNT1_UDF_INT_RAW_STATUS     : 1  ; // 4  : 4
    __IO uint32_t CH0_PWM_SET_INT_RAW_STATUS  : 1  ; // 5  : 5
    __IO uint32_t CH0_PWM_CLR_INT_RAW_STATUS  : 1  ; // 6  : 6
    __IO uint32_t CH1_PWM_SET_INT_RAW_STATUS  : 1  ; // 7  : 7
    __IO uint32_t CH1_PWM_CLR_INT_RAW_STATUS  : 1  ; // 8  : 8
    __IO uint32_t CH2_PWM_SET_INT_RAW_STATUS  : 1  ; // 9  : 9
    __IO uint32_t CH2_PWM_CLR_INT_RAW_STATUS  : 1  ; // 10 : 10
    __IO uint32_t CH3_PWM_SET_INT_RAW_STATUS  : 1  ; // 11 : 11
    __IO uint32_t CH3_PWM_CLR_INT_RAW_STATUS  : 1  ; // 12 : 12
    __IO uint32_t LDO5V_RDY_INT_RAW_STATUS    : 1  ; // 13 : 13
    __IO uint32_t LDO5V_OCP_RE_INT_RAW_STATUS : 1  ; // 14 : 14
    __IO uint32_t LDO5V_OCP_FE_INT_RAW_STATUS : 1  ; // 15 : 15
    __IO uint32_t RESERVED0                   : 16 ; // 31 : 16
} PWM_IRSR_FIELD_T;

typedef struct
{
    __IO uint32_t LED_EN                      : 1  ; // 0  : 0
    __IO uint32_t LED_LDO5V_EN                : 1  ; // 1  : 1
    __IO uint32_t RESERVED3                   : 2  ; // 3  : 2
    __IO uint32_t LED_LDO5V_OUT_SEL           : 1  ; // 4  : 4
    __IO uint32_t RESERVED2                   : 3  ; // 7  : 5
    __IO uint32_t LED_RESERVE                 : 9  ; // 16 : 8
    __IO uint32_t RESERVED1                   : 3  ; // 19 : 17
    __IO uint32_t LED_LDO_RDY                 : 1  ; // 20 : 20
    __IO uint32_t RESERVED0                   : 11 ; // 31 : 21
} PWM_LED_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t LED_LC0_DIAG_EN             : 1  ; // 0  : 0
    __IO uint32_t LED_LC0_DISPU_EN            : 1  ; // 1  : 1
    __IO uint32_t RESERVED1                   : 2  ; // 3  : 2
    __IO uint32_t LED_LC0_IOUT_SEL            : 4  ; // 7  : 4
    __IO uint32_t LED_LC0_SEL_TR              : 2  ; // 9  : 8
    __IO uint32_t LED_LC0_SEL_TF              : 2  ; // 11 : 10
    __IO uint32_t RESERVED0                   : 20 ; // 31 : 12
} PWM_LED_LC0_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t LED_LC1_DIAG_EN             : 1  ; // 0  : 0
    __IO uint32_t LED_LC1_DISPU_EN            : 1  ; // 1  : 1
    __IO uint32_t RESERVED1                   : 2  ; // 3  : 2
    __IO uint32_t LED_LC1_IOUT_SEL            : 4  ; // 7  : 4
    __IO uint32_t LED_LC1_SEL_TR              : 2  ; // 9  : 8
    __IO uint32_t LED_LC1_SEL_TF              : 2  ; // 11 : 10
    __IO uint32_t RESERVED0                   : 20 ; // 31 : 12
} PWM_LED_LC1_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t LED_LC2_DIAG_EN             : 1  ; // 0  : 0
    __IO uint32_t LED_LC2_DISPU_EN            : 1  ; // 1  : 1
    __IO uint32_t RESERVED1                   : 2  ; // 3  : 2
    __IO uint32_t LED_LC2_IOUT_SEL            : 4  ; // 7  : 4
    __IO uint32_t LED_LC2_SEL_TR              : 2  ; // 9  : 8
    __IO uint32_t LED_LC2_SEL_TF              : 2  ; // 11 : 10
    __IO uint32_t RESERVED0                   : 20 ; // 31 : 12
} PWM_LED_LC2_CTRL_FIELD_T;

typedef struct
{
    __IO uint32_t WR_LOCK                     : 1  ; // 0  : 0
    __IO uint32_t RESERVED0                   : 31 ; // 31 : 1
} PWM_WR_LOCK_FIELD_T;

// Register Map Structure Definition
typedef struct
{
    union
    {
        __IO  uint32_t CTRL                        ; // 0x0000
        PWM_CTRL_FIELD_T CTRL_F                    ;
    };

    union
    {
        __IO  uint32_t CNT_CTRL                    ; // 0x0004
        PWM_CNT_CTRL_FIELD_T CNT_CTRL_F            ;
    };

    union
    {
        __IO  uint32_t CNT_CFG                     ; // 0x0008
        PWM_CNT_CFG_FIELD_T CNT_CFG_F              ;
    };

    union
    {
        __IO  uint32_t CH_CTRL                     ; // 0x000C
        PWM_CH_CTRL_FIELD_T CH_CTRL_F              ;
    };

    union
    {
        __IO  uint32_t CH0_PWM_CFG                 ; // 0x0010
        PWM_CH0_PWM_CFG_FIELD_T CH0_PWM_CFG_F      ;
    };

    union
    {
        __IO  uint32_t CH1_PWM_CFG                 ; // 0x0014
        PWM_CH1_PWM_CFG_FIELD_T CH1_PWM_CFG_F      ;
    };

    union
    {
        __IO  uint32_t CH2_PWM_CFG                 ; // 0x0018
        PWM_CH2_PWM_CFG_FIELD_T CH2_PWM_CFG_F      ;
    };

    union
    {
        __IO  uint32_t CH3_PWM_CFG                 ; // 0x001C
        PWM_CH3_PWM_CFG_FIELD_T CH3_PWM_CFG_F      ;
    };

    union
    {
        __IO  uint32_t CNT_VAL                     ; // 0x0020
        PWM_CNT_VAL_FIELD_T CNT_VAL_F              ;
    };

    union
    {
        __IO  uint32_t STATUS                      ; // 0x0024
        PWM_STATUS_FIELD_T STATUS_F                ;
    };

    union
    {
        __IO  uint32_t IMR                         ; // 0x0028
        PWM_IMR_FIELD_T IMR_F                      ;
    };

    union
    {
        __IO  uint32_t ICR                         ; // 0x002C
        PWM_ICR_FIELD_T ICR_F                      ;
    };

    union
    {
        __IO  uint32_t ISR                         ; // 0x0030
        PWM_ISR_FIELD_T ISR_F                      ;
    };

    union
    {
        __IO  uint32_t IRSR                        ; // 0x0034
        PWM_IRSR_FIELD_T IRSR_F                    ;
    };

    union
    {
        __IO  uint32_t LED_CTRL                    ; // 0x0038
        PWM_LED_CTRL_FIELD_T LED_CTRL_F            ;
    };

    union
    {
        __IO  uint32_t LED_LC0_CTRL                ; // 0x003C
        PWM_LED_LC0_CTRL_FIELD_T LED_LC0_CTRL_F    ;
    };

    union
    {
        __IO  uint32_t LED_LC1_CTRL                ; // 0x0040
        PWM_LED_LC1_CTRL_FIELD_T LED_LC1_CTRL_F    ;
    };

    union
    {
        __IO  uint32_t LED_LC2_CTRL                ; // 0x0044
        PWM_LED_LC2_CTRL_FIELD_T LED_LC2_CTRL_F    ;
    };

    uint32_t RESERVED0[45]                          ;

    union
    {
        __IO  uint32_t WR_LOCK                     ; // 0x00FC
        PWM_WR_LOCK_FIELD_T WR_LOCK_F              ;
    };

} PWM_REG_TypeDef;

#endif
