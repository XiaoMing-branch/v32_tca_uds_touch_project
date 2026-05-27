/**
*****************************************************************************
* @brief  touch halnode source
* @file   touch_halnode.c
* @author AE/FAE team
* @date   28/JUL/2023
*****************************************************************************
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <b>&copy; Copyright (c) 2023 Tinychip Microelectronics Co.,Ltd.</b>
*
*****************************************************************************
*/

#include "string.h"
#include "tcae10_ll_def.h"
#include "tc_log.h"
#include "touch_halnode.h"
#include "touch_tool.h"
#include "touch_config.h"
#include "misc.h"

/** @brief 日志标签，用于TC_LOG系列宏的模块过滤 */
const static char *TAG = "TOUCH_HALNODE";

/** @brief 当前触摸节点指针，指向正在处理的触摸HAL接口实例 */
volatile TOUCH_HalInterface_Type *current_touch_node = NULL;
/** @brief 当前触摸是否触发ADC采集的标志
 *  @note 1:触发ADC采集；0:不触发ADC采集。由touch_charge_trig()根据para参数设置
 */
volatile int current_touch_trigadc = 1;

#if TOUCH_VAON_DENOISE
/** @name Chip1温度传感器标定参数
 *  @brief 用于VAON降噪的温度传感器-电压标定点（Chip1）
 *  @note 三个标定点对应温度分别为0℃、50℃、100℃
 *  @{
 */
#define TOUCH_DENOISE_TSENSOR_0     2936
#define TOUCH_DENOISE_VAON_0        2.284
#define TOUCH_DENOISE_TSENSOR_50    3414
#define TOUCH_DENOISE_VAON_50       2.263
#define TOUCH_DENOISE_TSENSOR_100   3905
#define TOUCH_DENOISE_VAON_100      2.234
/** @} */
//Chip 2（当前未使用，保留备选标定参数）
//#define TOUCH_DENOISE_TSENSOR_0     2931
//#define TOUCH_DENOISE_VAON_0        2.398
//#define TOUCH_DENOISE_TSENSOR_50    3416
//#define TOUCH_DENOISE_VAON_50       2.341
//#define TOUCH_DENOISE_TSENSOR_100   3889
//#define TOUCH_DENOISE_VAON_100      2.289

/** @brief VAON降噪二次拟合系数
 *  @note 通过三点标定(0℃/50℃/100℃)拟合二次曲线 vaon = a*T^2 + b*T + c
 */
static struct
{
    float a;    /**< 二次项系数 */
    float b;    /**< 一次项系数 */
    float c;    /**< 常数项系数 */
} TouchDenoiseFactor;
#endif

#if TOUCH_EXTSYNC_TRIG_ENABLE       //外部同步触发使能宏，在touch_config.h中定义
/** @brief 外部同步触发状态
 *  @note 用于多个触摸节点之间的同步触发，避免同时采集造成电源噪声
 */
volatile TOUCH_ExtSync_Type touch_extsync = {0, 0};
#endif

/** @brief 将ADC FIFO中的无符号数据转换为有符号数据
 *  @details ADC硬件FIFO_DATA为12位有效数据，通过左移2位再算术右移2位实现符号扩展
 *  @retval 有符号的ADC采样值（12位有效范围：-2048 ~ 2047）
 */
static __INLINE T_SiData adc_get_data(void)
{
    return ((int16_t)(ADC->FIFO_DATA << 2)) >> 2;
}

/** @brief 底层初始化（触摸与ADC寄存器）
 *  @param[in] self HAL接口实例指针
 */
static void touch_charge_low_init(struct TOUCH_HalInterface_Type *self);
/** @brief 设置当前采集通道参数
 *  @param[in] self HAL接口实例指针
 *  @param[in] channel_cfg 通道配置参数（通道号、充放电模式、Shield、补偿等）
 */
static void touch_charge_set_channel(struct TOUCH_HalInterface_Type *self, const TOUCH_HalChConfig_Type *channel_cfg);
/** @brief 设置增益器参数
 *  @param[in] self HAL接口实例指针
 *  @param[in] gainer 增益器配置参数（转移次数、PGA增益、Buffer开关等）
 */
static void touch_charge_set_gainer(struct TOUCH_HalInterface_Type *self, const TOUCH_HalGainer_Type *gainer);
/** @brief 设置对应touch通道IO的使能/接地
 *  @param[in] self HAL接口实例指针
 *  @param[in] channel 通道编号
 *  @param[in] enable 1:正常touch模式, 0:接地模式
 */
static void touch_charge_set_ioenable(struct TOUCH_HalInterface_Type *self, uint8_t channel, uint8_t enable);
/** @brief 触发触摸采集
 *  @param[in] self HAL接口实例指针
 *  @param[in] t 触发类型（TOUCH_TRIG_SOFTWARE:软件触发, TOUCH_TRIG_TINYWORK:定时器触发）
 *  @param[in] para 附加参数（bit0:是否触发ADC采集, 0:不触发, 1:触发）
 */
