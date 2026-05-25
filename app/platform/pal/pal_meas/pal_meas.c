/**
 *****************************************************************************
 * @brief   pal meas source file.
 *
 * @file    pal_meas.c
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

#include "pal_meas_def.h"
#include "utilities.h"

// to do: 不同通道以及通道中断接口注册
/**
* @brief 电池电压校准开关，关闭时采用底层原始的计算公式计算
*/
#define VBAT_CALIBRATION    0

/**
 * @brief VBAT增益切换阈值上限（原始码值大于此值时切换到低增益）
 */
#define VBAT_GAIN_CHANGE_MAX    0x380
/**
 * @brief VBAT增益切换阈值下限（原始码值小于此值时切换到高增益）
 */
#define VBAT_GAIN_CHANGE_MIN    0x135


/**
 * @brief ADC TRIG NUM
 */
#if defined (__TCPL01X__)
#define ADC_TRIGGER_NUMBER    4
#elif defined (__TCPL03X__)
#define ADC_TRIGGER_NUMBER    4
#else
#define ADC_TRIGGER_NUMBER    4
#endif

/**
 * @brief 默认VBAT/VTEMP ADC参数表（外部定义）
 */
extern adc_cfg_t default_vbvt_param_table[2];
/**
 * @brief RGB各通道ADC参数配置表（外部定义）
 */
extern adc_cfg_t seft_check_param_table[RGB_TYPE_MAX][LED_MEAS_SERIAL_MAX];
/**
 * @brief 获取LIN LL驱动SCI状态（外部定义）
 */
extern uint8_t lin_lld_sci_get_state(void);

/**
 * @brief ADC中断回调函数声明
 * @param  isr - ADC中断状态标志
 */
void adc_callback_handle(uint32_t isr);

/**
 * @brief PWM中断标志查找表，映射触发源到对应PWM中断标志
 */
const uint32_t led_mes_flag_table[CH_TRIG_SRC_MAX] = {PWM_INT_CH0_PWM_CLR, PWM_INT_CH0_CNT_MAX, PWM_INT_CH1_PWM_CLR, PWM_INT_CH1_CNT_MAX, PWM_INT_CH2_PWM_CLR, PWM_INT_CH2_CNT_MAX};


#if CFG_SUPPORT_SERIAL_NUM_CHECK
/**
 * @brief 灯珠串联检测自适应阈值结构体
 */
typedef struct
{
    uint16_t value_l; /**< 电压下限 */
    uint16_t value_h; /**< 电压上限 */
} seft_adpat_value_t;

/**
 * @brief RGB各通道串联检测自适应阈值表
 */
const seft_adpat_value_t seft_adpat_value[RGB_TYPE_MAX] =
{
    {
        .value_l = 1600,
        .value_h = 2000,
    },
    {
        .value_l = 2200,
        .value_h = 2600,
    },
    {
        .value_l = 2500,
        .value_h = 2900,
    },
};
#endif

/**
 * @brief LED测量上下文实例数组（每个通道一个）
 */
led_measure_context_t led_meas_context[LED_CHANNLE_MAX];


/**
 * @brief  设置VF采样状态标志位
 * @param  vf_sample - VF采样上下文指针
 * @param  type      - 状态类型
 * @param  enable    - 0:清除标志 1:置位标志
 * @retval None
 */
static void led_meas_vf_status_set(vf_sample_ctx_t *vf_sample, led_vf_status_type_e type, uint8_t enable)
{

    if (enable)
    {
        vf_sample->vf_status |= type;
    }
    else
    {
        vf_sample->vf_status &= ~type;
    }
}

/**
 * @brief  获取VF采样状态标志位
 * @param  vf_sample - VF采样上下文指针
 * @param  type      - 状态类型
 * @retval 0 - 标志未置位
 * @retval 1 - 标志已置位
 */
static uint8_t led_meas_vf_status_get(vf_sample_ctx_t *vf_sample, led_vf_status_type_e type)
{
    return (!!(vf_sample->vf_status & type));
}

/**
 * @brief  PN采样异常恢复，禁能ADC和PWM中断，清除采样状态
 * @param  ctx - LED测量上下文指针
 * @retval None
 */
