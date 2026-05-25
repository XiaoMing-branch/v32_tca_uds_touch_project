/**
 *****************************************************************************
 * @brief   gpio driver source file.
 *
 * @file    tcae10_ll_gpio.c
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

static ISR_FUNC_CALLBACK gpio_isr_callback = NULL;

/**
 * @brief   配置GPIO模块时钟
 * @param   None
 * @note    使能GPIO和PINMUX的PCLK和FCLK时钟门控
 * @retval  None
 */
static void ll_gpio_clk_config(void)
{
    CRG_CONFIG_UNLOCK();

    CRG->GPIO_CLKRST_CTRL_F.FCLK_EN_GPIO = 1;                   /* 使能GPIO FCLK */
    CRG->GPIO_CLKRST_CTRL_F.PCLK_EN_GPIO = 1;                   /* 使能GPIO PCLK */
    CRG->PINMUX_CLKRST_CTRL_F.PCLK_EN_PINMUX = 1;               /* 使能PINMUX PCLK */

    CRG_CONFIG_LOCK();
}

/**
 * @brief   配置GPIO中断触发方式
 * @param   flag     - 触发方式（上升沿/下降沿/高电平/低电平/双边沿）
 * @param   gpio_pin - GPIO引脚号
 * @note    配置INT_POL_SEL（极性选择）和INT_TYP_SEL（类型选择）寄存器
 * @retval  None
 */
static void ll_gpio_isr_config(gpio_trigger_flag_e flag, gpio_pin_e gpio_pin)
{

    switch (flag)
    {
        case GPIO_TRIGGER_FALLING_EDGE:                          /* 下降沿触发 */
            GPIO->INT_POL_SEL_F.INT_POL_SEL &= ~(1 << gpio_pin); /* 低电平/下降沿极性 */
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL |= 1 << gpio_pin;    /* 边沿触发 */
            break;

        case GPIO_TRIGGER_LOW_LEVEL:                             /* 低电平触发 */
            GPIO->INT_POL_SEL_F.INT_POL_SEL &= ~(1 << gpio_pin);
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL &= ~(1 << gpio_pin); /* 电平触发 */
            break;

        case GPIO_TRIGGER_RISING_EDGE:                           /* 上升沿触发 */
            GPIO->INT_POL_SEL_F.INT_POL_SEL |= 1 << gpio_pin;    /* 高电平/上升沿极性 */
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL |= 1 << gpio_pin;    /* 边沿触发 */
            break;

        case GPIO_TRIGGER_HIGH_LEVEL:                            /* 高电平触发 */
            GPIO->INT_POL_SEL_F.INT_POL_SEL |= 1 << gpio_pin;
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL &= ~(1 << gpio_pin); /* 电平触发 */
            break;

        case GPIO_TRIGGER_RISING_FALLING_EDGE:                   /* 双边沿触发 */
            GPIO->INT_EDGE_EN_F.INT_EDGE_EN |= 1 << gpio_pin;    /* 使能双边沿检测 */
            break;

        default:
            break;
    }
}

/**
 * @brief   反初始化GPIO外设
 * @param   None
 * @note    复位GPIO模块寄存器
 * @retval  None
 */
void ll_gpio_deinit(void)
{
    CRG_CONFIG_UNLOCK();

    CRG->GPIO_CLKRST_CTRL_F.RST_GPIO = 1 ;                      /* 复位GPIO */
    __NOP();
    __NOP();
    CRG->GPIO_CLKRST_CTRL_F.RST_GPIO = 0 ;                      /* 释放复位 */
    __NOP();
    __NOP();

    CRG_CONFIG_LOCK();
}

/**
 * @brief   初始化GPIO引脚
 * @param   config   - GPIO配置参数指针（包含引脚号、模式、上下拉、复用功能、中断触发）
 * @param   callback - GPIO中断回调函数指针
 * @note    配置引脚方向、复用功能、上下拉电阻、开漏/开源模式，以及中断触发方式
 * @retval  None
 */
