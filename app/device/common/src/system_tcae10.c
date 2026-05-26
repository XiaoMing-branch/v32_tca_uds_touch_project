/**
 ******************************************************************************
 * @brief       TCAE10 系统初始化及时钟配置实现
 * @note        本文件实现系统级初始化，包括 HCLK/PCLK 时钟配置、模拟修调值加载、
 *              外设时钟使能、看门狗禁用等功能。系统初始化顺序严格依赖硬件设计要求，
 *              修改时钟配置或初始化流程时需谨慎评估对各外设的影响。
 *
 *       Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE  DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************
 */

#include "tcae10.h"
#include "system_tcae10.h"

#if defined (__ARMCC_VERSION) /* Keil ��Vision 5.29.0.0 */

    extern uint32_t Load$$RW_IRAM1$$Base;     /* Load Address of DDR_RW_DATA region*/
    extern uint32_t Image$$RW_IRAM1$$Base;    /* Exec Address of DDR_RW_DATA region*/
    extern uint32_t Image$$RW_IRAM1$$Length;  /* Length of DDR_RW_DATA region*/
    extern uint32_t Image$$RW_IRAM1$$ZI$$Base;
    extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;
#endif

#ifndef CFG_HCLK_CLOCK          //配置hclk时钟
    #define CFG_HCLK_CLOCK DEFAULT_SYSTEM_CLOCK
#endif

uint32_t SystemCoreClock = DEFAULT_SYSTEM_CLOCK;

#if defined (__ARMCC_VERSION) /* Keil ��Vision 5.29.0.0 */
/**
 * @brief  将 RW 数据从加载地址拷贝到运行地址
 * @note   该函数仅在 ARMCC (Keil) 编译环境下生效，依赖于 scatter 文件中的
 *        Load$$RW_IRAM1$$Base / Image$$RW_IRAM1$$Base 等符号定义。
 *        若加载地址与运行地址相同则跳过拷贝。必须在任何 RW 数据访问前调用。
 */
void CopyDataRWtoImage(void)
{
    uint32_t *src;
    uint32_t *dst;
    uint32_t length;
    dst    = & (Image$$RW_IRAM1$$Base);     /**< RW 段运行基地址 */
    src    = & (Load$$RW_IRAM1$$Base);      /**< RW 段加载基地址 */
    length = (unsigned int) & (Image$$RW_IRAM1$$Length); /**< RW 段长度 */
    length /= sizeof(unsigned int);

    if (dst != src)
    {
        while (length > 0)
        {
            dst[length - 1] = src[length - 1];
            length--;
        }
    }
}

#endif

/**
 * @brief  获取系统 HCLK 时钟频率（对外接口）
 *
 * @retval uint32_t  系统 HCLK 时钟频率值，单位 Hz
 * @note   内部调用 sys_hclk_freq_get()，根据当前 HCLK 时钟源选择
 *        (RC48M/RC32K) 和分频系数计算实际频率
 */
uint32_t SystemGetHClkFreq(void)
{
    return sys_hclk_freq_get();
}

/**
 * @brief  获取系统 HCLK 频率
 *
 * @param  None
 *
 * @retval uint32_t  系统 HCLK 时钟频率值，单位 Hz
 * @note   根据 CRG->HCLK_CTRL_F 寄存器中的 HCLK_SEL 和 HCLK_DIV 字段计算：
 *         HCLK_SEL=0 时为 RC48M 时钟，HCLK_SEL=1 时为 RC32K 时钟，
 *         最终频率 = 时钟源频率 / (HCLK_DIV + 1)
 */
uint32_t sys_hclk_freq_get(void)
{
    uint8_t sel = 0;
    uint8_t div = 0;
    uint32_t clk;

    sel = CRG->HCLK_CTRL_F.HCLK_SEL;
    div = CRG->HCLK_CTRL_F.HCLK_DIV;

    clk = (sel == 0) ? CPU_INT_FAST_CLK_HZ : CPU_INT_SLOW_CLK_HZ;

    return clk / (div + 1);
}

