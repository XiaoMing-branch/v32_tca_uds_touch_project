#include "tcae10_ll_adc.h"
#include "tcae10_ll_wdg.h"
#include "misc.h"
#include "tc_log.h"

static const char *TAG = "MISC";

/**
 * @brief  获取VBAT电压值（单位mV）
 * @param  vref - ADC参考电压选择（ADC_VREF_2500/2000/1500）
 * @param  sampCount - 采样次数，多次采样取平均以提高精度
 * @retval VBAT电压值（mV），参数无效时返回0
 * @note   配置ADC为单次采样模式，选择VBAT通道，采样后关闭ADC
 *         计算公式：sumcode * 20 * vrefMv / sampCount / 8192
 */
int GetVbatMv(adc_vref_e vref, int sampCount)
{
    if (sampCount <= 0 || vref >= ADC_VREF_MAX)
    {
        return 0;
    }

    ADC->CTRL0_F.VCR_EN = 0;
    ADC->CTRL0_F.VCR_SEL = ADC_VCR_SEL_236_7;
    ADC->CTRL0_F.TRIG_EN = 0;
    ADC->CTRL0_F.FIFO_THRHLD = 0;
    ADC->CTRL0_F.IRQ_MODE = 2;
    ADC->CTRL0_F.VCM_SEL = ADC_VCM_SEL_NULL;
    ADC->CTRL0_F.VREFBUF_EN = 1;
    ADC->CTRL0_F.I_SEL = 1;
    ADC->CTRL0_F.AUTO_ADC_EN = 0;
    ADC->CTRL0_F.SW_ADC_EN = 1;

    ADC->CTRL1_F.INT1_SNS_EN = 0;
    ADC->CTRL1_F.INT0_SNS_EN = 0;
    ADC->CTRL1_F.PGA_EN = 0;
    ADC->CTRL1_F.PGA_BP = 1;
    ADC->CTRL1_F.PGA_GAIN_SEL = ADC_GAIN_X1;
    ADC->CTRL1_F.BUFP_EN = 0;
    ADC->CTRL1_F.BUFN_EN = 0;
    ADC->CTRL1_F.BUFP_BP = 1;
    ADC->CTRL1_F.BUFN_BP = 1;
    ADC->CTRL1_F.DIV_EN = 1;
    ADC->CTRL1_F.SW_CONT_MODE = 0;
    ADC->CTRL1_F.CHNL_SAMP_NUM = 0;
    ADC->CTRL1_F.SCAN_CHNL_NUM = 0;
    ADC->CTRL1_F.VREF_SEL = vref;
    ADC->CTRL1_F.IN_SEL = ADC_CHANNEL_VBAT;

    ADC->CTRL2_F.SAMP_CYCLE = 0xF;
    ADC->CTRL2_F.INIT_CYCLE = 0x1F;

    for (int i = 0; i <= 16 && ADC->FIFO_STATE_F.ENTRY_VALID > 0; ++i)         //清buffer
    {
        ADC->FIFO_DATA;
    }

    static const int vrefMvTable[] =
    {
        2500, 2000, 1500
    };
    int sumcode = 0;
    for (int i = 0; i < sampCount; ++i)
    {
        ADC->CTRL0_F.SW_START = 1;
        for (int j = 0; j < 0xFFFF && !ADC->FIFO_STATE_F.ENTRY_VALID; ++j)
        {
            continue;
        }
        sumcode += ll_adc_getcode();
    }

    ADC->CTRL0_F.SW_ADC_EN = 0; //关闭ADC

    return sumcode * 20 * vrefMvTable[vref] / sampCount / 8192;
}

/**
 * @brief  获取温度传感器ADC原始码值
 * @param  index - 温度传感器编号（0：内部传感器INT0，1：内部传感器INT1）
 * @param  vref - ADC参考电压选择
 * @param  sampCount - 采样次数，多次采样取平均
 * @retval ADC原始码平均值
 * @note   使能PGA（增益x16），选择对应温度传感器通道，采样后关闭ADC
 */
