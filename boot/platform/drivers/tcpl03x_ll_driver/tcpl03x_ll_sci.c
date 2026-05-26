/**
 *****************************************************************************
 * @brief   sci driver source file.
 *
 * @file    tcpl03x_ll_sci.c
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

#include "tcpl03x_ll_sci.h"
#include "tcpl03x_ll_cortex.h"
#include "tcpl03x_ll_flash.h"
#include "tcpl03x_ll_gpio.h"
#include "tcpl03x_ll_lpm.h"
#include "system_tcpl03x.h"

/** @brief SCI中断回调函数指针数组，每个总线对应一个回调 */
static ISR_FUNC_CALLBACK sci_isr_callback[LL_SCI_BUS_MAX - 1] = {NULL};

/** @brief LIN中断状态标志掩码，屏蔽bit23及以上保留位 */
#define LIN_ISR_FLAG       (0x7FFFFFUL)
/** @brief LIN校验和计算方式选择：0=硬件计算，1=软件计算 */
#define LIN_CHECKSUM_USE_SW     0

/**
 * @brief   配置SCI模块时钟
 * @param   bus    - SCI总线号（LL_SCI_BUS_0/UART调试口，LL_SCI_BUS_1/LIN SCI，LL_SCI_BUS_2/LIN SCI1）
 * @param   config - 时钟配置参数结构体指针（含FCLK分频系数）
 * @note    BUS_0仅需使能PCLK；BUS_1和BUS_2需先复位再使能PCLK和FCLK，并设置FCLK分频。
 *          首次配置或更改波特率前需调用本函数确保时钟就绪。
 * @retval  None
 */
static void ll_sci_clk_config(ll_sci_bus_e bus, ll_clk_config_t *config)
{
    CRG_CONFIG_UNLOCK();

    switch (bus)
    {
        case LL_SCI_BUS_0:
            /* enable bus0 pclk */
            CRG->PRINT_UART_CLKRST_CTRL_F.PCLK_EN_PRINT_UART = 1;
            break;

        case LL_SCI_BUS_1:
            /* lin baudrate change, need reset */
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
 * @brief   配置SCI模块的GPIO引脚复用功能
 * @param   bus  - SCI总线号
 * @param   mode - SCI工作模式（UART/LIN），UART模式下BUS_1额外配置GPIO7/8
 * @note    BUS_0: GPIO2 AFIO_MUX_1（Print UART TX）
 *          BUS_1: UART模式下GPIO7/8 AFIO_MUX_2（忽略LIN模式GPIO配置）
 *          BUS_2: GPIO3/4 AFIO_MUX_5
 * @retval  LL_OK - 配置成功
 * @retval  LL_ERROR - 总线号超出范围（>= LL_SCI_BUS_MAX）
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
            ll_gpio_afio_config(GPIO_PIN_2, AFIO_MUX_1);
            //            ll_gpio_afio_config(GPIO_PIN_3, AFIO_MUX_1);
            //            ll_gpio_afio_config(GPIO_PIN_6, AFIO_MUX_2);
            break;

        case LL_SCI_BUS_1:
            if (SCI_MODE_UART == mode)
            {
                ll_gpio_afio_config(GPIO_PIN_7, AFIO_MUX_2);
                ll_gpio_afio_config(GPIO_PIN_8,  AFIO_MUX_2);
            }

            break;

        case LL_SCI_BUS_2:
            ll_gpio_afio_config(GPIO_PIN_3, AFIO_MUX_5);
            ll_gpio_afio_config(GPIO_PIN_4, AFIO_MUX_5);
            break;

        default:
            break;
    }

    return LL_OK;
}

/**
 * @brief   清除SCI总线状态（FIFO和Abort标志）
 * @param   bus  - SCI总线号（仅BUS_1和BUS_2有效）
 * @param   type - 清除类型掩码，可组合使用：
 *                 SCI_CLEAR_TX_FIFO  | 清除发送FIFO
 *                 SCI_CLEAR_RX_FIFO  | 清除接收FIFO
 *                 SCI_CLEAR_TX_ABORT | 中止发送
 *                 SCI_CLEAR_RX_ABORT | 中止接收
 * @note    仅LIN SCI（BUS_1/2）支持此操作，BUS_0无FIFO控制。
 *          当type为0或bus<BUS_1时直接返回不做任何操作。
 *          写1到清除位后硬件自动清零。
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
        lin_sci_reg->CTRL_F.TX_FIFO_CLR = 1;
    }

    if (type & SCI_CLEAR_RX_FIFO)
    {
        lin_sci_reg->CTRL_F.RX_FIFO_CLR = 1;
    }

    if (type & SCI_CLEAR_TX_ABORT)
    {
        lin_sci_reg->CTRL_F.TX_ABORT = 1;
    }

    if (type & SCI_CLEAR_RX_ABORT)
    {
        lin_sci_reg->CTRL_F.RX_ABORT = 1;
    }
}

/**
 * @brief   配置SCI控制寄存器（CTRL）
 * @param   bus  - SCI总线号（仅BUS_1和BUS_2有效）
 * @param   mode - SCI工作模式
 * @note    UART模式：使能全局、TX、RX，清除TX/RX NUM模式及计数值。
 *          LIN模式：使能全局、RX、自动波特率、校验和（根据宏选择硬件/软件）、
 *          位错误检测、短路检测、TX等待FIFO有效。LIN主模式额外使能Master位。
 *          调用前必须先清除FIFO状态。
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

    /* setup master send config */
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
        reg_val |= LIN_SCI_CTRL_CHKSUM_EN_SET(1) | LIN_SCI_CTRL_CHKSUM_TYPE_SET(1) ; //crc