static void led_meas_pn_recovery(led_measure_context_t *ctx)
{
    ll_adc_isr_enable(false);
    ll_pwm_isr_enable(LL_PWM_BUS_0, false);
    led_meas_vf_status_set(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_DATA_STATUS | LED_VF_SUSPEND_STATUS), false);
}

/**
 * @brief  PN采样通道切换与设置，选择下一个ADC通道并配置触发源
 * @param  ctx       - LED测量上下文指针
 * @param  init_flag - true:首次初始化（使能中断） false:切换下一通道
 * @retval true  - 设置成功
 * @retval false - 设置失败
 */
static bool led_meas_pn_channel_setup(led_measure_context_t *ctx, bool init_flag)
{
    uint8_t adc_channel;

    if (PWM_CHANNEL_2 == ctx->vf_samp.channel)
    {
        ll_adc_isr_enable(false);
        ll_pwm_isr_enable(LL_PWM_BUS_0, false);
        led_meas_vf_status_set(&ctx->vf_samp, LED_VF_DATA_STATUS, true);
        led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SUSPEND_STATUS, false);
        ctx->vf_samp.channel = PWM_CHANNEL_0;
    }
    else
    {
        if (init_flag)
        {
            led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SAMPLE_STATUS, false);
            ctx->vf_samp.channel = PWM_CHANNEL_0;
        }
        else
        {
            ctx->vf_samp.channel += 1 ;
        }

        if (ctx->serial_num >= 3)
        {
            adc_channel = ADC_CHANNEL_VC0 + ctx->vf_samp.channel;
        }
        else
        {
            adc_channel = ADC_CHANNEL_VPN0 + ctx->vf_samp.channel;
        }

        rgb_type_e rgb;

        for (rgb = 0; rgb < RGB_TYPE_MAX; rgb++)
        {
            if (ctx->vf_samp.channel == ctx->rgb_vf_mux[rgb])
            {
                break;
            }
        }

        adc_cfg_t *cfg = &seft_check_param_table[rgb][ctx->serial_num];

        if (NULL == cfg)
        {
            return false;
        }

        ll_adc_select_channel((adc_channel_e)adc_channel, cfg);

        if (init_flag)
        {
            /*
            放在这里而不是放在开启中断后面是因为清零中断标识与使能中断之间有可能会有中断源进入导致使能中断后会立马进入中断，
            其实中断源发生在中断触发之前的30us以内,考虑到已经预留死区裕量大于30us，故将限制提前到此处。
            */
            led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SAMPLE_STATUS, true);
            ll_adc_fifo_clear();
            ll_adc_isr_enable(true);
#if defined (__TCPL01X__)
            ll_pwm_isr_flag_enable(LL_PWM_BUS_0, led_mes_flag_table[ctx->vf_samp.trig_src], 0, true);
#elif defined (__TCPL03X__)
            ll_pwm_isr_flag_enable(LL_PWM_BUS_0, led_mes_flag_table[ctx->vf_samp.trig_src], true);
            ll_pwm_isr_enable(LL_PWM_BUS_0, true);
#endif
        }
    }

    return true;
}

#if CFG_SUPPORT_SERIAL_NUM_CHECK
/**
 * @brief  检测LED灯珠串联数（通过测量各串数下的PN结电压判断）
 * @param  channel     - LED通道
 * @param  serial_num  - 输出检测到的串联数
 * @retval true  - 检测成功
 * @retval false - 检测失败
 */