static void touch_charge_trig(struct TOUCH_HalInterface_Type *self, TOUCH_HALTRIG_TYPE t, uint32_t para);
/** @brief 获取触摸原始数据
 *  @param[in] self HAL接口实例指针
 *  @retval T_SiData 触摸原始数据（采集超时时返回0）
 */
static T_SiData touch_charge_get_data(struct TOUCH_HalInterface_Type *self);

#if TOUCH_VAON_DENOISE
    /** @brief 计算VAON降噪补偿值
     *  @param[out] code_vaon_trim 经过OTP校准后的VAON编码值
     *  @retval float 当前温度下VAON电压值（单位：uV）
     */
    static float touch_calc_vaon(int *code_vaon_trim);
#endif

/** @brief 备份ADC寄存器配置
 *  @details 在触摸采集前保存当前ADC寄存器状态，用于后续恢复
 *  @param[out] reg 备份数据存储指针
 *  @note CTRL0寄存器的bit0（SW_ADC_EN）在备份时被清除，恢复时由调用方按需设置
 */
static __INLINE void touch_adcreg_bkup(TOUCH_AdcBkupReg_Type *reg)
{
    reg->imr = ADC->IMR;
    reg->ctrl0 = ADC->CTRL0 & (~0x1U);
    reg->ctrl1 = ADC->CTRL1;
    reg->ctrl2 = ADC->CTRL2;
    reg->ctrl3 = ADC->CTRL3;
    reg->ctrl_ana = ADC->CTRL_ANA;
}

/** @brief 恢复ADC寄存器配置
 *  @details 触摸采集完成后，将ADC寄存器恢复为备份前的状态
 *  @param[in] reg 已备份的寄存器数据指针
 */
static __INLINE void touch_adcreg_recover(const TOUCH_AdcBkupReg_Type *reg)
{
    ADC->IMR = reg->imr;
    ADC->CTRL0 = reg->ctrl0;
    ADC->CTRL1 = reg->ctrl1;
    ADC->CTRL2 = reg->ctrl2;
    ADC->CTRL3 = reg->ctrl3;
    ADC->CTRL_ANA = reg->ctrl_ana;
}

/** @brief 关闭ADC采集通道并进入低功耗状态
 *  @details 采集完成后关闭ADC、Shield和Touch模块以降低功耗
 *  @note 若未关闭CAPTOUCH_EN，低功耗模式下会产生约40uA的漏电流
 *  @note ADC采集通道配置成接地的方式当前芯片不支持
 */
static __INLINE void touch_adcchannel_close(void)
{
    ADC->CTRL0_F.SW_ADC_EN = 0;         //关闭ADC
    CAPTOUCH->CTRL1_F.SHLD_EN = 0;      //关闭shield通道
    CAPTOUCH->CTRL1_F.CAPTOUCH_EN = 0;  //关闭touch，避免低功耗下漏电
}

/** @brief 设置触摸HAL接口的就绪标志
 *  @param[in] interface 触摸HAL接口实例指针
 *  @param[in] v 就绪状态值（0:未就绪, 1:已就绪）
 *  @note ready标志由touch_charge_get_data()在采集完成后自动清零
 */
void Touch_HalInterface_SetReady(TOUCH_HalInterface_Type *interface, int v)
{
    interface->ready = (v);
};

/** @brief 创建触摸电荷采集HAL接口
 *  @details 初始化触摸节点配置，赋值函数指针表，返回HAL接口实例
 *  @param[in] nd 触摸电荷采集节点指针（由调用方分配内存）
 *  @param[in] touchcfg 触摸控制器配置参数（时钟分频、跳频周期、初始化时间等）
 *  @param[in] adccfg ADC配置参数（时钟分频、VCM、Vref、采样周期等）
 *  @retval TOUCH_HalInterface_Type* 返回HAL接口实例指针（即&nd->interface）
 *  @note 仅保存配置参数的副本（memcpy），后续调用方可释放原配置内存
 *  @note 使能TOUCH_VAON_DENOISE时，会利用三点标定数据计算二次拟合降噪系数
 */