#else
        reg_val |= LIN_SCI_CTRL_CHKSUM_EN_SET(0);
#endif
        reg_val |= LIN_SCI_CTRL_TX_NUM_MODE_SET(1) | LIN_SCI_CTRL_TX_NUM_SET(8);
        reg_val |= LIN_SCI_CTRL_RX_NUM_MODE_SET(1) | LIN_SCI_CTRL_RX_NUM_SET(8);
        reg_val |= LIN_SCI_CTRL_SHORT_GND_DET_EN_SET(1) | LIN_SCI_CTRL_BIT_ERR_DET_EN_SET(1);
        reg_val |= LIN_SCI_CTRL_TX_WAIT_FIFO_VLD_EN_SET(1);

        if (SCI_MODE_LIN_M == mode)
        {
            reg_val |= LIN_SCI_CTRL_MASTER_EN_SET(1);
        }
    }

    lin_sci_reg->CTRL = reg_val;


}

/**
 * @brief   配置SCI中断使能及回调函数
 * @param   bus      - SCI总线号（LL_SCI_BUS_1 -> LINSCI_IRQn, LL_SCI_BUS_2 -> LINSCI_UART_IRQn）
 * @param   config   - 中断配置参数（中断掩码、使能标志、优先级）
 * @param   callback - 中断回调函数指针，使能中断时注册，禁能时忽略
 * @note    先清除所有中断标志（ICR），再根据config->isr_enable决定使能或禁能。
 *          使能时：将指定中断位写入IMR（清0使能），设置NVIC优先级，注册回调。
 *          禁能时：将所有中断位写入IMR（置1禁能），不修改回调。
 * @retval  None
 */
static void ll_sci_isr_config(ll_sci_bus_e bus, ll_isr_config_t *config, ISR_FUNC_CALLBACK callback)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;
    IRQn_Type irq = (LL_SCI_BUS_1 == bus) ? LINSCI_IRQn : LINSCI_UART_IRQn;
    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR :
                                          LIN_SCI1_BASE_ADDR);

    lin_sci_reg->ICR |= LIN_ISR_FLAG;

    if (config->isr_enable)
    {
        lin_sci_reg->IMR &= ~(config->isr & LIN_ISR_FLAG);
        sci_isr_callback[bus - 1] = callback;
        NVIC_SetPriority(irq, config->priority);
    }
    else
    {
        lin_sci_reg->IMR |= LIN_ISR_FLAG;
    }
}

/**
 * @brief   反初始化SCI外设（复位并关闭时钟）
 * @param   bus - SCI总线号
 * @note    BUS_0：通过CRG复位Print UART，需要NOP等待2个周期确保复位完成。
 *          BUS_1/2：复位LIN SCI，清除NVIC中断挂号，禁能中断，清空回调指针。
 *          操作前需解锁CRG配置，操作完后重新上锁。
 * @retval  None
 */
