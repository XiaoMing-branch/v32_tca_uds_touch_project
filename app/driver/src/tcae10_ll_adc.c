/**
 *****************************************************************************
 * @brief   adc driver source file.
 *
 * @file    tcae10_ll_adc.c
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

#include "tcae10_ll_adc.h"
#include "system_tcae10.h"
#include "tcae10_ll_cortex.h"
#include "tcae10_ll_flash.h"

#define ADC_ISR_FLAG       (0x1FUL)

#define ADDR_TEMP_TRIM_PARAM            0x00800044UL
#define ADDR_VBAT_TRIM_PARAM            0x00800038UL

const uint16_t vcr_value[ADC_VCR_MAX] = {236, 314, 394, 437, 552, 631, 710, 789};
const uint16_t vref_value[ADC_VREF_MAX] = {2500, 2000, 1500};

static uint32_t adc_isr_flag = 0;
bool adc_isr_enable = false;
static ISR_FUNC_CALLBACK adc_isr_callback = NULL;

/**
 * @brief   ADC修调值结构体（16位code1 + 16位code2）
 */
typedef struct
{
    uint32_t code1      : 16;
    uint32_t code2      : 16;    /* 仅用于温度通道 */
} trim_value_t;

/**
 * @brief   温度修调值结构体（TOS和K值）
 */
typedef struct
{
    float tos_value;
    float k_value;
} temp_trim_value_t;

/**
 * @brief   VAON系数结构体（二次曲线拟合）
 */
typedef struct
{
    float a;
    float b;
    float c;
} vaon_coef_t;

/**
 * @brief   ADC通道修调结构体（增益和偏移）
 */
typedef struct
{
    uint16_t gain;
    int16_t offset;
} adc_ch_trim_t;

trim_value_t vbat_trim_value;
temp_trim_value_t vtemp_temp_value[2];
vaon_coef_t vaon_coef_value;
adc_ch_trim_t vf_ch_trim_value[3];
adc_ch_trim_t aon_ch_trim_value;

/**
 * @brief   配置ADC模块时钟
 * @param   config - 时钟配置参数（ADC固定使用RC48MHz作为FCLK）
 * @note    ADC的FCLK固定为RC48MHz，仅可配置分频系数
 * @retval  None
 */
static void ll_adc_clk_config(ll_clk_config_t *config)
{
    CRG_CONFIG_UNLOCK();

    /* ADC没有FCLK选择选项，默认且唯一的FCLK为RC48MHz */
    CRG->ADC_CLKRST_CTRL_F.PCLK_EN_ADC = true;                  /* 使能ADC PCLK */
    CRG->ADC_CLKRST_CTRL_F.FCLK_EN_ADC = true;                  /* 使能ADC FCLK */
    CRG->ADC_CLKRST_CTRL_F.FCLK_DIV_ADC = config->fclk_div;    /* 设置FCLK分频系数 */

    CRG_CONFIG_LOCK();
}

/**
 * @brief   配置ADC中断
 * @param   config   - 中断配置参数
 * @param   callback - 中断回调函数指针
 * @note    先清除所有中断标志，根据使能状态配置IMR寄存器和回调
 * @retval  None
 */
static void ll_adc_isr_config(ll_isr_config_t *config, ISR_FUNC_CALLBACK callback)
{
    ADC->IMR |= ADC_ISR_FLAG;                                     /* 先禁能所有ADC中断 */
    adc_isr_enable = false;

    if (config->isr_enable)
    {
        adc_isr_flag = config->isr & ADC_ISR_FLAG;                /* 保存要使能的中断标志 */

        adc_isr_callback = callback;
    }
    else
    {
        adc_isr_flag = 0;
        adc_isr_callback = NULL;
    }
}

/**
 * @brief   加载ADC修调值到TEST寄存器
 * @param   None
 * @note    将预定义的修调参数写入TEST模块的ADC_TRIM寄存器，
 *         用于校准ADC的模拟性能
 * @retval  None
 */
static void ll_adc_trim_value_load(void)
{
    TEST->TEST_LOCK = 0x76543210;                                  /* 解锁TEST模块 */

    TEST->ADC_TRIM0 = 0x74371F1D;  /* 配置ADC修调参数0 */
    TEST->ADC_TRIM1 = 0xFA007C;    /* 配置ADC修调参数1 */
    TEST->ADC_TRIM2 = 0x2EF0177;   /* 配置ADC修调参数2 */
    TEST->ADC_TRIM3 = 0xB0404F8;   /* 配置ADC修调参数3 */
    TEST->ADC_TRIM4 = 0x16010B04;  /* 配置ADC修调参数4 */
    TEST->ADC_TRIM5 = 0x108020ff;  /* 配置ADC修调参数5 */
    TEST->ADC_TRIM6 = 0x3F402100;  /* 配置ADC修调参数6 */
    TEST->ADC_TRIM7 = 0x7900;      /* 配置ADC修调参数7 */
}

/**
 * @brief   加载ADC校准值（VBAT/VTEMP/VAON）
 * @param   vbat_trim - VBAT修调值指针
 * @param   temp_trim - 温度修调值指针（2组）
 * @param   vaon_coef - VAON系数指针
 * @note    从Flash信息块读取修调参数，根据修调版本选择使用实际值或默认值
 * @retval  None
 */
