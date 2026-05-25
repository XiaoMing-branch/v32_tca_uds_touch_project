/**
*****************************************************************************
* @brief  touch halnode header
* @file   touch_halnode.h
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

#ifndef TOUCH_HALNODE_H__
#define TOUCH_HALNODE_H__

#include "tcae10.h"
#include "tcae10_ll_captouch.h"
#include "tcae10_ll_adc.h"
#include "tc_usermsg.h"
#include "si_include.h"
#include "touch_config.h"

typedef enum
{
    TOUCH_TRIG_SOFTWARE = 0,
    TOUCH_TRIG_TINYWORK
} TOUCH_HALTRIG_TYPE;

typedef struct
{
    uint16_t init_time;       //初始时间，0x0-0xFFF，当为0时，使用TOUCH_HalConfig_Type中的init_time，此值越大低功耗下功耗越高，设置太小会导致电放不干净而出现低功耗时信号阶跃下降
    uint8_t tran_loop;        //电荷转移次数
    uint8_t tran_time;        //电荷转移时间，0x0-0xFF
    uint8_t chg_time;         //充放电时间，0x0-0xFF
    struct
    {
        uint8_t enable;       //pga使能
        uint8_t pga_gain_sel; //pga运放倍数，0 - 15
        uint8_t buf_en;       //buffer开关
        uint8_t vcr_en;       //VCR开关
        adc_vcr_e vcr_sel; //VCR电压选择，偏置电压
    } pga;
} TOUCH_HalGainer_Type;       //增益器类型

typedef struct
{
    uint8_t channel;          //通道编号，CHANNEL_0 - CHANNEL_4
    uint8_t cref_sel;         //电荷转移方向选择，0表示cref->pad（cref电容<pad电容时使用），1相反
    Captouch_Mode_t captouch_mode; //充放电模式，支持CHARGE_MODE - CHARGE_DISCHARGE_BALANCE_MODE
    struct
    {
        uint8_t shld_en;      //防护开关
        uint8_t shld_sel;     //防护通道，CHANNEL_0_AS_SHIELD - SOURCE_FOLLOW_SHIELD
        uint8_t shld_i;       //防护电流，0 - 7，Iout = bit2*320u + bit1*160u+bit0*80u，仅在SOURCE_FOLLOW_SHIELD有用
    } shield;                 //防护通道配置
    struct
    {
        uint8_t cmp_en;       //补偿总开关
        uint8_t idac_inp;     //充电补偿电流，0x0-0xFF
        uint8_t idac_p_en;    //充电补偿开关
        uint8_t idac_inn;     //放电补偿电流，0x0-0xFF
        uint8_t idac_n_en;    //放电补偿开关
        uint8_t idac_time;    //补偿时间，0x0-0xFF
    } compensate;             //寄生电容补偿，补偿电容=补偿时间*补偿电流
} TOUCH_HalChConfig_Type;     //通道配置类型

typedef struct
{
    uint16_t init_time;       //初始时间，0x0-0xFFF
    uint8_t hop_period;       //跳频，0-7
    uint8_t clock_divider;    //时钟分频
} TOUCH_HalConfig_Type;         //touch配置类型

typedef struct
{
    uint8_t clock_divider;      //时钟分频
    adc_vcm_e vcm_sel;          //VCM，共模输入电压
    adc_ibais_e i_sel;          //ibias
    adc_vref_e vref_sel;        //Vref参考电压选择
    uint8_t samp_cycle;         //采样周期
    uint16_t init_cycle;        //init周期
} TOUCH_HalAdcConfig_Type;      //adc配置类型

typedef struct TOUCH_HalInterface_Type
{
    void *rself;
    volatile int ready;                                                                           //1表示准备就绪，可以读取touch数据，数据类型不要随意修改，应使用运算最快的类型
    void (*low_init)(struct TOUCH_HalInterface_Type *self);                                       //相关寄存器初始化
    void (*set_channel)(struct TOUCH_HalInterface_Type *self, const TOUCH_HalChConfig_Type *channel_cfg);//设置当前采集的通道参数
    void (*set_gainer)(struct TOUCH_HalInterface_Type *self, const TOUCH_HalGainer_Type *gainer); //设置增益器
    void (*set_ioenable)(struct TOUCH_HalInterface_Type *self, uint8_t channel, uint8_t enable);  //设置对应touch通道是否接地，enable为1表示正常touch模式，为0表示接地模式
    void (*trig)(struct TOUCH_HalInterface_Type *self, TOUCH_HALTRIG_TYPE t, uint32_t para);      //trig，para为0表示不触发adc采集
    T_SiData(*get_data)(struct TOUCH_HalInterface_Type *self);                                   //获取touch的rawdata
} TOUCH_HalInterface_Type;      //Touch的Hal层接口类型

typedef struct
{
    uint32_t imr;
    uint32_t ctrl0;
    uint32_t ctrl1;
    uint32_t ctrl2;
    uint32_t ctrl3;
    uint32_t ctrl_ana;
} TOUCH_AdcBkupReg_Type;            //Touch的adc模式时，备份的配置寄存器

typedef struct
{
    TOUCH_HalInterface_Type interface;
    TOUCH_HalConfig_Type touchcfg;
    TOUCH_HalAdcConfig_Type adccfg;
    Captouch_Mode_t captouch_mode; //充放电模式，支持CHARGE_MODE - CHARGE_DISCHARGE_BALANCE_MODE
    uint8_t shld_en;               //shield通道使能
    TOUCH_AdcBkupReg_Type adc_bkup_reg;
} TOUCH_HalCharge_Type;            //Touch的Hal层charge类型

extern volatile TOUCH_HalInterface_Type *current_touch_node;    //当前touch节点
extern volatile int current_touch_trigadc;          //当前touch是否触发adc采集

#if TOUCH_EXTSYNC_TRIG_ENABLE       //外部同步触发
typedef struct
{
    uint8_t enable;     //使能同步
    uint8_t sync_cnt;   //同步计数器
} TOUCH_ExtSync_Type;
extern volatile TOUCH_ExtSync_Type touch_extsync;
#endif

void Touch_HalInterface_SetReady(TOUCH_HalInterface_Type *interface, int v);
#define Touch_HalInterface_GetReady(interface)  ((interface)->ready)

TOUCH_HalInterface_Type *Touch_HalChargeCreate(TOUCH_HalCharge_Type *nd, const TOUCH_HalConfig_Type *touchcfg, const TOUCH_HalAdcConfig_Type *adccfg);

//void Touch_HalGainerInit(TOUCH_HalGainer_Type *gainer);     //根据增益器参数，初始化内部使用数据

#endif
