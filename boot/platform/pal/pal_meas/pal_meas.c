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

#define VBAT_GAIN_CHANGE_MAX    0x380
#define VBAT_GAIN_CHANGE_MIN    0x135

/**
 * @brief ADC TRIG NUM
 */
#if defined (__TCPL01X__)
#define ADC_TRIGGER_NUMBER          4
#define ADC_PN_TRIGGER_NUMBER       (ADC_TRIGGER_NUMBER)
#elif defined (__TCPL03X__)
#define ADC_TRIGGER_NUMBER          4
#define ADC_PN_TRIGGER_NUMBER       (ADC_TRIGGER_NUMBER +2)
#else
#define ADC_TRIGGER_NUMBER          4
#define ADC_PN_TRIGGER_NUMBER       (ADC_TRIGGER_NUMBER)
#endif

#if CFG_SUPPORT_MULTIPLEX_LED
#define MEAS_CLOSE_PWM_INIT         (PWM_INIT_ALL_FLAG & (~PWM_INT_CH0_CNT_MAX))
#else
#define MEAS_CLOSE_PWM_INIT         (PWM_INIT_ALL_FLAG)
#endif

extern adc_cfg_t default_vbvt_param_table[2];
extern adc_cfg_t seft_check_param_table[LED_TYPE_MAX][LED_MEAS_SERIAL_MAX];
extern uint8_t lin_lld_sci_get_state(void);

void adc_callback_handle(uint32_t isr);

const uint32_t led_mes_flag_table[CH_TRIG_SRC_MAX] = {PWM_INT_CH0_PWM_CLR, PWM_INT_CH0_CNT_MAX, PWM_INT_CH1_PWM_CLR, PWM_INT_CH1_CNT_MAX, PWM_INT_CH2_PWM_CLR, PWM_INT_CH2_CNT_MAX};

#if CFG_SUPPORT_SINGAL_BIN
const sft_adpat_value_t sft_adpat_value[LED_TYPE_MAX] =
{
    {
        .value_l = 1600,
        .value_h = 2000,
    },
    {
        .value_l = 2400,
        .value_h = 2900,
    },
    {
        .value_l = 2400,
        .value_h = 2900,
    },
};
#endif

led_measure_context_t led_meas_context[LED_CHANNEL_MAX];

/**
 * @brief  设置VF采样状态标志
 * @param  vf_sample - VF采样上下文指针
 * @param  type - 状态类型
 * @param  status - 状态值(0/1)
 * @retval 无
 */
static void led_meas_vf_status_set(vf_sample_ctx_t *vf_sample, led_vf_status_type_e type, uint8_t status)
{
    if (status)
    {
        vf_sample->vf_status |= type;
    }
    else
    {
        vf_sample->vf_status &= ~type;
    }
}

/**
 * @brief  获取VF采样状态标志
 * @param  vf_sample - VF采样上下文指针
 * @param  type - 状态类型
 * @retval 状态值(0或1)
 */
static uint8_t led_meas_vf_status_get(vf_sample_ctx_t *vf_sample, led_vf_status_type_e type)
{
    return (!!(vf_sample->vf_status & type));
}

/**
 * @brief  PN结测量恢复(关闭ADC中断和PWM采样标志)
 * @param  ctx - LED测量上下文指针
 * @retval 无
 */
static void led_meas_pn_recovery(led_measure_context_t *ctx)
{
    ll_adc_isr_enable(false);
    ll_pwm_isr_flag_enable(LL_PWM_BUS_0, MEAS_CLOSE_PWM_INIT, false);
    led_meas_vf_status_set(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_DATA_STATUS | LED_VF_SUSPEND_STATUS), false);
}

/**
 * @brief  PN结测量通道设置(切换通道或完成采集)
 * @param  ctx - LED测量上下文指针
 * @param  init_flag - true:初始化/切换通道, false:完成当前通道采集
 * @note   初始化时选择ADC通道和配置;完成时标记数据就绪并关闭中断
 * @retval true - 设置成功, false - 失败
 */