static bool led_meas_serial_num_check(led_channel_e channel, uint8_t *serial_num)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];


    uint8_t analog_channel;
    uint16_t temp_buffer[ADC_TRIGGER_NUMBER] = {0};
    rgb_type_e rgb = RGB_RED;
    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_STATIC_SAMP_STATUS, true);
    pal_led_static_pnvolt_set(channel, true);

    for (rgb_type_e  rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
    {
        ll_led_diagnose_enable(rgb, true);
    }

    // for (rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
    {
        uint16_t raw_code;
        int16_t volt;

        for (uint8_t i = 0; i < LED_MEAS_SERIAL_MAX; i++)
        {
            adc_cfg_t *cfg = (adc_cfg_t *)&seft_check_param_table[rgb][i];

            if (i >= 2) //大于等于三灯珠需要使用VC通道
            {
                analog_channel = ctx->rgb_vf_mux[rgb] + ADC_CHANNEL_VC0;
            }
            else
            {
                analog_channel = ctx->rgb_vf_mux[rgb] + ADC_CHANNEL_VPN0;
            }

            if (NULL != cfg)
            {
                cfg->vca = 0;
                /*VCA_SEL=0*/
                ll_adc_gain_config(analog_channel, cfg);
            }

            if (LL_OK == ll_adc_channnel_start((adc_channel_e)analog_channel, cfg, temp_buffer, ADC_TRIGGER_NUMBER))
            {
                raw_code = averge_calculate_utils(temp_buffer, ADC_TRIGGER_NUMBER);
            }

            ll_adc_volt_calculate_func(raw_code, cfg, (uint16_t *)&volt);

            if (volt >= seft_adpat_value[rgb].value_l * (i + 1) && volt <= seft_adpat_value[rgb].value_h * (i + 1))
            {
                *serial_num = i;
                break;
            }
        }
    }

    for (pwm_channel_e  ch = PWM_CHANNEL_0; ch < PWM_CHANNEL_MAX; ch++)
    {
        ll_led_diagnose_enable(ch, false);
    }

    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_STATIC_SAMP_STATUS, false);
    pal_led_static_pnvolt_set(channel, false);
    return true;
}
#endif

/**
 * @brief ADC全局配置实例（软件触发模式，FIFO中断使能）
 */
adc_config_t m_config =
{
    .clk_cfg = {
        .fclk_div = 1,
    },
    .isr_cfg = {
        .isr = ADC_INT_FIFO_RDY_FLAG,
        .isr_enable = true,
    },
    .trig_num = ADC_TRIGGER_NUMBER,
    .trig_mode = TRIG_SOFTWARE,
};

/**
 * @brief  信号链初始化，配置ADC、温度传感器及VF采样上下文
 * @param  channel - LED通道编号
 * @retval true  - 初始化成功
 * @retval false - 初始化失败
 */
MEAS_INSTANCE bool led_meas_init(led_channel_e channel)
{

    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];
    memset((void *)ctx, 0, sizeof(led_measure_context_t));

    ll_adc_init(&m_config, adc_callback_handle);

    /* Temperature sensor enable */
    ll_adc_tsensor_enable(true);

    pal_led_channel_mux_get(channel, &ctx->rgb_vf_mux);

    ctx->vf_samp.trig_src = CH0_TRIG_CNT_MAX;
    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SAMPLE_STATUS, true);


    return true;
}



/**
 * @brief  ADC增益配置，根据灯珠串联数配置各通道增益参数
 * @note   Vs-Vled = 40 * ((code * Vref / (1024 * Gain2) + VCR) / Gain1)
 *         VR = 40 * ((code * 1528 / (1024 * 8) + 668) / 17)
 *         VG = 40 * ((code * 1528 / (1024 * 4) + 769) / 15)
 *         VB = 40 * ((code * 1528 / (1024 * 8) + 820) / 15)
 * @param  channel     - LED通道编号
 * @param  serial_num  - 灯珠串联数指针（自动检测时回写）
 * @retval true  - 配置成功
 * @retval false - 配置失败
 */
MEAS_INSTANCE bool led_meas_gain_config(led_channel_e channel, uint8_t *serial_num)
{


    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];