TOUCH_HalInterface_Type *Touch_HalChargeCreate(TOUCH_HalCharge_Type *nd, const TOUCH_HalConfig_Type *touchcfg, const TOUCH_HalAdcConfig_Type *adccfg)
{
#if TOUCH_VAON_DENOISE
    //构建三点(0℃/50℃/100℃)温度-电压矩阵，求解二次拟合系数a,b,c
    float a[3][CRAMMER_BUF_LEN] =
    {
        {TOUCH_DENOISE_TSENSOR_0 * TOUCH_DENOISE_TSENSOR_0, TOUCH_DENOISE_TSENSOR_0, 1},
        {TOUCH_DENOISE_TSENSOR_50 * TOUCH_DENOISE_TSENSOR_50, TOUCH_DENOISE_TSENSOR_50, 1},
        {TOUCH_DENOISE_TSENSOR_100 * TOUCH_DENOISE_TSENSOR_100, TOUCH_DENOISE_TSENSOR_100, 1},
    };
    float b[3] = {TOUCH_DENOISE_VAON_0 * 1000000, TOUCH_DENOISE_VAON_50 * 1000000, TOUCH_DENOISE_VAON_100 * 1000000};

    if (!SiCrammerParseXYZ(a, b, &TouchDenoiseFactor.a, &TouchDenoiseFactor.b, &TouchDenoiseFactor.c))
    {
        TC_LOGE(TAG, "TouchDenoiseFactor Calc Error");
    }
#endif

    //保存配置副本
    memcpy(&nd->touchcfg, touchcfg, sizeof(*touchcfg));
    memcpy(&nd->adccfg, adccfg, sizeof(*adccfg));

    //初始化接口函数指针表
    nd->interface.rself = nd;
    nd->interface.ready = 0;        //初始状态为未就绪
    nd->interface.low_init = touch_charge_low_init;
    nd->interface.set_channel = touch_charge_set_channel;
    nd->interface.set_gainer = touch_charge_set_gainer;
    nd->interface.set_ioenable = touch_charge_set_ioenable;
    nd->interface.trig = touch_charge_trig;
    nd->interface.get_data = touch_charge_get_data;

    return &nd->interface;
}

//void Touch_HalGainerInit(TOUCH_HalGainer_Type *gainer)      //根据增益器参数，初始化内部使用数据
//{
//    if (!gainer)
//    {
//        TC_LOGE(TAG, "gainer is null");
//    }

//    if (gainer->pga.enable != 0)     //计算pga寄存器
//    {
//        gainer->pga.m_pga_ctrl_reg = ((uint32_t)gainer->pga.stg1_power | ((uint32_t)gainer->pga.stg2_power << 2));
//        gainer->pga.m_pga_ctrl_reg &= (~(3U << 7));
//        gainer->pga.m_pga_ctrl_reg |= (uint32_t)1U << 8; //bug:pga chop resetb control bit, low active
//        gainer->pga.m_pga_ctrl_reg <<= 8;
//        gainer->pga.m_pga_ctrl_reg |= 0x1U;

//        gainer->pga.m_pga_chnlx_reg = (((uint32_t)gainer->pga.stg1_gain << 24) | ((uint32_t)gainer->pga.stg2_gain << 27));

//        if (gainer->pga.offset_enable != 0)    //计算pga的offset寄存器值
//        {
//            SARADC_IDAC_CFG_T idac_cfg = Calculate_IdacGroup(gainer->pga.offset_uv, gainer->pga.stg1_gain);
//            gainer->pga.m_pga_chnlx_reg |= (0x1U | ((uint32_t)idac_cfg.idac1Cur << 1) | ((uint32_t)idac_cfg.idac1CurDir << 7) | ((uint32_t)idac_cfg.idac2Cur << 8) | ((uint32_t)idac_cfg.idac2CurDir << 20));
//        }
//    }
//}

/** @brief 底层初始化——配置触摸控制器和ADC寄存器
 *  @details 执行触摸控制器复位、时钟使能、采样超时设置、ADC模块复位和寄存器初始化
 *  @param[in] self HAL接口实例指针
 *  @note 非EXT_LDO模式下需使能内部LDO12C
 *  @note 初始化完成后备份ADC寄存器状态，供set_gainer恢复使用
 */