static void ll_adc_calibration_load(trim_value_t *vbat_trim, temp_trim_value_t *temp_trim, vaon_coef_t *vaon_coef)
{
    uint32_t trim_value;
    int32_t aon_value;
    trim_value_t *value = (trim_value_t *)&trim_value;
    uint32_t trim_version = REG_TRIM_VERSION();
    /* 读取VBAT修调值 */
    trim_value =  REG_READ32(0x0080005CUL);
    vbat_trim->code1 =  value->code1;
    vbat_trim->code2 = value->code2;

    if (((trim_version & 0xFF) > 0x03) || (((trim_version & 0xFF) == 0x03) && (((trim_version >> 8) & 0x3F) >= 0x01)))
    {

        /* 读取VTEMP修调值（通道0） */
        trim_value =  REG_READ32(0x00800078UL);
        temp_trim[0].tos_value  = (value->code1 != 0xFFFF) ? value->code1 : 0x0500;
        temp_trim[0].k_value  = (value->code2 != 0xFFFF) ? value->code2 : 0x0B00;
        temp_trim[0].k_value = temp_trim[0].k_value / 16;

        /* 读取VTEMP修调值（通道1） */
        trim_value =  REG_READ32(0x0080007CUL);
        temp_trim[1].tos_value  = (value->code1 != 0xFFFF) ? value->code1 : 0x0500;
        temp_trim[1].k_value  = (value->code2 != 0xFFFF) ? value->code2 : 0x0B00;
        temp_trim[1].k_value = temp_trim[1].k_value / 16;

        /* 读取VAON二次曲线系数 */
        aon_value =  REG_READ32(0x008000B8UL);
        vaon_coef->a =  5.0 * aon_value / 68719476.736 / 4096.0 / 4096.0;
        aon_value =  REG_READ32(0x008000BCUL);
        vaon_coef->b =  5.0 * aon_value / 268435.456 / 4096.0;
        aon_value =  REG_READ32(0x008000C0UL);
        vaon_coef->c =  5.0 * aon_value / 4.096;
    }
    else
    {
        temp_trim[0].tos_value =  temp_trim[0].k_value = 1;
        temp_trim[1].tos_value =  temp_trim[1].k_value = 1;
        vaon_coef->a = vaon_coef->b = vaon_coef->c = 1;
    }
}

/**
 * @brief   加载ADC温度通道修调值
 * @param   temp_trim - 温度修调值指针（2组）
 * @note    从Flash信息块读取温度传感器的TOS和K值修调参数
 * @retval  None
 */
static void ll_adc_temp_calibration_load(temp_trim_value_t *temp_trim)
{
    uint32_t trim_value;
    trim_value_t *value = (trim_value_t *)&trim_value;
    uint32_t trim_version = REG_TRIM_VERSION();

    if (((trim_version & 0xFF) > 0x03) || (((trim_version & 0xFF) == 0x03) && (((trim_version >> 8) & 0x3F) >= 0x01)))
    {

        /* 读取VTEMP修调值（通道0） */
        trim_value =  REG_READ32(0x00800078UL);
        temp_trim[0].tos_value  = (value->code1 != 0xFFFF) ? value->code1 : 0x0500;
        temp_trim[0].k_value  = (value->code2 != 0xFFFF) ? value->code2 : 0x0B00;
        temp_trim[0].k_value = temp_trim[0].k_value / 16;

        /* 读取VTEMP修调值（通道1） */
        trim_value =  REG_READ32(0x0080007CUL);
        temp_trim[1].tos_value  = (value->code1 != 0xFFFF) ? value->code1 : 0x0500;
        temp_trim[1].k_value  = (value->code2 != 0xFFFF) ? value->code2 : 0x0B00;
        temp_trim[1].k_value = temp_trim[1].k_value / 16;
    }
    else
    {
        temp_trim[0].tos_value =  temp_trim[0].k_value = 1;
        temp_trim[1].tos_value =  temp_trim[1].k_value = 1;
    }
}

/**
 * @brief   加载ADC VF通道修调值（VPN0~2和AON通道）
 * @param   vf_ch_trim  - VF通道修调数组指针（VPN0~2）
 * @param   aon_ch_trim - AON通道修调指针
 * @note    从Flash信息块读取增益和偏移修调值，处理符号位扩展
 * @retval  LL_OK - 加载成功
 */
static ll_status_e ll_adc_vf_calibration_load(adc_ch_trim_t *vf_ch_trim, adc_ch_trim_t *aon_ch_trim)
{
    uint32_t trim_value;

    for (adc_channel_e channel = ADC_CHANNEL_VPN0; channel <= ADC_CHANNEL_VPN2; channel++)
    {
        uint8_t index = channel - ADC_CHANNEL_VPN0;
        trim_value = REG_READ32(0x0080006CUL + (index << 2));      /* 读取VPN通道修调值 */
        vf_ch_trim[index].gain = (trim_value & 0x00003FFFUL);      /* 低14位为增益 */
        trim_value = (trim_value >> 16 & 0x00003FFFUL);             /* 高14位为偏移 */

        if (trim_value & (1 << 13))                                  /* 符号位判断 */
        {
            vf_ch_trim[index].offset = (int16_t)(trim_value | 0xE000);  /* 负数符号扩展 */
        }
        else
        {
            vf_ch_trim[index].offset = (int16_t)(trim_value & 0x1FFF);
        }
    }

    trim_value = REG_READ32(0x00800084UL);                          /* 读取AON通道修调值 */
    aon_ch_trim->gain = (trim_value & 0x0000FFFFUL);                /* 低16位为增益 */
    trim_value = (trim_value >> 16 & 0x00003FFFUL);                 /* 高14位为偏移 */

    if ((trim_value & (1 << 13)) != 0)                               /* 符号位判断 */
    {
        aon_ch_trim->offset = (int16_t)(trim_value | 0xE000);
    }
    else
    {
        aon_ch_trim->offset = (int16_t)(trim_value & 0x1FFF);
    }

    return LL_OK;
}

