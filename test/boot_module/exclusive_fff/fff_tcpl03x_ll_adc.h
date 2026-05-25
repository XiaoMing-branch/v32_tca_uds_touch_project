#ifndef __FFF_TCPL03X_LL_ADC_H__
#define __FFF_TCPL03X_LL_ADC_H__

#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_def.h"
#else
    #include "tcpl03x_ll_def.h"
#endif

#include "fff.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief ADC ISR FUNC FLAG
*/
/* ADC int1 */
#define ADC_INT_CMP_FLAG             (0x00000001UL) //Mask ADC compare interrupt
#define ADC_INT_DONE_FLAG            (0x00000002UL) //Mask ADC trig done interrupt
#define ADC_INT_FIFO_OVF_FLAG        (0x00000004UL) //Mask ADC fifo overflow interrupt
#define ADC_INT_FIFO_RDY_FLAG        (0x00000008UL) //Mask ADC TRIG DONE data ready interrupt
#define ADC_INT_FIFO_UNF_FLAG        (0x00000010UL) //Mask ADC fifo underflow interrupt

/* power check int0*/
// #define OTP_INT_FLAG                 AFE_SYSCFG_IMR0_OTP_INT_MSK_MASK                        //Mask OTP interrupt
// #define VS_LVD_INT_FLAG              AFE_SYSCFG_IMR0_VS_LVD_INT_MSK_MASK                     //Mask VS LVD interrupt

typedef enum
{
    ADC_CHANNEL_TAO_TEST = 0,
    ADC_CHANNEL_VBAT,
    ADC_CHANNEL_VC0,
    ADC_CHANNEL_VC1,
    ADC_CHANNEL_VC2,
    ADC_CHANNEL_VPN0,
    ADC_CHANNEL_VPN1,
    ADC_CHANNEL_VPN2,
    ADC_CHANNEL_TEMP,
    ADC_CHANNEL_TEMP1,
    ADC_CHANNEL_LIN,
    ADC_CHANNEL_IO2_IO4,
    ADC_CHANNEL_IO4_GND,
    ADC_CHANNEL_IO2_GND,
    ADC_CHANNEL_RESERVED_T,
    ADC_CHANNEL_VCR,
    ADC_CHANNEL_MAX,
} adc_channel_e;

/**
 * @brief  ll adc temp channel enumeration
 */
typedef enum
{
    TEMP_CHANNEL_0 = 0,
    TEMP_CHANNEL_1,
    TEMP_CHANNEL_MAX,
} temp_channel_e;

/**
 * @brief  ll adc scan channel enumeration
 */
typedef enum
{
    ADC_SCAN_CHANNEL_0 = 0,
    ADC_SCAN_CHANNEL_1,
    ADC_SCAN_CHANNEL_2,
    ADC_SCAN_CHANNEL_3,
    ADC_SCAN_CHANNEL_4,
    ADC_SCAN_CHANNEL_5,
    ADC_SCAN_CHANNEL_6,
    ADC_SCAN_CHANNEL_7,
    ADC_SCAN_CHANNEL_MAX,
} adc_scan_channel_e;

/**
 * @brief  ll adc  trigger mode enumeration
 */
typedef enum
{
    TRIG_SOFTWARE        = 0U,
    TRIG_PWM0_RISEEDGE   = 0x01U,
    TRIG_PWM0_FALLEDGE   = 0x02U,
    TRIG_PWM1_RISEEDGE   = 0x04U,
    TRIG_PWM1_FALLEDGE   = 0x08U,
    TRIG_PWM2_RISEEDGE   = 0x10U,
    TRIG_PWM2_FALLEDGE   = 0x20U,
    TRIG_RESERVED_T      = 0x80U,
    TRIG_TIMER           = 0x100U,
    TRIG_LIN             = 0x200U,
} adc_trig_mode_e;