void ll_sci_deinit(ll_sci_bus_e bus)
{
    CRG_CONFIG_UNLOCK();

    if (bus == LL_SCI_BUS_0)
    {
        CRG->PRINT_UART_CLKRST_CTRL_F.RST_PRINT_UART = 1 ;
        __NOP();
        __NOP();
        CRG->PRINT_UART_CLKRST_CTRL_F.RST_PRINT_UART = 0 ;
        __NOP();
        __NOP();

    }
    else
    {
        CRG->LIN_SCI_CLKRST_CTRL_F.RST_LIN_SCI = 1;
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
 * @param   bus           - SCI总线号（仅支持LL_SCI_BUS_1）
 * @param   type          - AA测量步骤类型（2步/3步/4步电流测量）
 * @param   ext_shunt_res - 是否使用外部并联电阻：true=外部，false=内部
 * @param   cur_th        - 电流阈值数组指针（4元素），根据电阻比例自动调整
 * @note    使能前先写入ANALOG_CTRL=4禁能旧配置，调用ll_lin_rx_delay_set(0)。
 *          电流源档位根据steps类型配置不同组合。
 *          当使用内部电阻时，从OTP读取电阻校准值计算比例系数调整阈值。
 *          时钟偏差时间（CLK_DEV_TIM）和PGA就绪时间（PGA_RDY_TIM）根据系统时钟
 *          （<48MHz或>=48MHz）选择不同的分频配置。
 * @retval  LL_OK - 配置成功
 * @retval  LL_ERROR - 总线不是LL_SCI_BUS_1
 */
ll_status_e ll_lin_aa_enable(ll_sci_bus_e bus, lin_aa_type_e type, bool ext_shunt_res, uint16_t *cur_th)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    /* ll_lin_aa_diable, need reset */
    LIN_SCI->ANALOG_CTRL = 4;
    ll_lin_rx_delay_set(bus, 0);

    if (LIN_AA_STYPE_STEPS_2 == type)
    {
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP1 = 0b10001;    /* 1.1mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP2 = 0b11000;    /* 1.1mA */
    }
    else if (LIN_AA_STYPE_STEPS_3 == type)
    {
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP1 = 0b00001;    /*   1mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP2 = 0b00111;    /*   4mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP3 = 0b01111;    /*   8mA */
    }
    else
    {
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP1 = 0b00000;    /* 0.5mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP2 = 0b00011;    /*   2mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP3 = 0b01001;    /*   5mA */
        LIN_SCI->CURRENT_SOURCE_ISET_F.LIN_ISET_STEP4 = 0b01111;    /*   8mA */
    }

    if (!ext_shunt_res)
    {
        uint32_t reg_val = 0xFFF;
        reg_val = REG_READ32(0x00800080);
        reg_val &= 0xFFF;

        if (reg_val != 0xFFF)
        {
            float resistor_ration = (float)reg_val / 800;

            for (uint8_t i = 0; i < 3; i++)
            {
                cur_th[i] = (uint16_t)((uint32_t)cur_th[i] * resistor_ration);
            }
        }
    }

    /* Current threshold for test value 0x043 0x075, 0x0b9, 0x032 */
    LIN_SCI->CUR_TH1_F.CUR_TH_STEP1 = cur_th[0];
    LIN_SCI->CUR_TH1_F.CUR_TH_STEP2 = cur_th[1];
    LIN_SCI->CUR_TH2_F.CUR_TH_STEP3 = cur_th[2];
    LIN_SCI->CUR_TH2_F.CUR_TH_STEP4 = cur_th[3];

    /* Clk deviation time config */
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

    /* Pga ready time config:time base = 20.8ns */
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

    /* Auto addressing analog config  */
    if (ext_shunt_res)
    {
        LIN_SCI->AUTO_ADDR_ANA_CFG = 0x04F;
    }
    else
    {
        LIN_SCI->AUTO_ADDR_ANA_CFG = (LIN_AA_STYPE_STEPS_2 == type) ? 0x047 : 0x042;
    }

    /* Auto addressing control */
    LIN_SCI->AUTO_ADDR_CTRL = LIN_SCI_AUTO_ADDR_CTRL_AUTO_ADDR_EN_SET(1) | LIN_SCI_AUTO_ADDR_CTRL_AUTO_ADDR_ANA_EN_SET(1) | LIN_SCI_AUTO_ADDR_CTRL_MEAS_STEP_SEL_SET(type);

    /* LIN_AA interrupt enable */
    LIN_SCI->ICR |= (SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);
    LIN_SCI->IMR &= ~(SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);

    return LL_OK;
}

/**
 * @brief   禁能LIN自动寻址（Auto Addressing）功能
 * @param   bus - SCI总线号（仅支持LL_SCI_BUS_1）
 * @note    禁能步骤：
 *          1. 清除并屏蔽AA相关中断（SLV_SELECTED和AUTO_ADDR_DONE）
 *          2. 清零所有AA配置寄存器（电流源、电流阈值、时钟偏差、PGA就绪、模拟配置、控制）
 *          3. 写入ANALOG_CTRL=7恢复默认模拟控制
 *          4. 调用ll_lin_rx_delay_set(3)恢复默认RX延时
 * @retval  LL_OK - 配置成功
 * @retval  LL_ERROR - 总线不是LL_SCI_BUS_1
 */
ll_status_e ll_lin_aa_disable(ll_sci_bus_e bus)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    LIN_SCI->ICR |= (SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);
    LIN_SCI->IMR |= (SCI_INT_SLV_SELECTED | SCI_INT_AUTO_ADDR_DONE);

    /* current_source_iset */
    LIN_SCI->CURRENT_SOURCE_ISET = 0;

    /* Current threshold */
    LIN_SCI->CUR_TH1 = 0;
    LIN_SCI->CUR_TH2 = 0;

    /* Clk deviation time config */
    LIN_SCI->CLK_DEV_TIM_CFG = 0;

    /* Pga ready time config */
    LIN_SCI->PGA_RDY_TIM_CFG = 0;

    /* Auto addressing analog config  */
    LIN_SCI->AUTO_ADDR_ANA_CFG = 0x04F;

    /* Auto addressing control */
    LIN_SCI->AUTO_ADDR_CTRL = 0x00;

    /* ll_lin_aa_enable, need 4 & 3 */
    LIN_SCI->ANALOG_CTRL = 7;
    ll_lin_rx_delay_set(bus, 3);

    return LL_OK;
}