int GetTempCode(uint8_t index, adc_vref_e vref, int sampCount)
{
    if (sampCount <= 0 || vref >= ADC_VREF_MAX)
    {
        return 0;
    }

    ADC->CTRL0_F.VCR_EN = FALSE;
    ADC->CTRL0_F.VCR_SEL = ADC_VCR_SEL_236_7;
    ADC->CTRL0_F.TRIG_EN = FALSE;
    ADC->CTRL0_F.FIFO_THRHLD = 0;
    ADC->CTRL0_F.IRQ_MODE = 2;
    ADC->CTRL0_F.VCM_SEL = ADC_VCM_SEL_205;
    ADC->CTRL0_F.VREFBUF_EN = ENABLE;
    ADC->CTRL0_F.I_SEL = ADC_IBIAS_0p5x;
    ADC->CTRL0_F.AUTO_ADC_EN = 0;
    ADC->CTRL0_F.SW_ADC_EN = ENABLE;

    if (index == 0)
    {
        ADC->CTRL1_F.INT0_SNS_EN = ENABLE;
        ADC->CTRL1_F.INT1_SNS_EN = DISABLE;
        ADC->CTRL1_F.IN_SEL = ADC_CHANNEL_TEMP;
    }
    else
    {
        ADC->CTRL1_F.INT0_SNS_EN = DISABLE;
        ADC->CTRL1_F.INT1_SNS_EN = ENABLE;
        ADC->CTRL1_F.IN_SEL = ADC_CHANNEL_TEMP1;
    }
    ADC->CTRL1_F.PGA_EN = ENABLE;
    ADC->CTRL1_F.PGA_BP = 0;
    ADC->CTRL1_F.PGA_GAIN_SEL = ADC_GAIN_X16;
    ADC->CTRL1_F.BUFP_EN = 0;
    ADC->CTRL1_F.BUFN_EN = 0;
    ADC->CTRL1_F.BUFP_BP = 1;
    ADC->CTRL1_F.BUFN_BP = 1;
    ADC->CTRL1_F.DIV_EN = 1;
    ADC->CTRL1_F.SW_CONT_MODE = 0;
    ADC->CTRL1_F.CHNL_SAMP_NUM = 0;
    ADC->CTRL1_F.SCAN_CHNL_NUM = 0;
    ADC->CTRL1_F.VREF_SEL = vref;

    ADC->CTRL2_F.SAMP_CYCLE = 0xF;
    ADC->CTRL2_F.INIT_CYCLE = 0x1F;

    for (int i = 0; i <= 16 && ADC->FIFO_STATE_F.ENTRY_VALID > 0; ++i)         //清buffer
    {
        ADC->FIFO_DATA;
    }

    int sumcode = 0;
    for (int i = 0; i < sampCount; ++i)
    {
        ADC->CTRL0_F.SW_START = TRUE;
        for (int j = 0; j < 0xFFFF && !ADC->FIFO_STATE_F.ENTRY_VALID; ++j)
        {
            continue;
        }
        sumcode += ll_adc_getcode();
    }

    ADC->CTRL0_F.SW_ADC_EN = 0; //关闭ADC

    return sumcode / sampCount;
}

/**
 * @brief  获取温度传感器温度值
 * @param  index - 传感器编号（0：内部传感器INT0，1：内部传感器INT1）
 * @retval 温度值（由ll_adc_temp_calculate转换，单位℃）
 * @note   固定使用ADC_VREF_2500参考电压，采样4次取平均
 */
int GetTemp(uint8_t index)
{
    int code = GetTempCode(index, ADC_VREF_2500, 4);
    return ll_adc_temp_calculate((temp_channel_e)index, code);
}

/**
 * @brief  将LIN高压口配置为GPIO模式
 * @param  pullup_enable - 内部30k上拉电阻使能（true=开启，false=关闭）
 * @note   配置流程：使能LIN_SCI时钟和复位->设置为Normal模式->
 *         开启软件控制->使能发送掩码->配置上拉
 */
