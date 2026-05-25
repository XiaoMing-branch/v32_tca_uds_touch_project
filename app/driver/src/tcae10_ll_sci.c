/**
 *****************************************************************************
 * @brief   sci driver source file.
 *
 * @file    tcae10_ll_sci.c
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

#include "tcae10_ll_sci.h"
#include "tcae10_ll_cortex.h"
#include "tcae10_ll_flash.h"
#include "tcae10_ll_gpio.h"
#include "tcae10_ll_lpm.h"
#include "system_tcae10.h"

static ISR_FUNC_CALLBACK sci_isr_callback[LL_SCI_BUS_MAX - 1] = {NULL};

#define LIN_ISR_FLAG       (0x7FFFFFUL)
#define LIN_CHECKSUM_USE_SW     0

/**
 * @brief   配置SCI模块时钟
 * @param   bus    - SCI总线号（BUS_0/UART调试口，BUS_1/LIN SCI，BUS_2/LIN SCI1）
 * @param   config - 时钟配置参数
 * @note    BUS_0仅需使能PCLK，BUS_1和BUS_2需复位后使能PCLK和FCLK并设置分频
 * @retval  None
 */
static void ll_sci_clk_config(ll_sci_bus_e bus, ll_clk_config_t *config)
{
    CRG_CONFIG_UNLOCK();

    switch (bus)
    {
        case LL_SCI_BUS_0:
            /* 使能UART调试口PCLK */
            CRG->PRINT_UART_CLKRST_CTRL_F.PCLK_EN_PRINT_UART = 1;
            break;

        case LL_SCI_BUS_1:
            /* 更改LIN波特率前需复位 */
            CRG->LIN_SCI_CLKRST_CTRL_F.RST_LIN_SCI = 1;
            CRG->LIN_SCI_CLKRST_CTRL_F.RST_LIN_SCI = 0;

            CRG->LIN_SCI_CLKRST_CTRL_F.PCLK_EN_LIN_SCI = 1;
            CRG->LIN_SCI_CLKRST_CTRL_F.FCLK_EN_LIN_SCI = 1;
            CRG->LIN_SCI_CLKRST_CTRL_F.FCLK_DIV_LIN_SCI = config->fclk_div;
            break;

        case LL_SCI_BUS_2:
            CRG->LIN_SCI1_CLKRST_CTRL_F.RST_LIN_SCI1 = 1;
            CRG->LIN_SCI1_CLKRST_CTRL_F.RST_LIN_SCI1 = 0;

            CRG->LIN_SCI1_CLKRST_CTRL_F.PCLK_EN_LIN_SCI1 = 1;
            CRG->LIN_SCI1_CLKRST_CTRL_F.FCLK_EN_LIN_SCI1 = 1;
            CRG->LIN_SCI1_CLKRST_CTRL_F.FCLK_DIV_LIN_SCI1 = config->fclk_div;
            break;

        default:
            break;
    }

    CRG_CONFIG_LOCK();
}

/**
 * @brief   配置SCI模块的GPIO引脚复用
 * @param   bus  - SCI总线号
 * @param   mode - SCI工作模式（UART/LIN）
 * @note    BUS_0: GPIO2 AFIO_MUX_1（UART TX）
 *          BUS_1: GPIO7/8 AFIO_MUX_2（UART模式）
 *          BUS_2: GPIO3/4 AFIO_MUX_5（LIN TX/RX）
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
static ll_status_e ll_sci_gpio_config(ll_sci_bus_e bus, ll_sci_mode_e mode)
{
    if (bus >= LL_SCI_BUS_MAX)
    {
        return LL_ERROR;
    }

    switch (bus)
    {
        case LL_SCI_BUS_0:
            ll_gpio_afio_config(GPIO_PIN_2, AFIO_MUX_1);           /* UART TX -> GPIO2 */
            break;

        case LL_SCI_BUS_1:
            if (SCI_MODE_UART == mode)
            {
                ll_gpio_afio_config(GPIO_PIN_7, AFIO_MUX_2);       /* UART TX -> GPIO7 */
                ll_gpio_afio_config(GPIO_PIN_8,  AFIO_MUX_2);     /* UART RX -> GPIO8 */
            }
            break;

        case LL_SCI_BUS_2:
            ll_gpio_afio_config(GPIO_PIN_3, AFIO_MUX_5);           /* LIN1 TX -> GPIO3 */
            ll_gpio_afio_config(GPIO_PIN_4, AFIO_MUX_5);           /* LIN1 RX -> GPIO4 */
            break;

        default:
            break;
    }

    return LL_OK;
}

/**
 * @brief   清除SCI状态（FIFO/Abort）
 * @param   bus  - SCI总线号
 * @param   type - 清除类型（TX FIFO/RX FIFO/TX Abort/RX Abort的组合）
 * @retval  None
 */