static bool led_meas_pn_channel_setup(led_measure_context_t *ctx, bool init_flag)
{
    uint8_t adc_channel;
    led_type_e rgb;

    if (init_flag || PWM_CHANNEL_2 != ctx->vf_samp.channel)
    {
        ctx->vf_samp.channel = init_flag ? PWM_CHANNEL_0 : (ctx->vf_samp.channel + 1);

        adc_channel = ADC_CHANNEL_VPN0 + ctx->vf_samp.channel;

        for (rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
        {
            if (ctx->vf_samp.channel == ctx->rgb_vf_mux[rgb])
            {
                break;
            }
        }

        adc_cfg_t *cfg = &seft_check_param_table[rgb][0];

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
            // ll_adc_fifo_clear();
            // ll_adc_isr_enable(true);
            ll_pwm_isr_flag_enable(LL_PWM_BUS_0, led_mes_flag_table[ctx->vf_samp.trig_src], true);
#if defined (__TCPL03X__)
            // ll_pwm_isr_enable(LL_PWM_BUS_0, true);
#endif
        }
    }
    else
    {
        ll_adc_isr_enable(false);
        ll_pwm_isr_flag_enable(LL_PWM_BUS_0, MEAS_CLOSE_PWM_INIT, false);
        led_meas_vf_status_set(&ctx->vf_samp, LED_VF_DATA_STATUS, true);
        led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SUSPEND_STATUS, false);
        ctx->vf_samp.channel = PWM_CHANNEL_0;
    }

    return true;
}

/**
 * @brief  LED测量初始化(ADC配置、温度传感器使能、上下文清零)
 * @param  channel - LED通道号
 * @retval true - 初始化成功, false - 失败
 */
MEAS_INSTANCE bool led_meas_init(led_channel_e channel)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    adc_config_t m_config =
    {
        .clk_cfg = {
            .fclk_div = 1,
        },
        .isr_cfg = {
            .isr = ADC_INT_FIFO_RDY_FLAG,
            .isr_enable = true,
            .priority = 2,
        },
        .trig_num = ADC_TRIGGER_NUMBER,
        .trig_mode = TRIG_SOFTWARE,
    };

    ll_adc_init(&m_config, adc_callback_handle);

    /* Temperature sensor enable */
    ll_adc_tsensor_enable(true);


    led_measure_context_t *ctx = &led_meas_context[channel];
    memset((void *)ctx, 0, sizeof(led_measure_context_t));

    pal_led_channel_mux_get(channel, &ctx->rgb_vf_mux);
    ctx->vf_samp.trig_src = CH0_TRIG_CNT_MAX;

    return true;
}

/**
 * @brief  LED测量增益配置
 * @param  channel - LED通道号
 * @note   配置ADC通道(VBAT/VTEMP/PN)的增益参数
 *         TCPL01x电压计算公式:
 *         Vs-Vled = 40 * ((code * Vref / (1024 * Gain2) + VCR) / Gain1)
 * @retval true - 配置成功, false - 失败
 */
MEAS_INSTANCE bool led_meas_gain_config(led_channel_e channel)
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

    for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
    {
        adc_cfg_t *cfg = &seft_check_param_table[rgb][0]; //待完善

        if (NULL != cfg)
        {
            cfg->vca = ADC_VCA_0;

            /*VCA_SEL=0,OP_SEL=1*/
            ll_adc_gain_config((adc_channel_e)(ctx->rgb_vf_mux[rgb] + ADC_CHANNEL_VPN0), cfg);
        }
    }

#elif defined (__TCPL03X__)
#endif

    return true;
}

/**
 * @brief  获取电压ADC原始码值(温度或电池电压)
 * @param  type - 电压测量类型(温度/电池)
 * @param  value - 输出取平均后的ADC原始码值
 * @retval true - 获取成功, false - 失败
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
 * @brief  获取PN结电压ADC原始码值
 * @param  channel - LED通道号
 * @note   关闭LED诊断，从ADC FIFO读取数据并取平均，
 *         TCPL03X额外采集AON和AON_T通道
 * @retval true - 获取成功, false - 失败
 */
