/**
 *****************************************************************************
 * @brief   system_tcpl03x Source file.
 *
 * @file    system_tcpl03x.c
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

#include "tcpl03x.h"
#include "system_tcpl03x.h"

uint32_t SystemCoreClock = DEFAULT_SYSTEM_CLOCK;  /* 系统核心时钟频率(Hz)，由sys_core_clock_update()更新 */

/* Keil uVision 5.29.0.0 */
#if defined (__ARMCC_VERSION)

/* Load Address of DDR_RW_DATA region */
extern uint32_t Load$$RW_IRAM1$$Base;
/* Exec Address of DDR_RW_DATA region */
extern uint32_t Image$$RW_IRAM1$$Base;
/* Length of DDR_RW_DATA region */
extern uint32_t Image$$RW_IRAM1$$Length;
extern uint32_t Image$$RW_IRAM1$$ZI$$Base;
extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;

/**
 * @brief   初始化RAM中的数据段，将已初始化的数据从ROM复制到RAM
 *
 * @param   无
 *
 * @retval  无
 *
 * @note    仅在ARMCC编译器下生效，依赖链接器符号(Loa$$/Image$$RW_IRAM1$$Base/Length)
 *          复制方向为从加载地址(Load)到执行地址(Image)，按32位字长逐字复制
 */
