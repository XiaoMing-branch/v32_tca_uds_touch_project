/**
 *****************************************************************************
 * @brief   gpio driver source file.
 *
 * @file    tcpl03x_ll_gpio.c
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

#include "tcpl03x_ll_gpio.h"

/**
 * @brief   GPIO 中断回调函数指针
 *
 * @note    由 ll_gpio_init() 注册，在 GPIO_IRQHandler() 中触发调用
 */
static ISR_FUNC_CALLBACK gpio_isr_callback = NULL;
/********************************************************
** @brief   GPIO 时钟配置
**
**          使能 GPIO 模块的功能时钟 (FCLK) 和外设时钟 (PCLK)，
**          同时使能引脚复用模块的时钟，确保 GPIO 功能正常使用。
**
** @param   None
**
** @retval  None
**
** @note    必须在操作 GPIO 寄存器前调用；执行时会暂时解锁 CRG 配置
*********************************************************/
static void ll_gpio_clk_config(void)
{
    CRG_CONFIG_UNLOCK();

    CRG->GPIO_CLKRST_CTRL_F.FCLK_EN_GPIO = 1;            /* 使能 GPIO 功能时钟 */
    CRG->GPIO_CLKRST_CTRL_F.PCLK_EN_GPIO = 1;            /* 使能 GPIO 外设时钟 */
    CRG->PINMUX_CLKRST_CTRL_F.PCLK_EN_PINMUX = 1;        /* 使能引脚复用模块时钟 */

    CRG_CONFIG_LOCK();
}

/********************************************************
** @brief   GPIO 中断触发方式配置
**
**          根据指定的触发标志，配置相应 GPIO 引脚的中断极性寄存器和
**          中断类型寄存器，支持边沿触发和电平触发两种模式。
**
** @param   flag        触发方式（上升沿/下降沿/高电平/低电平/双沿）
** @param   gpio_pin    目标 GPIO 引脚编号
**
** @retval  None
**
** @note    双沿触发 (RISING_FALLING_EDGE) 使用独立的 INT_EDGE_EN 寄存器
*********************************************************/
static void ll_gpio_isr_config(gpio_trigger_flag_e flag, gpio_pin_e gpio_pin)
{
    switch (flag)
    {
        case GPIO_TRIGGER_FALLING_EDGE:
            GPIO->INT_POL_SEL_F.INT_POL_SEL &= ~(1 << gpio_pin);
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL |= 1 << gpio_pin;
            break;

        case GPIO_TRIGGER_LOW_LEVEL:
            GPIO->INT_POL_SEL_F.INT_POL_SEL &= ~(1 << gpio_pin);
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL &= ~(1 << gpio_pin);
            break;

        case GPIO_TRIGGER_RISING_EDGE:
            GPIO->INT_POL_SEL_F.INT_POL_SEL |= 1 << gpio_pin;
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL |= 1 << gpio_pin;
            break;

        case GPIO_TRIGGER_HIGH_LEVEL:
            GPIO->INT_POL_SEL_F.INT_POL_SEL |= 1 << gpio_pin;
            GPIO->INT_TYP_SEL_F.INT_TYP_SEL &= ~(1 << gpio_pin);
            break;

        case GPIO_TRIGGER_RISING_FALLING_EDGE:
            GPIO->INT_EDGE_EN_F.INT_EDGE_EN |= 1 << gpio_pin;
            break;

        default:
            break;
    }
}

/********************************************************
** @brief   GPIO 模块去初始化
**
**          复位 GPIO 模块的所有寄存器到默认状态，
**          通过操作 GPIO_CLKRST_CTRL 寄存器的 RST_GPIO 位实现。
**
** @param   None
**
** @retval  None
**
** @note    复位后需等待至少 2 个 NOP 周期确保复位完成；
**          执行时会暂时解锁 CRG 配置寄存器
*********************************************************/
void ll_gpio_deinit(void)
{
    CRG_CONFIG_UNLOCK();

    CRG->GPIO_CLKRST_CTRL_F.RST_GPIO = 1 ;               /* 复位 GPIO 模块 */
    __NOP();
    __NOP();
    CRG->GPIO_CLKRST_CTRL_F.RST_GPIO = 0 ;               /* 释放 GPIO 模块复位 */
    __NOP();
    __NOP();

    CRG_CONFIG_LOCK();
}