void ll_gpio_init(gpio_config_t *config, ISR_FUNC_CALLBACK callback)
{
    uint32_t reg_val = 0;
    uint32_t reg = (config->gpio_pin < GPIO_PIN_6) ? PINMUX_IO0_CFG_ADDR : PINMUX_LED0_CFG_ADDR;

    ll_gpio_clk_config();

    /* 配置引脚方向 */
    if (config->mode == GPIO_MODE_IN_PP || config->mode == GPIO_MODE_IN_OD)
    {
        GPIO->OUTEN_CLR_F.OUTEN_CLR |= 1 << config->gpio_pin;   /* 清除输出使能，设为输入 */
    }
    else if (config->mode == GPIO_MODE_OUT_PP || config->mode == GPIO_MODE_OUT_OD)
    {
        GPIO->OUTEN_SET_F.OUTEN_SET |= 1 << config->gpio_pin;   /* 设置输出使能，设为输出 */
    }

    reg_val = PINMUX_IO0_CFG_IO0_SRC_SEL_SET(config->afio);     /* 配置复用功能选择 */

    if (config->pull_mode == GPIO_PULL_DOWN)
    {
        reg_val |= PINMUX_IO0_CFG_IO0_PD_SET(1);                 /* 使能下拉 */
    }
    else if (config->pull_mode == GPIO_PULL_UP)
    {
        reg_val |= PINMUX_IO0_CFG_IO0_PU_SET(1);                 /* 使能上拉 */
    }

    reg_val  |= PINMUX_IO0_CFG_IO0_PULL_SEL_SET(config->pull_down_type);  /* 上下拉类型 */

    /* 开漏和开源模式配置 */
    if (config->mode == GPIO_MODE_IN_OD || config->mode == GPIO_MODE_OUT_OD)
    {
        reg_val |= PINMUX_IO0_CFG_IO0_OD_SET(1);                  /* 使能开漏模式 */
    }

    reg += ((config->gpio_pin % GPIO_PIN_6) * 4);                /* 计算引脚配置寄存器地址 */

    *((volatile uint32_t *)(reg)) = reg_val;                     /* 写入引脚配置寄存器 */

    if (config->trigger_flag)
    {
        ll_gpio_isr_config(config->trigger_flag, config->gpio_pin);  /* 配置中断触发方式 */
        GPIO->INT_CLR = 0xFFFFFFFF;                              /* 清除所有中断标志 */
        GPIO->INT_RAW_STATUS = 0xFFFFFFFF;                       /* 清除原始中断状态 */
        gpio_isr_callback = callback;
    }
}

/**
 * @brief   读取GPIO引脚电平
 * @param   gpio_pin - 要读取的引脚号
 * @retval  true - 高电平，false - 低电平
 */
bool ll_gpio_read(gpio_pin_e gpio_pin)
{
    return (GPIO->DATAIN_F.DATAIN & (1 << gpio_pin));            /* 读取输入数据寄存器 */
}

/**
 * @brief   设置GPIO引脚输出电平
 * @param   gpio_pin - 目标引脚号
 * @param   state    - true输出高电平，false输出低电平
 * @retval  None
 */
void ll_gpio_output(gpio_pin_e gpio_pin, bool state)
{
    if (state)
    {
        GPIO->DATAOUT_F.DATAOUT |= 1 << gpio_pin;                /* 置位输出数据寄存器 */
    }
    else
    {
        GPIO->DATAOUT_F.DATAOUT &= ~(1 << gpio_pin);             /* 清除输出数据寄存器 */
    }
}

/**
 * @brief   翻转GPIO引脚输出电平
 * @param   gpio_pin - 目标引脚号
 * @retval  None
 */
void ll_gpio_toggle(gpio_pin_e gpio_pin)
{
    GPIO->DATAOUT_F.DATAOUT ^= 1 << gpio_pin;                   /* 异或翻转输出电平 */
}

/**
 * @brief   使能或禁能GPIO引脚中断（掩码控制）
 * @param   gpio_pin - 目标引脚号
 * @param   enable   - true使能中断，false禁能中断
 * @retval  None
 */
void ll_gpio_isr_enable(gpio_pin_e gpio_pin, bool enable)
{
    if (!enable)
    {
        GPIO->INT_MSK_F.INT_MSK |= 1 << gpio_pin;                /* 设置掩码位，禁能中断 */
    }
    else
    {
        GPIO->INT_MSK_F.INT_MSK &= ~(1 << gpio_pin);             /* 清除掩码位，使能中断 */
    }
}

/**
 * @brief   获取GPIO中断标志
 * @param   gpio_pin - 目标引脚号
 * @retval  true - 该引脚有中断发生，false - 无中断
 */
bool ll_gpio_interrupt_flag_get(gpio_pin_e gpio_pin)
{
    return (GPIO->INT_STATUS_F.INT_STATUS & (1 << gpio_pin));    /* 读取中断状态寄存器 */
}

/**
 * @brief   清除GPIO中断标志
 * @param   gpio_pin - 目标引脚号
 * @retval  None
 */
