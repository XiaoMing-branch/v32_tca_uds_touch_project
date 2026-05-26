/**
 *****************************************************************************
 * @brief   syscfg Source file.
 *
 * @file    tcpl03x_ll_sys.c
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

#include "tcpl03x_ll_sys.h"
#include "tcpl03x_ll_def.h"
#include "tcpl03x_ll_cortex.h"

/**
 * @brief   获取 MCU 芯片的版本 ID 和芯片 ID
 *
 * @param[out]  revision_id  指向存储版本 ID 的缓冲区指针（可为 NULL）
 * @param[out]  chip_id      指向存储芯片 ID 的缓冲区指针（可为 NULL）
 *
 * @retval  None
 *
 * @note    读取寄存器前需解锁系统配置区，完成后重新锁定防止误写。
 */
void ll_syscfg_info_get(uint8_t *revision_id, uint16_t *chip_id)
{
    /* 解锁系统配置寄存器，允许读取芯片信息 */
    SYSCFG_CONFIG_UNLOCK();

    if (NULL != chip_id)
    {
        /* 读取芯片 ID（CHIP_ID 字段） */
        *chip_id = (uint16_t)(SYSCFG->REVISION_F.CHIP_ID);
    }

    if (NULL != revision_id)
    {
        /* 读取版本 ID（REV_ID 字段） */
        *revision_id = (uint8_t)(SYSCFG->REVISION_F.REV_ID);
    }

    /* 重新锁定系统配置寄存器 */
    SYSCFG_CONFIG_LOCK();
}

/**
 * @brief   初始化唤醒功能
 *
 * @param   source   唤醒源选择，参考 @ref wakeup_source_e
 * @param   time     唤醒信号最大持续时间，时钟周期 = 1/32KHz ≈ 31.25us，参考 @ref wakeup_time_e
 * @param   filter   唤醒信号滤波最大时钟数，参考 @ref wakeup_filter_e
 *
 * @retval  None
 *
 * @note    配置完成后会自动使能唤醒功能（WU_EN 置 1），
 *          操作前需解锁异步系统配置区，完成后重新锁定。
 */