static void touch_charge_low_init(struct TOUCH_HalInterface_Type *self)
{
    TOUCH_HalCharge_Type *rself = self->rself;

    /* ---- Touch初始化 ---- */
#if TOUCH_USE_EXT_LDO
    //使用外部LDO，跳过内部LDO使能
#else
    //使能内部LDO12C
    ASYSCFG_CONFIG_UNLOCK();
    ASYSCFG->LDO12C_CTRL_F.LDO12C_EN = 1;
    ASYSCFG_CONFIG_LOCK();
#endif

    //复位触摸控制器
    CRG_CONFIG_UNLOCK();
    CRG->CAPTOUCH_CLKRST_CTRL_F.RST_CAPTOUCH = 1;
    __NOP();
    __NOP();
    CRG->CAPTOUCH_CLKRST_CTRL_F.RST_CAPTOUCH = 0;
    __NOP();
    __NOP();

    //使能触摸时钟，设置分频
    CRG->CAPTOUCH_CLKRST_CTRL_F.PCLK_EN_CAPTOUCH = 0x1;
    CRG->CAPTOUCH_CLKRST_CTRL_F.FCLK_EN_CAPTOUCH = 0x1;
    CRG->CAPTOUCH_CLKRST_CTRL_F.FCLK_DIV_CAPTOUCH = rself->touchcfg.clock_divider;
    CRG_CONFIG_LOCK();
		
    CAPTOUCH->ICR = 0xFFFFFFFF;       //清空中断标志

    //采样超时使能，超时时间设为最大值
    CAPTOUCH->CTRL1_F.SAMP_OVF_EN = 1;
    CAPTOUCH->CTRL1_F.SAMP_OVF_TIME = 0xFFF;
    CAPTOUCH->CTRL1_F.TW_TRIG_EN = 0;        //关闭TinyWork触发（使用软件触发或外部触发）
    CAPTOUCH->CTRL1_F.TRIG_ADC_EN = 1;       //触摸采集完成后触发ADC
    CAPTOUCH->CTRL1_F.DSTSW_DIS = 0;         //使能DSTSW，高温下减小漏电
    CAPTOUCH->CTRL2_F.INIT_TIME = rself->touchcfg.init_time;
    CapTouch_Hopping(rself->touchcfg.hop_period);   //设置跳频周期

    CapTouch_Enable(ENABLE);

    /* ---- ADC初始化 ---- */
    //复位ADC模块
    CRG_CONFIG_UNLOCK();
    CRG->ADC_CLKRST_CTRL_F.RST_ADC = 1;
    __NOP();
    __NOP();
    CRG->ADC_CLKRST_CTRL_F.RST_ADC = 0;
    __NOP();
    __NOP();

    //使能ADC时钟
    CRG->ADC_CLKRST_CTRL_F.PCLK_EN_ADC = 1;
    CRG->ADC_CLKRST_CTRL_F.FCLK_DIV_ADC = rself->adccfg.clock_divider;
    CRG->ADC_CLKRST_CTRL_F.FCLK_EN_ADC = 0x1;
    CRG_CONFIG_LOCK();

    //CTRL0：触发源选Touch，FIFO深度1，中断模式2
    ADC->CTRL0_F.TRIG_EN = TRIG_TOUCH;
    ADC->CTRL0_F.FIFO_THRHLD = 1;
    ADC->CTRL0_F.IRQ_MODE = 2;
    ADC->CTRL0_F.VCM_SEL = rself->adccfg.vcm_sel;
    ADC->CTRL0_F.I_SEL = rself->adccfg.i_sel;
    ADC->CTRL0_F.SW_ADC_EN = 0x0;       //关闭软件触发
    ADC->CTRL0_F.AUTO_ADC_EN = 0x0;     //关闭自动采集

    //CTRL1：单通道采集，输入源选Touch
    ADC->CTRL1_F.INT1_SNS_EN = 0;
    ADC->CTRL1_F.INT0_SNS_EN = 0;
    ADC->CTRL1_F.DIV_EN = 0;
    ADC->CTRL1_F.SW_CONT_MODE = 0;
    ADC->CTRL1_F.CHNL_SAMP_NUM = 0;
    ADC->CTRL1_F.SCAN_CHNL_NUM = 0;
    ADC->CTRL1_F.VREF_SEL = rself->adccfg.vref_sel;
    ADC->CTRL1_F.IN_SEL = ADC_CHANNEL_TOUCH;

    //CTRL2：采样周期和初始化周期
    ADC->CTRL2_F.SAMP_CYCLE = rself->adccfg.samp_cycle;
    ADC->CTRL2_F.INIT_CYCLE = rself->adccfg.init_cycle;

    ADC->CTRL0_F.VREFBUF_EN = 1;        //打开VREF缓冲器

    touch_adcreg_bkup(&rself->adc_bkup_reg);     //备份ADC寄存器，供后续set_channel/set_gainer恢复

    CapTouch_InterruptEnable(CAPTOUCH_SAMP_OVF | CAPTOUCH_TRIG_DONE);
}

/** @brief 设置当前采集通道参数
 *  @details 配置触摸通道选择、充放电模式、Shield防护通道和寄生电容补偿参数
 *  @param[in] self HAL接口实例指针
 *  @param[in] channel_cfg 通道配置参数（通道号、充放电模式、Shield、补偿等）
 *  @note Shield通道使能移至trig()阶段执行，避免在通道切换时持续消耗约1mA电流
 *  @note CAPTOUCH_EN在此处使能，在get_data()中关闭以降低功耗
 */