/**
 * @brief   配置ADC扫描通道参数
 * @param   scan_channel - 扫描通道号（0~7）
 * @param   config       - 扫描配置（包含通道选择、PGA旁路、缓冲旁路、增益）
 * @retval  None
 */
static void ll_adc_scan_channel_config(adc_scan_channel_e scan_channel, adc_scan_config_t *config)
{

    switch (scan_channel)
    {
    case ADC_SCAN_CHANNEL_0:
        ADC->CTRL_SCAN01_F.SCAN_CHNL_0 = config->scan_channel_cfg;  /* 配置扫描通道0 */
        break;

    case ADC_SCAN_CHANNEL_1:
        ADC->CTRL_SCAN01_F.SCAN_CHNL_1 = config->scan_channel_cfg;  /* 配置扫描通道1 */
        break;

    case ADC_SCAN_CHANNEL_2:
        ADC->CTRL_SCAN23_F.SCAN_CHNL_2 = config->scan_channel_cfg;
        break;

    case ADC_SCAN_CHANNEL_3:
        ADC->CTRL_SCAN23_F.SCAN_CHNL_3 = config->scan_channel_cfg;
        break;

    case ADC_SCAN_CHANNEL_4:
        ADC->CTRL_SCAN45_F.SCAN_CHNL_4 = config->scan_channel_cfg;
        break;

    case ADC_SCAN_CHANNEL_5:
        ADC->CTRL_SCAN45_F.SCAN_CHNL_5 = config->scan_channel_cfg;
        break;

    case ADC_SCAN_CHANNEL_6:
        ADC->CTRL_SCAN67_F.SCAN_CHNL_6 = config->scan_channel_cfg;
        break;

    case ADC_SCAN_CHANNEL_7:
        ADC->CTRL_SCAN67_F.SCAN_CHNL_7 = config->scan_channel_cfg;
        break;

    default:
        break;
    }
}

/**
 * @brief   平均值计算工具函数（弱符号，可重写）
 * @param   data   - 数据数组
 * @param   length - 数组长度
 * @note    去掉最大值和最小值后计算平均值，如果长度小于3则直接平均
 * @retval  平均值
 */
__attribute__((weak)) uint16_t averge_calculate_utils(uint16_t *data, uint16_t length)
{
    uint16_t avg, min, max;
    uint32_t sum = 0;

    min = UINT16_MAX;
    max = 0;

    for (uint8_t i = 0; i < length; i++)
    {
        min = ((min < data[i]) ? min : data[i]);                   /* 找最小值 */
        max = ((max > data[i]) ? max : data[i]);                   /* 找最大值 */
        sum += data[i];
    }

    if (length >= 3)
    {
        avg = (sum - max - min + ((length - 2) >> 1)) / (length - 2);  /* 去掉极值后平均 */
    }
    else
    {
        avg = (sum + (length >> 1)) / length;                          /* 直接平均 */
    }

    return avg;
}

/**
 * @brief   反初始化ADC外设
 * @param   None
 * @note    清除中断标志和掩码，复位ADC模块
 * @retval  None
 */
void ll_adc_deinit(void)
{
    ADC->ICR |= 0x1F;                                              /* 清除所有中断标志 */
    ADC->IMR &= ~0x1F;                                             /* 禁能所有中断 */

    adc_isr_enable = false;

    CRG_CONFIG_UNLOCK();
    CRG->ADC_CLKRST_CTRL_F.RST_ADC = 1;                          /* 复位ADC */
    __NOP();
    __NOP();
    CRG->ADC_CLKRST_CTRL_F.RST_ADC = 0;                          /* 释放复位 */
    __NOP();
    __NOP();
    CRG_CONFIG_LOCK();

    adc_isr_flag = 0;

    adc_isr_callback = NULL;
}

/**
 * @brief   初始化ADC外设
 * @param   config   - ADC配置参数（包含时钟、触发模式、触发次数、中断配置）
 * @param   callback - 中断回调函数指针
 * @note    先反初始化，再配置时钟、加载修调值、配置触发/FIFO/采样周期等
 * @retval  LL_OK - 成功
 */
ll_status_e ll_adc_init(adc_config_t *config, ISR_FUNC_CALLBACK callback)
{
    ll_adc_deinit();                                               /* 先反初始化 */

    ll_adc_clk_config(&config->clk_cfg);                           /* 配置时钟 */

    ll_adc_trim_value_load();                                      /* 加载模拟修调值 */

    ll_adc_vf_calibration_load(vf_ch_trim_value, &aon_ch_trim_value);  /* 加载VF通道校准 */

    ll_adc_calibration_load(&vbat_trim_value, vtemp_temp_value, &vaon_coef_value);  /* 加载校准值 */

    ADC->CTRL0_F.TRIG_EN = config->trig_mode;                     /* 设置触发模式 */

    ADC->CTRL0_F.FIFO_THRHLD = config->trig_num - 1;              /* 设置FIFO阈值 */
    ADC->CTRL0_F.I_SEL = 1;                                       /* 选择中断源 */

    ADC->CTRL0_F.SW_ADC_EN = true;                                 /* 使能软件触发 */
    ADC->CTRL0_F.AUTO_ADC_EN = false;                              /* 禁能自动触发 */

    /* ADC CTRL1配置 */
    ADC->CTRL1_F.SW_CONT_MODE = 0;                                 /* 连续模式 */
    ADC->CTRL1_F.SCAN_CHNL_NUM = 0;                                /* 扫描通道数，N+1 */
    ADC->CTRL1_F.CHNL_SAMP_NUM =  config->trig_num - 1;            /* 通道采样次数 */

    ADC->CTRL1_F.DIV_EN = 1;                                       /* 使能时钟分频 */

    /* ADC CTRL2配置：使用默认值 */
    if (DEFAULT_SYSTEM_CLOCK < 48000000UL)
    {
        ADC->CTRL2_F.SAMP_CYCLE = 0x03 >> 1;                      /* 采样周期（低速） */
        ADC->CTRL2_F.INIT_CYCLE = 0x1E0 >> 1;                     /* 初始化周期（低速） */
    }
    else
    {
        ADC->CTRL2_F.SAMP_CYCLE = 0x0F;                            /* 采样周期（高速） */
        ADC->CTRL2_F.INIT_CYCLE = 0x320;                           /* 初始化周期（高速） */
    }

    /* 清除ADC FIFO */
    ll_adc_fifo_clear();

    ll_adc_isr_config(&config->isr_cfg, callback);                 /* 配置中断 */

    return LL_OK;
}

