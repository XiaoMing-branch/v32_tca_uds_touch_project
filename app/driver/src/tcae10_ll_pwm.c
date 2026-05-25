/**
 *****************************************************************************
 * @brief   pwm driver source file.
 *
 * @file    tcae10_ll_pwm.c
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

#include "tcae10_ll_gpio.h"
#include "tcae10_ll_pwm.h"
#include "system_tcae10.h"
#include "tcae10_ll_cortex.h"

#define PWM_ISR_FLAG       (0xFFFFUL)

static ISR_FUNC_CALLBACK pwm_isr_callback = NULL;

/**
 * @brief   配置PWM模块时钟
 * @param   config - 时钟配置参数（包含时钟源选择和分频系数）
 * @retval  None
 */
static void ll_pwm_clk_config(ll_clk_config_t *config)
{
    CRG_CONFIG_UNLOCK();

    CRG->PWM_CLKRST_CTRL_F.PCLK_EN_PWM = 1;                     /* 使能PWM PCLK */
    CRG->PWM_CLKRST_CTRL_F.FCLK_EN_PWM = 1;                     /* 使能PWM FCLK */
    CRG->PWM_CLKRST_CTRL_F.FCLK_SEL_PWM = config->clk_source;   /* 选择时钟源 */
    CRG->PWM_CLKRST_CTRL_F.FCLK_DIV_PWM = config->fclk_div;     /* 设置分频系数 */

    CRG_CONFIG_LOCK();
}

/**
 * @brief   配置PWM输出GPIO引脚复用
 * @param   mode - HVIO模式（LED模式或PWM模式）
 * @note    根据模式选择AFIO复用通道，配置GPIO6~8的复用功能
 * @retval  None
 */
static void ll_pwm_gpio_config(pwm_hvio_mode_e mode)
{
    gpio_afio_mux_e afio_mux = (HVIO_MODE_LED == mode) ? AFIO_MUX_0 : AFIO_MUX_1;
    ll_gpio_afio_config(GPIO_PIN_6, afio_mux);
    ll_gpio_afio_config(GPIO_PIN_7, afio_mux);
    ll_gpio_afio_config(GPIO_PIN_8, afio_mux);
}

/**
 * @brief   配置PWM中断
 * @param   config   - 中断配置参数
 * @param   callback - 中断回调函数指针
 * @note    清除中断标志，根据使能状态配置IMR寄存器和回调
 * @retval  None
 */
static void ll_pwm_isr_config(ll_isr_config_t *config, ISR_FUNC_CALLBACK callback)
{
    PWM->ICR |= PWM_ISR_FLAG;                                    /* 清除PWM中断标志 */

    if (config->isr_enable)
    {
        PWM->IMR &= ~(config->isr & PWM_ISR_FLAG);               /* 使能指定中断 */
        pwm_isr_callback = callback;
    }
    else
    {
        PWM->IMR = PWM_ISR_FLAG;                                 /* 禁能所有中断 */
        pwm_isr_callback = NULL;
    }
}

/**
 * @brief   使能或禁能PWM中断标志位（掩码控制）
 * @param   bus    - PWM总线号
 * @param   isr    - 中断标志位
 * @param   enable - true使能，false禁能
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_isr_flag_enable(ll_pwm_bus_e bus, uint32_t isr, bool enable)
{
    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    PWM->ICR |= PWM_ISR_FLAG;                                     /* 清除中断标志 */

    if (enable)
    {
        PWM->IMR = PWM_ISR_FLAG;
        PWM->IMR &= ~(isr & PWM_ISR_FLAG);                         /* 使能指定中断位 */
    }
    else
    {
        PWM->IMR |= (isr & PWM_ISR_FLAG);                          /* 禁能指定中断位 */
    }

    return LL_OK;
}

/**
 * @brief   使能或禁能PWM NVIC中断
 * @param   bus    - PWM总线号
 * @param   enable - true使能NVIC中断，false禁能
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_isr_enable(ll_pwm_bus_e bus, bool enable)
{
    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    PWM->ICR |= PWM_ISR_FLAG;                                     /* 清除中断标志 */

    if (enable)
    {
        NVIC_ClearPendingIRQ(PWM_IRQn);
        NVIC_EnableIRQ(PWM_IRQn);
    }
    else
    {
        PWM->IMR |= PWM_ISR_FLAG;                                  /* 禁能所有中断 */
    }

    return LL_OK;
}

