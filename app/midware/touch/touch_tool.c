/**
*****************************************************************************
* @brief  touch tool source
* @file   touch_tool.c
* @author AE/FAE team
* @date   28/JUL/2023
*****************************************************************************
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <b>&copy; Copyright (c) 2023 Tinychip Microelectronics Co.,Ltd.</b>
*
*****************************************************************************
*/

#include "string.h"
#include "touch_tool.h"
#include "tcae10_ll_def.h"
#include "tc_log.h"
#include "tc.h"
#include "si_touch_port.h"
#include "touch_haldispatch.h"

//const static char *TAG = "TOUCH_TOOL";

/** @brief 触摸通道总数 */
#define TOUCH_CHANNEL_NUM       5

/**
 ******************************************************************************
 * @brief  触摸IO引脚配置
 *
 * @param[in]  channel  触摸通道号
 *                       - CAPTOUCH_CHANNEL_0 ~ CAPTOUCH_CHANNEL_4
 * @retval void          无返回值
 *
 * @note 配置指定通道的IO引脚为触摸功能，同时配置GPIO_PIN_3为参考电容引脚
 ******************************************************************************/
void Touch_IOConfig(uint8_t channel)
{
    static const struct
    {
        gpio_pin_e pin;         /**< 触摸引脚号 */
        uint8_t mux;            /**< 触摸功能复用选择 */
    } touchPins[TOUCH_CHANNEL_NUM] =
    {
        {GPIO_PIN_1, GPIO1_SOFTWARE_INPUT_FUNCTION_CAP0}, {GPIO_PIN_0, GPIO0_SOFTWARE_INPUT_FUNCTION_CAP1}, {GPIO_PIN_2, GPIO2_SOFTWARE_INPUT_FUNCTION_CAP2},
        {GPIO_PIN_4, GPIO4_SOFTWARE_INPUT_FUNCTION_CAP3}, {GPIO_PIN_5, GPIO5_SOFTWARE_INPUT_FUNCTION_CAP4}
    };

    if (channel >= TOUCH_CHANNEL_NUM)
    {
        return;
    }

    /* 配置GPIO_PIN_3为触摸参考电容引脚 */
    ll_gpio_afio_config(GPIO_PIN_3, (gpio_afio_mux_e)GPIO3_SOFTWARE_INPUT_FUNCTION_CAP_REF);

    ll_gpio_afio_config(touchPins[channel].pin, (gpio_afio_mux_e)touchPins[channel].mux);
    GPIO->OUTEN_SET_F.OUTEN_SET |= 1 << touchPins[channel].pin;     /* 使能触摸引脚输出 */
    GPIO->DATAOUT_F.DATAOUT &= ~(1 << touchPins[channel].pin);      /* 触摸引脚输出低电平 */
}

/**
 ******************************************************************************
 * @brief  触摸通道使能控制
 *
 * @param[in]  channel  触摸通道号 (CAPTOUCH_CHANNEL_0 ~ CAPTOUCH_CHANNEL_4)
 * @param[in]  enable   使能标志
 *                       - 1: 切换为触摸功能IO
 *                       - 0: 切回GPIO功能
 * @retval void          无返回值
 *
 * @note 使能时配置IO引脚为触摸功能，关闭时恢复为GPIO功能
 ******************************************************************************/
void Touch_IOEnable(uint8_t channel, uint8_t enable)
{
    static const struct
    {
        gpio_pin_e pin;         /**< 触摸引脚号 */
        uint8_t mux;            /**< 触摸功能复用选择 */
        uint8_t mux_gpio;       /**< GPIO功能复用选择 */
    } touchPins[TOUCH_CHANNEL_NUM] =
    {
        {GPIO_PIN_1, GPIO1_SOFTWARE_INPUT_FUNCTION_CAP0, GPIO1_SOFTWARE_INPUT_FUNCTION_GPIO}, {GPIO_PIN_0, GPIO0_SOFTWARE_INPUT_FUNCTION_CAP1, GPIO0_SOFTWARE_INPUT_FUNCTION_GPIO},
        {GPIO_PIN_2, GPIO2_SOFTWARE_INPUT_FUNCTION_CAP2, GPIO2_SOFTWARE_INPUT_FUNCTION_GPIO}, {GPIO_PIN_4, GPIO4_SOFTWARE_INPUT_FUNCTION_CAP3, GPIO4_SOFTWARE_INPUT_FUNCTION_GPIO},
        {GPIO_PIN_5, GPIO5_SOFTWARE_INPUT_FUNCTION_CAP4, GPIO5_SOFTWARE_INPUT_FUNCTION_GPIO}
    };

    if (channel >= TOUCH_CHANNEL_NUM)
    {
        return;
    }

    if (enable)     /* 使能：切换为触摸功能IO */
    {
        ll_gpio_afio_config(touchPins[channel].pin, (gpio_afio_mux_e)touchPins[channel].mux);
    }
    else            /* 关闭：恢复为GPIO功能IO */
    {
        ll_gpio_afio_config(touchPins[channel].pin, (gpio_afio_mux_e)touchPins[channel].mux_gpio);
    }
}

/**
 ******************************************************************************
 * @brief  复位触摸模块
 *
 * @retval void  无返回值
 *
 * @note 通过CRG控制触摸模块硬件复位，流程：解锁→置位复位位→延时→清除复位位→加锁
 ******************************************************************************/