/**
 * @brief   配置VF扫描模式
 * @param   channel  - ADC通道号
 * @param   cfg      - 通道配置
 * @param   scan_num - 扫描次数
 * @note    配置CTRL0/CTRL1寄存器，设置扫描通道2~5的PGA/BUF/增益参数，
 *         并固定配置通道0为TEMP1，通道1为TAO_TEST
 * @retval  LL_OK
 */
ll_status_e ll_adc_vf_scan_config(adc_channel_e channel, adc_cfg_t *cfg, uint8_t scan_num)
{
    /* ADC CTRL0配置 */
    ADC->CTRL0_F.FIFO_THRHLD = scan_num - 1;                      /* 设置FIFO阈值 */
    /* ADC CTRL1配置 */
    ADC->CTRL1_F.SW_CONT_MODE = 0;                                 /* 连续模式 */
    ADC->CTRL1_F.SCAN_CHNL_NUM = scan_num - 1;                    /* 扫描通道数，N+1 */
    ADC->CTRL1_F.CHNL_SAMP_NUM =  0;                               /* 通道采样次数 */

    adc_scan_config_t scan_config =
    {
        .scan_config_bit.pga_bypass = cfg->adc_pag_bypass,          /* PGA旁路 */
        .scan_config_bit.pga_bufn_bypass = cfg->adc_buf_bypass,     /* 负缓冲旁路 */
        .scan_config_bit.pga_bufp_bypass = cfg->adc_buf_bypass,     /* 正缓冲旁路 */
        .scan_config_bit.pga_gain = cfg->gain,                     /* PGA增益 */
    };
    scan_config.scan_config_bit.channel = channel;
    ll_adc_scan_channel_config(ADC_SCAN_CHANNEL_2, &scan_config);  /* 配置扫描通道2 */
    ll_adc_scan_channel_config(ADC_SCAN_CHANNEL_3, &scan_config);  /* 配置扫描通道3 */
    ll_adc_scan_channel_config(ADC_SCAN_CHANNEL_4, &scan_config);  /* 配置扫描通道4 */
    ll_adc_scan_channel_config(ADC_SCAN_CHANNEL_5, &scan_config);  /* 配置扫描通道5 */

    scan_config.scan_config_bit.channel = ADC_CHANNEL_TEMP1;
    scan_config.scan_config_bit.pga_bypass = false;
    scan_config.scan_config_bit.pga_bufn_bypass = false;
    scan_config.scan_config_bit.pga_bufp_bypass = false;
    scan_config.scan_config_bit.pga_gain = ADC_GAIN_X16;
    ll_adc_scan_channel_config(ADC_SCAN_CHANNEL_0, &scan_config);  /* 配置温度为扫描通道0 */

    scan_config.scan_config_bit.channel = ADC_CHANNEL_TAO_TEST;
    scan_config.scan_config_bit.pga_bypass = true;
    scan_config.scan_config_bit.pga_bufn_bypass = true;
    scan_config.scan_config_bit.pga_bufp_bypass = false;
    scan_config.scan_config_bit.pga_gain = ADC_GAIN_X1;
    ll_adc_scan_channel_config(ADC_SCAN_CHANNEL_1, &scan_config);  /* 配置TAO_TEST为扫描通道1 */

    return LL_OK;
}

/**
 * @brief   配置ADC通道增益参数
 * @param   channel - ADC通道号
 * @param   cfg     - 通道配置（VCR/VCM/PGA增益/VREF等）
 * @retval  LL_OK - 成功，LL_PARAM_INVALID - 通道号无效
 */
ll_status_e ll_adc_gain_config(adc_channel_e channel, adc_cfg_t *cfg)
{
    if (channel > ADC_CHANNEL_MAX)
    {
        return LL_PARAM_INVALID;
    }

    ADC->CTRL0_F.VCR_EN = cfg->vcr_enable;                        /* 使能/禁能VCR */

    if (cfg->vcr_enable)
    {
        ADC->CTRL0_F.VCR_SEL = cfg->vcr;                          /* 选择VCR档位 */
    }

    ADC->CTRL0_F.VCM_SEL = cfg->vcm;                              /* 设置共模电压 */
    ADC->CTRL1_F.PGA_GAIN_SEL = (cfg->adc_pag_en && !cfg->adc_pag_bypass) ? cfg->gain : 0;  /* 设置PGA增益 */
    ADC->CTRL1_F.VREF_SEL = cfg->vref;                             /* 选择参考电压 */

    return LL_OK;
}

/**
 * @brief   使能或禁能ADC中断
 * @param   enable - true使能NVIC中断，false禁能
 * @retval  LL_OK
 */
