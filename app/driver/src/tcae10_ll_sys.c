/**
 *****************************************************************************
 * @brief   syscfg Source file.
 *
 * @file    tcae10_ll_sys.c
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

#include "tcae10_ll_sys.h"
#include "tcae10_ll_def.h"
#include "tcae10_ll_cortex.h"

/**
 * @brief   配置中断向量表重映射
 * @param   vetor_offset - 向量表偏移地址（需8字节对齐）
 * @param   enable       - true使能重映射，false恢复默认
 * @note    将Cortex-M0+向量表基地址重映射到指定地址，用于Bootloader跳转后重定向中断向量
 * @retval  None
 */
void ll_syscfg_remap_config(uint32_t vetor_offset, bool enable)
{
    SYSCFG_CONFIG_UNLOCK();

    SYSCFG->REMAP_F.CM0_VECT_BASE_ADDR = vetor_offset >> 8;     /* 设置向量表基址（高24位） */
    SYSCFG->REMAP_F.CM0_REMAP_EN = (enable) ? 1 : 0;            /* 使能/禁能重映射 */

    SYSCFG_CONFIG_LOCK();
}

/**
 * @brief   获取MCU芯片的Revision ID和Chip ID
 * @param   revision_id - 指向存储Revision ID的变量指针（可为NULL）
 * @param   chip_id     - 指向存储Chip ID的变量指针（可为NULL）
 * @note    从SYSCFG的REVISION寄存器读取芯片版本和标识信息
 * @retval  None
 */
void ll_syscfg_info_get(uint8_t *revision_id, uint16_t *chip_id)
{
    SYSCFG_CONFIG_UNLOCK();

    if (NULL != chip_id)
    {
        *chip_id = (uint16_t)(SYSCFG->REVISION_F.CHIP_ID);      /* 读取芯片ID */
    }

    if (NULL != revision_id)
    {
        *revision_id = (uint8_t)(SYSCFG->REVISION_F.REV_ID);    /* 读取版本号 */
    }

    SYSCFG_CONFIG_LOCK();
}

/**
 * @brief   初始化唤醒源配置
 * @param   source - 唤醒源选择（LIN/IO0等）
 * @param   time   - 唤醒信号最大等待时间，周期=1/32KHz≈31.25us
 * @param   filter - 唤醒信号滤波时钟个数
 * @note    配置ASYSCFG的WU_CTRL寄存器，选择唤醒源、定时和滤波参数
 * @retval  None
 */
