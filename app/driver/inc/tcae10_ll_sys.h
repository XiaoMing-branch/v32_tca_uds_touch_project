/**
 *****************************************************************************
 * @brief   syscfg header file.
 *
 * @file    tcpl03x_ll_sys.h
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

#ifndef __TCPL03X_LL_SYS_H__
#define __TCPL03X_LL_SYS_H__

#include <stdint.h>
#include <stdbool.h>
#include "tcae10.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
  * @defgroup ASYSCFG_OTP_RST_Definitions
  */
/** @brief  使能OTP复位 */
#define ASYSCFG_OTP_RST_ENABLE()    (ASYSCFG->OTP_RST_CTRL_F.OTP_RST_EN=1)
/** @brief  禁能OTP复位 */
#define ASYSCFG_OTP_RST_DISABLE()   (ASYSCFG->OTP_RST_CTRL_F.OTP_RST_EN=0)

/**
  * @defgroup ASYSCFG_HRC_CLOCK_Config_Macro
  */
/** @brief  使能软件HRC时钟（PMU高功率模式） */
#define ASYSCFG_SW_HRC_ENABLE()                     (ASYSCFG->RCCLK_CTRL_F.SW_HRC_EN=1)
/** @brief  禁能软件HRC时钟 */
#define ASYSCFG_SW_HRC_DISABLE()                    (ASYSCFG->RCCLK_CTRL_F.SW_HRC_EN=0)
/** @brief  睡眠漫步模式禁能HRC时钟 */
#define ASYSCFG_HRC_CLOCK_DISABLE_ON_SLEEPWALK()    (ASYSCFG->PMU_CTRL_F.SLWK_HRC_EN=0)
/** @brief  睡眠漫步模式使能HRC时钟 */
#define ASYSCFG_HRC_CLOCK_ENABLE_ON_SLEEPWALK()     (ASYSCFG->PMU_CTRL_F.SLWK_HRC_EN=1)


/** ]
  * @defgroup ASYSCFG_RESET_CAUSE_Definitions
  */
/** @brief  复位原因：软件POR请求 */
#define ASYSCFG_RST_CAUSE_SW_POR_REQ        0X01
/** @brief  复位原因：IO4引脚复位 */
#define ASYSCFG_RST_CAUSE_IO4_PAD_RST       0X02
/** @brief  复位原因：独立看门狗 */
#define ASYSCFG_RST_CAUSE_IWDG              0X04
/** @brief  复位原因：过温保护 */
#define ASYSCFG_RST_CAUSE_OTP               0X08
/** @brief  复位原因：VS电压跌落 */
#define ASYSCFG_RST_CAUSE_VS_ALT            0X10
/** @brief  复位原因：CM0复位或锁死 */
#define ASYSCFG_RST_CAUSE_CM0_RST           0X20

/**
  * @defgroup ASYSCFG_RESET_CONTROL_Definitions
  */
/** @brief  使能CM0锁死复位 */
#define ASYSCFG_CM0_LOCKUP_RST_ENABLE()     (ASYSCFG->RST_CTRL_F.M0_LOCKUP_EN=1)
/** @brief  禁能CM0锁死复位 */
#define ASYSCFG_CM0_LOCKUP_RST_DISABLE()    (ASYSCFG->RST_CTRL_F.M0_LOCKUP_EN=0)
/** @brief  请求软件复位（复位所有逻辑） */
#define ASYSCFG_RST_REQUEST()               (ASYSCFG->RST_CTRL_F.SW_POR_REQ=1)
/** @brief  获取复位原因 */
#define ASYSCFG_RST_CAUSE_GET()             (ASYSCFG->RST_CTRL_F.RST_FLAG)
/** @brief  清除复位原因标志 */
#define ASYSCFG_RST_CAUSE_CLEAR()           (ASYSCFG->RST_CTRL_F.CLR_RST=1)


/**
  * @defgroup SYSCFG_MISC_CONFIGS_Definitions
  */