ll_status_e ll_adc_isr_enable(bool enable)
{
    adc_isr_enable = enable;

    ADC->ICR |= ADC_ISR_FLAG;                                      /* 清除中断标志 */


    if (enable)
    {
        NVIC_ClearPendingIRQ(ADC_IRQn);
        ADC->IMR &= ~adc_isr_flag;                                 /* 使能指定中断 */
        NVIC_EnableIRQ(ADC_IRQn);
    }
    else
    {
        ADC->IMR |= adc_isr_flag;                                   /* 禁能指定中断 */
        // NVIC_DisableIRQ(ADC_IRQn);
    }

    return LL_OK;
}

/**
 * @brief   加载ADC通道偏移/增益校准值
 * @param   channel - ADC通道号
 * @note    根据修调版本从Flash信息块读取各通道的校准值，
 *         写入ASYSCFG的ADC_CAL_CTRL寄存器
 * @retval  None
 */
static void ll_adc_channel_offset_load(adc_channel_e channel)
{
    uint32_t trim_version = REG_TRIM_VERSION();

    ASYSCFG_CONFIG_UNLOCK();

    /* 根据修调版本加载各通道的偏移和增益校准 */
    if (((trim_version & 0xFF) > 0x03) || (trim_version & 0xFF) == 0x03 && (((trim_version >> 8) & 0x3F) >= 0x01))
    {
        switch (channel)
        {
        case ADC_CHANNEL_VBAT:
            if (((trim_version & 0xFF) > 0x03) || (trim_version & 0xFF) == 0x03 && (((trim_version >> 8) & 0x3F) >= 0x02))
            {
                ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x0080005c);      /* 读取VBAT校准 */
                ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x0080005c) >> 16;
            }
            else
            {
                ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = 0x1000;                      /* 默认校准值 */
                ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = 0;
            }

            break;

        case ADC_CHANNEL_VC0:
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x00800060);           /* 读取VC0校准 */
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x00800060) >> 16;
            break;

        case ADC_CHANNEL_VC1:
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x00800064);
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x00800064) >> 16;
            break;

        case ADC_CHANNEL_VC2:
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x00800068);
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x00800068) >> 16;
            break;

        case ADC_CHANNEL_VPN0:
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x0080006c);
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x0080006c) >> 16;
            break;

        case ADC_CHANNEL_VPN1:
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x00800070);
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x00800070) >> 16;
            break;

        case ADC_CHANNEL_VPN2:
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x00800074);
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x00800074) >> 16;
            break;

        default:
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x00800058);           /* 默认通道校准 */
            ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x00800058) >> 16;
            break;
        }
    }
    else
    {
        ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = 0x1000;                              /* 旧版本默认值 */
        ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = 0;
    }

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   选择ADC通道并配置参数
 * @param   channel - ADC通道号
 * @param   cfg     - 通道配置
 * @note    等待ADC空闲，配置增益、路径选择（VC/VPN通道需特殊TEST配置）、
 *         FIFO阈值和采样次数
 * @retval  LL_OK - 成功，LL_PARAM_INVALID - 参数无效
 */
ll_status_e ll_adc_select_channel(adc_channel_e channel, adc_cfg_t *cfg)
{
    if (channel >= ADC_CHANNEL_MAX || cfg->vref >= ADC_VREF_MAX)
    {
        return LL_PARAM_INVALID;
    }

    while (ADC->CHNL_STATUS_F.ADC_BUSY);                          /* 等待ADC空闲 */

    ll_adc_gain_config(channel, cfg);                              /* 配置增益 */

    if (channel >= ADC_CHANNEL_VC0 && channel <= ADC_CHANNEL_VPN2)
    {
        TEST_CONFIG_UNLOCK();

        TEST->TEST_ANA_CTRL_F.TESTMUX_TAO_SEL = 2;                 /* 选择TAO测试信号 */
        TEST->TEST_ANA_CTRL_F.TEST_AON = 1;                        /* 使能AON测试 */
        TEST->TEST_ANA_CTRL_F.TESTMUX_TO_ADC_EN = 1;              /* 使能测试MUX到ADC */
        TEST->TEST_ANA_CTRL_F.TESTMUX_SEL_BUF = 1;                /* 选择缓冲器 */
        TEST->TEST_ANA_CTRL_F.TESTMUX_BUF_EN = 1;                  /* 使能缓冲器 */
        TEST->TEST_ANA_CTRL_F.TESTMUX_TAO_EN = 1;                  /* 使能TAO测试MUX */

        TEST_CONFIG_LOCK();

        ASYSCFG_CONFIG_UNLOCK();

        ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = 0x1000;             /* 设置默认校准 */
        ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = 0;

        ASYSCFG_CONFIG_LOCK();

        ADC->CTRL0_F.VREFBUF_EN = true;                            /* 使能参考电压缓冲 */
        ADC->CTRL1_F.BUFP_EN = cfg->adc_buf_en;                    /* 使能正输入缓冲 */
        ADC->CTRL1_F.BUFN_EN = cfg->adc_buf_en;                    /* 使能负输入缓冲 */
        ll_adc_vf_scan_config(channel, cfg, 6);                    /* 配置VF扫描 */
    }
    else
    {
        ll_adc_channel_offset_load(channel);                       /* 加载通道校准 */

        ADC->CTRL0_F.VREFBUF_EN = true;                            /* 使能参考电压缓冲 */

        ADC->CTRL1_F.BUFP_EN = cfg->adc_buf_en;                    /* 使能正输入缓冲 */
        ADC->CTRL1_F.BUFN_EN = cfg->adc_buf_en;                    /* 使能负输入缓冲 */

        ADC->CTRL1_F.BUFP_BP = cfg->adc_buf_bypass;                /* 正缓冲旁路 */
        ADC->CTRL1_F.BUFN_BP = cfg->adc_buf_bypass;                /* 负缓冲旁路 */

        ADC->CTRL1_F.PGA_EN = cfg->adc_pag_en;                     /* 使能PGA */
        ADC->CTRL1_F.PGA_BP = cfg->adc_pag_bypass;                 /* PGA旁路 */

        /* ADC CTRL0配置 */
        ADC->CTRL0_F.FIFO_THRHLD = 3;                              /* 设置FIFO阈值 */
        /* ADC CTRL1配置 */
        ADC->CTRL1_F.SW_CONT_MODE = 0;                              /* 连续模式 */
        ADC->CTRL1_F.SCAN_CHNL_NUM = 0;                             /* 扫描通道数 */
        ADC->CTRL1_F.CHNL_SAMP_NUM =  3;                            /* 采样次数 */

        ADC->CTRL1_F.IN_SEL = channel;                              /* 选择输入通道 */
    }

    return LL_OK;
}