static void touch_charge_set_channel(struct TOUCH_HalInterface_Type *self, const TOUCH_HalChConfig_Type *channel_cfg)
{
    TOUCH_HalCharge_Type *rself = self->rself;

    rself->captouch_mode = channel_cfg->captouch_mode;      //保存当前充放电模式（get_data中根据此模式决定读取次数）

    //通道选择和充放电模式
    CAPTOUCH->CTRL0_F.CHNL_SEL = channel_cfg->channel;
    CAPTOUCH->CTRL1_F.CREF_SEL = channel_cfg->cref_sel;
    CAPTOUCH->CTRL1_F.CAPTOUCH_MODE = channel_cfg->captouch_mode;

    //Shield防护通道配置（使能移至trig时执行）
    rself->shld_en = channel_cfg->shield.shld_en;   //暂存shield使能，trig时实际设置
    CAPTOUCH->CTRL1_F.SHLD_EN = 0;                  //当前关闭shield
    CAPTOUCH->CTRL0_F.SHLD_I = channel_cfg->shield.shld_i;
    CAPTOUCH->CTRL0_F.SHLD_SEL = channel_cfg->shield.shld_sel;

    //寄生电容补偿配置（IDAC）
    CAPTOUCH->CTRL1_F.CMP_EN = channel_cfg->compensate.cmp_en;
    CAPTOUCH->CTRL3_F.IDAC_INP = channel_cfg->compensate.idac_inp;
    CAPTOUCH->CTRL3_F.IDAC_INN = channel_cfg->compensate.idac_inn;
    CAPTOUCH->CTRL3_F.IDAC_TIME = channel_cfg->compensate.idac_time;
    CAPTOUCH->CTRL3_F.SW_IDAC_SEL_P = channel_cfg->compensate.idac_p_en;
    CAPTOUCH->CTRL3_F.SW_IDAC_SEL_N = channel_cfg->compensate.idac_n_en;
    CAPTOUCH->CTRL3_F.SW_IDAC_MODE = 0;
    if (channel_cfg->compensate.cmp_en)
    {
        CAPTOUCH->CTRL3_F.IDAC_EN = 1;      //使能IDAC补偿
    }
    else
    {
        CAPTOUCH->CTRL3_F.IDAC_EN = 0;
    }

    CAPTOUCH->CTRL1_F.CAPTOUCH_EN = 1;      //开启Touch模块
}

/** @brief 设置增益器参数
 *  @details 恢复ADC备份配置后，设置电荷转移次数/时间、充电时间、PGA增益及Buffer模式
 *  @param[in] self HAL接口实例指针
 *  @param[in] gainer 增益器配置参数（转移次数、PGA增益、Buffer开关等）
 *  @note 若gainer->init_time为0，则使用touchcfg中的默认init_time
 *  @note TOUCH_VAON_DENOISE使能时强制开启Buffer并旁路，确保VAON测量精度
 */
static void touch_charge_set_gainer(struct TOUCH_HalInterface_Type *self, const TOUCH_HalGainer_Type *gainer)
{
    TOUCH_HalCharge_Type *rself = self->rself;

    //恢复ADC配置到low_init后的备份状态（清除上一通道的设置）
    touch_adcreg_recover(&rself->adc_bkup_reg);

    //初始化时间：支持逐通道独立配置，为0则使用默认值
    if (gainer->init_time == 0x0)
    {
        CAPTOUCH->CTRL2_F.INIT_TIME = rself->touchcfg.init_time;
    }
    else
    {
        CAPTOUCH->CTRL2_F.INIT_TIME = gainer->init_time;
    }
    //电荷转移和充放电时序
    CAPTOUCH->CTRL1_F.TRAN_LOOP = gainer->tran_loop;
    CAPTOUCH->CTRL2_F.TRAN_TIME = gainer->tran_time;
    CAPTOUCH->CTRL2_F.CHG_TIME = gainer->chg_time;
    //PGA配置
    ADC->CTRL1_F.PGA_EN = gainer->pga.enable;
    if (gainer->pga.enable)
    {
        ADC->CTRL1_F.PGA_BP = 0;       //PGA不旁路
    }
    else
    {
        ADC->CTRL1_F.PGA_BP = 1;       //PGA旁路
    }
    ADC->CTRL1_F.PGA_GAIN_SEL = gainer->pga.pga_gain_sel;
#if TOUCH_VAON_DENOISE
    //VAON降噪模式下强制使能Buffer
    ADC->CTRL1_F.BUFP_EN = 1;
    ADC->CTRL1_F.BUFN_EN = 1;
    ADC->CTRL1_F.BUFP_BP = 0;
    ADC->CTRL1_F.BUFN_BP = 0;
#else
    if (gainer->pga.buf_en)
    {
        ADC->CTRL1_F.BUFP_EN = 1;
        ADC->CTRL1_F.BUFN_EN = 1;
        ADC->CTRL1_F.BUFP_BP = 0;
        ADC->CTRL1_F.BUFN_BP = 0;
    }
    else
    {
        ADC->CTRL1_F.BUFP_EN = 0;
        ADC->CTRL1_F.BUFN_EN = 0;
        ADC->CTRL1_F.BUFP_BP = 1;
        ADC->CTRL1_F.BUFN_BP = 1;
    }
#endif
    //VCR（偏置电压）配置
    ADC->CTRL0_F.VCR_EN = gainer->pga.vcr_en;
    ADC->CTRL0_F.VCR_SEL = gainer->pga.vcr_sel;
}

/** @brief 设置触摸通道IO的使能/接地模式
 *  @details 控制触摸通道IO引脚的模式，正常触摸模式或接地模式（用于省电或防干扰）
 *  @param[in] self HAL接口实例指针（本函数未使用，保留以保持接口一致）
 *  @param[in] channel 通道编号
 *  @param[in] enable 1:正常触摸模式, 0:接地模式
 */