void ll_gpio_interrupt_clear(gpio_pin_e gpio_pin)
{
    GPIO->INT_CLR_F.INT_CLR |= 1 << gpio_pin;                    /* 写INT_CLR清除中断 */
}

/**
 * @brief   配置GPIO引脚的复用功能
 * @param   gpio_pin - 目标引脚号
 * @param   afio_mux - 复用功能选择（AFIO_MUX_0 ~ AFIO_MUX_7）
 * @note    通过写PINMUX配置寄存器实现引脚功能重映射
 * @retval  None
 */
void ll_gpio_afio_config(gpio_pin_e gpio_pin, gpio_afio_mux_e afio_mux)
{
    uint32_t reg_val = 0;
    uint32_t reg = (gpio_pin < GPIO_PIN_6) ? PINMUX_IO0_CFG_ADDR : PINMUX_LED0_CFG_ADDR;

    reg += (gpio_pin % GPIO_PIN_6) * 4 ;                         /* 计算引脚配置寄存器地址 */

    reg_val = *((volatile uint32_t *)(reg));                     /* 读取当前配置值 */

    reg_val &= ~(7 << PINMUX_IO0_CFG_IO0_SRC_SEL_SHIFT);         /* 清除原功能选择位 */
    reg_val |= afio_mux;                                         /* 设置新的复用功能 */
    *((volatile uint32_t *)(reg)) = reg_val;                     /* 写入配置寄存器 */
}

/**
 * @brief   获取GPIO引脚的当前复用功能配置
 * @param   gpio_pin - 目标引脚号
 * @retval  当前复用的AFIO值（gpio_afio_mux_e类型）
 */
gpio_afio_mux_e ll_gpio_afio_get(gpio_pin_e gpio_pin)
{
    uint32_t reg_val = 0;
    uint32_t reg = (gpio_pin < GPIO_PIN_6) ? PINMUX_IO0_CFG_ADDR : PINMUX_LED0_CFG_ADDR;

    reg += (gpio_pin % GPIO_PIN_6) * 4 ;                         /* 计算引脚配置寄存器地址 */

    reg_val = *((volatile uint32_t *)(reg));                     /* 读取配置值 */

    reg_val &= 0x7UL;                                            /* 取低3位功能选择 */

    return (gpio_afio_mux_e)reg_val;
}

/**
 * @brief   配置GPIO在低功耗模式下保持指定输出值（仅GPIO1~4支持）
 * @param   gpio_pin  - 目标引脚（仅GPIO1~4支持AON IO功能）
 * @param   keep_high - true保持高电平，false保持低电平
 * @note    只有一个GPIO引脚可以在低功耗模式下保持固定输出值。
 *          AON_IO_OUT_SEL选择引脚，AON_IO_OUT_VAL设置输出值
 * @retval  None
 */
void ll_gpio_ano_lpm_config(gpio_pin_e gpio_pin, bool keep_high)
{
    /* 仅GPIO1,2,3,4支持AON IO */
    if (gpio_pin < GPIO_PIN_1 || gpio_pin > GPIO_PIN_4)
    {
        return;
    }

    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->AON_IO_CTRL_F.AON_IO_OUT_SEL = gpio_pin - 1;       /* 选择AON输出引脚 */
    ASYSCFG->AON_IO_CTRL_F.AON_IO_OUT_VAL = keep_high;          /* 设置输出值 */
    ASYSCFG->AON_IO_CTRL_F.AON_IO_OUT_EN = 1;                   /* 使能AON输出 */

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   使能或禁能GPIO4的Pad复位功能
 * @param   enable - true使能复位功能（写入0x12345678），false禁能（写入0xAA55AA55）
 * @note    通过配置ASYSCFG的IO4_RST_CTRL寄存器，使能后GPIO4可触发系统复位
 * @retval  None
 */
void ll_gpio_reset_enable(bool enable)
{
    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->IO4_RST_CTRL = enable ? 0x12345678 : 0xAA55AA55;   /* 写解锁值使能/禁能复位 */

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   GPIO中断处理函数
 * @note    （注释掉）- 读取中断状态，调用用户回调，清除中断标志
 * @retval  None
 */
//void GPIO_IRQHandler(void)
//{
//    uint32_t isr = GPIO->INT_STATUS_F.INT_STATUS;

//    if (gpio_isr_callback)
//    {
//        gpio_isr_callback(isr);
//    }

//    GPIO->INT_CLR_F.INT_CLR |= isr; /* clear the enable flag */
//}