/**
 * @brief   清除PWM中断标志
 * @param   bus  - PWM总线号
 * @param   flag - 要清除的中断标志
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_isr_clear(ll_pwm_bus_e bus, uint32_t flag)
{
    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    PWM->ICR |= flag;                                             /* 写ICR清除中断标志 */

    return LL_OK;
}

/**
 * @brief   获取PWM中断标志
 * @param   bus  - PWM总线号
 * @param   flag - 指向存储中断标志的变量指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_isr_flag_get(ll_pwm_bus_e bus, uint32_t *flag)
{
    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    *flag = PWM->ISR;                                             /* 读取中断状态寄存器 */

    return LL_OK;
}

/**
 * @brief   使能或禁能PWM输出
 * @param   mode   - HVIO模式（LED模式需使能5V LDO）
 * @param   enable - true使能PWM输出，false禁能
 * @note    LED模式下需要先使能5V LDO并等待其就绪，再使能LED输出。
 *         所有通道（CH0~2）使能后，同步使能计数器。
 * @retval  None
 */
void ll_pwm_enable(pwm_hvio_mode_e mode, bool enable)
{
    if (HVIO_MODE_LED == mode)
    {
        /* LED模式需要使能5V LDO */
        PWM->LED_CTRL_F.LED_LDO5V_EN = enable;                    /* 使能5V LDO */

        while (PWM->LED_CTRL_F.LED_LDO_RDY == 0);                 /* 等待LDO就绪 */

        PWM->LED_CTRL_F.LED_EN = enable ? 1 : 0;                  /* 使能/禁能LED输出 */
    }

    PWM->CH_CTRL_F.CH0_EN = 1;                                    /* 使能通道0 */
    PWM->CH_CTRL_F.CH1_EN = 1;                                    /* 使能通道1 */
    PWM->CH_CTRL_F.CH2_EN = 1;                                    /* 使能通道2 */

    PWM->CTRL_F.CH_SYNC_EN = (enable ? 1 : 0);                    /* 使能同步 */
    PWM->CNT_CTRL_F.CNT0_EN = (enable ? 1 : 0);                   /* 使能计数器0 */
}

/**
 * @brief   配置PWM通道阈值（决定占空比）
 * @param   channel     - PWM通道号（CH0~CH2）
 * @param   threshold_h - 高阈值（决定PWM置位点）
 * @param   threshold_l - 低阈值（决定PWM清除点）
 * @note    通过设定高/低阈值控制PWM波形的占空比和相位
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_channel_threshold_config(pwm_channel_e channel, uint16_t threshold_h, uint16_t threshold_l)
{
    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    switch (channel)
    {
        case PWM_CHANNEL_0:
            PWM->CH0_PWM_CFG_F.HT0 = threshold_h;                 /* 设置通道0高阈值 */
            PWM->CH0_PWM_CFG_F.LT0 = threshold_l;                 /* 设置通道0低阈值 */
            break;

        case PWM_CHANNEL_1:
            PWM->CH1_PWM_CFG_F.HT1 = threshold_h;                 /* 设置通道1高阈值 */
            PWM->CH1_PWM_CFG_F.LT1 = threshold_l;                 /* 设置通道1低阈值 */
            break;

        case PWM_CHANNEL_2:
            PWM->CH2_PWM_CFG_F.HT2 = threshold_h;                 /* 设置通道2高阈值 */
            PWM->CH2_PWM_CFG_F.LT2 = threshold_l;                 /* 设置通道2低阈值 */
            break;

        default:
            break;
    }

    return LL_OK;
}

/**
 * @brief   配置PWM通道周期
 * @param   channel - PWM通道号
 * @param   period  - 周期值
 * @note    设置计数器0的周期值，所有通道共享
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_channel_period_config(pwm_channel_e channel, uint16_t period)
{
    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    PWM->CNT_CFG_F.PERIOD0 = period;                             /* 设置计数器0周期 */

    return LL_OK;
}