/**
 * @brief   初始化SCI外设
 * @param   bus      - SCI总线号
 * @param   config   - SCI配置参数（时钟、波特率、模式、中断配置）
 * @param   callback - 中断回调函数指针
 * @note    初始化流程：GPIO复用配置 -> 时钟配置 -> 波特率配置。
 *          BUS_1/2额外操作：
 *          - UART模式：设置MODE=1（UART模式）、停止位1位
 *          - LIN模式：设置MODE=0；主模式额外添加TX_PID_DONE中断
 *          - 波特率>19200时关闭TX_RX冲突检测中断（高速模式需关闭）
 *          - 设置位错误检测点为末尾（CHK_PT_SEL=1）
 *          - 配置控制寄存器并设置中断
 *          调用前需通过assert_param验证bus和mode参数有效性。
 * @retval  None
 */
void ll_sci_init(ll_sci_bus_e bus, sci_config_t *config, ISR_FUNC_CALLBACK callback)
{
    assert_param(IS_SCI_BUS(bus));
    assert_param(IS_SCI_MODE(config->mode));

    ll_sci_gpio_config(bus, config->mode);
    ll_sci_clk_config(bus, &config->clk_cfg);
    ll_sci_baudrate_config(bus, config->baudrate);

    if (bus == LL_SCI_BUS_0)
    {
    }
    else
    {
        LIN_SCI_REG_TypeDef *lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                           LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

        if (SCI_MODE_UART == config->mode)
        {
            lin_sci_reg->CTRL_UART_F.MODE = 1;
            /* set uart stop 1 bit*/
            lin_sci_reg->CTRL_UART_F.STP_BIT_SEL = 0;
            /* default disable mp mode*/
            // /* set uart MP mode */
            // LIN_SCI->CTRL_UART_F.MP_MODE_EN = 1;
            // LIN_SCI->CTRL_UART_F.MP_RX_ADDR_WR_EN = 1;
            // LIN_SCI->CTRL_UART_F.MP_TX_ADDR_DATA_SEL = 1;
            // /* set uart MP address */
            // LIN_SCI->RX_CFG_F.MP_SLAVE_ADDR = 0;
            // LIN_SCI->RX_CFG_F.MP_SLAVE_ADDR = 0xAA;
            // LIN_SCI->RX_CFG_F.MP_SLAVE_ADDR_MSK = 1;
        }
        else
        {
            lin_sci_reg->CTRL_UART_F.MODE = 0;

            if (SCI_MODE_LIN_M == config->mode)
            {
                config->isr_cfg.isr |= SCI_INT_TX_PID_DONE;
            }
        }

        /* bit error check point sel:0@middle, 1@last */
        lin_sci_reg->TX_CFG_F.CHK_PT_SEL = 1;

        if (config->baudrate > 19200UL)
        {
            /* 高速模式下需要关闭TX_RX检测 */
            config->isr_cfg.isr &= ~SCI_INT_TX_RX_CONF;
        }

        ll_sci_contrl_config(bus, config->mode);
        ll_sci_isr_config(bus, &config->isr_cfg, callback);
    }
}

/**
 * @brief   配置SCI波特率
 * @param   bus      - SCI总线号
 * @param   baudrate - 目标波特率值（单位：bps）
 * @note    BUS_0（Print UART）：波特率 = PCLK / (PRESCALE + 1)
 *          BUS_1/2（LIN SCI）：波特率 = FCLK / (INTR + FRAC/16)
 *          其中FCLK = HCLK / (FCLK_DIV + 1)
 *          波特率>19200时（高速模式）：
 *          - RX滤波器时间设为8，模拟控制关断，RX延时为0，禁能EMC反馈
 *          波特率≤19200时（低速模式）：
 *          - RX滤波器时间设为200，模拟控制使能，RX延时为3，使能EMC反馈
 * @retval  LL_OK - 配置成功
 * @retval  LL_ERROR - 总线号超出范围
 */