typedef enum
{
    ADC_VCR_SEL_236_7 = 0,
    ADC_VCR_SEL_315_6,
    ADC_VCR_SEL_394_54,
    ADC_VCR_SEL_437_45,
    ADC_VCR_SEL_552_4,
    ADC_VCR_SEL_631_3,
    ADC_VCR_SEL_710_17,
    ADC_VCR_SEL_789_1,
    ADC_VCR_MAX,
} adc_vcr_e;

typedef enum
{
    ADC_GAIN_X1  = 0,
    ADC_GAIN_X2,
    ADC_GAIN_X3,
    ADC_GAIN_X4,
    ADC_GAIN_X5,
    ADC_GAIN_X6,
    ADC_GAIN_X7,
    ADC_GAIN_X8,
    ADC_GAIN_X9,
    ADC_GAIN_X10,
    ADC_GAIN_X11,
    ADC_GAIN_X12,
    ADC_GAIN_X13,
    ADC_GAIN_X14,
    ADC_GAIN_X15,
    ADC_GAIN_X16,
    ADC_GAIN_MAX,
} adc_gain_e;

typedef enum
{
    ADC_VCM_SEL_NULL = 0,
    ADC_VCM_SEL_165,
    ADC_VCM_SEL_125,
    ADC_VCM_SEL_205,
    ADC_VCM_MAX,
} adc_vcm_e;

typedef enum
{
    ADC_VREF_2500 =  0,
    ADC_VREF_2000,
    ADC_VREF_1500,
    ADC_VREF_EXT,
    ADC_VREF_MAX,
} adc_vref_e;

/**
 * @brief  ll adc scan config struct
 */
typedef union
{
    uint16_t    scan_channel_cfg;
    struct
    {
        uint16_t channel            : 5;
        uint16_t pga_bypass         : 1;
        uint16_t pga_bufn_bypass    : 1;
        uint16_t pga_bufp_bypass    : 1;
        uint16_t pga_gain           : 4;
        uint16_t reserved           : 4;
    } scan_config_bit;
} adc_scan_config_t;
typedef struct
{
    uint8_t ratio;

#if 1
    bool adc_buf_en;
    bool adc_buf_bypass;

    bool adc_pag_en;
    bool adc_pag_bypass;
#endif
    bool vcr_enable;
    adc_vcr_e vcr;
    adc_gain_e gain;

    adc_vcm_e vcm;
    adc_vref_e vref;
} adc_cfg_t;

/**
 * @brief  ll adc enumeration
 */
typedef struct
{
    ll_clk_config_t clk_cfg;
    ll_isr_config_t isr_cfg;
    uint8_t trig_num;
    adc_trig_mode_e trig_mode;
} adc_config_t;



DECLARE_FAKE_VOID_FUNC(ll_adc_deinit);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_init,adc_config_t *,ISR_FUNC_CALLBACK);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_vref_config,adc_vref_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_gain_config,adc_channel_e,adc_cfg_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_isr_enable,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_lin_aa_enable,lin_aa_type_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_select_channel,adc_channel_e,adc_cfg_t *);
DECLARE_FAKE_VALUE_FUNC(uint8_t,ll_adc_fifo_length_get);
DECLARE_FAKE_VALUE_FUNC(uint16_t,ll_adc_fifo_get,uint16_t *,uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint16_t,ll_adc_fifo_clear);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_crtl_config,uint32_t *,bool);
DECLARE_FAKE_VOID_FUNC(ll_adc_softwart_start,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_it_start,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_vf_calculate_func,adc_channel_e,uint16_t *,uint16_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_volt_calculate_func,int16_t,adc_cfg_t *,uint16_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_vbat_calculate_func,int16_t,uint16_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_temp_calculate_func,temp_channel_e,int16_t,adc_cfg_t *,uint16_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_channnel_start,adc_channel_e,adc_cfg_t *,uint16_t *,uint8_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_tsensor_enable,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_bias_control_enable,bool);

#ifdef __cplusplus
}
#endif
#endif /* __TCPL03X_LL_ADC_H__ */