MEAS_INSTANCE bool led_meas_pn_voltage_get(led_channel_e channel)
{
    uint16_t adc_value[ADC_PN_TRIGGER_NUMBER];

    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    ll_led_diagnose_enable((pwm_channel_e)ctx->vf_samp.channel, false);

    ll_adc_fifo_get(adc_value, ADC_PN_TRIGGER_NUMBER);
#if defined (__TCPL01X__)
    ctx->vf_samp.vf_code[ctx->vf_samp.channel][0] = averge_calculate_utils(adc_value, ADC_TRIGGER_NUMBER);
#elif defined (__TCPL03X__)
    ctx->vf_samp.vf_code[ctx->vf_samp.channel][0] = averge_calculate_utils(&adc_value[2], ADC_TRIGGER_NUMBER);
    ctx->vf_samp.vf_code[ctx->vf_samp.channel][1] = adc_value[0]; /* aon T */
    ctx->vf_samp.vf_code[ctx->vf_samp.channel][2] = adc_value[1]; /* aon */
#else
    ctx->vf_samp.vf_code[ctx->vf_samp.channel][0] = averge_calculate_utils(adc_value, ADC_TRIGGER_NUMBER);
#endif

    ctx->vf_samp.ch_status[ctx->vf_samp.channel].data_ready = true;

    led_meas_pn_channel_setup(ctx, false);

    return true;
}

/**
 * @brief  电压值计算(温度或电池电压)
 * @param  type - 电压测量类型
 * @param  raw_code - ADC原始码值
 * @param  value - 输出计算后的电压/温度值
 * @note   电池电压支持增益自动切换(当原始码超出范围时)
 * @retval true - 计算成功
 */
MEAS_INSTANCE bool led_meas_volt_calc_func(meas_volt_type_e type, uint16_t raw_code, int16_t *value)
{
    adc_cfg_t *cfg = NULL;

    if (MEAS_VOLT_TEMP == type)
    {
        cfg = &default_vbvt_param_table[MEAS_VOLT_TEMP];
#if defined (__TCPL01X__)
        ll_adc_temp_calculate_func(raw_code, cfg, (uint16_t *)value);
#else
        ll_adc_temp_calculate_func(TEMP_CHANNEL_0, raw_code, cfg, (uint16_t *)value);
#endif
    }
    else
    {
        cfg = &default_vbvt_param_table[MEAS_VOLT_VBAT];

#if VBAT_CALIBRATION
        ll_adc_vbat_calculate_func(raw_code, (uint16_t *)value);
#else
        ll_adc_volt_calculate_func(raw_code, cfg, (uint16_t *)value);
#endif

#if defined (__TCPL01X__)

        if (raw_code > VBAT_GAIN_CHANGE_MAX && ADC_GAIN1_X5 == cfg->gain1)
        {
            cfg->gain1 = ADC_GAIN1_X2;
            ll_adc_gain_config(ADC_CHANNEL_VBAT, cfg);
        }

        if (raw_code < VBAT_GAIN_CHANGE_MIN && ADC_GAIN1_X2 == cfg->gain1)
        {
            cfg->gain1 = ADC_GAIN1_X5;
            ll_adc_gain_config(ADC_CHANNEL_VBAT, cfg);
        }

#endif
    }

    return true;
}

/**
 * @brief  PN结电压计算(原始码转电压值)
 * @param  channel - LED通道号
 * @param  rgb - RGB类型
 * @param  raw_code - 输出原始ADC码值
 * @param  value - 输出计算后的电压值(mV)
 * @note   检查数据就绪状态，调用底层计算函数转换
 * @retval true - 计算成功, false - 参数无效或数据未就绪
 */