static void touch_charge_set_ioenable(struct TOUCH_HalInterface_Type *self, uint8_t channel, uint8_t enable)
{
    (void)self;
    Touch_IOEnable(channel, enable);
}

/** @brief 触发触摸采集
 *  @details 设置全局节点状态、清空ADC FIFO、开启ADC和Shield，然后按指定触发类型启动采集
 *  @param[in] self HAL接口实例指针
 *  @param[in] t 触发类型（TOUCH_TRIG_SOFTWARE:软件触发, TOUCH_TRIG_TINYWORK:定时器触发）
 *  @param[in] para 附加参数（bit0:是否触发ADC采集，0:不触发, 1:触发）
 *  @note 全局变量current_touch_node和current_touch_trigadc在此处更新，供ISR/轮询使用
 *  @note Shield通道在trig时才使能，避免在通道切换期间持续消耗~1mA电流
 *  @note TOUCH_TRIG_IN_SLEEP_MODE使能时，触发后CPU进入Sleep模式以降低功耗
 */
static void touch_charge_trig(struct TOUCH_HalInterface_Type *self, TOUCH_HALTRIG_TYPE t, uint32_t para)
{
    TOUCH_HalCharge_Type *rself = self->rself;

    /* 更新全局节点状态 */
    current_touch_node = self;                 //记录当前节点指针，供外部查询
    current_touch_trigadc = (int32_t)para;     //记录是否需触发ADC
    if (para)
    {
        CAPTOUCH->CTRL1_F.TRIG_ADC_EN = 1;    //使能触摸完成触发ADC
    }
    else
    {
        CAPTOUCH->CTRL1_F.TRIG_ADC_EN = 0;    //仅触摸不触发ADC
    }

    /* 清空ADC FIFO中的残留数据 */
    uint16_t i = 0;
    while (ADC->FIFO_STATE_F.ENTRY_VALID != 0 && i < 64)
    {
        ++i;
        (void)(adc_get_data());
    }

#if TOUCH_EXTSYNC_TRIG_ENABLE
    /* 外部同步触发：等待sync_cnt变化，实现多个节点同步触发 */
    if (touch_extsync.enable)
    {
        uint8_t last_sync_cnt = touch_extsync.sync_cnt;
        while (touch_extsync.enable && last_sync_cnt == touch_extsync.sync_cnt)
        {
            continue;
        }
    }
#endif

    ADC->CTRL0_F.SW_ADC_EN = 0x1;            //开启ADC
    CAPTOUCH->CTRL1_F.SHLD_EN = rself->shld_en;   //使能Shield通道

    /* 启动采集 */
    if (t == TOUCH_TRIG_SOFTWARE)
    {
        CAPTOUCH->CTRL1_F.TW_TRIG_EN = 0;    //关闭TinyWork触发
        CAPTOUCH->CTRL0_F.SW_TRIG = 1;       //软件触发
    }
    else
    {
        CAPTOUCH->CTRL1_F.TW_TRIG_EN = 1;    //使能TinyWork定时触发
    }

#if TOUCH_TRIG_IN_SLEEP_MODE
    /* Sleep模式下触发，CPU和SysTick停止运行以降低功耗 */
    ll_lpm_mcu_enter(SLEEP_MODE, 0);
#endif
}

/** @brief 获取触摸原始数据
 *  @details 关闭Shield，等待ADC采集完成，根据充放电模式读取1或2次ADC数据，
 *           对差分数据进行修正，可选VAON降噪补偿，最后关闭ADC通道并清就绪标志
 *  @param[in] self HAL接口实例指针
 *  @retval T_SiData 触摸原始数据（经VAON降噪修正后的值）
 *  @note 充放电平衡模式(CHARGE_DISCHARGE_BALANCE_MODE)需读取2次ADC数据并做差分
 *  @note 差分修正值+6554 = 1.2/1.5*8192，将数据偏移至正值范围
 *  @note ADC超时时返回0并输出错误日志
 *  @note TOUCH_CHECK_RAWDATA使能时，会检查ADC数值是否溢出(>6000或<-6000)
 */