/********************************************************
** @brief   GPIO 引脚初始化
**
**          配置 GPIO 引脚的工作模式（输入/输出/复用）、
**          上下拉模式、复用功能选择及中断触发方式。
**          是 GPIO 驱动的主入口函数。
**
** @param   config      GPIO 配置结构体指针，包含引脚号、模式、上拉/下拉等参数
** @param   callback    中断回调函数指针，当 trigger_flag 非零时注册
**
** @retval  None
**
** @note    当 trigger_flag 有效时，会自动注册回调函数并使能中断；
**          引脚 0-5 使用 IO0_CFG 寄存器，引脚 6-11 使用 LED0_CFG 寄存器
*********************************************************/
void ll_gpio_init(gpio_config_t *config, ISR_FUNC_CALLBACK callback)
{
    uint32_t reg_val = 0;
    uint32_t reg = (config->gpio_pin < GPIO_PIN_6) ? PINMUX_IO0_CFG_ADDR : PINMUX_LED0_CFG_ADDR;

    ll_gpio_clk_config();

    if (config->mode == GPIO_MODE_IN_PP || config->mode == GPIO_MODE_IN_OD)
    {
        GPIO->OUTEN_CLR_F.OUTEN_CLR |= 1 << config->gpio_pin;   /* 输入模式：清除输出使能 */
    }
    else if (config->mode == GPIO_MODE_OUT_PP || config->mode == GPIO_MODE_OUT_OD)
    {
        GPIO->OUTEN_SET_F.OUTEN_SET |= 1 << config->gpio_pin;   /* 输出模式：置位输出使能 */
    }

    reg_val = PINMUX_IO0_CFG_IO0_SRC_SEL_SET(config->afio);

    if (config->pull_mode == GPIO_PULL_DOWN)
    {
        reg_val |= PINMUX_IO0_CFG_IO0_PD_SET(1);
    }
    else if (config->pull_mode == GPIO_PULL_UP)
    {
        reg_val |= PINMUX_IO0_CFG_IO0_PU_SET(1);
    }

    reg_val  |= PINMUX_IO0_CFG_IO0_PULL_SEL_SET(config->pull_down_type);

    /*open drain bit10 and open source bit11 will be set in a single step*/
    if (config->mode == GPIO_MODE_IN_OD || config->mode == GPIO_MODE_OUT_OD)
    {
        reg_val |= PINMUX_IO0_CFG_IO0_OD_SET(1);
    }

    reg += ((config->gpio_pin % GPIO_PIN_6) * 4);

    *((volatile uint32_t *)(reg)) = reg_val;

    if (config->trigger_flag)
    {
        ll_gpio_isr_config(config->trigger_flag, config->gpio_pin);
        GPIO->INT_CLR = 0xFFFFFFFF;                              /* 清除所有 GPIO 中断标志 */
        GPIO->INT_RAW_STATUS = 0xFFFFFFFF;                       /* 清除所有 GPIO 原始中断状态 */
        gpio_isr_callback = callback;                            /* 注册中断回调函数 */
        NVIC_SetPriority(GPIO_IRQn, 2);                          /* 设置 GPIO 中断优先级为 2 */
    }
}

/********************************************************
** @brief   GPIO 引脚电平读取
**
**          读取指定 GPIO 引脚的当前输入电平状态。
**
** @param   gpio_pin    目标 GPIO 引脚编号
**
** @retval  true        引脚为高电平
** @retval  false       引脚为低电平
*********************************************************/
bool ll_gpio_read(gpio_pin_e gpio_pin)
{
    return (GPIO->DATAIN_F.DATAIN & (1 << gpio_pin));
}