/**
 * @brief   获取ADC FIFO中的数据个数
 * @param   None
 * @retval  FIFO中当前数据个数（0~15）
 */
uint8_t ll_adc_fifo_length_get(void)
{
    return (ADC->FIFO_STATE & 0x0F);                               /* 读取FIFO状态寄存器低4位 */
}

/**
 * @brief   从ADC FIFO读取指定数量的数据
 * @param   buffer - 数据缓冲区指针
 * @param   length - 期望读取的数据个数
 * @note    如果FIFO中数据多于需求，先丢弃多余数据；返回实际读取数
 * @retval  实际读取的数据个数
 */
uint16_t ll_adc_fifo_get(uint16_t *buffer, uint16_t length)
{
    uint8_t len = ll_adc_fifo_length_get();

    if (len >= length)
    {
        for (uint8_t i = 0; i < len - length; i++)
        {
            buffer[0] = ADC->FIFO_DATA;                            /* 丢弃多余数据 */
        }
    }

    len = length > len ? len : length;                              /* 取较小值 */

    for (uint8_t i = 0; i < len; i++)
    {
        buffer[i] = ADC->FIFO_DATA & 0x3FFF;                       /* 读取数据并取低14位 */
    }

    return len;
}

/**
 * @brief   清除ADC FIFO
 * @param   None
 * @note    通过读取FIFO_DATA寄存器耗尽所有数据
 * @retval  清除的数据个数
 */
uint16_t ll_adc_fifo_clear(void)
{
    uint32_t reg_val = 0;
    (void)(&reg_val);

    uint8_t len = ll_adc_fifo_length_get();

    for (uint8_t i = 0; i < len; i++)
    {
        reg_val = ADC->FIFO_DATA;                                  /* 读取并丢弃FIFO数据 */
    }

    return len;
}

/**
 * @brief   启动或停止ADC软件触发
 * @param   enable - true启动，false停止
 * @retval  None
 */
void ll_adc_softwart_start(bool enable)
{
    if (enable)
    {
        ADC->CTRL0_F.SW_START = true;                              /* 软件触发开始 */
    }
    else
    {
        ADC->CTRL0_F.SW_START = false;                             /* 软件触发停止 */
    }
}

/**
 * @brief   获取ADC增益对应的数值
 * @param   gain  - 增益档位
 * @param   value - 指向存储增益值的变量指针
 * @retval  LL_OK - 成功，LL_PARAM_INVALID - 参数无效
 */
ll_status_e ll_adc_gain_value_get(adc_gain_e gain, uint8_t *value)
{
    if (gain >= ADC_GAIN_MAX)
    {
        return LL_PARAM_INVALID;
    }

    *value = gain + 1;                                             /* 增益值 = 枚举值 + 1 */
    return LL_OK;
}

/**
 * @brief   获取VCR（电压比较参考）值
 * @param   vcr   - VCR档位
 * @param   value - 指向存储VCR值的变量指针（单位mV）
 * @retval  LL_OK - 成功，LL_PARAM_INVALID - 参数无效
 */
ll_status_e ll_adc_vcr_value_get(adc_vcr_e vcr, uint16_t *value)
{

    if (vcr >= ADC_VCR_MAX)
    {
        return LL_PARAM_INVALID;
    }

    *value = vcr_value[vcr];                                       /* 查表获取VCR值 */
    return LL_OK;
}

/**
 * @brief   获取VREF（参考电压）值
 * @param   vref  - VREF档位
 * @param   value - 指向存储VREF值的变量指针（单位mV）
 * @retval  LL_OK - 成功，LL_PARAM_INVALID - 参数无效
 */
ll_status_e ll_adc_vref_value_get(adc_vref_e vref, uint16_t *value)
{

    if (vref >= ADC_VREF_MAX)
    {
        return LL_PARAM_INVALID;
    }

    *value = vref_value[vref];                                     /* 查表获取VREF值 */
    return LL_OK;
}

/**
 * @brief   温度值计算函数（使用传入配置）
 * @param   channel - 温度通道号
 * @param   code    - ADC采样码值
 * @param   cfg     - ADC配置（预留）
 * @param   value   - 指向存储温度值的变量指针（单位℃）
 * @note    温度计算公式：(code * 2.5 - TOS) / K - 273.15
 * @retval  LL_OK - 成功，LL_PARAM_INVALID - 参数无效
 */