void LinAsGpioInit(bool pullup_enable)
{
    CRG_CONFIG_UNLOCK();

    CRG->LIN_SCI_CLKRST_CTRL_F.PCLK_EN_LIN_SCI = 1;
    CRG->LIN_SCI_CLKRST_CTRL_F.FCLK_EN_LIN_SCI = 1;

    CRG->LIN_SCI_CLKRST_CTRL_F.RST_LIN_SCI = 1;
    __NOP();
    __NOP();
    CRG->LIN_SCI_CLKRST_CTRL_F.RST_LIN_SCI = 0;
    __NOP();
    __NOP();

    CRG->LIN_SCI_CLKRST_CTRL = 0x6;
    CRG_CONFIG_LOCK();

    LIN_SCI->ANALOG_CTRL_F.SW_LIN_NORMAL_MODE = 1;//normal mode
    LIN_SCI->AUTO_ADDR_CTRL_F.SW_CTRL_EN = 1;//SW_CTRL_EN
    LIN_SCI->CTRL_F.GLB_EN = 1;
    LIN_SCI->CTRL_F.TX_MSK = 1;
    LIN_SCI->AUTO_ADDR_SW_CTRL_F.SW_LIN_PU_RES_EN = (pullup_enable ? 1 : 0);
    LIN_SCI->TX_CFG_F.SW_TXD_VAL = 0;
}

/**
 * @brief  设置LIN口GPIO输出电平
 * @param  state - 输出状态（true=高电平，false=低电平）
 * @note   通过SW_TXD_VAL控制输出电平。输出0时电压到不了0V：
 *         上拉10k时约0.7V，上拉30k时约0.6V
 */
void LinAsGpioOutput(bool state)
{
    LIN_SCI->TX_CFG_F.SW_TXD_VAL = (state ? 1 : 0);
}

/**
 * @brief  独立看门狗（IWDG）初始化
 * @note   配置流程：使能IWDG时钟->解锁->设置调试停止使能->
 *         复位使能->最大计数值0x3FFFF->比较值1->开启中断->使能
 */
void WdgInit(void)
{
    CRG_CONFIG_UNLOCK();
    CRG->IWDG_CLKRST_CTRL_F.PCLK_EN_IWDG = 1; /* enable wdg pclk*/
    CRG_CONFIG_LOCK();

    IWDG->LOCK = 0XAAAA5555;

    IWDG->CTRL_F.DBG_STOP_EN = 1;
    IWDG->CTRL_F.RST_EN = 1;
    IWDG->CNT_MAX_F.CNT_MAX = 0x3FFFF;
    IWDG->CCR_F.CCR = 1;
    IWDG->IRQ_F.IMR = 1;
    IWDG->CTRL_F.EN = 1;

    IWDG->LOCK = 0X12345678;
}

/**
 * @brief  打印MCU复位原因
 * @note   从ASYSCFG寄存器读取复位原因标志，支持检测：
 *         CM0复位/lockup、OTP复位、IWDG复位、软件POR复位、
 *         IO4引脚复位、VS_ALT复位等
 */