void ll_sci_state_clear(ll_sci_bus_e bus, ll_sci_clear_type_e type)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (!type || bus < LL_SCI_BUS_1)
    {
        return;
    }

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR :
                                          LIN_SCI1_BASE_ADDR);

    if (type & SCI_CLEAR_TX_FIFO)
    {
        lin_sci_reg->CTRL_F.TX_FIFO_CLR = 1;                      /* 清除TX FIFO */
    }

    if (type & SCI_CLEAR_RX_FIFO)
    {
        lin_sci_reg->CTRL_F.RX_FIFO_CLR = 1;                      /* 清除RX FIFO */
    }

    if (type & SCI_CLEAR_TX_ABORT)
    {
        lin_sci_reg->CTRL_F.TX_ABORT = 1;                         /* 中止TX */
    }

    if (type & SCI_CLEAR_RX_ABORT)
    {
        lin_sci_reg->CTRL_F.RX_ABORT = 1;                         /* 中止RX */
    }
}

/**
 * @brief   配置SCI控制寄存器
 * @param   bus  - SCI总线号
 * @param   mode - SCI工作模式
 * @note    UART模式：全局使能+TX/RX使能
 *          LIN模式：使能自动波特率、校验和、位错误检测、短路检测等
 *          LIN主模式额外使能主设备标志
 * @retval  None
 */
static void ll_sci_contrl_config(ll_sci_bus_e bus, ll_sci_mode_e mode)
{
    uint32_t reg_val = 0;

    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (bus < LL_SCI_BUS_1)
    {
        return;
    }

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR :
                                          LIN_SCI1_BASE_ADDR);

    /* 配置主设备发送参数 */
    ll_sci_state_clear(bus, (ll_sci_clear_type_e)(SCI_CLEAR_TX_FIFO | SCI_CLEAR_RX_FIFO));

    if (SCI_MODE_UART == mode)
    {
        reg_val |= LIN_SCI_CTRL_GLB_EN_SET(1) | LIN_SCI_CTRL_TX_EN_SET(1) | LIN_SCI_CTRL_RX_EN_SET(1);
        reg_val &= (LIN_SCI_CTRL_TX_NUM_MODE_CLR & LIN_SCI_CTRL_TX_NUM_CLR);
        reg_val &= (LIN_SCI_CTRL_RX_NUM_MODE_CLR & LIN_SCI_CTRL_RX_NUM_CLR);
    }
    else
    {
        reg_val |= LIN_SCI_CTRL_GLB_EN_SET(1) | LIN_SCI_CTRL_RX_EN_SET(1) | LIN_SCI_CTRL_AUTO_BAUD_EN_SET(1);
#if !LIN_CHECKSUM_USE_SW
        reg_val |= LIN_SCI_CTRL_CHKSUM_EN_SET(1) | LIN_SCI_CTRL_CHKSUM_TYPE_SET(1);  /* 硬件CRC */
#else
        reg_val |= LIN_SCI_CTRL_CHKSUM_EN_SET(0);
#endif
        reg_val |= LIN_SCI_CTRL_TX_NUM_MODE_SET(1) | LIN_SCI_CTRL_TX_NUM_SET(8);
        reg_val |= LIN_SCI_CTRL_RX_NUM_MODE_SET(1) | LIN_SCI_CTRL_RX_NUM_SET(8);
        reg_val |= LIN_SCI_CTRL_SHORT_GND_DET_EN_SET(1) | LIN_SCI_CTRL_BIT_ERR_DET_EN_SET(1);  /* 短路和位错误检测 */

        if (SCI_MODE_LIN_M == mode)
        {
            reg_val |= LIN_SCI_CTRL_MASTER_EN_SET(1);             /* LIN主模式 */
        }
    }

    lin_sci_reg->CTRL = reg_val;
}

/**
 * @brief   配置SCI中断
 * @param   bus      - SCI总线号
 * @param   config   - 中断配置参数
 * @param   callback - 中断回调函数指针
 * @note    清除中断标志，根据使能状态配置IMR寄存器和回调
 * @retval  None
 */
static void ll_sci_isr_config(ll_sci_bus_e bus, ll_isr_config_t *config, ISR_FUNC_CALLBACK callback)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;
    IRQn_Type irq = (LL_SCI_BUS_1 == bus) ? LINSCI_IRQn : LINSCI_UART_IRQn;
    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR :
                                          LIN_SCI1_BASE_ADDR);

    lin_sci_reg->ICR |= LIN_ISR_FLAG;                              /* 清除所有中断标志 */

    if (config->isr_enable)
    {
        lin_sci_reg->IMR &= ~(config->isr & LIN_ISR_FLAG);         /* 使能指定中断 */
        sci_isr_callback[bus - 1] = callback;
        NVIC_SetPriority(irq, config->priority);                   /* 设置中断优先级 */
    }
    else
    {
        lin_sci_reg->IMR |= LIN_ISR_FLAG;                          /* 禁能所有中断 */
    }
}