void Touch_Reset(void)
{
    CRG_CONFIG_UNLOCK();                                    /* 解锁CRG配置寄存器 */
    CRG->CAPTOUCH_CLKRST_CTRL_F.RST_CAPTOUCH = 1;           /* 置位触摸模块复位位 */
    __NOP();                                                /* 等待复位生效 */
    __NOP();
    CRG->CAPTOUCH_CLKRST_CTRL_F.RST_CAPTOUCH = 0;           /* 清除触摸模块复位位 */
    __NOP();                                                /* 等待复位释放 */
    __NOP();
    CRG_CONFIG_LOCK();                                      /* 锁定CRG配置寄存器 */
}

/**
 ******************************************************************************
 * @brief  RTC触发触摸配置
 *
 * @param[in]  freq  触发频率(Hz)，用于计算TIM_LITE定时器装载值
 * @param[in]  sw    开关控制
 *                   - 1: 开启TIM_LITE定时器触发
 *                   - 0: 关闭TIM_LITE定时器触发
 * @retval void      无返回值
 *
 * @note 使用TIM_LITE定时器以32K时钟驱动触摸采样，实现RTC触发触摸功能
 ******************************************************************************/
void TouchRtcTrigConfig(uint8_t freq, uint8_t sw)
{
    ll_timer_deinit();

    CRG_CONFIG_UNLOCK();                                    /* 解锁CRG配置寄存器 */
    CRG->TIM_LITE_CLKRST_CTRL_F.PCLK_EN_TIM_LITE = 1;       /* 使能TIM_LITE外设时钟 */
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_SEL_TIM_LITE = FCLK_SRC_32K; /* 选择32KHz低频时钟源 */
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_DIV_TIM_LITE = 0;      /* 功能时钟不分频 */
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_EN_TIM_LITE = 0x1;     /* 使能功能时钟输出 */
    CRG_CONFIG_LOCK();                                      /* 锁定CRG配置寄存器 */

    /* 配置TIM_LITE定时器控制寄存器 */
    TIM_LITE->CTRL_F.TRIG_EN = 0;                           /* 禁用外部触发模式 */
    TIM_LITE->CTRL_F.PAUSE = 0;                             /* 禁用暂停功能 */
    TIM_LITE->CTRL_F.STP = 0;                               /* 禁用停止功能 */
    TIM_LITE->CTRL_F.LOOP_DIS = 0;                          /* 允许循环计数（非单次模式） */

    TIM_LITE->INIT_VAL = (32768 / freq);                    /* 设置定时器装载值：32KHz时钟 / 目标频率 */

    if (sw)     /* 开启RTC触发触摸功能 */
    {
        TIM_LITE->ICR = 0xFFFFFFFF;                         /* 清除所有中断标志 */
        TIM_LITE->IMR_F.CNT_UDF_INT_MSK = 0;                /* 使能定时器溢出中断 */  //TIMERLITE_INTERRUPT_ENABLE();
        EnableNvic(TIMER_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, ENABLE);   /* 使能NVIC定时器中断 */
        TIM_LITE->CTRL_F.EN = 1;                            /* 启动TIM_LITE定时器 */  //TIMERLITE_ENABLE();
    }
    else        /* 关闭RTC触发触摸功能 */
    {
        TIM_LITE->IMR_F.CNT_UDF_INT_MSK = 1;                /* 禁用定时器溢出中断 */  //TIMERLITE_INTERRUPT_DISABLE();
        EnableNvic(TIMER_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, DISABLE);  /* 禁用NVIC定时器中断 */
        TIM_LITE->CTRL_F.EN = 0;                            /* 停止TIM_LITE定时器 */  //TIMERLITE_DISABLE();
    }
}

/**
 ******************************************************************************
 * @brief  获取当前系统时间（毫秒）
 *
 * @retval uint32_t  当前时间(ms)
 *
 * @note 根据 TC_SYSTICK_HZ 配置进行时间基准换算
 ******************************************************************************/
uint32_t TouchGetTime(void)
{
    uint32_t curTick = TcTimeGet();                             /* 获取当前系统tick值 */
#if (TC_SYSTICK_HZ==1000)                                       /* 系统节拍为1000Hz时tick即毫秒 */
    return curTick;                                             /* tick值即为毫秒数 */
#else                                                           /* 其他节拍频率需换算 */
    return (uint64_t)curTick * 1000 / TC_SYSTICK_HZ;            /* tick * 1000 / 节拍频率 = 毫秒 */
#endif
}

/**
 ******************************************************************************
 * @brief  低功耗参数调节选择器回调接口
 *
 * @retval uint32_t  选择结果
 *                   - 0: 非低功耗模式
 *                   - 1: 低功耗模式
 *
 * @note 用于判断当前是否处于低功耗模式，以选择低功耗参数调节策略
 ******************************************************************************/
uint32_t lpParaAdjusterSelectCallback(void)
{
    if (Touch_HalDispatch_GetInSleep(touchDispatch))            /* 检测是否处于低功耗模式 */  //处在低功耗模式
    {
        return 1;                                               /* 低功耗模式，选择低功耗参数 */
    }
    return 0;                                                   /* 非低功耗模式 */
}