MEAS_INSTANCE bool led_meas_pn_calc_func(led_channel_e channel, led_type_e rgb, uint16_t *raw_code, int16_t *value)
{
    if (MEAS_LED_CHANNEL_INVALID(channel) || rgb >= LED_TYPE_MAX)
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    if (NULL == ctx || !ctx->vf_samp.ch_status[ctx->rgb_vf_mux[rgb]].data_ready || !ctx->vf_samp.vf_code[ctx->rgb_vf_mux[rgb]][0])
    {
        return false;
    }

#if defined (__TCPL01X__)
    adc_cfg_t *cfg = &seft_check_param_table[rgb][0]; //TODO

    ll_adc_volt_calculate_func(ctx->vf_samp.vf_code[ctx->rgb_vf_mux[rgb]][0], cfg, (uint16_t *)value);
#elif defined (__TCPL03X__)
    ll_adc_vf_calculate_func((adc_channel_e)(ctx->rgb_vf_mux[rgb] + ADC_CHANNEL_VPN0), \
                             &ctx->vf_samp.vf_code[ctx->rgb_vf_mux[rgb]][0], (uint16_t *)value);
#endif
    *raw_code = ctx->vf_samp.vf_code[ctx->rgb_vf_mux[rgb]][0];
    ctx->vf_samp.ch_status[ctx->rgb_vf_mux[rgb]].data_ready = false;

    return true;
}

/**
 * @brief  刷新PN结测量状态(确定采样触发源)
 * @param  channel - LED通道号
 * @note   根据当前PWM占空比确定最大通道,
 *         从最后一个关闭的通道开始采样以避开LED开启噪声
 * @retval true - 刷新成功, false - 失败
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

    // __disable_irq();
    uint16_t rgb_pwm_duty[LED_TYPE_MAX] __attribute__((unused));
    uint16_t pwm_ch_duty[PWM_CHANNEL_MAX] __attribute__((unused));

    pal_led_dutcycle_get(channel, &rgb_pwm_duty[LED_R]);

    for (led_type_e rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
    {
        pwm_ch_duty[ctx->rgb_vf_mux[rgb]] = rgb_pwm_duty[rgb];
    }

    uint16_t max_value = MAX3_VALUE_GET(pwm_ch_duty[PWM_CHANNEL_0], pwm_ch_duty[PWM_CHANNEL_1], pwm_ch_duty[PWM_CHANNEL_2]);

    /* begin sampling from the last channel that was turned off */
    for (pwm_channel_e pwm_ch = PWM_CHANNEL_0; pwm_ch < PWM_CHANNEL_MAX; pwm_ch++)
    {
        if (max_value == pwm_ch_duty[pwm_ch])
        {

            ctx->vf_samp.trig_src = CH0_TRIG_PWM_CLR + (max_value ? (pwm_ch << 1) : ((pwm_ch << 1) + 1));

            break;
        }
    }

    // __enable_irq();
    return true;
}

/**
 * @brief  PN结采样处理(PWM中断中调用)
 * @param  channel - LED通道号
 * @param  pwm_isr - PWM中断状态
 * @note   在分时采集中处理触发源匹配，使能LED诊断并启动ADC采集
 *         避免因分时方案导致的采集恢复问题
 * @retval true - 处理成功, false - 无需处理或参数无效
 */
MEAS_INSTANCE bool led_meas_pn_process(led_channel_e channel, uint32_t pwm_isr)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    /* avoid the collection recovery caused by the time-sharing scheme */
    if (!(~PWM_INT_CH0_CNT_MAX & pwm_isr) && !led_meas_vf_status_get(&ctx->vf_samp, LED_VF_STATIC_SAMP_STATUS))
    {
        return false;
    }

    if (led_meas_vf_status_get(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_STATIC_SAMP_STATUS | LED_VF_ACQUIRE_STATUS)))
    {
        if ((lin_lld_sci_get_state() == 0 || lin_lld_sci_get_state() == 0x0A) && led_meas_vf_status_get(&ctx->vf_samp, LED_VF_SUSPEND_STATUS))
        {
            if (pwm_isr & led_mes_flag_table[ctx->vf_samp.trig_src])
                // if ((pwm_isr & led_mes_flag_table[ctx->vf_samp.trig_src]) && \
                //     led_meas_vf_status_get(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_STATIC_SAMP_STATUS | LED_VF_ACQUIRE_STATUS)))
            {

                ll_led_diagnose_enable((pwm_channel_e)ctx->vf_samp.channel, true);
                ll_adc_it_start(true);
            }

            else
            {
                led_meas_pn_recovery(ctx);
                return false;
            }
        }
    }
    else
    {
        led_meas_vf_status_set(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_DATA_STATUS | LED_VF_SUSPEND_STATUS), false);
    }

    return true;
}