/**
 * @brief   反初始化SCI外设
 * @param   bus - SCI总线号
 * @note    复位外设，对于BUS_1/2额外禁能NVIC中断
 * @retval  None
 */
void ll_sci_deinit(ll_sci_bus_e bus)
{
    CRG_CONFIG_UNLOCK();

    if (bus == LL_SCI_BUS_0)
    {
        CRG->PRINT_UART_CLKRST_CTRL_F.RST_PRINT_UART = 1 ;        /* 复位Print UART */
        __NOP();
        __NOP();
        CRG->PRINT_UART_CLKRST_CTRL_F.RST_PRINT_UART = 0 ;
        __NOP();
        __NOP();
    }
    else
    {
        CRG->LIN_SCI_CLKRST_CTRL_F.RST_LIN_SCI = 1;               /* 复位LIN SCI */
        __NOP();
        __NOP();
        CRG->LIN_SCI_CLKRST_CTRL_F.RST_LIN_SCI = 0;
        __NOP();
        __NOP();

        NVIC_ClearPendingIRQ(LINSCI_IRQn);
        NVIC_DisableIRQ(LINSCI_IRQn);

        sci_isr_callback[bus - 1] = NULL;
    }

    CRG_CONFIG_LOCK();
}

/**
 * @brief   使能LIN自动寻址（Auto Addressing）功能
 * @param   bus           - SCI总线号（仅支持BUS_1）
 * @param   type          - AA类型（2步/3步/4步电流测量）
 * @param   ext_shunt_res - 是否使用外部并联电阻
 * @param   cur_th        - 电流阈值数组
 * @note    配置电流源档位、电流阈值、时钟偏差时间、PGA就绪时间等。
 *         支持外部或内部并联电阻的自适应配置。
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_aa_enable(ll_sci_bus_e bus, lin_aa_type_e type, bool ext_shunt_res, uint16_t *cur_th)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    /* 先禁能AA配置，需要复位 */
    LIN_SCI->ANALOG_CTRL = 4;
    ll_lin_rx_delay_set(bus, 0);

    if (LIN_AA_STYPE_STEPS_2 == type)
    {
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP1 = 0b10001;  /* 1.1mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP2 = 0b11000;  /* 1.1mA */
    }
    else if (LIN_AA_STYPE_STEPS_3 == type)
    {
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP1 = 0b00001;  /* 1mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP2 = 0b00111;  /* 4mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP3 = 0b01111;  /* 8mA */
    }
    else
    {
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP1 = 0b00000;  /* 0.5mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP2 = 0b00011;  /* 2mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP3 = 0b01001;  /* 5mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP4 = 0b01111;  /* 8mA */
    }

    if (!ext_shunt_res)
    {
        uint32_t reg_val = 0xFFF;
        reg_val = REG_READ32(0x00800080);                          /* 读取内部并联电阻修调值 */
        reg_val &= 0xFFF;

        if (reg_val != 0xFFF)
        {
            float resistor_ration = (float)reg_val / 800;

            for (uint8_t i = 0; i < 3; i++)
            {
                cur_th[i] = (uint16_t)((uint32_t)cur_th[i] * resistor_ration);  /* 调整电流阈值 */
            }
        }
    }

    /* 设置电流阈值 */
    LIN_SCI->CUR_TH1_F.CUR_TH_STEP1 = cur_th[0];
    LIN_SCI->CUR_TH1_F.CUR_TH_STEP2 = cur_th[1];
    LIN_SCI->CUR_TH2_F.CUR_TH_STEP3 = cur_th[2];
    LIN_SCI->CUR_TH2_F.CUR_TH_STEP4 = cur_th[3];

    /* 时钟偏差时间配置 */
    if (DEFAULT_SYSTEM_CLOCK < 48000000UL)
    {
        if (LIN_AA_STYPE_STEPS_2 == type)
        {
            LIN_SCI->CLK_DEV_TIM_CFG_F.CLK_DEV_TIM = 0x3FF >> 1;
        }
        else
        {
            LIN_SCI->CLK_DEV_TIM_CFG_F.CLK_DEV_TIM = 0x290 >> 1;
            LIN_SCI->CLK_DEV_TIM_CFG_F.CLK_DEV_TIM_DELTA = 0xF0 >> 1;
        }
    }
    else
    {
        if (LIN_AA_STYPE_STEPS_2 == type)
        {
            LIN_SCI->CLK_DEV_TIM_CFG_F.CLK_DEV_TIM = 0x3FF;
        }
        else
        {
            LIN_SCI->CLK_DEV_TIM_CFG_F.CLK_DEV_TIM = 0x290;
            LIN_SCI->CLK_DEV_TIM_CFG_F.CLK_DEV_TIM_DELTA = 0xF0;
        }
    }

    /* PGA就绪时间配置（时基=20.8ns） */
    if (DEFAULT_SYSTEM_CLOCK < 48000000UL)
    {
        if (LIN_AA_STYPE_STEPS_2 == type)
        {
            LIN_SCI->PGA_RDY_TIM_CFG_F.PGA_RDY_TIM =  0xB20 >> 1;
        }
        else
        {
            LIN_SCI->PGA_RDY_TIM_CFG_F.PGA_RDY_TIM =  0xA50 >> 1;
        }
    }
    else
    {
        if (LIN_AA_STYPE_STEPS_2 == type)
        {
            LIN_SCI->PGA_RDY_TIM_CFG_F.PGA_RDY_TIM =  0xB20;
        }
        else
        {
            LIN_SCI->PGA_RDY_TIM_CFG_F.PGA_RDY_TIM =  0xA50;
        }
    }

    /* 自动寻址模拟配置 */
    if (ext_shunt_res)
    {
        LIN_SCI->AUTO_ADDR_ANA_CFG = 0x04F;
    }
    else
    {
        LIN_SCI->AUTO_ADDR_ANA_CFG = (LIN_AA_STYPE_STEPS_2 == type) ? 0x047 : 0x042;
    }

    /* 自动寻址控制 */
    LIN_SCI->AUTO_ADDR_CTRL = LIN_SCI_AUTO_ADDR_CTRL_AUTO_ADDR_EN_SET(1) |
                              LIN_SCI_AUTO_ADDR_CTRL_AUTO_ADDR_ANA_EN_SET(1) |
                              LIN_SCI_AUTO_ADDR_CTRL_MEAS_STEP_SEL_SET(type);

    /* 使能AA相关中断 */
    LIN_SCI->ICR |= (SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);
    LIN_SCI->IMR &= ~(SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);

    return LL_OK;
}