void PrintRstCause(void)
{
    uint32_t rst_cause = 0;

    ASYSCFG_CONFIG_UNLOCK();

    rst_cause = ASYSCFG_RST_CAUSE_GET();

    if (rst_cause & ASYSCFG_RST_CAUSE_CM0_RST)
    {
        TC_LOGI(TAG, "reset:cm0 reset or lockup");
    }
    if (rst_cause & ASYSCFG_RST_CAUSE_OTP)
    {
        TC_LOGI(TAG, " reset:otp");
    }
    if (rst_cause & ASYSCFG_RST_CAUSE_IWDG)
    {
        TC_LOGI(TAG, "reset:iwdg");
    }
    if (rst_cause & ASYSCFG_RST_CAUSE_SW_POR_REQ)
    {
        TC_LOGI(TAG, " reset:sw por req");
    }
    if (rst_cause & ASYSCFG_RST_CAUSE_IO4_PAD_RST)
    {
        TC_LOGI(TAG, "reset:io4 pad");
    }
    if (rst_cause & ASYSCFG_RST_CAUSE_VS_ALT)
    {
        TC_LOGI(TAG, " reset:vs alt");
    }
    if (rst_cause == 0)
    {
        TC_LOGI(TAG, "reset:unknon");
    }

    ASYSCFG_RST_CAUSE_CLEAR();

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief  RTC定时器触发配置（使用TIM_LITE）
 * @param  freq - 中断频率（Hz），用于计算INIT_VAL = 32768 / freq
 * @param  sw - 开关控制（1：开启并使能NVIC中断，0：关闭并禁用中断）
 * @note   时钟源为32KHz，先反初始化TIM_LITE再重新配置
 */
void RtcTrigConfig(uint8_t freq, uint8_t sw)
{
    ll_timer_deinit();

    CRG_CONFIG_UNLOCK();
    CRG->TIM_LITE_CLKRST_CTRL_F.PCLK_EN_TIM_LITE = 1;
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_SEL_TIM_LITE = FCLK_SRC_32K;
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_DIV_TIM_LITE = 0;
    CRG->TIM_LITE_CLKRST_CTRL_F.FCLK_EN_TIM_LITE = 0x1;
    CRG_CONFIG_LOCK();

    TIM_LITE->CTRL_F.TRIG_EN = 0;
    TIM_LITE->CTRL_F.PAUSE = 0;
    TIM_LITE->CTRL_F.STP = 0;
    TIM_LITE->CTRL_F.LOOP_DIS = 0;

    TIM_LITE->INIT_VAL = (32768 / freq);

    if (sw)
    {
        TIM_LITE->ICR = 0xFFFFFFFF;
        TIM_LITE->IMR_F.CNT_UDF_INT_MSK = 0;    //TIMERLITE_INTERRUPT_ENABLE();
        EnableNvic(TIMER_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, ENABLE);
        TIM_LITE->CTRL_F.EN = 1; //TIMERLITE_ENABLE();
    }
    else
    {
        TIM_LITE->IMR_F.CNT_UDF_INT_MSK = 1;    //TIMERLITE_INTERRUPT_DISABLE();
        EnableNvic(TIMER_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, DISABLE);
        TIM_LITE->CTRL_F.EN = 0;        //TIMERLITE_DISABLE();
    }
}

/**
 * @brief  将GPIO6配置为外部ADC参考电压输入（VREF）
 * @note   配置LED0引脚为TAI模式，使能测试复用开关，
 *         选择ADC_VREF_TEST_SEL = 2（外部VREF）
 */
void AdcExtVrefInit(void)
{
    /*ADC_VREF_EXTERNAL*/
    PINMUX->LED0_CFG_F.LED0_SRC_SEL = 5;//TAI
    PINMUX->LED0_CFG_F.LED0_ASW = 1;

    TEST->TEST_LOCK = 0x76543210;
    TEST->TEST_ANA_CTRL_F.TESTMUX_TAI_SEL = 1;
    TEST->TEST_ANA_CTRL_F.TESTMUX_TAI_EN = 1;
    TEST->TEST_ADC_CTRL_F.ADC_VREF_TEST_SEL = 2;
    TEST->TEST_LOCK = 0xFEDCBA98;
}

/**
 * @brief  VS电源LVD（低电压检测）初始化
 * @param  threshold - LVD电压阈值选择（vs_lvd_threshold_e）
 * @param  int_type - 中断触发方式（LVD_INT_NONE=不开启中断）
 * @note   先清除LVD中断标志，配置阈值并使能LVD，
 *         若int_type非NONE则开启AON_IRQn中断
 *         LVD中断信号：低于阈值=1，高于阈值=0，
 *         电压从高到低跌落时应选上升沿中断
 */
void VsLvdInit(vs_lvd_threshold_e threshold, vs_lvd_interrupt_e int_type)
{
    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->ICR |= ASYSCFG_INT_VSLVD;  /* 清LVD中断标志 */

    ASYSCFG->VS_LVD_CTRL_F.VS_LVD_SEL = threshold;
    ASYSCFG->VS_LVD_CTRL_F.VS_LVD_EN = 1;
    if (int_type != LVD_INT_NONE)    /* 开中断 */
    {
        ASYSCFG->IMR_F.VS_LVD_INT_MSK = 0;
        ASYSCFG->PMU_IRQ_CTRL_F.VS_LVD_IRQ_MODE = (0x1 << int_type);
        EnableNvic(AON_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, true);
    }

    ASYSCFG_CONFIG_LOCK();
}