/**
 * @brief   获取PWM通道高阈值
 * @param   channel - PWM通道号
 * @param   value   - 指向存储高阈值的变量指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_high_threshold_get(pwm_channel_e channel, uint16_t *value)
{
    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    switch (channel)
    {
        case PWM_CHANNEL_0:
            *value = PWM->CH0_PWM_CFG_F.HT0;                     /* 读取通道0高阈值 */
            break;

        case PWM_CHANNEL_1:
            *value = PWM->CH1_PWM_CFG_F.HT1;                     /* 读取通道1高阈值 */
            break;

        case PWM_CHANNEL_2:
            *value = PWM->CH2_PWM_CFG_F.HT2;                     /* 读取通道2高阈值 */
            break;

        default:
            break;
    }

    return LL_OK;
}

/**
 * @brief   获取PWM通道当前计数器值
 * @param   channel - PWM通道号
 * @param   value   - 指向存储计数值的变量指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e LL_pwm_channel_counter_get(pwm_channel_e channel, uint16_t *value)
{
    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    return LL_OK;
}

/**
 * @brief   设置PWM刹车（Break）功能
 * @param   enable - true使能刹车（禁能5V LDO和LED），false恢复
 * @note    刹车时关闭5V LDO和LED输出，实现快速关断
 * @retval  None
 */
void ll_pwm_break_set(bool enable)
{
    PWM->LED_CTRL_F.LED_LDO5V_EN = !enable;                      /* 反向控制5V LDO */

    while (PWM->LED_CTRL_F.LED_LDO_RDY == 0);                    /* 等待LDO状态 */

    PWM->LED_CTRL_F.LED_EN = !enable ? 1 : 0;                    /* 反向控制LED输出 */
}

/**
 * @brief   获取PWM状态
 * @param   channel - PWM通道号
 * @param   value   - 指向存储状态值的变量指针
 * @note    状态寄存器低6位包含通道状态信息
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_status_get(pwm_channel_e channel, uint8_t *value)
{
    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    *value = PWM->STATUS  & 0x3F;                                /* 读取状态寄存器低6位 */
    return LL_OK;
}

/**
 * @brief   配置LED驱动电流
 * @param   channel - LED通道号
 * @param   current - 驱动电流档位（5mA~45mA）
 * @note    同时禁能放电PU电路
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_led_driver_current_config(pwm_channel_e channel, led_driver_current_e current)
{
    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    switch (channel)
    {
        case PWM_CHANNEL_0:
            PWM->LED_LC0_CTRL_F.LED_LC0_DISPU_EN = 0;             /* 禁能放电PU */
            PWM->LED_LC0_CTRL_F.LED_LC0_IOUT_SEL = current;       /* 设置驱动电流 */
            break;

        case PWM_CHANNEL_1:
            PWM->LED_LC1_CTRL_F.LED_LC1_DISPU_EN = 0;
            PWM->LED_LC1_CTRL_F.LED_LC1_IOUT_SEL = current;
            break;

        case PWM_CHANNEL_2:
            PWM->LED_LC2_CTRL_F.LED_LC2_DISPU_EN = 0;
            PWM->LED_LC2_CTRL_F.LED_LC2_IOUT_SEL = current;
            break;

        default:
            break;
    }

    return LL_OK;
}

/**
 * @brief   配置LED诊断电流
 * @param   channel - LED通道号
 * @param   current - 诊断电流档位
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_led_diag_current_config(pwm_channel_e channel, led_diag_current_e current)
{
    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    return LL_OK;
}

/**
 * @brief   使能或禁能LED诊断功能
 * @param   channel - LED通道号
 * @param   enable  - true使能诊断，false禁能
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_led_diagnose_enable(pwm_channel_e channel, bool enable)
{

    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    switch (channel)
    {
        case PWM_CHANNEL_0:
            PWM->LED_LC0_CTRL_F.LED_LC0_DIAG_EN = enable ? 1 : 0;  /* 使能/禁能诊断 */
            break;

        case PWM_CHANNEL_1:
            PWM->LED_LC1_CTRL_F.LED_LC1_DIAG_EN = enable ? 1 : 0;
            break;

        case PWM_CHANNEL_2:
            PWM->LED_LC2_CTRL_F.LED_LC2_DIAG_EN = enable ? 1 : 0;
            break;

        default:
            break;
    }

    return LL_OK;
}