/**
 * @brief   禁能LIN自动寻址功能
 * @param   bus - SCI总线号（仅支持BUS_1）
 * @note    清除所有AA相关配置寄存器，恢复LIN模拟控制
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_aa_disable(ll_sci_bus_e bus)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    LIN_SCI->ICR |= (SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);
    LIN_SCI->IMR |= (SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);

    /* 清零电流源配置 */
    LIN_SCI->CURRENT_SOURCE_ISET = 0;

    /* 清零电流阈值 */
    LIN_SCI->CUR_TH1 = 0;
    LIN_SCI->CUR_TH2 = 0;

    /* 清零时钟偏差时间配置 */
    LIN_SCI->CLK_DEV_TIM_CFG = 0;

    /* 清零PGA就绪时间配置 */
    LIN_SCI->PGA_RDY_TIM_CFG = 0;

    /* 自动寻址模拟配置 */
    LIN_SCI->AUTO_ADDR_ANA_CFG = 0x04F;

    /* 自动寻址控制 */
    LIN_SCI->AUTO_ADDR_CTRL = 0x00;

    /* 恢复LIN模拟控制 */
    LIN_SCI->ANALOG_CTRL = 7;
    ll_lin_rx_delay_set(bus, 3);

    return LL_OK;
}

/**
 * @brief   初始化SCI外设
 * @param   bus      - SCI总线号
 * @param   config   - SCI配置参数（包含时钟、模式、波特率、中断配置）
 * @param   callback - 中断回调函数指针
 * @note    配置GPIO、时钟、波特率、UART/LIN模式寄存器、位错误检测点等
 * @retval  None
 */
void ll_sci_init(ll_sci_bus_e bus, sci_config_t *config, ISR_FUNC_CALLBACK callback)
{
    assert_param(IS_SCI_BUS(bus));
    assert_param(IS_SCI_MODE(config->mode));

    ll_sci_gpio_config(bus, config->mode);                         /* 配置GPIO引脚 */
    ll_sci_clk_config(bus, &config->clk_cfg);                       /* 配置时钟 */
    ll_sci_baudrate_config(bus, config->baudrate);                 /* 配置波特率 */

    if (bus == LL_SCI_BUS_0)
    {
        /* BUS_0是调试UART，无需额外配置 */
    }
    else
    {
        LIN_SCI_REG_TypeDef *lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                            LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

        if (SCI_MODE_UART == config->mode)
        {
            lin_sci_reg->CTRL_UART_F.MODE = 1;                    /* 设置为UART模式 */
            lin_sci_reg->CTRL_UART_F.STP_BIT_SEL = 0;             /* 1位停止位 */
        }
        else
        {
            lin_sci_reg->CTRL_UART_F.MODE = 0;                    /* 设置为LIN模式 */

            if (SCI_MODE_LIN_M == config->mode)
            {
                config->isr_cfg.isr |= SCI_INT_TX_PID_DONE;       /* LIN主模式使能PID发送完成中断 */
            }
        }

        /* 位错误检测点选择：0@位中间，1@位末尾 */
        lin_sci_reg->TX_CFG_F.CHK_PT_SEL = 1;

        if (config->baudrate > 19200UL)
        {
            /* 高速模式下关闭TX-RX冲突检测 */
            config->isr_cfg.isr &= ~SCI_INT_TX_RX_CONF;
        }

        ll_sci_contrl_config(bus, config->mode);                   /* 配置控制寄存器 */
        ll_sci_isr_config(bus, &config->isr_cfg, callback);        /* 配置中断 */
    }
}