static T_SiData touch_charge_get_data(struct TOUCH_HalInterface_Type *self)
{
    T_SiData touchData;
    uint16_t i;
    TOUCH_HalCharge_Type *rself = self->rself;

    CAPTOUCH->CTRL1_F.SHLD_EN = 0;  //采集完成，关闭Shield通道（降低功耗）

    /* 忙等ADC采集结果 */
    i = 0;
    //平衡模式需要2个数据（正负半周各1个），普通模式只需1个
    uint32_t waitNum = (rself->captouch_mode == CHARGE_DISCHARGE_BALANCE_MODE ? 2 : 1);
    while (ADC->FIFO_STATE_F.ENTRY_VALID < waitNum && i < 0xFFFFU)
    {
        ++i;
        continue;
    }

    if (i < 0xFFFFU)
    {
        /* 采集成功 */
        if (rself->captouch_mode == CHARGE_DISCHARGE_BALANCE_MODE)
        {
            //差分模式：正半周 - 负半周，消除共模噪声
            T_SiData touchData1 = adc_get_data();
            T_SiData touchData2 = adc_get_data();
#if LOG_LOCAL_LEVEL != TC_LOG_NONE
#if TOUCH_CHECK_RAWDATA
            if (touchData1 > 6000 || touchData1 < -6000)
            {
                TC_LOGE(TAG, "touch1 adc overflow : %d", touchData1);
            }
            if (touchData2 > 6000 || touchData2 < -6000)
            {
                TC_LOGE(TAG, "touch2 adc overflow : %d", touchData2);
            }
#endif
#endif
            touchData = touchData1 - touchData2 + 6554;     //+6554将差分结果偏移至正值范围
        }
        else
        {
            //普通模式：单次读取
            touchData = adc_get_data();
#if LOG_LOCAL_LEVEL != TC_LOG_NONE
#if TOUCH_CHECK_RAWDATA
            if (touchData > 6000 || touchData < -6000)
            {
                TC_LOGE(TAG, "touch adc overflow : %d", touchData);
            }
#endif
#endif
        }
    }
    else
    {
        /* 采集超时 */
        touchData = 0;
        TC_LOGE(TAG, "adc timeout");
    }

#if TOUCH_VAON_DENOISE
    /* VAON降噪：根据当前温度修正ADC参考电压变化 */
    int code_vaon_trim;
    float vaon = touch_calc_vaon(&code_vaon_trim) / 1000000;
    //归一化修正
    if (rself->adccfg.vref_sel == ADC_VREF_1500)
    {
        touchData = ((vaon * touchData) / code_vaon_trim) * 3000;
    }
    else
    {
        touchData = ((vaon * touchData) / code_vaon_trim) * 3000;
    }
#endif

    /* 采集结束清理 */
    touch_adcchannel_close();                    //关闭ADC/Shield/Touch，降低噪声和功耗
    Touch_HalInterface_SetReady(self, 0);        //清除就绪标志，标记本次采集完成

    return touchData;
}

#if TOUCH_VAON_DENOISE
/** @brief 计算VAON降噪补偿值
 *  @details 通过TESTMUX测量VAON电压和温度传感器代码，结合OTP校准值，
 *           利用预拟合的二次系数计算当前温度下的VAON值，用于ADC结果归一化修正
 *  @param[out] code_vaon_trim 经过OTP增益/偏移校准后的VAON编码值
 *  @retval float 当前温度下的VAON电压值（单位：uV）
 *  @note 需使用ADC VREF=2.5V，PGA旁路，使能Buffer
 *  @note 扫描2通道：CH0采集VAON（PGA旁路），CH1采集温度传感器（PGA x16）
 *  @note OTP校准地址0x00800084：[31:16]为偏移校准，[15:0]为增益校准
 *  @note 函数会临时修改TEST和ADC寄存器，调用方需确保后续恢复配置
 */