/**
 * @brief   设置LED上升沿和下降沿时间（斜率控制）
 * @param   channel  - LED通道号
 * @param   rise_time - 上升沿时间选择
 * @param   fall_time - 下降沿时间选择
 * @note    用于控制LED亮灭的柔和度，减少EMI
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_led_fall_rise_time_set(pwm_channel_e channel, led_rise_sr_time_e rise_time, led_fall_sr_time_e fall_time)
{

    if (channel >= PWM_CHANNEL_MAX)
    {
        return LL_ERROR;
    }

    switch (channel)
    {
        case PWM_CHANNEL_0:
            PWM->LED_LC0_CTRL_F.LED_LC0_SEL_TR = rise_time;         /* 设置上升时间 */
            PWM->LED_LC0_CTRL_F.LED_LC0_SEL_TF = fall_time;         /* 设置下降时间 */
            break;

        case PWM_CHANNEL_1:
            PWM->LED_LC1_CTRL_F.LED_LC1_SEL_TR = rise_time;
            PWM->LED_LC1_CTRL_F.LED_LC1_SEL_TF = fall_time;
            break;

        case PWM_CHANNEL_2:
            PWM->LED_LC2_CTRL_F.LED_LC2_SEL_TR = rise_time;
            PWM->LED_LC2_CTRL_F.LED_LC2_SEL_TF = fall_time;
            break;

        default:
            break;
    }

    return LL_OK;
}

/**
 * @brief   使能或禁能LED开关（通道3）
 * @param   bus    - PWM总线号
 * @param   enable - true使能，false禁能
 * @note    通道3使用独立的计数器1，用于LED开关控制
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_led_switch_enable(ll_pwm_bus_e bus, bool enable)
{
    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    PWM->CH_CTRL_F.CH3_EN = (enable ? 1 : 0);                    /* 使能通道3 */
    PWM->CNT_CTRL_F.CNT1_EN = (enable ? 1 : 0);                  /* 使能计数器1 */
    return LL_OK;
}

/**
 * @brief   反初始化PWM外设
 * @param   bus - PWM总线号
 * @note    复位PWM模块寄存器，清空回调指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_deinit(ll_pwm_bus_e bus)
{

    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    CRG_CONFIG_UNLOCK();
    CRG->PWM_CLKRST_CTRL_F.RST_PWM = 1;                          /* 复位PWM */
    __NOP();
    __NOP();
    CRG->PWM_CLKRST_CTRL_F.RST_PWM = 0;                          /* 释放复位 */
    __NOP();
    __NOP();
    CRG_CONFIG_LOCK();

    pwm_isr_callback = NULL;

    return LL_OK;
}