/**
 * @brief   配置SCI波特率
 * @param   bus      - SCI总线号
 * @param   baudrate - 目标波特率
 * @note    BUS_0通过分频系数直接设置，BUS_1/2通过整数+小数分频实现。
 *         高速模式下（>19200）调整RX滤波和模拟控制以优化性能。
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_sci_baudrate_config(ll_sci_bus_e bus, uint32_t baudrate)
{
    if (bus >= LL_SCI_BUS_MAX)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_0 == bus)
    {
        /* 波特率 = pclk / (prescale + 1) */
        PRINT_UART->PRESCALE_F.PRESCALE = sys_pclk_freq_get() / baudrate - 1;
    }
    else
    {
        uint32_t fclk, div = 0;
        uint32_t frac;
        uint32_t intr;
        LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

        lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                              LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);
        if (LL_SCI_BUS_1 == bus)
        {
            div = CRG->LIN_SCI_CLKRST_CTRL_F.FCLK_DIV_LIN_SCI;
        }
        else  if (LL_SCI_BUS_2 == bus)
        {
            div = CRG->LIN_SCI1_CLKRST_CTRL_F.FCLK_DIV_LIN_SCI1;
        }
        /* fclk(sci) = hclk / (div(sci) + 1) */
        fclk = DEFAULT_SYSTEM_CLOCK / (div + 1);

        /* 波特率 = fclk / (intr + frac / 16) */
        frac = (fclk >> 4) % baudrate;
        frac = (frac << 4) / baudrate;
        intr = (fclk >> 4) / baudrate;
        lin_sci_reg->BAUD_CFG = ((frac & 0xF) << 12) | (intr & 0x1FF);  /* 写入整数和小数分频 */

        if (baudrate > 19200UL)
        {
            /* 高速模式配置 */
            lin_sci_reg->RX_FILTER_CFG_F.RX_FILTER_TIM = 8;
            lin_sci_reg->ANALOG_CTRL = 0;
            ll_lin_rx_delay_set(bus, 0);
            TEST_CONFIG_UNLOCK();
            TEST->TEST_LIN_CTRL_F.LIN_EMC_FBNEG_EN = false;       /* 关EMC反馈负向使能 */
            TEST_CONFIG_LOCK();
        }
        else
        {
            /* 低速模式配置 */
            lin_sci_reg->RX_FILTER_CFG_F.RX_FILTER_TIM = 200;
            lin_sci_reg->ANALOG_CTRL = 3;
            ll_lin_rx_delay_set(bus, 3);
            TEST_CONFIG_UNLOCK();
            TEST->TEST_LIN_CTRL_F.LIN_EMC_FBNEG_EN = true;        /* 开EMC反馈负向使能 */
            TEST_CONFIG_LOCK();
        }
    }

    return LL_OK;
}

/**
 * @brief   使能或禁能SCI NVIC中断
 * @param   bus    - SCI总线号（BUS_0不支持）
 * @param   enable - true使能中断，false禁能
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_sci_isr_enable(ll_sci_bus_e bus, bool enable)
{
    if (bus >= LL_SCI_BUS_MAX || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    IRQn_Type irq = (LL_SCI_BUS_1 == bus) ? LINSCI_IRQn : LINSCI_UART_IRQn;

    NVIC_ClearPendingIRQ(irq);

    if (enable)
    {
        NVIC_EnableIRQ(irq);
    }
    else
    {
        NVIC_DisableIRQ(irq);
    }

    return LL_OK;
}

/**
 * @brief   设置LIN RX延迟
 * @param   bus   - SCI总线号（仅BUS_1支持）
 * @param   count - 延迟计数
 * @note    通过TEST模块配置LIN RX采样延迟
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_rx_delay_set(ll_sci_bus_e bus, uint8_t count)
{
    if (bus >= LL_SCI_BUS_MAX || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        TEST_CONFIG_UNLOCK();
        TEST->TEST_LIN_CTRL_F.LIN_RX_DELAY = count;               /* 设置RX延迟 */
        TEST_CONFIG_LOCK();
    }

    return LL_OK;
}