ll_status_e ll_adc_temp_calculate_func(temp_channel_e channel, int16_t code, adc_cfg_t *cfg,  uint16_t *value)
{
    if (NULL == cfg || channel >= TEMP_CHANNEL_MAX)
    {
        return LL_PARAM_INVALID;
    }

    *value = (code * 2.5 - vtemp_temp_value[channel].tos_value) / vtemp_temp_value[channel].k_value - 273.15;

    return LL_OK;
}

/**
 * @brief   温度值计算（自动加载修调值）
 * @param   channel - 温度通道号
 * @param   code    - ADC采样码值
 * @note    首次调用自动加载温度修调值
 * @retval  温度值（单位℃）
 */
int ll_adc_temp_calculate(temp_channel_e channel, int16_t code)
{
    static uint8_t load_trim = 0;

    if (!load_trim)
    {
        ll_adc_temp_calibration_load(vtemp_temp_value);            /* 首次调用加载温度修调 */
        load_trim = 1;
    }

    return (code * 2.5 - vtemp_temp_value[channel].tos_value) / vtemp_temp_value[channel].k_value - 273.15;
}

/**
 * @brief   VBAT电压值计算函数
 * @param   code  - ADC采样码值
 * @param   value - 指向存储电压值的变量指针
 * @note    当前为空实现
 * @retval  LL_OK
 */
ll_status_e ll_adc_vbat_calculate_func(int16_t code, uint16_t *value)
{
    return LL_OK;
}

/**
 * @brief   电压值通用计算函数
 * @param   code  - ADC采样码值
 * @param   cfg   - ADC配置（包含分压比、VCR/VREF等）
 * @param   value - 指向存储电压值的变量指针
 * @note    计算公式：value = ratio * ((code_val * vref) / (gain * 8192) + vcr)
 *         处理14位有符号码值转换
 * @retval  LL_OK - 成功，LL_PARAM_INVALID - 参数无效
 */
ll_status_e ll_adc_volt_calculate_func(int16_t code, adc_cfg_t *cfg, uint16_t *value)
{

    if (NULL == cfg)
    {
        return LL_PARAM_INVALID;
    }

    int16_t code_val = code;
    uint16_t vref, vcr;
    uint8_t gain;

    if (cfg->adc_pag_en && !cfg->adc_pag_bypass)
    {
        ll_adc_gain_value_get(cfg->gain, &gain);                   /* 获取增益值 */
    }
    else
    {
        gain = 1;
    }

    if (cfg->vcr_enable)
    {
        ll_adc_vcr_value_get(cfg->vcr, &vcr);                     /* 获取VCR值 */
    }
    else
    {
        vcr = 0;
    }

    ll_adc_vref_value_get(cfg->vref, &vref);                      /* 获取VREF值 */

    if (code & (1 << 13))                                          /* 14位有符号扩展 */
    {
        code_val = (int16_t)(code | 0xE000);
    }
    else
    {
        code_val = (int16_t)(code & 0x1FFF);
    }

    *value = cfg->ratio * ((code_val * vref) / (gain * 8192.f) + vcr);  /* 电压计算公式 */

    return LL_OK;
}

/**
 * @brief   VF通道电压值计算函数
 * @param   channel - VF通道号（VPN0~2）
 * @param   buffer  - ADC采样数据缓冲区（[0]=LED_V, [1]=AON_T, [2]=AON_V）
 * @param   value   - 指向存储计算结果的变量指针
 * @note    使用VAON二次曲线校正，通过AON温度和电压修正LED电压测量值
 * @retval  LL_OK
 */
ll_status_e ll_adc_vf_calculate_func(adc_channel_e channel, uint16_t *buffer, uint16_t *value)
{
    uint8_t index;
    int32_t aon_t_code;
    int32_t aon_v_code;
    int32_t led_v_code;
    float vaon;

    led_v_code = buffer[0];                                        /* LED电压码值 */
    aon_t_code = buffer[1];                                        /* AON温度码值 */
    aon_v_code = buffer[2];                                        /* AON电压码值 */
    index = channel - ADC_CHANNEL_VPN0;                            /* 计算通道索引 */

    aon_v_code = (aon_v_code - aon_ch_trim_value.offset) * aon_ch_trim_value.gain;  /* AON电压校准 */
    aon_t_code = (aon_t_code - aon_ch_trim_value.offset) * aon_ch_trim_value.gain;  /* AON温度校准 */
    led_v_code = (led_v_code - vf_ch_trim_value[index].offset) * vf_ch_trim_value[index].gain;  /* LED电压校准 */

    vaon = vaon_coef_value.a * aon_t_code * aon_t_code + vaon_coef_value.b * aon_t_code + vaon_coef_value.c;  /* VAON二次拟合 */
    *value = 0.5 + vaon * led_v_code  / aon_v_code;               /* 最终电压值 */

    return LL_OK;
}

/**
 * @brief   ADC通道启动采集
 * @param   channel  - ADC通道号
 * @param   cfg      - 通道配置
 * @param   buffer   - 数据缓冲区
 * @param   trig_num - 触发采样次数
 * @note    选择通道后软件触发，等待FIFO数据达到要求，超时返回错误
 * @retval  LL_OK - 成功，LL_ERROR - 超时或FIFO错误
 */