#if defined (__TCPL01X__)

    for (adc_channel_e ch = ADC_CHANNEL_TEMP; ch < ADC_CHANNEL_VC0; ch++)
    {
        adc_cfg_t *cfg = &default_vbvt_param_table[ch];

        if (NULL != cfg)
        {
            /*VCA_SEL=0,OP_SEL=1*/
            cfg->vca = ADC_VCA_0;
            ll_adc_gain_config((adc_channel_e)ch, cfg);
        }
    }

    if (*serial_num == LED_MEAS_SERIAL_MAX)
    {

#if CFG_SUPPORT_SERIAL_NUM_CHECK

        if (!led_meas_serial_num_check(channel, serial_num))
        {
            *serial_num = DEFAULT_LED_SERIAL_NUM;
        }

#else
        *serial_num = DEFAULT_LED_SERIAL_NUM;
#endif

    }

    ctx->serial_num = *serial_num;
    adc_channel_e adc_channel = ctx->serial_num >= LED_MEAS_SERIAL_3 ? ADC_CHANNEL_VC0 : ADC_CHANNEL_VPN0;

    for (rgb_type_e rgb = RGB_RED; rgb < RGB_TYPE_MAX; rgb++)
    {
        adc_cfg_t *cfg = &seft_check_param_table[rgb][ctx->serial_num]; //待完善

        if (NULL != cfg)
        {
            cfg->vca = ADC_VCA_0;

            /*VCA_SEL=0,OP_SEL=1*/
            ll_adc_gain_config((adc_channel_e)(ctx->rgb_vf_mux[rgb] + adc_channel), cfg);
        }
    }

#elif defined (__TCPL03X__)
    ctx->serial_num = DEFAULT_LED_SERIAL_NUM;
#endif


    return true;
}


/**
 * @brief  获取ADC电压原始码值（VBAT或TEMP）
 * @param  type  - 电压类型（VBAT/TEMP）
 * @param  value - 输出平均后的ADC原始码值
 * @retval true  - 获取成功
 * @retval false - 获取失败
 */
MEAS_INSTANCE bool led_meas_voltage_code_get(meas_volt_type_e type, uint16_t *value)
{
    uint8_t adc_channel ;
    adc_cfg_t *cfg = NULL;
    uint16_t adc_value[ADC_TRIGGER_NUMBER];

    if (MEAS_VOLT_TEMP == type)
    {
        cfg = &default_vbvt_param_table[MEAS_VOLT_TEMP];
        adc_channel = ADC_CHANNEL_TEMP;
    }
    else
    {
        cfg = &default_vbvt_param_table[MEAS_VOLT_VBAT];
        adc_channel = ADC_CHANNEL_VBAT;
    }

    if (LL_OK != ll_adc_channnel_start((adc_channel_e)adc_channel, cfg, adc_value, ADC_TRIGGER_NUMBER))
    {
        return (false);
    }

    *value = averge_calculate_utils(adc_value, ADC_TRIGGER_NUMBER);

    return (true);
}


/**
 * @brief  读取PN结ADC采样原始码值，求平均后存入上下文
 * @param  channel - LED通道编号
 * @retval true  - 读取成功
 * @retval false - 读取失败
 */
MEAS_INSTANCE bool led_meas_pn_code_read(led_channel_e channel)
{
    uint16_t adc_value[ADC_TRIGGER_NUMBER];

    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    ll_led_diagnose_enable((pwm_channel_e)ctx->vf_samp.channel, false);

    ll_adc_fifo_get(adc_value, ADC_TRIGGER_NUMBER);

    ctx->vf_samp.vf_code[ctx->vf_samp.channel] = averge_calculate_utils(adc_value, ADC_TRIGGER_NUMBER);

    ctx->vf_samp.ch_status[ctx->vf_samp.channel].data_ready = true;

    led_meas_pn_channel_setup(ctx, false);

    return true;
}

/**
 * @brief  刷新PN采样状态，重新计算各通道亮度并选择最优触发源
 * @param  channel - LED通道编号
 * @retval true  - 刷新成功
 * @retval false - 参数无效
 */