/**
 * @brief  获取系统 PCLK（APB 总线）频率
 *
 * @param  None
 *
 * @retval uint32_t  系统 PCLK 时钟频率值，单位 Hz
 * @note   基于 sys_hclk_freq_get() 获取的 HCLK 频率，
 *         再除以 CRG->PCLK_CTRL_F.PCLK_DIV + 1 得到 PCLK 频率
 */
uint32_t sys_pclk_freq_get(void)
{
    uint8_t div = 0;
    uint32_t clk;

    div = CRG->PCLK_CTRL_F.PCLK_DIV;

    clk = sys_hclk_freq_get();

    return clk / (div + 1);
}

/**
 * @brief  更新全局系统时钟变量 SystemCoreClock
 *
 * @param  None
 *
 * @retval None
 * @note   调用 SystemGetHClkFreq() 获取当前 HCLK 频率并写入
 *         SystemCoreClock 全局变量。在时钟源或分频系数改变后应调用此函数
 *         以保持 SystemCoreClock 与实际时钟同步
 */
void SystemCoreClockUpdate(void)
{
    /*Later update SystemCoreClock according to clock tree*/
    SystemCoreClock = SystemGetHClkFreq();
}

/**
 * @brief  从 OTP 空间拷贝模拟修调值到对应的 TEST 寄存器
 *
 * @param  index  修调项索引（0~12）
 *         @arg 0      RC32K 修调
 *         @arg 1      RC48M 修调
 *         @arg 2      TPREF 修调
 *         @arg 3      BIAS 修调
 *         @arg 4      LDO15 修调
 *         @arg 5      LDO33 修调
 *         @arg 6      LED_LDO5 修调
 *         @arg 7      LED 修调
 *         @arg 8      LED_IB_DIAG 修调
 *         @arg 9      OTP 修调
 *         @arg 10     LIN 修调
 *         @arg 11     TOUCH 修调
 *         @arg 12     SARADC 修调（使用默认修调值而非 OTP 值）
 *
 * @retval None
 * @note   修调值存储于 OTP 区域（地址 0x00800008~0x00800034），
 *         通过索引选择对应的修调项。索引 12 (SARADC) 使用硬编码默认值而非 OTP 值。
 *         必须在 TEST->TEST_LOCK 解锁后调用
 */