/**
 * @brief   初始化PWM外设
 * @param   bus      - PWM总线号
 * @param   config   - PWM配置参数（包含时钟、计数模式、同步模式、周期、极性、输出模式等）
 * @param   callback - 中断回调函数指针
 * @note    配置时钟、GPIO、计数器模式、周期、同步、极性、输出模式等
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_pwm_init(ll_pwm_bus_e bus, pwm_config_t *config, ISR_FUNC_CALLBACK callback)
{
    uint32_t reg_val, period, threshold  = 0;
    (void)(&reg_val);
    (void)(&period);
    (void)(&threshold);

    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    ll_pwm_clk_config(&config->clk_cfg);                          /* 配置时钟 */

    /* 配置GPIO引脚复用 */
    ll_pwm_gpio_config(config->hvio_mode);

    PWM->CNT_CTRL_F.CNT0_MODE = config->count_mode;               /* 设置计数模式（递增/增减） */
    PWM->CNT_CTRL_F.CNT0_ONE_SHOT_EN = 0;                        /* 禁能单次模式 */
    /* 使能条件：cnt0_en=1且sync_en=1 */
    PWM->CNT_CTRL_F.CNT0_EN_MODE = 0;                             /* 计数器使能模式 */
    PWM->CNT_CFG_F.PERIOD0 = config->period;                     /* 设置周期 */

    if (config->sync_mode != PWM_SYNC_MODE_NULL)
    {
        PWM->CTRL_F.CH_SYNC_EN = 1;                               /* 使能通道同步 */
        PWM->CTRL_F.CH_SYNC_SEL = config->sync_mode;              /* 选择同步模式 */
    }
    else
    {
        PWM->CTRL_F.CH_SYNC_EN = 0;                               /* 禁能同步 */
    }

    PWM->CTRL_F.AUTO_LD_EN = 1;                                   /* 自动加载使能 */
    PWM->CTRL_F.BRK_EN = 1;                                       /* 刹车功能使能 */
    PWM->CTRL_F.PWM_SW_EN = false;                                /* 禁能软件控制 */

    PWM->CH_CTRL_F.CH0_CNT_SEL = 0;                               /* 通道0选择计数器0 */
    PWM->CH_CTRL_F.CH1_CNT_SEL = 0;                               /* 通道1选择计数器0 */
    PWM->CH_CTRL_F.CH2_CNT_SEL = 0;                               /* 通道2选择计数器0 */

    PWM->CH_CTRL_F.CH0_PTY = config->polarity;                   /* 设置通道0极性 */
    PWM->CH_CTRL_F.CH1_PTY = config->polarity;                   /* 设置通道1极性 */
    PWM->CH_CTRL_F.CH2_PTY = config->polarity;                   /* 设置通道2极性 */

    PWM->CH_CTRL_F.CH0_MODE = config->out_mode;                   /* 设置通道0输出模式 */
    PWM->CH_CTRL_F.CH1_MODE = config->out_mode;                   /* 设置通道1输出模式 */
    PWM->CH_CTRL_F.CH2_MODE = config->out_mode;                   /* 设置通道2输出模式 */

    ADC->CTRL0_F.VREFBUF_EN = true;                               /* 使能ADC参考缓冲（与TCPL01x兼容） */
    ll_pwm_isr_config(&config->isr_cfg, callback);                /* 配置中断 */

    return LL_OK;
}

/**
 * @brief   初始化LED开关通道（通道3）
 * @param   bus      - PWM总线号
 * @param   period   - 开关周期
 * @param   polarity - 输出极性
 * @note    通道3使用独立的计数器1，支持独立输出模式
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_led_switch_init(ll_pwm_bus_e bus, uint16_t period, pwm_polarity_e polarity)
{

    if (bus >= LL_PWM_BUS_MAX)
    {
        return LL_ERROR;
    }

    PWM->CNT_CTRL_F.CNT1_MODE = PWM_COUNTER_MODE_UP;             /* 计数器1递增模式 */
    PWM->CNT_CTRL_F.CNT1_ONE_SHOT_EN = 0;                        /* 禁能单次模式 */
    PWM->CNT_CTRL_F.CNT1_EN_MODE = 1;                             /* 使能条件：cnt1_en=1且sync_en=1 */
    PWM->CNT_CFG_F.PERIOD1 = period;                              /* 设置计数器1周期 */

    PWM->CH_CTRL_F.CH3_CNT_SEL = 1;                               /* 通道3选择计数器1 */
    PWM->CH_CTRL_F.CH3_PTY = polarity;                            /* 设置极性 */

    PWM->CH_CTRL_F.CH3_MODE = PWM_MODE_INDEPENDENT;               /* 独立输出模式 */

    PWM->CH3_PWM_CFG_F.HT3 = period >> 2;                         /* 设置高阈值（25%占空比） */
    PWM->CH3_PWM_CFG_F.LT3 = 0;                                   /* 设置低阈值 */

    return LL_OK;
}

/**
 * @brief   PWM中断处理函数
 * @note    （注释掉）- 读取中断状态，调用用户回调，清除中断标志
 * @retval  None
 */
//void PWM_IRQHandler(void)
//{
//    uint32_t isr = PWM->ISR;

//    if (isr & PWM_ISR_FLAG)
//    {
//        if (NULL != pwm_isr_callback)
//        {
//            pwm_isr_callback(isr);
//        }

//        PWM->ICR |= isr;
//    }
//}