static float touch_calc_vaon(int *code_vaon_trim)
{
    int tsensor_code;
    int vaon_code;
    /** @brief ADC通道配置联合体，用于构造CTRL_SCAN01寄存器值 */
    typedef union
    {
        uint16_t    ChannelCfg;                             /**< 原始通道配置字（16位） */
        struct
        {
            uint16_t                ADC_Channel_Sel     : 5;    /**< ADC通道选择（0~31） */
            uint16_t                PGA_Bypass          : 1;    /**< PGA旁路使能（1:旁路PGA, 直通模式） */
            uint16_t                PGA_Bufn_Bypass     : 1;    /**< PGA负端Buffer旁路（1:旁路） */
            uint16_t                PGA_Bufp_Bypass     : 1;    /**< PGA正端Buffer旁路（1:旁路） */
            uint16_t                ADC_PGA_GainSel     : 4;    /**< PGA增益档位选择（0~15） */
            uint16_t                reserved            : 4;    /**< 保留位 */
        } ChannelCfg_f;

    } ADC_ChannelConfig_t;

    /* ---- 配置TESTMUX：选择VAON作为ADC输入 ---- */
    TEST->TEST_LOCK = 0x76543210;
    TEST->TEST_ANA_CTRL_F.TESTMUX_TAO_SEL = 2;
    TEST->TEST_ANA_CTRL_F.TEST_AON = 1;
    TEST->TEST_ANA_CTRL_F.TESTMUX_TO_ADC_EN = 1;
    TEST->TEST_ANA_CTRL_F.TESTMUX_SEL_BUF = 1;
    TEST->TEST_ANA_CTRL_F.TESTMUX_BUF_EN = 1;
    TEST->TEST_ANA_CTRL_F.TESTMUX_TAO_EN = 1;

    /* ---- ADC配置：VREF=2.5V，单次采集模式 ---- */
    ADC->CTRL0_F.VCR_EN = 0;
    ADC->CTRL0_F.VCR_SEL = ADC_VCR_SEL_236_7;
    ADC->CTRL0_F.TRIG_EN = 0;
    ADC->CTRL0_F.FIFO_THRHLD = 0;
    ADC->CTRL0_F.IRQ_MODE = 2;
    ADC->CTRL0_F.VCM_SEL = ADC_VCM_SEL_205;
    ADC->CTRL0_F.VREFBUF_EN = 1;
    ADC->CTRL0_F.I_SEL = ADC_IBIAS_0p5x;
    ADC->CTRL0_F.AUTO_ADC_EN = 0;
    ADC->CTRL0_F.SW_ADC_EN = 1;

    ADC->CTRL2_F.SAMP_CYCLE = 0x3;
    ADC->CTRL2_F.INIT_CYCLE = 0xF;

    ADC->CTRL1_F.INT1_SNS_EN = 1;
    ADC->CTRL1_F.INT0_SNS_EN = 1;
    ADC->CTRL1_F.PGA_EN = 0;
    ADC->CTRL1_F.PGA_BP = 1;
    ADC->CTRL1_F.PGA_GAIN_SEL = ADC_GAIN_X1;
    ADC->CTRL1_F.BUFP_EN = 1;
    ADC->CTRL1_F.BUFN_EN = 1;
    ADC->CTRL1_F.BUFP_BP = 0;
    ADC->CTRL1_F.BUFN_BP = 0;
    ADC->CTRL1_F.DIV_EN = 1;
    ADC->CTRL1_F.SW_CONT_MODE = 0;
    ADC->CTRL1_F.CHNL_SAMP_NUM = 0;
    ADC->CTRL1_F.SCAN_CHNL_NUM = 1;              //扫描2个通道（CH0和CH1）

    ADC->CTRL1_F.VREF_SEL = ADC_VREF_2500;       //VAON测量需使用2.5V参考电压
    ADC->CTRL1_F.IN_SEL = ADC_CHANNEL_TAO_TEST;

    /* ---- 配置扫描通道 ---- */
    ADC_ChannelConfig_t channel_cfg;
    //CH0: VAON测试信号（PGA旁路）
    channel_cfg.ChannelCfg_f.PGA_Bufp_Bypass = 0;
    channel_cfg.ChannelCfg_f.PGA_Bufn_Bypass = 0;
    channel_cfg.ChannelCfg_f.PGA_Bypass = 1;
    channel_cfg.ChannelCfg_f.ADC_PGA_GainSel = ADC_GAIN_X1;
    channel_cfg.ChannelCfg_f.ADC_Channel_Sel = ADC_CHANNEL_TAO_TEST;
    ADC->CTRL_SCAN01_F.SCAN_CHNL_0 = channel_cfg.ChannelCfg;

    //CH1: 温度传感器（PGA x16放大）
    channel_cfg.ChannelCfg_f.PGA_Bufp_Bypass = 0;
    channel_cfg.ChannelCfg_f.PGA_Bufn_Bypass = 0;
    channel_cfg.ChannelCfg_f.PGA_Bypass = 0;
    channel_cfg.ChannelCfg_f.ADC_PGA_GainSel = ADC_GAIN_X16;
    channel_cfg.ChannelCfg_f.ADC_Channel_Sel = ADC_CHANNEL_TEMP1;
    ADC->CTRL_SCAN01_F.SCAN_CHNL_1 = channel_cfg.ChannelCfg;

    /* 清空FIFO残留数据 */
    for (int i = 0; i <= 16 && ADC->FIFO_STATE_F.ENTRY_VALID > 0; ++i)
    {
        ADC->FIFO_DATA;
    }
    ADC->CTRL0_F.SW_START = 1;       //启动ADC采集

    //等待CH0（VAON）完成
    for (int j = 0; j < 0xFFFF && !ADC->FIFO_STATE_F.ENTRY_VALID; ++j)
    {
        continue;
    }
    vaon_code = ll_adc_getcode();

    //等待CH1（温度传感器）完成
    for (int j = 0; j < 0xFFFF && !ADC->FIFO_STATE_F.ENTRY_VALID; ++j)
    {
        continue;
    }
    tsensor_code = ll_adc_getcode();

    ADC->CTRL0_F.SW_ADC_EN = 0;      //关闭ADC

    /* OTP校准：读取芯片出厂校准值 */
    int ostrim = ((* (volatile uint32_t *)(0x00800084)) >> 16) & 0x3FFF;   //偏移校准
    int gain_trim = (* (volatile uint32_t *)(0x00800084)) & 0x3FFF;        //增益校准

    /* 应用OTP校准：减去偏移量后乘以增益系数，得到修正后的温度传感器编码和VAON编码 */
    int code_t_trim = (tsensor_code - ostrim) * (((float)gain_trim) / 4096);
    *code_vaon_trim = (vaon_code - ostrim) * (((float)gain_trim) / 4096);

    /* 利用预拟合二次曲线计算VAON电压 */
    return TouchDenoiseFactor.a * code_t_trim * code_t_trim + TouchDenoiseFactor.b * code_t_trim + TouchDenoiseFactor.c;
}
#endif