void data_rw_init(void)
{
    uint32_t *src;
    uint32_t *dst;
    uint32_t length;
    dst    = & (Image$$RW_IRAM1$$Base);
    src    = & (Load$$RW_IRAM1$$Base);
    length = (unsigned int) & (Image$$RW_IRAM1$$Length);
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
 * @brief   获取系统HCLK(AHB)时钟频率
 *
 * @param   无
 *
 * @retval  uint32_t  系统HCLK时钟频率值(Hz)
 *
 * @note    根据HCLK_SEL选择快速时钟(CPU_INT_FAST_CLK_HZ)或慢速时钟(CPU_INT_SLOW_CLK_HZ)，
 *          再右移HCLK_DIV位完成分频
 */
uint32_t sys_hclk_freq_get(void)
{
    uint8_t sel = 0;
    uint8_t div = 0;
    uint32_t clk;

    sel = CRG->HCLK_CTRL_F.HCLK_SEL;
    div = CRG->HCLK_CTRL_F.HCLK_DIV;

    clk = (sel == 0) ? CPU_INT_FAST_CLK_HZ : CPU_INT_SLOW_CLK_HZ;

    return clk >> div;
}

/**
 * @brief   获取系统PCLK(APB)时钟频率
 *
 * @param   无
 *
 * @retval  uint32_t  系统PCLK时钟频率值(Hz)
 *
 * @note    PCLK = HCLK >> PCLK_DIV，基于当前HCLK频率右移APB分频值
 */
uint32_t sys_pclk_freq_get(void)
{
    uint8_t div = 0;
    uint32_t clk;

    div = CRG->PCLK_CTRL_F.PCLK_DIV;

    clk = sys_hclk_freq_get();

    return clk >> div;
}

/**
 * @brief   更新系统核心时钟变量SystemCoreClock并重配SysTick定时器
 *
 * @param   无
 *
 * @retval  无
 *
 * @note    调用sys_hclk_freq_get()获取当前HCLK频率赋值给SystemCoreClock，
 *          然后配置SysTick为1ms周期性中断(SystemCoreClock / 1000)
 */
void sys_core_clock_update(void)
{
    /*Later update SystemCoreClock according to clock tree*/
    SystemCoreClock = sys_hclk_freq_get();
    SysTick_Config(SystemCoreClock / 1000);
}

/**
 * @brief   加载指定索引的模拟校准值到对应Trim寄存器
 *
 * @param[in] index 校准值索引
 *                   - 0:  LRC32K校准(lrc32k_trim)
 *                   - 1:  HRC48M校准(hrc48m_trim)
 *                   - 2:  TPREF校准(tpref_trim)
 *                   - 3:  BIAS校准(bias_trim)
 *                   - 4:  LDO15校准(ldo15_trim)
 *                   - 5:  LDO33校准(ldo33_trim)
 *                   - 6:  LED_LDO5校准(led_ldo5_trim)
 *                   - 7:  LED校准(led_trim)
 *                   - 8:  LED_IB_DIAG校准(led_ib_diag_trim)
 *                   - 9:  OTP校准(otp_trim)
 *                   - 10: LIN校准(lin_trim)
 *
 * @retval  无
 *
 * @note    操作前需解锁TEST模块(TEST_LOCK=0x76543210)，校准值从OTP地址
 *          0x00800008~0x00800030中读取，写入对应Trim寄存器
 */
static void trimvalue_load(uint8_t index)
{
    /* 解锁TEST模块，允许写入Trim寄存器 */
    TEST->TEST_LOCK = 0x76543210;

    switch (index)
    {
        /* lrc32k_trim */
        case 0:
            TEST->TEST_LRC32K_TRIM = REG_READ32(0x00800008);
            break;

        /* hrc48m_trim */
        case 1:
            TEST->TEST_HRC48M_TRIM = REG_READ32(0x0080000C);
            break;

        /* tpref_trim */
        case 2:
            TEST->TEST_TPREF_TRIM = REG_READ32(0x00800010);
            break;

        /* bias_trim */
        case 3:
            TEST->TEST_BIAS_TRIM = REG_READ32(0x00800014);
            break;

        /* ldo15_trim */
        case 4:
            TEST->TEST_LDO15_TRIM = REG_READ32(0x00800018);
            break;

        /* ldo33_trim */
        case 5:
            TEST->TEST_LDO33_TRIM = REG_READ32(0x0080001C);
            break;

        /* led_ldo5_trim */
        case 6:
            TEST->TEST_LED_LDO5_TRIM = REG_READ32(0x00800020);
            break;

        /* led_trim */
        case 7:
            TEST->TEST_LED_TRIM = REG_READ32(0x00800024);
            break;

        /* led_ib_diag_trim */
        case 8:
            TEST->TEST_LED_IB_DIAG_TRIM = REG_READ32(0x00800028);
            break;

        /* otp_trim */
        case 9:
            TEST->TEST_OTP_TRIM = REG_READ32(0x0080002C);
            break;

        /* lin_trim */
        case 10:
            TEST->TEST_LIN_TRIM = REG_READ32(0x00800030);
            break;

        default:
            break;
    }

    //    TEST->TEST_LOCK = 0xFEDCBA98;
}

/**
 * @brief   批量加载所有模拟系统的校准值(Trim Value)
 *
 * @param   无
 *
 * @retval  无
 *
 * @note    读取OTP校准状态寄存器(0x00800004)，遍历索引0~31，
 *          跳过已加载的校准项(状态位为1)，未加载的项调用trimvalue_load()加载
 */
void sys_trimvalue_load(void)
{
    uint8_t i;
    /* 读取OTP校准状态寄存器，每位表示对应索引的校准项是否已加载 */
    uint32_t trim_status = REG_READ32(0x00800004);

    /* 遍历所有校准项(索引0~31) */
    for (i = 0; i < 32; i++)
    {
        /* 跳过已加载的校准项(状态位为1) */
        if ((trim_status >> i) & 0x01)
        {
            continue;
        }

        /* 加载未完成的校准项 */
        trimvalue_load(i);
    }
}

/**
 * @brief   配置系统HCLK(AHB总线)时钟源和分频值
 *
 * @param[in] hclk_src AHB时钟源选择
 *                     - HCLK_SRC_RC48M(0): 使用RC48M高速时钟
 *                     - HCLK_SRC_RC32K(1): 使用RC32K低速时钟
 * @param[in] div      AHB时钟分频值(HCLK_DIV)，0为不分频
 *
 * @retval  无
 *
 * @note    操作CRG寄存器前需解锁(CRG_LOCK=0x5A5A5A5A)，配置后等待约50个
 *          时钟周期使时钟稳定，完成后重新锁定(CRG_LOCK=0x12345678)
 */
void sys_hclk_config(uint8_t hclk_src, uint8_t div)
{
    uint8_t ready_cnt = 50;

    /* 解锁CRG寄存器，允许配置时钟控制 */
    CRG->CRG_LOCK = 0X5A5A5A5A;

    switch (hclk_src)
    {
        case HCLK_SRC_RC48M:
            /* 选择RC48M高速时钟作为HCLK源 */
            CRG->HCLK_CTRL_F.HCLK_SEL = 0;
            break;

        case HCLK_SRC_RC32K:
            /* 选择RC32K低速时钟作为HCLK源 */
            CRG->HCLK_CTRL_F.HCLK_SEL = 1;
            break;

        default:
            break;
    }

    /* 配置HCLK分频值并加载 */
    CRG->HCLK_CTRL_F.HCLK_DIV = div;
    CRG->HCLK_CTRL_F.HCLK_DIV_LOAD = 1;

    /* 等待时钟切换稳定(约50个时钟周期) */
    while (ready_cnt--);

    /* 重新锁定CRG寄存器 */
    CRG->CRG_LOCK = 0X12345678;
}

/**
 * @brief   配置系统PCLK(APB总线)分频值并关闭未使用外设的PCLK时钟
 *
 * @param[in] div APB时钟分频值(PCLK_DIV)
 *
 * @retval  无
 *
 * @note    操作CRG寄存器前需解锁(CRG_LOCK=0x5A5A5A5A)，
 *          设置分频后依次关闭TIM_LITE/PRINT_UART/IWDG/PWM/ADC/LIN_SCI
 *          等外设的PCLK时钟以降低系统功耗，完成后重新锁定
 */
void sys_pclk_config(uint8_t div)
{
    /* 解锁CRG寄存器，允许配置时钟控制 */
    CRG->CRG_LOCK = 0X5A5A5A5A;

    /* 设置APB时钟(PCLK)分频值并加载 */
    CRG->PCLK_CTRL_F.PCLK_DIV = div;
    CRG->PCLK_CTRL_F.PCLK_DIV_LOAD = 1;

    /* 关闭未使用外设的PCLK时钟门控，降低系统动态功耗 */
    CRG->TIM_LITE_CLKRST_CTRL_F.PCLK_EN_TIM_LITE = 0;       /* 关闭TIM_LITE时钟 */
    CRG->PRINT_UART_CLKRST_CTRL_F.PCLK_EN_PRINT_UART = 0;   /* 关闭PRINT_UART时钟 */
    CRG->IWDG_CLKRST_CTRL_F.PCLK_EN_IWDG = 0;               /* 关闭IWDG时钟 */
    CRG->PWM_CLKRST_CTRL_F.PCLK_EN_PWM = 0;                 /* 关闭PWM时钟 */
    CRG->ADC_CLKRST_CTRL_F.PCLK_EN_ADC = 0;                 /* 关闭ADC时钟 */
    CRG->LIN_SCI_CLKRST_CTRL_F.PCLK_EN_LIN_SCI = 0;         /* 关闭LIN_SCI时钟 */
    CRG->RESERVED_T_CLKRST_CTRL_F.PCLK_EN_RESERVED_T = 0;   /* 关闭RESERVED_T时钟 */
    CRG->RESERVED_S_CLKRST_CTRL_F.PCLK_EN_RESERVED_S = 0;   /* 关闭RESERVED_S时钟 */
    CRG->LIN_SCI1_CLKRST_CTRL_F.PCLK_EN_LIN_SCI1 = 0;       /* 关闭LIN_SCI1时钟 */

    /* 重新锁定CRG寄存器 */
    CRG->CRG_LOCK = 0X12345678;
}

/**
 * @brief   使能LDO15和LDO33的虚负载(Dummy Load)配置
 *
 * @param   无
 *
 * @retval  无
 *
 * @note    关闭LDO15和LDO33的虚负载开关使能(DL_SW_ENB=0)和
 *          基极电流选择(DL_IBASE_SEL=0)，用于系统稳定性调节
 */
static void sys_dummy_load_enable(void)
{
    /* LDO15虚负载：关闭开关使能 */
    ASYSCFG->LDO15_CTRL_F.LDO15_DL_SW_ENB = 0;
    /* LDO15虚负载：基极电流选择置0 */
    ASYSCFG->LDO15_CTRL_F.LDO15_DL_IBASE_SEL = 0;
    /* LDO33虚负载：关闭开关使能 */
    ASYSCFG->LDO33_CTRL_F.LDO33_DL_SW_ENB = 0;
    /* LDO33虚负载：基极电流选择置0 */
    ASYSCFG->LDO33_CTRL_F.LDO33_DL_IBASE_SEL = 0;
}

/**
 * @brief   关闭独立看门狗(IWDG)
 *
 * @param   无
 *
 * @retval  无
 *
 * @note    仅在宏DISABLE_WDOG使能时执行，操作前需解锁IWDG(LOCK=0xAAAA5555)，
 *          关闭使能位EN和复位使能位RST_EN，完成后重新锁定(LOCK=0x12345678)
 */
static void wdg_disable(void)
{
#if (DISABLE_WDOG)
    /* 解锁IWDG寄存器 */
    IWDG->LOCK = 0xAAAA5555;
    /* 关闭看门狗计数使能 */
    IWDG->CTRL_F.EN = false;
    /* 关闭看门狗复位使能 */
    IWDG->CTRL_F.RST_EN = false;
    /* 重新锁定IWDG寄存器 */
    IWDG->LOCK = 0x12345678;
#endif
}

/**
 * @brief   系统初始化主入口，按硬件依赖顺序依次初始化各模块
 *
 * @param   无
 *
 * @retval  无
 *
 * @note    初始化顺序有严格依赖关系：
 *          1. data_rw_init()        - 数据段复制(RAM初始化)
 *          2. sys_hclk_config()     - HCLK时钟源和分频配置
 *          3. sys_core_clock_update() - 系统核心时钟更新及SysTick配置
 *          4. wdg_disable()         - 关闭独立看门狗
 *          5. sys_pclk_config()     - PCLK分频及外设时钟门控
 *          6. sys_dummy_load_enable() - LDO虚负载使能
 *          7. sys_trimvalue_load()  - 模拟校准值批量加载
 */
void SystemInit(void)
{
#if defined ( __ARMCC_VERSION )
    /* ARMCC编译器：复制数据段(RW)和代码段从ROM到RAM */
    data_rw_init();
#elif defined ( __ICCARM__ )
    /* IAR编译器：待实现 */
#elif defined (__GNUC__)
    /* GCC编译器：待实现 */
#endif

    /* 第1步：配置HCLK时钟源为RC48M，不分频 */
    sys_hclk_config(HCLK_SRC_RC48M, 0);

    /* 第2步：更新SystemCoreClock全局变量并配置SysTick为1ms中断 */
    sys_core_clock_update();

    /* 第3步：关闭独立看门狗(IWDG)，防止系统启动过程中误复位 */
    wdg_disable();

    /* 第4步：配置PCLK分频(不分频)并关闭未用外设时钟 */
    sys_pclk_config(0);

    /* 第5步：使能LDO虚负载，保证电源稳定性 */
    sys_dummy_load_enable();

    /* 第6步：从OTP加载模拟校准值(Trim) */
    sys_trimvalue_load();
}

/**
 * @brief   配置系统中断向量表重映射
 *
 * @param[in] vetor_offset 向量表偏移地址，需8位对齐(右移8位写入寄存器)
 * @param[in] enable       重映射使能(true: 使能重映射, false: 禁止重映射)
 *
 * @retval  无
 *
 * @note    操作前需解锁SYSCFG(SYSCFG_LOCK=0xaa55aa55)，
 *          向量表基地址写REMAP寄存器的CM0_VECT_BASE_ADDR域，
 *          重映射使能写CM0_REMAP_EN域
 */
void system_remap_config(uint32_t vetor_offset, bool enable)
{
    /* 解锁SYSCFG寄存器 */
    SYSCFG->SYSCFG_LOCK = 0xaa55aa55;

    /* 设置向量表基地址(偏移量右移8位，与寄存器位宽对齐) */
    SYSCFG->REMAP_F.CM0_VECT_BASE_ADDR = vetor_offset >> 8;
    /* 使能/禁止中断向量表重映射 */
    SYSCFG->REMAP_F.CM0_REMAP_EN = (enable) ? 1 : 0;
}
