/**
 *****************************************************************************
 * @brief   timer driver source file.
 *
 * @file    tcae10_ll_timer.c
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

#include "tcae10_ll_timer.h"
#include "tcae10_ll_cortex.h"

static ISR_FUNC_CALLBACK timer_callback = NULL;

/**
 * @brief   配置TIM_LITE定时器时钟
 * @param   config - 时钟配置参数指针（包含时钟源选择和分频系数）
 * @note    使能PCLK和FCLK，设置时钟源和分频系数
 * @retval  None
 */
static void ll_time_clk_config(ll_clk_config_t *config)
{
    CRG_CONFIG_UNLOCK();

    CRG->TIM_LITE_CLKRST_CTRL_F.PCLK_EN_TIM_LITE = 1;                       /* 使能定时器PCLK */
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_EN_TIM_LITE = 1;                       /* 使能定时器FCLK */
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_SEL_TIM_LITE = config->clk_source;     /* 选择时钟源 */
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_DIV_TIM_LITE = config->fclk_div;       /* 设置分频系数 */

    CRG_CONFIG_LOCK();
}

/**
 * @brief   初始化TIM_LITE定时器
 * @param   config   - 定时器配置参数指针（包含时钟、触发模式、初始值、中断配置）
 * @param   callback - 定时器中断回调函数指针
 * @note    先调用ll_timer_deinit复位，然后配置时钟、工作模式、初始计数值和中断掩码
 * @retval  None
 */
void ll_timer_init(timer_config_t *config, ISR_FUNC_CALLBACK callback)
{
    //to do assert_param
    ll_timer_deinit();

    ll_time_clk_config(&config->clk_cfg);

    TIM_LITE->CTRL_F.TRIG_EN = config->trigger_mode;            /* 配置触发模式 */
    TIM_LITE->CTRL_F.LOOP_DIS = config->repeat_disable;         /* 配置单次/循环模式 */
    TIM_LITE->INIT_VAL_F.CNT_INIT_VAL = config->initial_value;  /* 设置初始计数值 */

    TIM_LITE->IMR_F.CNT_UDF_INT_MSK = (config->isr_cfg.isr_enable) ? 0 : 1; /* 配置下溢中断掩码 */

    // if (config->isr_cfg.isr_enable)
    // {
    //     if (NULL != callback)
    //     {
    //         timer_callback = callback;
    //     }

    //     NVIC_ClearPendingIRQ(TIMER_IRQn);
    //     NVIC_SetPriority(TIMER_IRQn, config->isr_cfg.priority);
    // }
}

/**
 * @brief   反初始化TIM_LITE定时器
 * @param   None
 * @note    禁能NVIC中断，复位定时器外设寄存器
 * @retval  None
 */
void ll_timer_deinit(void)
{

    NVIC_DisableIRQ(TIMER_IRQn);
    CRG_CONFIG_UNLOCK();
    CRG->TIM_LITE_CLKRST_CTRL_F.RST_TIM_LITE = 1;               /* 复位定时器 */
    __NOP();
    __NOP();
    CRG->TIM_LITE_CLKRST_CTRL_F.RST_TIM_LITE = 0;               /* 释放复位 */
    __NOP();
    __NOP();
    CRG_CONFIG_LOCK();
}

/**
 * @brief   使能或禁能定时器计数
 * @param   enable - true启动定时器，false停止定时器
 * @retval  None
 */
void ll_timer_enable(bool enable)
{
    TIM_LITE->CTRL_F.EN = (enable) ? 1 : 0;                    /* 设置使能位 */
}

/**
 * @brief   使能或禁能定时器触发
 * @param   enable - true使能触发启动计数，false禁能
 * @retval  None
 */
void ll_timer_trig_enable(bool enable)
{
    TIM_LITE->CTRL_F.CNT_TRIG = (enable) ? 1 : 0;              /* 设置触发使能 */
}

/**
 * @brief   使能或禁能定时器中断（NVIC层）
 * @param   enable - true使能定时器中断，false禁能
 * @retval  LL_OK - 操作成功
 */
ll_status_e ll_timer_isr_enable(bool enable)
{
    NVIC_ClearPendingIRQ(TIMER_IRQn);                           /* 清除挂起中断 */

    if (enable)
    {
        NVIC_EnableIRQ(TIMER_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(TIMER_IRQn);
    }

    return LL_OK;
}

/**
 * @brief   获取定时器中断标志状态
 * @param   None
 * @retval  true - 下溢中断已发生，false - 无中断
 */
bool ll_timer_isr_get(void)
{
    return (TIM_LITE->ISR_F.CNT_UDF_INT_STATUS);                /* 读取下溢中断状态 */
}

/**
 * @brief   设置定时器初始计数值
 * @param   value - 初始计数值（0x0000~0xFFFF）
 * @note    设置后需写INIT_VALUE_LD位加载新值
 * @retval  None
 */
void ll_timer_counter_set(uint16_t value)
{
    TIM_LITE->INIT_VAL_F.CNT_INIT_VAL = value;                  /* 设置初始值寄存器 */
    TIM_LITE->CTRL_F.INIT_VALUE_LD = 1;                        /* 触发加载初始值 */
}

/**
 * @brief   获取定时器当前计数值
 * @param   None
 * @retval  当前计数值（0x0000~0xFFFF）
 */
uint16_t ll_timer_counter_get(void)
{
    return TIM_LITE->CNT_VAL_F.CNT_VAL;                         /* 读取当前计数值 */
}

/**
 * @brief   定时器中断处理函数
 * @note    （注释掉）- 清除下溢中断并调用用户回调
 * @retval  None
 */
//void TIMER_IRQHandler(void)
//{
//   TIM_LITE->ICR_F.CNT_UDF_INT_CLR = TRUE; //clear underflow interrupt

//   if (NULL != timer_callback)
//   {
//       timer_callback(0);
//   }
//}