/********************************************************
** @brief   GPIO 引脚输出电平设置
**
**          设置指定 GPIO 引脚的输出电平为高或低。
**
** @param   gpio_pin    目标 GPIO 引脚编号
** @param   state       输出电平状态：true - 高电平，false - 低电平
**
** @retval  None
*********************************************************/
void ll_gpio_output(gpio_pin_e gpio_pin, bool state)
{
    if (state)
    {
        GPIO->DATAOUT_F.DATAOUT |= 1 << gpio_pin;            /* 输出高电平 */
    }
    else
    {
        GPIO->DATAOUT_F.DATAOUT &= ~(1 << gpio_pin);         /* 输出低电平 */
    }
}

/********************************************************
** @brief   GPIO 引脚输出电平翻转
**
**          翻转指定 GPIO 引脚的当前输出电平状态。
**
** @param   gpio_pin    目标 GPIO 引脚编号
**
** @retval  None
*********************************************************/
void ll_gpio_toggle(gpio_pin_e gpio_pin)
{
    GPIO->DATAOUT_F.DATAOUT ^= 1 << gpio_pin;                /* 翻转输出电平 */
}

/********************************************************
** @brief   GPIO 中断使能/禁能
**
**          控制指定 GPIO 引脚的中断屏蔽位，同时管理 NVIC 中断的使能与禁能。
**
** @param   gpio_pin    目标 GPIO 引脚编号
** @param   enable      使能控制：true - 使能中断，false - 禁能中断
**
** @retval  None
**
** @note    禁能前会清除 NVIC 的挂起中断标志，防止误触发
*********************************************************/
void ll_gpio_isr_enable(gpio_pin_e gpio_pin, bool enable)
{
    NVIC_ClearPendingIRQ(GPIO_IRQn);                             /* 清除 NVIC 挂起中断，防止误触发 */

    if (!enable)
    {
        GPIO->INT_MSK_F.INT_MSK |= 1 << gpio_pin;                /* 屏蔽该引脚的中断 */
        NVIC_DisableIRQ(GPIO_IRQn);
    }
    else
    {
        GPIO->INT_MSK_F.INT_MSK &= ~(1 << gpio_pin);             /* 取消屏蔽该引脚的中断 */
        NVIC_EnableIRQ(GPIO_IRQn);                               /* 使能 NVIC GPIO 中断 */
    }
}

/********************************************************
** @brief   GPIO 中断状态查询
**
**          查询指定 GPIO 引脚是否有中断发生。
**
** @param   gpio_pin    目标 GPIO 引脚编号
**
** @retval  true        有中断发生
** @retval  false       无中断发生
**
** @note    读取的是屏蔽后的中断状态 (INT_STATUS)，而非原始状态 (INT_RAW_STATUS)
*********************************************************/
bool ll_gpio_interrupt_flag_get(gpio_pin_e gpio_pin)
{
    return (GPIO->INT_STATUS_F.INT_STATUS & (1 << gpio_pin));
}

/********************************************************
** @brief   GPIO 中断标志清除
**
**          清除指定 GPIO 引脚的中断标志位。
**
** @param   gpio_pin    目标 GPIO 引脚编号
**
** @retval  None
**
** @note    向 INT_CLR 寄存器对应位写 1 清除中断标志
*********************************************************/
void ll_gpio_interrupt_clear(gpio_pin_e gpio_pin)
{
    GPIO->INT_CLR_F.INT_CLR |= 1 << gpio_pin;                /* 写 1 清除中断标志 */
}