void CopyAnalogTrimValue(uint8_t index)
{
#if 1

    switch (index)
    {
    case 0:
        /* RC32K trim */
        TEST->TEST_LRC32K_TRIM = * (volatile uint32_t *)(0x00800008);  /**< 从 OTP 加载 RC32K 修调值 */
        break;

    case 1:
        /* RC48M trim */
        TEST->TEST_HRC48M_TRIM = * (volatile uint32_t *)(0x0080000C);  /**< 从 OTP 加载 RC48M 修调值 */
        break;

    case 2:
        /* TPREF trim */
        TEST->TEST_TPREF_TRIM = * (volatile uint32_t *)(0x00800010);   /**< 从 OTP 加载 TPREF 修调值 */
        break;

    case 3:
        /* BIAS trim */
        TEST->TEST_BIAS_TRIM  = * (volatile uint32_t *)(0x00800014);   /**< 从 OTP 加载 BIAS 修调值 */
        break;

    case 4:
        /* LDO15 trim */
        TEST->TEST_LDO15_TRIM  = * (volatile uint32_t *)(0x00800018);  /**< 从 OTP 加载 LDO15 修调值 */
        break;

    case 5:
        /* LDO33 trim */
        TEST->TEST_LDO33_TRIM  = * (volatile uint32_t *)(0x0080001C);  /**< 从 OTP 加载 LDO33 修调值 */
        break;

    case 6:
        /* LED_LDO5 trim */
        TEST->TEST_LED_LDO5_TRIM  = * (volatile uint32_t *)(0x00800020);  /**< 从 OTP 加载 LED_LDO5 修调值 */
        break;

    case 7:
        /* LED trim */
        TEST->TEST_LED_TRIM  = * (volatile uint32_t *)(0x00800024);    /**< 从 OTP 加载 LED 修调值 */
        break;

    case 8:
        /* LED_IB_DIAG trim */
        TEST->TEST_LED_IB_DIAG_TRIM  = * (volatile uint32_t *)(0x00800028);  /**< 从 OTP 加载 LED_IB_DIAG 修调值 */
        break;

    case 9:
        /* OTP trim */
        TEST->TEST_OTP_TRIM  = * (volatile uint32_t *)(0x0080002C);    /**< 从 OTP 加载 OTP 修调值 */
        break;

    case 10:
        /* LIN trim */
        TEST->TEST_LIN_TRIM  = * (volatile uint32_t *)(0x00800030);    /**< 从 OTP 加载 LIN 修调值 */
        break;

    case 11:
        /* TOUCH trim */
        TEST->TEST_TOUCH_TRIM  = * (volatile uint32_t *)(0x00800034); /**< 从 OTP 加载 TOUCH 修调值 */
        break;

    case 12:
    {
        /* SARADC trim (使用默认修调值而非 OTP 值) */
//        TEST->ADC_TRIM0       = * (volatile uint32_t *)(0x00800038);
//        TEST->ADC_TRIM1       = * (volatile uint32_t *)(0x0080003C);
//        TEST->ADC_TRIM2       = * (volatile uint32_t *)(0x00800040);
//        TEST->ADC_TRIM3       = * (volatile uint32_t *)(0x00800044);
//        TEST->ADC_TRIM4       = * (volatile uint32_t *)(0x00800048);
//        TEST->ADC_TRIM5       = * (volatile uint32_t *)(0x0080004C);
//        TEST->ADC_TRIM6       = * (volatile uint32_t *)(0x00800050);
//        TEST->ADC_TRIM7       = * (volatile uint32_t *)(0x00800054);
        TEST->ADC_TRIM0     = 0x74371F1D;    /**< SARADC 默认修调值 0 */
        TEST->ADC_TRIM1     = 0xFA007C;      /**< SARADC 默认修调值 1 */
        TEST->ADC_TRIM2     = 0x2EF0177;     /**< SARADC 默认修调值 2 */
        TEST->ADC_TRIM3     = 0xB0404F8;     /**< SARADC 默认修调值 3 */
        TEST->ADC_TRIM4     = 0x16010B04;    /**< SARADC 默认修调值 4 */
        TEST->ADC_TRIM5     = 0x108020ff;    /**< SARADC 默认修调值 5 */
        TEST->ADC_TRIM6     = 0x3F402100;    /**< SARADC 默认修调值 6 */
        TEST->ADC_TRIM7     = 0x7900;        /**< SARADC 默认修调值 7 */

        break;
    }

    default:
        break;
    }

#endif
}
/**
 * @brief  系统模拟修调加载函数
 *
 * @param  None
 *
 * @retval None
 * @note   依次加载索引 0~12 的所有模拟修调值。在执行本函数前必须先解锁
 *         TEST->TEST_LOCK（已在本函数内完成），修调完成后应锁定 TEST 寄存器。
 *         修调值来源于 OTP 存储空间，对芯片的模拟性能（时钟精度、LDO 输出、
 *         ADC 精度等）至关重要
 */
void SystemAnlogTrim()
{
    uint8_t i;
    uint32_t trimStatus = (uint32_t)(*(volatile uint32_t *)(0x00800004)); /**< 读取 OTP 修调状态标志 */
    TEST->TEST_LOCK = 0x76543210;       /**< 解锁 TEST 寄存器，允许写入修调值 */

//    for ( i = 0; i <= 12 ; i++ )
//    {
//        if ( ( trimStatus  & ( 0x1 << i ) ) == 0 )
//        {
//            CopyAnalogTrimValue ( i );
//        }
//    }

    for (i = 0; i <= 12 ; i++)
    {
        CopyAnalogTrimValue(i);
    }
}

/**
 * @brief  This function initializes the HCLK
 * @param  hclk_src specifies the HCLK source
 *         This parameter can be one of the following values:
 *            @arg 0: the HCLK source is RC48 MHz
 *            @arg 1: the HCLK source is RC32 KHz
 *            seer @ref CRG_CLOCK_SOURCE_Definitions
 * @retval div specifies the HCLK source divider
 *         this parameter is 4-bit length, and
 *         can be any value between 0 and 15
 */