/**
 * @brief   使能LIN唤醒功能
 * @param   bus    - SCI总线号（仅BUS_1支持）
 * @param   enable - true使能，false禁能
 * @note    配置LIN为唤醒源，设置唤醒时间和滤波，使能ASYSCFG和NVIC中断
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_wakeup_enable(ll_sci_bus_e bus, bool enable)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    ll_wakeup_init(WAKEUP_SOUERCE_LIN, WAKEUP_TIME_5, WAKEUP_FILTER_3);  /* 初始化LIN唤醒 */

    ll_syscfg_isr_enable(ASYSCFG_INT_WAKEUP, enable);             /* 使能/禁能唤醒中断 */

    NVIC_SetPriority(AON_IRQn, 3);                                /* 设置AON中断优先级 */
    NVIC_EnableIRQ(AON_IRQn);                                     /* 使能AON中断 */

    return LL_OK;
}

/**
 * @brief   设置LIN自动寻址就绪标志
 * @param   bus    - SCI总线号（仅BUS_1支持）
 * @param   enable - true表示地址已就绪，false表示未就绪
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_aa_ready_set(ll_sci_bus_e bus, bool enable)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    LIN_SCI->AUTO_ADDR_CTRL_F.ADDR_ALREADY_FLAG = enable ? 1 : 0;  /* 设置地址就绪标志 */

    return LL_OK;
}

/**
 * @brief   SCI发送数据
 * @param   bus    - SCI总线号
 * @param   buffer - 待发送数据缓冲区
 * @param   length - 数据长度
 * @note    根据总线号选择对应的硬件模块，阻塞等待TX FIFO非满后写入数据
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_sci_transmit(ll_sci_bus_e bus, uint8_t *buffer, uint16_t length)
{
    if (bus >= LL_SCI_BUS_MAX || NULL == buffer || !length)
    {
        return LL_ERROR;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        switch (bus)
        {
            case LL_SCI_BUS_0:
                while (PRINT_UART->STATUS_F.TX_BUSY == 1);          /* 等待TX空闲 */
                PRINT_UART->TX_DATA_F.TX_DATA = buffer[i];
                break;

            case LL_SCI_BUS_1:
                while (LIN_SCI->STATUS_F.TX_FIFO_FULL);             /* 等待TX FIFO非满 */
                LIN_SCI->TX_DATA_F.TX_DATA = buffer[i];
                break;

            case LL_SCI_BUS_2:
                while (LIN_SCI1->STATUS_F.TX_FIFO_FULL);
                LIN_SCI1->TX_DATA_F.TX_DATA = buffer[i];
                break;

            default:
                break;
        }
    }

    return LL_OK;
}

/**
 * @brief   SCI接收数据
 * @param   bus    - SCI总线号（BUS_0不支持）
 * @param   buffer - 接收缓冲区
 * @param   length - 数据长度
 * @note    根据总线号选择对应的硬件模块，从RX FIFO读取数据
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_sci_receive(ll_sci_bus_e bus, uint8_t *buffer, uint16_t length)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus || NULL == buffer || !length)
    {
        return LL_ERROR;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        if (LL_SCI_BUS_1 == bus)
        {
            buffer[i] = LIN_SCI->RX_DATA_F.RX_DATA;                /* 读取BUS_1 RX数据 */
        }
        else
        {
            buffer[i] = LIN_SCI1->RX_DATA_F.RX_DATA;               /* 读取BUS_2 RX数据 */
        }
    }

    return LL_OK;
}

/**
 * @brief   LIN发送数据帧（主模式）
 * @param   bus    - SCI总线号（BUS_0不支持）
 * @param   pid    - LIN协议ID（Protected Identifier）
 * @param   buffer - 待发送数据缓冲区（最多4字节）
 * @param   length - 数据长度
 * @note    先检查是否有STOP错误，清除TX FIFO，设置PID和校验和类型，
 *         发送数据字节，可选软件计算校验和
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误，LL_COMM_ERROR - 通信错误
 */
ll_status_e ll_lin_transmit(ll_sci_bus_e bus, uint8_t pid, uint8_t *buffer, uint16_t length)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus || NULL == buffer || !length)
    {
        return LL_ERROR;
    }

#if !LIN_CHECKSUM_USE_SW
    ll_lin_checksum_e checksum_type = ((0x3C == pid) || (0x7D == pid)) ? LIN_CHECKSUM_CLASSIC : LIN_CHECKSUM_ENHANCED;