ll_status_e ll_sci_baudrate_config(ll_sci_bus_e bus, uint32_t baudrate)
{
    if (bus >= LL_SCI_BUS_MAX)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_0 == bus)
    {
        /* baudrate = pclk / (prescale + 1) */
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
        fclk = sys_hclk_freq_get() / (div + 1);

        /* baudrate = fclk / (intr + frac / 16) */
        frac = (fclk >> 4) % baudrate;
        frac = (frac << 4) / baudrate;
        intr = (fclk >> 4) / baudrate;
        lin_sci_reg->BAUD_CFG = ((frac & 0xF) << 12) | (intr & 0x1FF);

        if (baudrate > 19200UL)
        {
            // if (LL_SCI_BUS_1 == bus)
            {
                lin_sci_reg->RX_FILTER_CFG_F.RX_FILTER_TIM = 8;
                lin_sci_reg->ANALOG_CTRL = 0;
                ll_lin_rx_delay_set(bus, 0);
                TEST_CONFIG_UNLOCK();
                TEST->TEST_LIN_CTRL_F.LIN_EMC_FBNEG_EN = false;
                TEST_CONFIG_LOCK();
            }
        }
        else
        {
            // if (LL_SCI_BUS_1 == bus)
            {
                lin_sci_reg->RX_FILTER_CFG_F.RX_FILTER_TIM = 200;
                lin_sci_reg->ANALOG_CTRL = 7;
                ll_lin_rx_delay_set(bus, 3);
                TEST_CONFIG_UNLOCK();
                TEST->TEST_LIN_CTRL_F.LIN_EMC_FBNEG_EN = true;
                TEST_CONFIG_LOCK();
            }
        }
    }

    return LL_OK;
}

/**
 * @brief   使能或禁能SCI中断（NVIC级控制）
 * @param   bus    - SCI总线号（BUS_0不支持，仅BUS_1/2有效）
 * @param   enable - true=使能中断，false=禁能中断
 * @note    操作前先清除NVIC中断挂号位。仅控制NVIC使能/禁能，
 *          IMR寄存器配置在ll_sci_isr_config中完成。
 *          TODO: LIN速度10ms时可能需要调整优先级。
 * @retval  LL_OK - 操作成功
 * @retval  LL_ERROR - 总线号无效（>=LL_SCI_BUS_MAX或==LL_SCI_BUS_0）
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
        /* TODO: lin speed: 10ms need change priority? */
        NVIC_EnableIRQ(irq);
    }
    else
    {
        NVIC_DisableIRQ(irq);
    }

    return LL_OK;
}

/**
 * @brief   设置LIN接收延时（用于时序调整）
 * @param   bus   - SCI总线号（仅BUS_1有效）
 * @param   count - RX延时计数值（0-255）
 * @note    通过TEST模块的LIN_RX_DELAY寄存器配置接收路径延时。
 *          写入前需解锁TEST配置寄存器，写入后重新上锁。
 *          高速模式通常设为0，低速模式设为3。
 * @retval  LL_OK - 设置成功
 * @retval  LL_ERROR - 总线号无效
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
        TEST->TEST_LIN_CTRL_F.LIN_RX_DELAY = count;
        TEST_CONFIG_LOCK();
    }

    return LL_OK;
}

/**
 * @brief   使能LIN唤醒功能
 * @param   bus    - SCI总线号（仅LL_SCI_BUS_1有效）
 * @param   enable - 唤醒中断使能标志
 * @note    初始化唤醒源为LIN，唤醒时间为WAKEUP_TIME_5，滤波为WAKEUP_FILTER_3。
 *          调用ll_syscfg_isr_enable使能ASYSCFG唤醒中断。
 *          AON_IRQn优先级固定为3并使能。
 * @retval  LL_OK - 配置成功
 * @retval  LL_ERROR - 总线不是LL_SCI_BUS_1
 */
ll_status_e ll_lin_wakeup_enable(ll_sci_bus_e bus, bool enable)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    ll_wakeup_init(WAKEUP_SOUERCE_LIN, WAKEUP_TIME_5, WAKEUP_FILTER_3);

    ll_syscfg_isr_enable(ASYSCFG_INT_WAKEUP, enable);

    NVIC_SetPriority(AON_IRQn, 3);
    NVIC_EnableIRQ(AON_IRQn);

    return LL_OK;
}

/**
 * @brief   设置LIN自动寻址就绪标志
 * @param   bus    - SCI总线号（仅LL_SCI_BUS_1有效）
 * @param   enable - 寻址就绪标志值（true=已就绪，false=未就绪）
 * @note    通过AUTO_ADDR_CTRL寄存器的ADDR_ALREADY_FLAG位通知硬件
 *          自动寻址状态，用于控制AA流程的启动。
 * @retval  LL_OK - 设置成功
 * @retval  LL_ERROR - 总线不是LL_SCI_BUS_1
 */