void System_HclkConfig(uint8_t hclk_src, uint8_t div)
{
    uint8_t ready_cnt = 50;

    CRG->CRG_LOCK = 0X5A5A5A5A;        /**< 解锁 CRG 寄存器，允许时钟配置 */

    switch (hclk_src)
    {
    case HCLK_SRC_RC48M:
        CRG->HCLK_CTRL_F.HCLK_SEL = 0;         /**< 选择 HCLK 时钟源为 RC48M */
        CRG->HCLK_CTRL_F.HCLK_DIV = div;       /**< 设置 HCLK 分频系数 */
        while (ready_cnt--);                    /**< 等待时钟稳定 */
        break;

    case HCLK_SRC_RC32K:
        CRG->HCLK_CTRL_F.HCLK_SEL = 1;         /**< 选择 HCLK 时钟源为 RC32K */
        CRG->HCLK_CTRL_F.HCLK_DIV_LOAD = 1;    /**< 加载分频配置 */
        while (ready_cnt--);                    /**< 等待时钟切换稳定 */
        break;

    default:
        break;
    }

    CRG->HCLK_CTRL_F.HCLK_DIV_LOAD = 1;         /**< 加载 HCLK 分频配置 */

    CRG->CRG_LOCK = 0X12345678;                 /**< 锁定 CRG 寄存器 */
}


/**
 * @brief  配置 PCLK（APB 总线时钟）分频系数及各外设时钟使能
 *
 * @param  div  APB 时钟分频系数（0~15），PCLK = HCLK / (div + 1)
 *
 * @retval None
 * @note   本函数同时管理各外设模块的 PCLK 时钟门控，默认仅使能 IWDG（看门狗）
 *         时钟，其余外设时钟（TIM_LITE、PRINT_UART、PWM、ADC、LIN_SCI、
 *         CAPTOUCH、SPI、LIN_SCI1）默认关闭以降低功耗。
 *         需在 CRG 解锁状态下调用
 */
static void sys_pclk_config(uint8_t div)
{
    CRG->CRG_LOCK = 0X5A5A5A5A;    /**< 解锁 CRG 寄存器，允许时钟配置 */

    /* set the divider for the Pclk */
    CRG->PCLK_CTRL_F.PCLK_DIV = div;        /**< 设置 PCLK 分频系数 */
    CRG->PCLK_CTRL_F.PCLK_DIV_LOAD = 1;     /**< 加载 PCLK 分频配置 */

    CRG->TIM_LITE_CLKRST_CTRL_F.PCLK_EN_TIM_LITE = 0;        /**< 关闭 TIM_LITE 时钟 */
    CRG->PRINT_UART_CLKRST_CTRL_F.PCLK_EN_PRINT_UART = 0;    /**< 关闭 PRINT_UART 时钟 */
    CRG->IWDG_CLKRST_CTRL_F.PCLK_EN_IWDG = 1;                /**< 开启看门狗时钟 */
    CRG->PWM_CLKRST_CTRL_F.PCLK_EN_PWM = 0;                  /**< 关闭 PWM 时钟 */
    CRG->ADC_CLKRST_CTRL_F.PCLK_EN_ADC = 0;                  /**< 关闭 ADC 时钟 */
    CRG->LIN_SCI_CLKRST_CTRL_F.PCLK_EN_LIN_SCI = 0;          /**< 关闭 LIN_SCI 时钟 */
    CRG->CAPTOUCH_CLKRST_CTRL_F.PCLK_EN_CAPTOUCH = 0;        /**< 关闭 CAPTOUCH 时钟 */
    CRG->SPI_CLKRST_CTRL_F.PCLK_EN_SPI = 0;                  /**< 关闭 SPI 时钟 */
    CRG->LIN_SCI1_CLKRST_CTRL_F.PCLK_EN_LIN_SCI1 = 0;        /**< 关闭 LIN_SCI1 时钟 */

    CRG->CRG_LOCK = 0X12345678;    /**< 锁定 CRG 寄存器 */
}

/**
 * @brief  使能 LDO15 和 LDO33 的 Dummy Load（虚拟负载）
 *
 * @param  None
 *
 * @retval None
 * @note   虚拟负载用于在系统轻载或无载时维持 LDO 输出稳定，
 *         防止 LDO 进入间歇性工作模式导致输出纹波增大。
 *         LDO15_DL_SW_ENB/LDO33_DL_SW_ENB 使能虚拟负载开关，
 *         LDO15_DL_IBASE_SEL/LDO33_DL_IBASE_SEL 选择基极电流（配置为 0）
 */
