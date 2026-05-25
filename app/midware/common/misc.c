#include "tcae10_ll_adc.h"
#include "tcae10_ll_wdg.h"
#include "misc.h"
#include "tc_log.h"

static const char *TAG = "MISC";

int GetVbatMv(adc_vref_e vref, int sampCount)   //sampCount：采样计数器，多次采样求平均，获取vbat电压，单位mv
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

int GetTempCode(uint8_t index, adc_vref_e vref, int sampCount)  //sampCount：采样计数器，多次采样求平均，获取vbat电压，单位mv
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

int GetTemp(uint8_t index)  //index：传感器编号0-1
{
    int code = GetTempCode(index, ADC_VREF_2500, 4);
    return ll_adc_temp_calculate((temp_channel_e)index, code);
}

void LinAsGpioInit(bool pullup_enable)      //lin高压口配置成gpio，pullup_enable内部30k上拉电阻使能
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

void LinAsGpioOutput(bool state)    //设置lin口输出
{
    LIN_SCI->TX_CFG_F.SW_TXD_VAL = (state ? 1 : 0);
}

void WdgInit(void)      //看门狗初始化
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

void PrintRstCause(void)    //打印复位原因
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

//RTC触发配置，freq:中断频率,sw:1开0关
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

void AdcExtVrefInit(void)   //GPIO6作为外部adc vref
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

void VsLvdInit(vs_lvd_threshold_e threshold, vs_lvd_interrupt_e int_type)
{
    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->ICR |= ASYSCFG_INT_VSLVD;  //清lvd中断标志

    ASYSCFG->VS_LVD_CTRL_F.VS_LVD_SEL = threshold;
    ASYSCFG->VS_LVD_CTRL_F.VS_LVD_EN = 1;
    if (int_type != LVD_INT_NONE)    //开中断
    {
        ASYSCFG->IMR_F.VS_LVD_INT_MSK = 0;
        ASYSCFG->PMU_IRQ_CTRL_F.VS_LVD_IRQ_MODE = (0x1 << int_type);
        EnableNvic(AON_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, true);
    }

    ASYSCFG_CONFIG_LOCK();
}