#endif

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

    /* PID中断先于STOP ERROR中断，发送前先查询是否有STOP ERROR并做ABORT处理 */
    for (int i = 0; i < 100; ++i)                                   /* 等待足够时间让STOP ERROR置位 */
    {
        __NOP();
    }
    if (lin_sci_reg->ISR & SCI_INT_RX_STP_ERR)                     /* 检查STOP错误 */
    {
        lin_sci_reg->ICR |= SCI_INT_RX_STP_ERR;
        ll_sci_state_clear(bus, (ll_sci_clear_type_e)(SCI_CLEAR_TX_ABORT | SCI_CLEAR_RX_ABORT));
        return LL_COMM_ERROR;
    }

    lin_sci_reg->CTRL_F.TX_EN = 0;                                 /* 先禁能发送 */

    /* 清除TX FIFO */
    ll_sci_state_clear(bus, SCI_CLEAR_TX_FIFO);

#if !LIN_CHECKSUM_USE_SW
    lin_sci_reg->CTRL_F.CHKSUM_TYPE = (LIN_CHECKSUM_CLASSIC == checksum_type) ? 0 : 1;  /* 设置校验和类型 */
    lin_sci_reg->CTRL_F.TX_NUM = length;
#endif

    lin_sci_reg->CTRL_F.TX_EN = 1;                                 /* 使能发送 */

    length = (length > 4) ? 4 : length;                             /* LIN数据最多4字节 */
    for (uint8_t i = 0; i < length; i++)
    {
        while (lin_sci_reg->STATUS_F.TX_FIFO_FULL);
        lin_sci_reg->TX_DATA_F.TX_DATA = buffer[i];                /* 写入发送数据 */
    }

#if LIN_CHECKSUM_USE_SW

    while (lin_sci_reg->STATUS_F.TX_FIFO_FULL);

    lin_sci_reg->TX_DATA_F.TX_DATA = ll_lin_checksum_calib_func(pid, buffer, length);  /* 软件校验和 */
#endif

    return LL_OK;
}

/**
 * @brief   LIN接收数据帧（从模式）
 * @param   bus    - SCI总线号（BUS_0不支持）
 * @param   pid    - LIN协议ID
 * @param   buffer - 接收缓冲区
 * @param   length - 数据长度
 * @note    禁能发送，使能接收，清除RX FIFO
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_receive(ll_sci_bus_e bus, uint8_t pid, uint8_t *buffer,  uint16_t length)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus || NULL == buffer || !length)
    {
        return LL_ERROR;
    }

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

    lin_sci_reg->CTRL_F.TX_EN = 0;                                 /* 禁能发送 */
    lin_sci_reg->CTRL_F.RX_EN = 1;                                 /* 使能接收 */

    ll_sci_state_clear(bus, SCI_CLEAR_RX_FIFO);                    /* 清除RX FIFO */

    return LL_OK;
}

/**
 * @brief   LIN校验和计算函数
 * @param   pid    - LIN协议ID
 * @param   buffer - 数据缓冲区
 * @param   length - 数据长度
 * @note    经典校验和：仅对数据字节求和取反；
 *         增强校验和：对PID和数据字节求和取反。
 *         PID=0x3C(MasterReq)和0x7D(SlaveResp)使用经典校验和。
 * @retval  校验和字节
 */
uint8_t ll_lin_checksum_calib_func(uint8_t pid, uint8_t *buffer, uint16_t length)
{
    uint16_t check_sum;

    /* PID=0x3C(Master Request)或0x7D(Slave Response)使用经典校验和 */
    check_sum = ((0x3C == pid) || (0x7D == pid))  ? 0 : pid;

    for (uint8_t i = 0; i < length; i++)
    {

        check_sum += buffer[i];

        /* 处理进位 */
        if (check_sum > 0xFF)
        {
            check_sum -= 0xFF;
        }
    }

    /* 取反 */
    return (uint8_t)(~check_sum);
}

/**
 * @brief   读取LIN接收到的PID
 * @param   bus - SCI总线号（BUS_0不支持）
 * @param   pid - 指向存储PID的变量指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_pid_read(ll_sci_bus_e bus, uint8_t *pid)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *pid = LIN_SCI->RX_PID_F.RX_PID;                          /* 读取BUS_1 PID */
    }
    else
    {
        *pid = LIN_SCI1->RX_PID_F.RX_PID;                         /* 读取BUS_2 PID */
    }

    return LL_OK;
}

/**
 * @brief   读取LIN接收的一个字节
 * @param   bus  - SCI总线号（BUS_0不支持）
 * @param   byte - 指向存储字节数据的变量指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_read_byte(ll_sci_bus_e bus, uint8_t *byte)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus || NULL == byte)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *byte = LIN_SCI->RX_DATA_F.RX_DATA;                       /* 读取BUS_1 RX数据 */
    }
    else
    {

        *byte = LIN_SCI1->RX_DATA_F.RX_DATA;                      /* 读取BUS_2 RX数据 */
    }

    return LL_OK;
}