ll_status_e ll_lin_aa_ready_set(ll_sci_bus_e bus, bool enable)
{
    if (LL_SCI_BUS_1 != bus)
    {
        return LL_ERROR;
    }

    LIN_SCI->AUTO_ADDR_CTRL_F.ADDR_ALREADY_FLAG = enable ? 1 : 0;

    return LL_OK;
}

/**
 * @brief   SCI发送数据（UART模式）
 * @param   bus    - SCI总线号
 * @param   buffer - 待发送数据缓冲区指针
 * @param   length - 发送数据长度（字节数）
 * @note    BUS_0：等待TX_BUSY清除后写入TX_DATA。
 *          BUS_1：等待TX_FIFO空后写入LIN_SCI的TX_DATA。
 *          BUS_2：等待TX_FIFO空后写入LIN_SCI1的TX_DATA。
 *          逐字节发送，每字节前轮询等待FIFO/busy就绪。
 * @retval  LL_OK - 发送成功
 * @retval  LL_ERROR - 参数无效（总线号越界、buffer为空、length为0）
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
                while (PRINT_UART->STATUS_F.TX_BUSY == 1);

                PRINT_UART->TX_DATA_F.TX_DATA = buffer[i];
                break;

            case LL_SCI_BUS_1:
                while (LIN_SCI->STATUS_F.TX_FIFO_FULL);

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
 * @brief   SCI接收数据（UART/LIN模式，读取RX FIFO）
 * @param   bus    - SCI总线号（仅BUS_1和BUS_2有效，BUS_0不支持）
 * @param   buffer - 接收数据存放缓冲区指针
 * @param   length - 期望接收的字节数
 * @note    从RX_DATA寄存器读取指定长度的数据到buffer。
 *          不等待数据到达，直接读取FIFO内容。
 *          调用前需确保RX FIFO中有足够数据（通常由中断驱动通知）。
 * @retval  LL_OK - 读取成功
 * @retval  LL_ERROR - 参数无效
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
            buffer[i] = LIN_SCI->RX_DATA_F.RX_DATA;
        }
        else
        {
            buffer[i] = LIN_SCI1->RX_DATA_F.RX_DATA;
        }
    }

    return LL_OK;
}

/**
 * @brief   LIN总线发送响应数据（Slave Response）
 * @param   bus    - SCI总线号（仅BUS_1和BUS_2有效）
 * @param   pid    - LIN协议标识符（PID，用于校验和类型判断）
 * @param   buffer - 待发送数据缓冲区指针
 * @param   length - 发送数据长度（字节数，最大4字节）
 * @note    发送前检查STOP错误标志，如有错误则清除状态并返回LL_COMM_ERROR。
 *          发送流程：禁能TX -> 清TX FIFO -> 配置校验和类型和数据长度 ->
 *          使能TX -> 写入数据（最多4字节）。
 *          当LIN_CHECKSUM_USE_SW=1时，使用软件计算校验和并追加发送。
 *          PID为0x3C或0x7D时使用经典校验和（Classic），否则使用增强校验和（Enhanced）。
 *          校验和类型在硬件模式下通过CHKSUM_TYPE位配置。
 * @retval  LL_OK - 发送成功
 * @retval  LL_ERROR - 参数无效
 * @retval  LL_COMM_ERROR - 检测到接收STOP错误
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

    //pid 中断先于 stop error中断，发送前先查询是否有stop error并做相应TX RX ABORT处理，避免发出无效数据
    if (lin_sci_reg->ISR & SCI_INT_RX_STP_ERR)
    {
        lin_sci_reg->ICR |= SCI_INT_RX_STP_ERR;
        ll_sci_state_clear(bus, (ll_sci_clear_type_e)(SCI_CLEAR_TX_ABORT | SCI_CLEAR_RX_ABORT));
        return LL_COMM_ERROR;
    }

    lin_sci_reg->CTRL_F.TX_EN = 0;

    //First: clear tx fifo
    ll_sci_state_clear(bus, SCI_CLEAR_TX_FIFO);

#if !LIN_CHECKSUM_USE_SW
    lin_sci_reg->CTRL_F.CHKSUM_TYPE = (LIN_CHECKSUM_CLASSIC == checksum_type) ? 0 : 1;
    lin_sci_reg->CTRL_F.TX_NUM = length;
#endif

    lin_sci_reg->CTRL_F.TX_EN = 1;

    length = (length > 4) ? 4 : length;

    for (uint8_t i = 0; i < length; i++)
    {
        while (lin_sci_reg->STATUS_F.TX_FIFO_FULL);

        lin_sci_reg->TX_DATA_F.TX_DATA = buffer[i];
    }

#if LIN_CHECKSUM_USE_SW

    while (lin_sci_reg->STATUS_F.TX_FIFO_FULL);

    lin_sci_reg->TX_DATA_F.TX_DATA = ll_lin_checksum_calib_func(pid, buffer, length);
#endif

    return LL_OK;
}

/**
 * @brief   LIN总线准备接收响应数据（Slave Response接收配置）
 * @param   bus    - SCI总线号（仅BUS_1和BUS_2有效）
 * @param   pid    - LIN协议标识符（当前未用于接收逻辑，保留接口一致性）
 * @param   buffer - 接收数据存放缓冲区指针
 * @param   length - 期望接收的数据长度
 * @note    配置LIN硬件进入响应接收模式：禁能TX、使能RX、清除RX FIFO。
 *          数据接收完成后由中断通知并调用ll_lin_read_byte() / ll_sci_receive()读取。
 *          本函数仅完成接收前的硬件状态配置。
 * @retval  LL_OK - 配置成功
 * @retval  LL_ERROR - 参数无效
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

    lin_sci_reg->CTRL_F.TX_EN = 0;
    lin_sci_reg->CTRL_F.RX_EN = 1;

    ll_sci_state_clear(bus, SCI_CLEAR_RX_FIFO);

    return LL_OK;
}

/**
 * @brief   计算LIN协议校验和（Classic/Enhanced）
 * @param   pid    - LIN协议标识符，用于判断校验和类型：
 *                   0x3C(Master Request)或0x7D(Slave Response)使用Classic校验和
 *                   其余PID使用Enhanced校验和
 * @param   buffer - 数据缓冲区指针
 * @param   length - 数据长度
 * @note    Classic校验和：仅对数据字节求和取反（PID不参与）。
 *          Enhanced校验和：PID与数据字节一起求和取反。
 *          算法：逐字节累加，每次溢出时减0xFF处理进位，最后按位取反。
 *          当LIN_CHECKSUM_USE_SW=1时由本函数计算校验和并手动发送。
 * @retval  计算得到的8位校验和值（~sum & 0xFF）
 */