MEAS_INSTANCE bool led_meas_pn_status_reflash(led_channel_e channel)
{

    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    if (NULL == ctx)
    {
        return false;
    }

    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SAMPLE_STATUS, false);

    uint16_t rgb_pwm_th[RGB_TYPE_MAX] __attribute__((unused));
    //获取当前rgb通道的亮度情况
    pal_led_dutcycle_get(LED_CHANNLE_0, &rgb_pwm_th[RGB_RED]);

    for (uint8_t rgb = 0; rgb < RGB_TYPE_MAX; rgb++)
    {
        ctx->vf_samp.ch_status[ctx->rgb_vf_mux[rgb]].intensity = rgb_pwm_th[rgb];
    }

    uint16_t max_value = MAX3_VALUE_GET(ctx->vf_samp.ch_status[PWM_CHANNEL_0].intensity, ctx->vf_samp.ch_status[PWM_CHANNEL_1].intensity, ctx->vf_samp.ch_status[PWM_CHANNEL_2].intensity);

    if (ctx->vf_samp.ch_status[PWM_CHANNEL_0].intensity == max_value)
    {
        ctx->vf_samp.trig_src = (!ctx->vf_samp.ch_status[PWM_CHANNEL_0].intensity) ? CH0_TRIG_CNT_MAX : CH0_TRIG_PWM_CLR;
    }
    else
    {
        ctx->vf_samp.trig_src = (ctx->vf_samp.ch_status[PWM_CHANNEL_1].intensity == max_value) ? CH1_TRIG_PWM_CLR : CH2_TRIG_PWM_CLR;
    }

    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SAMPLE_STATUS, true);

    return true;
}


/**
 * @brief  PN采样处理主函数，PWM中断触发时启动ADC转换
 * @param  channel - LED通道编号
 * @param  pwm_isr - PWM中断状态标志
 * @retval true  - 处理成功，已启动ADC转换
 * @retval false - 处理失败或跳过当前采样时隙
 */
MEAS_INSTANCE bool led_meas_pn_process(led_channel_e channel, uint32_t pwm_isr)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    if (!led_meas_vf_status_get(&ctx->vf_samp, LED_VF_SAMPLE_STATUS))
    {
        led_meas_pn_recovery(ctx);
        return false;
    }

    if (lin_lld_sci_get_state() == 0 || lin_lld_sci_get_state() == 0x0A || led_meas_vf_status_get(&ctx->vf_samp, LED_VF_STATIC_SAMP_STATUS))
    {
        if (pwm_isr & led_mes_flag_table[ctx->vf_samp.trig_src])
        {
            if ((ctx->vf_samp.ch_status[ctx->vf_samp.channel].intensity != 0) || led_meas_vf_status_get(&ctx->vf_samp, LED_VF_STATIC_SAMP_STATUS))
            {
                ll_led_diagnose_enable((pwm_channel_e)ctx->vf_samp.channel, true);
                ll_adc_softwart_start(true);
            }
            else
            {
                //为了避免参考电压切换后直接进行采样带来数据不稳定，本采样方案丢弃当前的采样时隙，等待一下采样时序到来再进行采样
                led_meas_pn_channel_setup(ctx, false);
                return false;
            }
        }
        else
        {
            led_meas_pn_recovery(ctx);
            return false;
        }
    }

    return true;
}

/**
 * @brief  暂停PN采样流程，去初始化ADC并禁能PWM中断
 * @note   若ADC正在运行，用户需在重新启动前复位ADC并加载校准值
 * @param  channel - LED通道编号
 * @retval true  - 暂停成功
 * @retval false - 参数无效
 */
MEAS_INSTANCE bool led_meas_pn_suspend(led_channel_e channel)
{

    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    ll_adc_deinit();
    ll_pwm_isr_enable(LL_PWM_BUS_0, false);

    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SUSPEND_STATUS, true);

    return true;
}

/**
 * @brief  恢复PN采样流程，重新初始化ADC和增益配置
 * @note   若ADC之前正在运行，恢复时自动复位并加载校准值
 * @param  channel - LED通道编号
 * @retval true  - 恢复成功
 * @retval false - 参数无效
 */
MEAS_INSTANCE bool led_meas_pn_resume(led_channel_e channel)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    led_meas_init(channel);
    led_meas_gain_config(channel, &ctx->serial_num);
    led_meas_vf_status_set(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_SUSPEND_STATUS | LED_VF_DATA_STATUS), false);

    return true;
}

/**
 * @brief  启动PWM中断触发的ADC采样采集序列
 * @param  channel - LED通道编号
 * @retval true  - 采集启动成功
 * @retval false - 采集条件不满足（正在采集中或已暂停）
 */