static void sys_dummy_load_enable(void)
{
    ASYSCFG->LDO15_CTRL_F.LDO15_DL_SW_ENB = 1;        /**< 使能 LDO15 虚拟负载 */
    ASYSCFG->LDO15_CTRL_F.LDO15_DL_IBASE_SEL = 0;     /**< 选择 LDO15 基极电流 */
    ASYSCFG->LDO33_CTRL_F.LDO33_DL_SW_ENB = 1;        /**< 使能 LDO33 虚拟负载 */
    ASYSCFG->LDO33_CTRL_F.LDO33_DL_IBASE_SEL = 0;     /**< 选择 LDO33 基极电流 */
}

/**
 * @brief  禁用独立看门狗（IWDG）
 *
 * @param  None
 *
 * @retval None
 * @note   先解锁 IWDG 寄存器（LOCK=0xAAAA5555），
 *         然后清除 EN（使能位）和 RST_EN（复位使能位），
 *         最后重新锁定 IWDG 寄存器（LOCK=0x12345678）
 */
static void wdg_disable(void)
{
    IWDG->LOCK = 0xAAAA5555;        /**< 解锁独立看门狗寄存器 */

    IWDG->CTRL_F.EN = false;        /**< 禁用看门狗计数 */
    IWDG->CTRL_F.RST_EN = false;    /**< 禁用看门狗复位输出 */

    IWDG->LOCK = 0x12345678;        /**< 锁定独立看门狗寄存器 */
}

/**
 * @brief  系统初始化主函数
 *
 * @param  None
 *
 * @retval None
 * @note   系统初始化严格按照以下顺序执行，依赖关系如下：
 *         1. 数据段拷贝（仅 Keil）—— 必须先于任何 RW 数据访问
 *         2. HCLK 时钟配置 —— 配置系统主时钟源和分频
 *         3. 更新系统时钟变量 —— 使 SystemCoreClock 反映实际时钟
 *         4. 模拟修调值加载 —— 必须在时钟稳定后进行
 *         5. PCLK 配置 —— 依赖于 HCLK 已配置完成
 *         6. Dummy Load 使能 —— 在 LDO 输出稳定后使能虚拟负载
 *         7. 看门狗禁用 —— 默认关闭，由应用程序按需使能
 *         8. Flash 时序配置 —— 根据系统时钟配置 Flash 读取时序
 */
void SystemInit(void)
{
#if defined   ( __ICCARM__   ) /* iar */
    /* IAR 编译器：无需额外操作 */

#elif defined ( __ARMCC_VERSION )     /* keil */
    CopyDataRWtoImage();    /**< 第一步：将 RW 数据从加载地址拷贝到运行地址 */
#elif defined (__GNUC__)        /* GNU GCC*/
    /* GCC 编译器：无需额外操作 */
#endif
    System_HclkConfig(HCLK_SRC_RC48M, DEFAULT_SYSTEM_CLOCK / CFG_HCLK_CLOCK - 1); /**< 第二步：配置 HCLK 时钟源为 RC48M */
    SystemCoreClockUpdate();  /**< 第三步：更新 SystemCoreClock 全局变量 */
    SystemAnlogTrim();        /**< 第四步：加载模拟修调值 */
    sys_pclk_config(0);       /**< 第五步：配置 PCLK 分频 (PCLK = HCLK) */

    /* enable dummy load */
    sys_dummy_load_enable();  /**< 第六步：使能 LDO 虚拟负载 */

    wdg_disable();            /**< 第七步：禁用独立看门狗 */

#ifdef CUTOFF_POWER_EN      //降低功耗
    EFLASH->WR_LOCK = 0X12345678;       /**< 锁定 Flash，禁止写入 */
#else
    EFLASH->WR_LOCK = 0XAA55AA55;       /**< 解锁 Flash 写保护 */
    EFLASH->RD_TIME_CFG_F.RC_CYC = 0x1; /**< 配置 Flash 读取时序周期 */
    EFLASH->WR_LOCK = 0X12345678;       /**< 锁定 Flash 写保护 */
#endif
}