uint8_t ll_lin_checksum_calib_func(uint8_t pid, uint8_t *buffer, uint16_t length)
{
    uint16_t check_sum;

    /* 1. PID correspond to Master request and Slave response,
     * their checksum cal is classic the non-diagnostic frame is calculated in Enhanced
     */
    check_sum = ((0x3C == pid) || (0x7D == pid))  ? 0 : pid;

    for (uint8_t i = 0; i < length; i++)
    {

        check_sum += buffer[i];

        /* 2. to deal with the carry */
        if (check_sum > 0xFF)
        {
            check_sum -= 0xFF;
        }
    }

    /* 3. to reverse */
    return (uint8_t)(~check_sum);
}

/**
 * @brief   读取LIN硬件接收到的PID（协议标识符）
 * @param   bus - SCI总线号（仅BUS_1和BUS_2有效）
 * @param   pid - 存放读取到的PID值的字节指针
 * @note    从RX_PID寄存器读取，该寄存器由硬件在接收到LIN帧头部后自动填充。
 *          PID包含6位标识符和2位奇偶校验位。
 * @retval  LL_OK - 读取成功
 * @retval  LL_ERROR - 参数无效
 */
ll_status_e ll_lin_pid_read(ll_sci_bus_e bus, uint8_t *pid)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *pid = LIN_SCI->RX_PID_F.RX_PID;
    }
    else
    {
        *pid = LIN_SCI1->RX_PID_F.RX_PID;
    }

    return LL_OK;
}

/**
 * @brief   从LIN RX FIFO读取一个字节
 * @param   bus  - SCI总线号（仅BUS_1和BUS_2有效）
 * @param   byte - 存放读取到的字节数据的指针
 * @note    从RX_DATA寄存器直接读取一个字节，不等待数据可用。
 *          调用前需确保RX FIFO非空（通常由中断通知）。
 * @retval  LL_OK - 读取成功
 * @retval  LL_ERROR - 参数无效
 */
ll_status_e ll_lin_read_byte(ll_sci_bus_e bus, uint8_t *byte)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus || NULL == byte)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *byte = LIN_SCI->RX_DATA_F.RX_DATA;
    }
    else
    {

        *byte = LIN_SCI1->RX_DATA_F.RX_DATA;
    }

    return LL_OK;
}

/**
 * @brief   读取LIN自动波特率测量结果（整数部分）
 * @param   bus  - SCI总线号（仅BUS_1和BUS_2有效）
 * @param   baud - 存放波特率整数值的指针（AUTO_BD_INTR字段）
 * @note    读取硬件自动测量的波特率整数部分，结合小数部分可计算实际波特率。
 *          自动波特率功能在LIN从模式下由硬件在同步场（Sync Field）期间自动测量。
 *          波特率 = FCLK / (AUTO_BD_INTR + AUTO_BD_FRAC/16)
 * @retval  LL_OK - 读取成功
 * @retval  LL_ERROR - 参数无效
 */