ll_status_e ll_adc_channnel_start(adc_channel_e channel, adc_cfg_t *cfg, uint16_t *buffer, uint8_t trig_num)
{
    ll_status_e st = LL_OK;
    uint16_t timeout = 50;

    ll_adc_select_channel(channel, cfg);                           /* 选择并配置通道 */

    ll_adc_isr_enable(false);                                      /* 禁能中断 */

    ll_adc_softwart_start(true);                                   /* 软件触发启动 */

    while (trig_num != ll_adc_fifo_length_get())                   /* 等待FIFO数据达到预期 */
    {
        if (timeout-- <= 0)
        {
            st = LL_ERROR;
            break;
        }

        for (uint8_t i = 0; i < 100; i++)                          /* 约10us延时 */
        {
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
        }
    }

    if (trig_num != ll_adc_fifo_get(buffer, trig_num))             /* 读取FIFO数据 */
    {
        st = LL_ERROR;
    }

    return st;
}

/**
 * @brief   使能或禁能温度传感器
 * @param   enable - true使能内部温度传感器，false禁能
 * @note    控制ADC内部温度传感器通道INT0和INT1的使能
 * @retval  LL_OK
 */
ll_status_e ll_adc_tsensor_enable(bool enable)
{
    if (enable)
    {
        ADC->CTRL1_F.INT0_SNS_EN = true;                           /* 使能温度传感器通道0 */
        ADC->CTRL1_F.INT1_SNS_EN = true;                           /* 使能温度传感器通道1 */
    }
    else
    {
        ADC->CTRL1_F.INT0_SNS_EN = false;                          /* 禁能温度传感器通道0 */
        ADC->CTRL1_F.INT1_SNS_EN = false;                          /* 禁能温度传感器通道1 */
    }

    return LL_OK;
}

/**
 * @brief   使能LIN自动寻址ADC模式
 * @param   type   - LIN AA类型（2步或3步等）
 * @param   enable - true使能，false禁能
 * @note    反初始化ADC后重新配置为LIN AA模式，设置触发源为LIN，
 *         配置采样路径（旁路PGA和缓冲器）
 * @retval  LL_OK
 */
ll_status_e ll_adc_lin_aa_enable(lin_aa_type_e type, bool enable)
{
    ll_adc_deinit();                                               /* 先反初始化 */
    ll_adc_trim_value_load();                                      /* 加载修调值 */

    ASYSCFG_CONFIG_UNLOCK();

    ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_CAL = REG_READ32(0x00800058); /* 加载默认校准 */
    ASYSCFG->ADC_CAL_CTRL_F.ADC_OUT_OFFSET = REG_READ32(0x00800058) >> 16;

    ASYSCFG_CONFIG_LOCK();

    PWM->LED_CTRL_F.LED_LDO5V_EN = 1;                              /* 使能5V LDO */
    /* ADC CTRL0配置 */
    ADC->CTRL0_F.VREFBUF_EN = 1;                                   /* 使能参考缓冲 */
    ADC->CTRL0_F.TRIG_EN = TRIG_LIN;                               /* 选择LIN触发 */
    ADC->CTRL0_F.SW_ADC_EN = false;                                /* 禁能软件触发 */
    ADC->CTRL0_F.AUTO_ADC_EN = true;                               /* 使能自动触发 */
    ADC->CTRL0_F.FIFO_THRHLD = 0;                                  /* 设置FIFO阈值 */

    /* ADC CTRL1配置 */
    ADC->CTRL1_F.IN_SEL = ADC_CHANNEL_LIN;                        /* 选择LIN输入通道 */
    ADC->CTRL1_F.SW_CONT_MODE = false;                             /* 单次模式 */
    ADC->CTRL1_F.CHNL_SAMP_NUM = 0;                                /* 采样次数 */
    ADC->CTRL1_F.SCAN_CHNL_NUM = 0;                                /* 扫描通道数 */

    if (LIN_AA_STYPE_STEPS_2 == type)
    {
        ADC->CTRL2_F.INIT_CYCLE = 0x3EF >> 1;                     /* 2步模式初始化周期 */
    }
    else
    {
        ADC->CTRL2_F.INIT_CYCLE = 0xF0 >> 1;                      /* 多步模式初始化周期 */
    }

    /* 路径配置：旁路所有缓冲器和PGA */
    ADC->CTRL1_F.BUFN_BP = true;                                   /* 旁路负缓冲 */
    ADC->CTRL1_F.BUFP_BP = true;                                   /* 旁路正缓冲 */
    ADC->CTRL1_F.BUFN_EN = false;                                  /* 禁能负缓冲 */
    ADC->CTRL1_F.BUFP_EN = false;                                  /* 禁能正缓冲 */
    ADC->CTRL1_F.PGA_BP = true;                                    /* 旁路PGA */
    ADC->CTRL1_F.PGA_EN = false;                                   /* 禁能PGA */

    ADC->CTRL1_F.PGA_GAIN_SEL = ADC_GAIN_X1;                      /* PGA增益1倍 */
    ADC->CTRL1_F.DIV_EN = false;                                   /* 禁能时钟分频 */

    /* 清除ADC FIFO */
    ll_adc_fifo_clear();

    return LL_OK;
}

/**
 * @brief   控制偏置使能
 * @param   enable - true使能，false禁能
 * @note    当前为空实现，预留接口
 * @retval  LL_OK
 */
ll_status_e ll_bias_control_enable(bool enable)
{

    return LL_OK;
}

/**
 * @brief   ADC中断处理函数
 * @note    读取中断状态寄存器，调用用户回调函数，清除中断标志
 * @retval  None
 */
void ADC_IRQHandler(void)
{
    uint32_t isr = ADC->ISR & ADC_ISR_FLAG;                       /* 读取ADC中断状态 */

    if (isr)
    {
        if (NULL != adc_isr_callback)
        {
            adc_isr_callback(isr);                                 /* 调用用户回调 */
        }

        ADC->ICR |= isr;                                           /* 清除中断标志 */
    }
}