#define SYSCFG_PWM_DISABLE_WHEN_OTP(status) \
    (SYSCFG->MISC_CTRL_F.OTP_DIS_PWM_EN=status)     /*!< Enable or disable pwm on OTP event happens
                                                         status can be ENABLE or DISALBE*/

#define SYSCFG_PWM_DISABLE_WHEN_VS_ALTER(status) \
    (SYSCFG->MISC_CTRL_F.VS_ALT_DIS_PWM_EN=status)  /*!< Enable or disable pwm on VS ALTER event happens
                                                         status can be ENABLE or DISALBE*/

#define SYSCFG_PWM_DISABLE_WHEN_VS_LVD(status) \
    (SYSCFG->MISC_CTRL_F.VS_LVD_DIS_PWM_EN=status)  /*!< Enable or disable pwm on VS LVD event happens
                                                         status can be ENABLE or DISALBE*/
/**
 * @brief  wakeup source enumeration
 */
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
    ASYSCFG_INT_WAKEUP  = ASYSCFG_IMR_WU_INT_MSK_MASK,
    ASYSCFG_INT_VSLVD   = ASYSCFG_IMR_VS_LVD_INT_MSK_MASK,
    ASYSCFG_INT_OTP     = ASYSCFG_IMR_OTP_INT_MSK_MASK,
    ASYSCFG_INT_LDO12C_OCP = ASYSCFG_ICR_LDO12C_OCP_INT_CLR_MASK,
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

/**
 * @brief  配置系统向量表重映射
 * @param vetor_offset - 向量表偏移地址
 * @param enable - true: 使能重映射，false: 禁能
 */
void ll_syscfg_remap_config(uint32_t vetor_offset, bool enable);
/**
 * @brief  获取芯片版本和ID信息
 * @param revision_id - 版本号输出指针
 * @param chip_id - 芯片ID输出指针
 */
void ll_syscfg_info_get(uint8_t *revision_id, uint16_t *chip_id);
/**
 * @brief  初始化唤醒源配置
 * @param source - 唤醒源选择 @ref wakeup_source_e
 * @param time - 唤醒时间 @ref wakeup_time_e
 * @param filter - 唤醒滤波 @ref wakeup_filter_e
 */
void ll_wakeup_init(wakeup_source_e source, wakeup_time_e time, wakeup_filter_e filter);
/**
 * @brief  使能/禁能系统配置中断
 * @param isr - 中断类型 @ref asyscfg_isr_type_e
 * @param enable - true: 使能，false: 禁能
 */
void ll_syscfg_isr_enable(asyscfg_isr_type_e isr, bool enable);
/**
 * @brief  使能/禁能OTP过温保护
 * @param enable - true: 使能，false: 禁能
 */
void ll_syscfg_otp_enable(bool enable);
/**
 * @brief  获取OTP过温保护状态
 * @retval true: 过温触发，false: 正常
 */
bool ll_syscfg_otp_status(void);
/**
 * @brief  清除系统配置中断标志
 * @param isr - 中断类型 @ref asyscfg_isr_type_e
 */
void ll_syscfg_isr_clear(asyscfg_isr_type_e isr);
/**
 * @brief  获取系统配置中断标志状态
 * @param isr - 中断类型 @ref asyscfg_isr_type_e
 * @retval 中断状态值
 */
uint8_t ll_syscfg_isr_get(asyscfg_isr_type_e isr);
/**
 * @brief  写入系统备份寄存器
 * @param zone - 备份区选择 @ref sys_backup_zone_e
 * @param data - 要写入的数据
 */
void ll_syscfg_backup_reg_write(sys_backup_zone_e zone, uint32_t data);
/**
 * @brief  读取系统备份寄存器
 * @param zone - 备份区选择 @ref sys_backup_zone_e
 * @param data - 读取数据输出指针
 */
void ll_syscfg_backup_reg_read(sys_backup_zone_e zone, uint32_t *data);


#if defined(__cplusplus)
}
#endif
#endif /* __TCPL03X_LL_SYS_H__ */