/**
 * @brief   读取LIN自动波特率测量值
 * @param   bus  - SCI总线号（BUS_0不支持）
 * @param   baud - 指向存储波特率值的变量指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_auto_baudrate_read(ll_sci_bus_e bus, uint32_t *baud)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *baud = LIN_SCI->AUTO_BAUD_VAL_F.AUTO_BD_INTR;            /* 读取BUS_1自动波特率 */
    }
    else
    {
        *baud = LIN_SCI1->AUTO_BAUD_VAL_F.AUTO_BD_INTR;           /* 读取BUS_2自动波特率 */
    }

    return LL_OK;
}

/**
 * @brief   读取LIN当前波特率配置值
 * @param   bus  - SCI总线号（BUS_0不支持）
 * @param   baud - 指向存储波特率配置值的变量指针
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_baudrate_read(ll_sci_bus_e bus, uint32_t *baud)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *baud = LIN_SCI->BAUD_CFG_F.BD_INTR;                      /* 读取BUS_1波特率配置 */
    }
    else
    {
        *baud = LIN_SCI1->BAUD_CFG_F.BD_INTR;                     /* 读取BUS_2波特率配置 */
    }

    return LL_OK;
}

/**
 * @brief   设置LIN全局使能
 * @param   bus - SCI总线号（BUS_0不支持）
 * @param   sw  - true全局使能，false全局禁能
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_ctrl_glben(ll_sci_bus_e bus, bool sw)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        LIN_SCI->CTRL_F.GLB_EN = sw;                              /* 设置BUS_1全局使能 */
    }
    else
    {
        LIN_SCI1->CTRL_F.GLB_EN = sw;                             /* 设置BUS_2全局使能 */
    }

    return LL_OK;
}

/**
 * @brief   设置LIN RX中止
 * @param   bus - SCI总线号（BUS_0不支持）
 * @param   sw  - true中止接收，false恢复
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_ctrl_rx_abort(ll_sci_bus_e bus, bool sw)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        LIN_SCI->CTRL_F.RX_ABORT = sw;                            /* BUS_1 RX中止 */
    }
    else
    {
        LIN_SCI1->CTRL_F.RX_ABORT = sw;                           /* BUS_2 RX中止 */
    }

    return LL_OK;
}

/**
 * @brief   触发LIN Break信号发送
 * @param   bus     - SCI总线号（BUS_0不支持）
 * @param   brk_num - Break位数
 * @note    设置Break信号长度并触发发送
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_ctrl_brk_tx(ll_sci_bus_e bus, uint8_t brk_num)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

    lin_sci_reg->BRK_SYNC_CFG_F.BRK_NUM = brk_num;                /* 设置Break位数 */
    lin_sci_reg->CTRL_F.BRK_TX_TRIG = true;                       /* 触发Break发送 */

    return LL_OK;
}

/**
 * @brief   发送LIN帧头（Break + Sync + PID）
 * @param   bus - SCI总线号（BUS_0不支持）
 * @param   pid - LIN协议ID
 * @note    禁能接收，配置TX PID，触发Break发送（硬件自动发送Break+Sync+PID）
 * @retval  LL_OK - 成功，LL_ERROR - 参数错误
 */
ll_status_e ll_lin_tx_header(ll_sci_bus_e bus, uint8_t pid)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

    lin_sci_reg->CTRL_F.RX_EN = 0;                                 /* 禁能接收 */
    lin_sci_reg->TX_CFG_F.TX_PID = pid;                           /* 设置要发送的PID */
    lin_sci_reg->CTRL_F.BRK_TX_TRIG = true;                       /* 触发Break+Sync+PID发送 */

    return LL_OK;
}

/**
 * @brief   LIN SCI中断处理函数（BUS_1）
 * @note    读取中断状态，调用用户回调函数，清除中断标志
 * @retval  None
 */
void SCI_IRQHandler(void)
{
    uint32_t isr = LIN_SCI->ISR & LIN_ISR_FLAG;                   /* 读取中断状态 */

    if (isr)
    {
        if (NULL != sci_isr_callback[0])
        {
            sci_isr_callback[0](isr);                              /* 调用用户回调 */
        }

        LIN_SCI->ICR |= isr;                                       /* 清除中断标志 */
    }
}

/**
 * @brief   UART中断处理函数（BUS_2 / LIN SCI1）
 * @note    读取中断状态，调用用户回调函数，清除中断标志
 * @retval  None
 */
void UART_IRQHandler(void)
{
    uint32_t isr = LIN_SCI1->ISR & LIN_ISR_FLAG;                  /* 读取中断状态 */

    if (isr)
    {
        if (NULL != sci_isr_callback[1])
        {
            sci_isr_callback[1](isr);                              /* 调用用户回调 */
        }

        LIN_SCI1->ICR |= isr;                                      /* 清除中断标志 */
    }
}