/**
 * @brief  暂停PN结采样例程
 * @param  channel - LED通道号
 * @note   去初始化ADC并关闭PWM采样中断，设置暂停标志
 *         若ADC正在运行，需先复位ADC并在恢复时重新加载校准值
 * @retval true - 暂停成功, false - 失败
 */
MEAS_INSTANCE bool led_meas_pn_suspend(led_channel_e channel)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    ll_adc_deinit();
    ll_pwm_isr_flag_enable(LL_PWM_BUS_0, PWM_INIT_ALL_FLAG, false);

    led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SUSPEND_STATUS, true);

    return true;
}

/**
 * @brief  恢复PN结采样例程
 * @param  channel - LED通道号
 * @note   重新初始化ADC和增益配置，清除暂停和数据标志
 * @retval true - 恢复成功, false - 失败
 */
MEAS_INSTANCE bool led_meas_pn_resume(led_channel_e channel)
{
    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    led_meas_init(channel);
    led_meas_gain_config(channel);
    led_meas_vf_status_set(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_SUSPEND_STATUS | LED_VF_DATA_STATUS), false);

    return true;
}

/**
 * @brief  触发PN结采样采集
 * @param  channel - LED通道号
 * @note   若非动态/静态采样中且未就绪或正在采集，则设置暂停标志
 *         并初始化PN通道设置开始采样
 * @retval true - 采集触发成功
 */
MEAS_INSTANCE bool led_meas_pn_acquire(led_channel_e channel)
{
    bool ret = false;

    if (MEAS_LED_CHANNEL_INVALID(channel))
    {
        return false;
    }

    led_measure_context_t *ctx = &led_meas_context[channel];

    /* 非动态以及静态采样、采样ready或者采样进行时，无需触发采样*/
    if (led_meas_vf_status_get(&ctx->vf_samp, (led_vf_status_type_e)(LED_VF_ACQUIRE_STATUS | LED_VF_STATIC_SAMP_STATUS)))
    {
        led_meas_vf_status_set(&ctx->vf_samp, LED_VF_SUSPEND_STATUS, true);
        led_meas_pn_channel_setup(ctx, true);
    }


    return true;
}

/**
 * @brief  获取PN结采样状态
 * @param  channel - LED通道号
 * @param  type - 状态类型
 * @param  status - 输出状态值
 * @retval true - 获取成功, false - 通道无效
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
 * @brief  设置PN结采样状态
 * @param  channel - LED通道号
 * @param  type - 状态类型
 * @param  status - 状态值(0/1)
 * @retval true - 设置成功, false - 通道无效
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


const meas_manager_instance_t led_meas_instance_def =
{
    .meas_init = led_meas_init,
    .meas_gain_config = led_meas_gain_config,
    .meas_voltage_code_get = led_meas_voltage_code_get,
    .meas_volt_calc_func = led_meas_volt_calc_func,
    .meas_pn_voltage_get = led_meas_pn_voltage_get,
    .meas_pn_calc_func = led_meas_pn_calc_func,
    .meas_pn_status_reflash = led_meas_pn_status_reflash,
    .meas_pn_process = led_meas_pn_process,

    .meas_pn_suspend = led_meas_pn_suspend,
    .meas_pn_resume = led_meas_pn_resume,
    .meas_pn_acquire = led_meas_pn_acquire,
    .meas_pn_status_get = led_meas_pn_status_get,

    .meas_pn_status_set = led_meas_pn_status_set,
};