void ll_wakeup_init(wakeup_source_e source, wakeup_time_e time, wakeup_filter_e filter)
{
    /* 解锁异步系统配置寄存器 */
    ASYSCFG_CONFIG_UNLOCK();

    /* 配置唤醒源选择 */
    ASYSCFG->WU_CTRL_F.WU_SRC_SEL = source;
    /* 配置唤醒信号持续时间 */
    ASYSCFG->WU_CTRL_F.WU_TIM = time;
    /* 配置唤醒信号滤波时钟数 */
    ASYSCFG->WU_CTRL_F.WU_FILTER_CNT_MAX = filter;
    /* 使能唤醒功能 */
    ASYSCFG->WU_CTRL_F.WU_EN = 1;

    /* 重新锁定异步系统配置寄存器 */
    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   使能或禁能异步系统配置相关中断
 *
 * @param   isr      中断类型，参考 @ref asyscfg_isr_type_e
 * @param   enable   使能（true）/ 禁能（false）
 *
 * @retval  None
 *
 * @note    中断使能时开放中断屏蔽（IMR 对应位清零），
 *          禁能时屏蔽中断（IMR 对应位置位）。
 *          操作前需解锁异步系统配置区。
 */
void ll_syscfg_isr_enable(asyscfg_isr_type_e isr, bool enable)
{
    /* 解锁异步系统配置寄存器 */
    ASYSCFG_CONFIG_UNLOCK();

    /* 清除待处理的中断标志 */
    ASYSCFG->ICR |= (isr);

    if (enable)
    {
        /* 使能中断：清除中断屏蔽寄存器对应位 */
        ASYSCFG->IMR &= ~(isr);
    }
    else
    {
        /* 禁能中断：置位中断屏蔽寄存器对应位 */
        ASYSCFG->IMR |= (isr);
    }

    /* 重新锁定异步系统配置寄存器 */
    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   清除异步系统配置相关中断标志
 *
 * @param   isr  中断类型，参考 @ref asyscfg_isr_type_e
 *
 * @retval  None
 *
 * @note    通过写中断清除寄存器（ICR）对应位来清除中断标志，
 *          操作前需解锁异步系统配置区。
 */
void ll_syscfg_isr_clear(asyscfg_isr_type_e isr)
{
    /* 解锁异步系统配置寄存器 */
    ASYSCFG_CONFIG_UNLOCK();

    /* 写 ICR 寄存器清除指定中断标志 */
    ASYSCFG->ICR |= (isr);

    /* 重新锁定异步系统配置寄存器 */
    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   获取异步系统配置相关中断标志状态
 *
 * @param   isr  中断类型，参考 @ref asyscfg_isr_type_e
 *
 * @retval  0    中断标志未置位
 * @retval  !0   中断标志已置位（返回对应 ISR 寄存器位值）
 *
 * @note    该函数不涉及寄存器解锁操作，仅读取中断状态寄存器（ISR）。
 */
uint8_t ll_syscfg_isr_get(asyscfg_isr_type_e isr)
{
    /* 读取中断状态寄存器（ISR），与指定中断类型屏蔽后返回 */
    return (uint8_t)(ASYSCFG->ISR & (isr));
}

/**
 * @brief   向指定的备份寄存器写入数据
 *
 * @param   zone  备份寄存器区域选择，参考 @ref sys_backup_zone_e
 * @param   data  待写入的 32 位数据
 *
 * @retval  None
 *
 * @note    支持 BKUP0 和 BKUP1 两个备份寄存器，
 *          操作前需解锁异步系统配置区，完成后重新锁定。
 *          备份寄存器在系统休眠期间内容保持。
 */
void ll_syscfg_backup_reg_write(sys_backup_zone_e zone, uint32_t data)
{
    /* 解锁异步系统配置寄存器 */
    ASYSCFG_CONFIG_UNLOCK();

    if (SYSTEM_BACKUP_ZONE_0 == zone)
    {
        /* 写入备份寄存器 BKUP0 */
        ASYSCFG->BKUP0 = data;
    }
    else
    {
        /* 写入备份寄存器 BKUP1 */
        ASYSCFG->BKUP1 = data;
    }

    /* 重新锁定异步系统配置寄存器 */
    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   从指定的备份寄存器读取数据
 *
 * @param   zone  备份寄存器区域选择，参考 @ref sys_backup_zone_e
 * @param[out]  data  指向存储读取数据的 32 位缓冲区指针
 *
 * @retval  None
 *
 * @note    该函数不涉及寄存器解锁操作，直接读取备份寄存器内容。
 *          调用前需确保 data 指针非空。
 */
void ll_syscfg_backup_reg_read(sys_backup_zone_e zone, uint32_t *data)
{
    if (SYSTEM_BACKUP_ZONE_0 == zone)
    {
        /* 从备份寄存器 BKUP0 读取数据 */
        *data = ASYSCFG->BKUP0;
    }
    else
    {
        /* 从备份寄存器 BKUP1 读取数据 */
        *data = ASYSCFG->BKUP1;
    }
}

/**
 * @brief   使能或禁能 OTP（过温保护）中断功能
 *
 * @param   enable  使能（true）/ 禁能（false）
 *
 * @retval  None
 *
 * @note    使能时配置 OTP 中断触发方式为上升沿触发，
 *          设置 AON_IRQn 中断优先级为 3，
 *          并使能 NVIC 中的 AON 中断线。
 *          禁能时关闭 AON 中断线。
 */
void ll_syscfg_otp_enable(bool enable)
{
    /* 解锁异步系统配置寄存器 */
    ASYSCFG_CONFIG_UNLOCK();

    /* 配置 OTP 中断触发模式为上升沿触发 */
    ASYSCFG->PMU_IRQ_CTRL_F.OTP_IRQ_MODE = ASYSCFG_TRIGGER_POSEDGE;

    /* 重新锁定异步系统配置寄存器 */
    ASYSCFG_CONFIG_LOCK();

    /* 使能/禁能 OTP 中断 */
    ll_syscfg_isr_enable(ASYSCFG_INT_OTP, enable);
    /* 清除 AON 中断待处理标志 */
    NVIC_ClearPendingIRQ(AON_IRQn);

    if (enable)
    {
        /* 设置 AON 中断优先级为 3 */
        NVIC_SetPriority(AON_IRQn, 3);
        /* 使能 NVIC 中的 AON 中断线 */
        NVIC_EnableIRQ(AON_IRQn);
    }
    else
    {
        /* 禁能 NVIC 中的 AON 中断线 */
        NVIC_DisableIRQ(AON_IRQn);
    }
}

/**
 * @brief   获取 OTP（过温保护）状态
 *
 * @param   None
 *
 * @retval  true   OTP 触发（过温事件发生）
 * @retval  false  OTP 未触发（温度正常）
 *
 * @note    直接读取 OTP 状态寄存器（OPT_STATUS_F.OTP_OUT）获得过温标志。
 */
bool ll_syscfg_otp_status(void)
{
    /* 读取 OTP 输出状态寄存器位 */
    return !!(ASYSCFG->OPT_STATUS_F.OTP_OUT);
}

/**
 * @brief   AON（Always-On）中断服务函数
 *
 * @param   None
 *
 * @retval  None
 *
 * @note    读取中断状态寄存器（ISR）后通过 ICR 寄存器清除所有已触发的中断标志。
 *          该处理程序响应来自异步系统配置模块的中断事件。
 */
void AON_IRQHandler(void)
{
    /* 读取中断状态寄存器，保存所有待处理中断标志 */
    uint32_t isr = ASYSCFG->ISR;

    /* 解锁异步系统配置寄存器 */
    ASYSCFG_CONFIG_UNLOCK();

    /* 写 ICR 寄存器清除所有已触发的中断标志 */
    ASYSCFG->ICR |= isr;

    /* 重新锁定异步系统配置寄存器 */
    ASYSCFG_CONFIG_LOCK();
}