ll_status_e ll_lin_auto_baudrate_read(ll_sci_bus_e bus, uint32_t *baud)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *baud = LIN_SCI->AUTO_BAUD_VAL_F.AUTO_BD_INTR;
    }
    else
    {
        *baud = LIN_SCI1->AUTO_BAUD_VAL_F.AUTO_BD_INTR;
    }

    return LL_OK;
}

/********************************************************
** \brief   ll_lin_baudrate_read
**
** \param   ll_sci_bus_e    bus
** \param   uint32_t*       baud
**
** \retval  ll_status_e
*********************************************************/
ll_status_e ll_lin_baudrate_read(ll_sci_bus_e bus, uint32_t *baud)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        *baud = LIN_SCI->BAUD_CFG_F.BD_INTR;
    }
    else
    {
        *baud = LIN_SCI1->BAUD_CFG_F.BD_INTR;
    }

    return LL_OK;
}

/********************************************************
** \brief   ll_lin_ctrl_glben
**
** \param   ll_sci_bus_e    bus
** \param   bool            sw
**
** \retval  ll_status_e
*********************************************************/
ll_status_e ll_lin_ctrl_glben(ll_sci_bus_e bus, bool sw)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        LIN_SCI->CTRL_F.GLB_EN = sw;
    }
    else
    {
        LIN_SCI1->CTRL_F.GLB_EN = sw;
    }

    return LL_OK;
}

/********************************************************
** \brief   ll_lin_ctrl_rx_abort
**
** \param   ll_sci_bus_e    bus
** \param   bool            sw
**
** \retval  ll_status_e
*********************************************************/
ll_status_e ll_lin_ctrl_rx_abort(ll_sci_bus_e bus, bool sw)
{
    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    if (LL_SCI_BUS_1 == bus)
    {
        LIN_SCI->CTRL_F.RX_ABORT = sw;
    }
    else
    {
        LIN_SCI1->CTRL_F.RX_ABORT = sw;
    }

    return LL_OK;
}

/********************************************************
** \brief   ll_lin_ctrl_brk_tx
**
** \param   ll_sci_bus_e    bus
** \param   uint8_t         brk_num
**
** \retval  ll_status_e
*********************************************************/
ll_status_e ll_lin_ctrl_brk_tx(ll_sci_bus_e bus, uint8_t brk_num)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

    lin_sci_reg->BRK_SYNC_CFG_F.BRK_NUM = brk_num;
    lin_sci_reg->CTRL_F.BRK_TX_TRIG = true;

    return LL_OK;
}

/********************************************************
** \brief   ll_lin_tx_header
**
** \param   ll_sci_bus_e    bus
** \param   uint8_t         pid
**
** \retval  ll_status_e
*********************************************************/
ll_status_e ll_lin_tx_header(ll_sci_bus_e bus, uint8_t pid)
{
    LIN_SCI_REG_TypeDef *lin_sci_reg = NULL;

    if (bus >= LL_SCI_BUS_MAX  || LL_SCI_BUS_0 == bus)
    {
        return LL_ERROR;
    }

    lin_sci_reg = (LIN_SCI_REG_TypeDef *)((LL_SCI_BUS_1 == bus) ?
                                          LIN_SCI_BASE_ADDR : LIN_SCI1_BASE_ADDR);

    lin_sci_reg->CTRL_F.RX_EN = 0;
    lin_sci_reg->TX_CFG_F.TX_PID = pid;
    lin_sci_reg->CTRL_F.BRK_TX_TRIG = true;

    return LL_OK;
}

/********************************************************
** \brief   SCI_IRQHandler
**
** \param   None
**
** \retval  None
*********************************************************/
void SCI_IRQHandler(void)
{
    uint32_t isr = LIN_SCI->ISR & LIN_ISR_FLAG;

    if (isr)
    {
        if (isr & SCI_INT_TX_DONE)
        {
            LIN_SCI->CTRL_F.TX_EN = 0; /* disable tx */
            LIN_SCI->CTRL_F.RX_EN = 1; /* enable rx */
        }
        if (NULL != sci_isr_callback[0])
        {
            sci_isr_callback[0](isr);
        }

        LIN_SCI->ICR |= isr;
    }
}

/********************************************************
** \brief   UART_IRQHandler
**
** \param   None
**
** \retval  None
*********************************************************/
void UART_IRQHandler(void)
{
    uint32_t isr = LIN_SCI1->ISR & LIN_ISR_FLAG;

    if (isr)
    {
        if (NULL != sci_isr_callback[1])
        {
            sci_isr_callback[1](isr);
        }

        LIN_SCI1->ICR |= isr;
    }
}