MEAS_INSTANCE bool led_meas_pn_acquire(led_channel_e channel)
{

    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];


    if (!led_meas_vf_status_get(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_ACQUIRE_STATUS | LED_VF_STATIC_SAMP_STATUS)) || led_meas_vf_status_get(&ctx->vf_samp, (led_vf_status_type_e)LED_VF_DATA_STATUS))
    {
        return false;
    }

    __disable_irq();

    if (led_meas_vf_status_get(&ctx->vf_samp, LED_VF_SUSPEND_STATUS))
    {
        __enable_irq();
        return false;
    }

    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SUSPEND_STATUS, true);

    __enable_irq();
    led_meas_pn_channel_setup(ctx, true);
    return true;
}

/**
 * @brief  获取PN采样状态标志
 * @param  channel - LED通道编号
 * @param  type    - 状态类型
 * @param  status  - 输出状态值（0/1）
 * @retval true  - 获取成功
 * @retval false - 参数无效
 */
MEAS_INSTANCE bool led_meas_pn_status_get(led_channel_e channel, led_vf_status_type_e type, uint8_t *status)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    *status = led_meas_vf_status_get(&ctx->vf_samp, type);
    return true;
}

/**
 * @brief  设置PN采样状态标志
 * @param  channel - LED通道编号
 * @param  type    - 状态类型
 * @param  status  - 状态值（0:清除 1:置位）
 * @retval true  - 设置成功
 * @retval false - 参数无效
 */
MEAS_INSTANCE bool led_meas_pn_status_set(led_channel_e channel, led_vf_status_type_e type, uint8_t status)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];
    led_meas_vf_status_set(&ctx->vf_samp, type, status);

    return true;
}

/**
 * @brief  获取RGB通道PN结电压安全阈值（上限/下限）
 * @param  channel - LED通道编号
 * @param  rgb     - RGB通道索引
 * @param  value   - 输出阈值结构体（ceil_value/floor_value）
 * @retval true  - 获取成功
 * @retval false - 参数无效
 */
MEAS_INSTANCE bool led_meas_pn_threshold_get(led_channel_e channel, rgb_type_e rgb, rgb_safty_threshold_t *value)
{
    if (MEAS_LED_CHANNEL_INVALID(channel) || rgb >= RGB_TYPE_MAX)
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    // adc_cfg_t *cfg = &seft_check_param_table[rgb][ctx->serial_num];

    // if (NULL == cfg)
    // {
    //     return false;
    // }

    if (LED_MEAS_SERIAL_1 == ctx->serial_num)
    {
        switch (rgb)
        {
            case RGB_RED:
                value->ceil_value = 2020;
                value->floor_value = 1585;
                break;

            case RGB_GREEN:
                value->ceil_value = 3204;
                value->floor_value = 2217;
                break;

            case RGB_BLUE:
                value->ceil_value = 3032;
                value->floor_value = 2415;
                break;

            default:
                break;
        }
    }

    // ll_adc_volt_calculate_func(0x3ff, cfg, &value->ceil_value);
    // ll_adc_volt_calculate_func(0x1f, cfg, &value->floor_value);
    return true;
}

/**
 * @brief LED测量管理器实例定义，聚合所有测量操作函数指针
 */
const meas_manager_instance_t led_meas_instance_def =
{
    .meas_init = led_meas_init,
    .meas_gain_config = led_meas_gain_config,
    .meas_voltage_code_get = led_meas_voltage_code_get,
    .meas_volt_calc_func = led_meas_volt_calc_func,
    .meas_pn_code_read = led_meas_pn_code_read,
    .meas_pn_voltage_get = led_meas_pn_voltage_get,
    .meas_pn_calc_func = led_meas_pn_calc_func,
    .meas_pn_status_reflash = led_meas_pn_status_reflash,
    .meas_pn_process = led_meas_pn_process,

    .meas_pn_suspend = led_meas_pn_suspend,
    .meas_pn_resume = led_meas_pn_resume,
    .meas_pn_acquire = led_meas_pn_acquire,
    .meas_pn_status_get = led_meas_pn_status_get,

    .meas_pn_status_set = led_meas_pn_status_set,
    .meas_pn_threshold_get = led_meas_pn_threshold_get,
};