/********************************************************
** @brief   GPIO 复用功能配置
**
**          配置指定 GPIO 引脚的复用功能选择（AFIO），
**          通过修改引脚复用寄存器中的 SRC_SEL 字段实现。
**
** @param   gpio_pin    目标 GPIO 引脚编号
** @param   afio_mux    复用功能选择（AF0 ~ AF7）
**
** @retval  None
**
** @note    引脚 0-5 使用 IO0_CFG 寄存器，引脚 6-11 使用 LED0_CFG 寄存器；
**          该函数采用读-改-写方式，不影响其他配置位
*********************************************************/
void ll_gpio_afio_config(gpio_pin_e gpio_pin, gpio_afio_mux_e afio_mux)
{
    uint32_t reg_val = 0;
    uint32_t reg = (gpio_pin < GPIO_PIN_6) ? PINMUX_IO0_CFG_ADDR : PINMUX_LED0_CFG_ADDR;

    reg += (gpio_pin % GPIO_PIN_6) * 4 ;

    reg_val = *((volatile uint32_t *)(reg));                     /* 读取当前寄存器值 */

    reg_val &= ~(7 << PINMUX_IO0_CFG_IO0_SRC_SEL_SHIFT);        /* 清除 SRC_SEL 字段（位 0-2） */
    reg_val |= afio_mux;                                         /* 设置新的复用功能选择 */
    *((volatile uint32_t *)(reg)) = reg_val;                     /* 写回寄存器 */
}

/********************************************************
** @brief   GPIO 低功耗模式输出保持配置
**
**          配置指定 GPIO 引脚在进入低功耗模式时，
**          输出保持指定的固定电平值。
**
** @param   gpio_pin    目标 GPIO 引脚编号（仅支持 GPIO1 ~ GPIO4）
** @param   keep_high   保持电平：true - 高电平，false - 低电平
**
** @retval  None
**
** @note    仅 GPIO1、GPIO2、GPIO3、GPIO4 支持 AON IO 功能；
**          执行时会暂时解锁 ASYSCFG 配置寄存器
*********************************************************/
void ll_gpio_ano_lpm_config(gpio_pin_e gpio_pin, bool keep_high)
{
    /*only GPIO1,2,3,4 are supported with AON IO*/
    if (gpio_pin < GPIO_PIN_1 || gpio_pin > GPIO_PIN_4)
    {
        return;
    }

    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->AON_IO_CTRL_F.AON_IO_OUT_SEL = gpio_pin - 1;       /* 选择低功耗保持的 AON IO 引脚 */
    ASYSCFG->AON_IO_CTRL_F.AON_IO_OUT_VAL = keep_high;          /* 设置保持的电平值 */
    ASYSCFG->AON_IO_CTRL_F.AON_IO_OUT_EN = 1;                   /* 使能 AON IO 输出保持功能 */

    ASYSCFG_CONFIG_LOCK();
}

/********************************************************
** @brief   GPIO4 复位功能使能/禁能
**
**          控制 GPIO4 引脚的 Pad Reset 功能。
**          使能时写入 0x12345678，禁能时写入 0xAA55AA55。
**
** @param   enable      使能控制：true - 使能 Pad Reset，false - 禁能 Pad Reset
**
** @retval  None
**
** @note    写入特定密钥值是为了防止误操作；
**          执行时会暂时解锁 ASYSCFG 配置寄存器
*********************************************************/
void ll_gpio_reset_enable(bool enable)
{
    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->IO4_RST_CTRL = enable ? 0x12345678 : 0xAA55AA55;   /* 写入密钥值使能/禁能 Pad Reset */

    ASYSCFG_CONFIG_LOCK();
}

/********************************************************
** @brief   GPIO 中断服务函数
**
**          GPIO 模块的中断入口函数。读取中断状态寄存器，
**          调用注册的回调函数处理中断事件，最后清除中断标志。
**
** @param   None
**
** @retval  None
**
** @note    中断标志的清除必须在回调函数执行之后进行；
**          回调函数通过 ll_gpio_init() 注册
*********************************************************/
void GPIO_IRQHandler(void)
{
    uint32_t isr = GPIO->INT_STATUS_F.INT_STATUS;

    if (gpio_isr_callback)
    {
        gpio_isr_callback(isr);
    }

    GPIO->INT_CLR_F.INT_CLR |= isr; /* clear the enable flag */
}