void ll_wakeup_init(wakeup_source_e source, wakeup_time_e time, wakeup_filter_e filter)
{
    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->WU_CTRL_F.WU_SRC_SEL = source;                     /* 选择唤醒源 */
    ASYSCFG->WU_CTRL_F.WU_TIM = time;                           /* 设置唤醒等待时间 */
    ASYSCFG->WU_CTRL_F.WU_FILTER_CNT_MAX = filter;              /* 设置滤波计数最大值 */
    ASYSCFG->WU_CTRL_F.WU_EN = 1;                               /* 使能唤醒功能 */

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   使能或禁能ASYSCFG相关中断
 * @param   isr    - 中断类型，可取@ref asyscfg_isr_type_e中的值
 * @param   enable - true使能中断，false禁能
 * @note    先清除中断标志，再设置IMR寄存器的对应位
 * @retval  None
 */
void ll_syscfg_isr_enable(asyscfg_isr_type_e isr, bool enable)
{
    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->ICR |= (isr);                                      /* 清除中断标志 */

    if (enable)
    {
        ASYSCFG->IMR &= ~(isr);                                 /* 清除IMR对应位，使能中断 */
    }
    else
    {
        ASYSCFG->IMR |= (isr);                                  /* 设置IMR对应位，禁能中断 */
    }

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   清除ASYSCFG相关中断标志
 * @param   isr - 要清除的中断类型，可取@ref asyscfg_isr_type_e中的值
 * @retval  None
 */
void ll_syscfg_isr_clear(asyscfg_isr_type_e isr)
{
    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->ICR |= (isr);                                      /* 写ICR清除中断标志 */

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   获取ASYSCFG中断标志状态
 * @param   isr - 要查询的中断类型，可取@ref asyscfg_isr_type_e中的值
 * @retval  中断标志状态（非0表示中断已发生）
 */
uint8_t ll_syscfg_isr_get(asyscfg_isr_type_e isr)
{
    return (uint8_t)(ASYSCFG->ISR & (isr));                     /* 读取ISR寄存器 */
}

/**
 * @brief   写数据到备份寄存器
 * @param   zone - 备份区选择（SYSTEM_BACKUP_ZONE_0或SYSTEM_BACKUP_ZONE_1）
 * @param   data - 要写入的32位数据
 * @note    备份寄存器在低功耗模式下保持数据不丢失
 * @retval  None
 */
void ll_syscfg_backup_reg_write(sys_backup_zone_e zone, uint32_t data)
{
    ASYSCFG_CONFIG_UNLOCK();

    if (SYSTEM_BACKUP_ZONE_0 == zone)
    {
        ASYSCFG->BKUP0 = data;                                  /* 写入备份寄存器0 */
    }
    else
    {
        ASYSCFG->BKUP1 = data;                                  /* 写入备份寄存器1 */
    }

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   从备份寄存器读取数据
 * @param   zone - 备份区选择（SYSTEM_BACKUP_ZONE_0或SYSTEM_BACKUP_ZONE_1）
 * @param   data - 指向存储读取数据的变量指针
 * @retval  None
 */
void ll_syscfg_backup_reg_read(sys_backup_zone_e zone, uint32_t *data)
{
    if (SYSTEM_BACKUP_ZONE_0 == zone)
    {
        *data = ASYSCFG->BKUP0;                                 /* 读取备份寄存器0 */
    }
    else
    {
        *data = ASYSCFG->BKUP1;                                 /* 读取备份寄存器1 */
    }
}

/**
 * @brief   使能或禁能OTP（过温保护）中断
 * @param   enable - true使能OTP中断，false禁能
 * @note    配置OTP触发模式为上升沿触发，使能ASYSCFG中断和NVIC
 * @retval  None
 */
void ll_syscfg_otp_enable(bool enable)
{
    ASYSCFG->PMU_IRQ_CTRL_F.OTP_IRQ_MODE = ASYSCFG_TRIGGER_POSEDGE;  /* 设置OTP触发方式 */

    ll_syscfg_isr_enable(ASYSCFG_INT_OTP, enable);              /* 使能/禁能OTP中断 */
    NVIC_ClearPendingIRQ(AON_IRQn);                             /* 清除AON中断挂起 */

    if (enable)
    {
        NVIC_SetPriority(AON_IRQn, 3);                          /* 设置中断优先级 */
        NVIC_EnableIRQ(AON_IRQn);                               /* 使能AON中断 */
    }
    else
    {
        NVIC_DisableIRQ(AON_IRQn);                              /* 禁能AON中断 */
    }
}

/**
 * @brief   获取OTP（过温保护）状态
 * @param   None
 * @retval  true - 过温保护触发，false - 正常温度
 */
bool ll_syscfg_otp_status(void)
{
    return !!(ASYSCFG->OPT_STATUS_F.OTP_OUT);                   /* 读取OTP输出状态 */
}

/**
 * @brief   AON（Always-On）中断处理函数
 * @note    （注释掉）- 清除ASYSCFG中断标志
 * @retval  None
 */
//void AON_IRQHandler(void)
//{
//    uint32_t isr = ASYSCFG->ISR;

//    ASYSCFG_CONFIG_UNLOCK();

//    ASYSCFG->ICR |= isr;

//    ASYSCFG_CONFIG_LOCK();
//}
