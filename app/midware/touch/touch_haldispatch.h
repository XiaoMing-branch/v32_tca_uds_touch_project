/**
*****************************************************************************
* @brief  touch haldispatch header
* @file   touch_haldispatch.h
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

#ifndef TOUCH_HALDISPATCH_H__
#define TOUCH_HALDISPATCH_H__

#include "si_include.h"
#include "touch_halnode.h"
#include "touch_config.h"

#if !defined(TOUCH_CENTER_FILTER_BUFLEN)
    #define TOUCH_CENTER_FILTER_BUFLEN  3
#endif
typedef struct
{
    uint8_t enable;             //1表示开启
    uint8_t discardMinvNum;     //丢弃最小值个数
    uint8_t discardMaxvNum;     //丢弃最大值个数
    uint8_t buflen;
    T_SiData buf[TOUCH_CENTER_FILTER_BUFLEN];
} TOUCH_HalCenterFilter_Type;   //中值滤波

#define TOUCH_MAX_HALSCANERS        1
typedef struct
{
    uint8_t scaner_len;         //最多TOUCH_MAX_HALSCANERS
    struct
    {
        uint8_t channel_len;                             //通道表长度
        const TOUCH_HalChConfig_Type *channel_table;     //待扫描通道表
        const TOUCH_HalGainer_Type *gainer_table;        //待扫描通道增益器表，NULL表示不使用增益器，gainer_table和sleep_gainer_table需配对使用
        const TOUCH_HalGainer_Type *sleep_gainer_table;  //低功耗模式下待扫描通道增益器表，NULL表示不使用增益器，仅被sleep_channelmask非屏蔽的通道使用
        TOUCH_HalDoubleSamp_Type *double_samp_table;     //倍采样功能配置表,NULL表示不使用
        TOUCH_HalNoiseAvoid_Type *noise_avoid_table;     //噪音抑制功能配置表,NULL表示不使用,注意：噪音检测通道不要使用本功能
        TOUCH_HalNoiseAvoid_Type *sleep_noise_avoid_table;//低功耗模式下噪音抑制功能配置表,NULL表示不使用,注意：噪音检测通道不要使用本功能
        TOUCH_HalCenterFilter_Type *center_filter_table;    //中值滤波配置表,NULL表示不使用,注意：噪音检测通道不要使用本功能
        TOUCH_HalCenterFilter_Type *sleep_center_filter_table;  //低功耗模式下中值滤波配置表,NULL表示不使用,注意：噪音检测通道不要使用本功能
        TOUCH_HalLowPassFilter_Type *low_pass_table;     //低通滤波器配置表,NULL表示不使用,注意：噪音检测通道不要使用本功能
        const int *sleep_offset_table;                   //低功耗模式下补偿表，NULL表示不补偿，补偿值可正可负
        uint32_t sleep_channelmask;                      //低功耗模式下，通道屏蔽掩码，被屏蔽的通道会以sleep_mask_freq频率扫描
        uint32_t iodisable_channelmask;                  //对应通道采集完成后会被配置为iodisable状态，实现近似不采集的通道接地功能
        uint32_t sleep_iodisable_channelmask;            //低功耗模式下，对应通道采集完成后会被配置为iodisable状态，实现近似不采集的通道接地功能
        uint16_t sleep_scan_channel_waitcycle;           //低功耗模式下每扫描一个通道等待的间隔，单位为clock，增加等待时间可降低部分噪音，但会增加功耗、降低采样率
#if TOUCH_LAZY_SCAN_ENABLE
        struct
        {
            uint16_t channelmask;                        //懒扫描通道掩码，置1的通道被延迟扫描
            uint16_t wait_timems;                        //懒扫描等待时间，单位ms
            uint16_t waitflag;                           //等待标志（运行时数据，不需要用户指定）
            uint32_t begin_timems;                       //起始时间（运行时数据，不需要用户指定）
        } lazy_scan;                                     //懒扫描
#endif
        const uint8_t *remove_residues_enable_table;     //通道抗残留使能表，为1表示对应的抗残留开启，噪音检测通道强烈建议开启抗残留
        TOUCH_HalInterface_Type *touch_node;
    } scaners[TOUCH_MAX_HALSCANERS];
} TOUCH_HalScanerTable_Type;

typedef struct TOUCH_HalDispatch_Type
{
    struct
    {
        uint8_t fast_scan;      //快扫模式，1表示快扫模式
        uint8_t in_sleep;       //是否处在低功耗模式，1表示在低功耗
    } flag;
    struct
    {
        uint16_t fast_freq;         //快扫频率
        uint16_t slow_freq;         //慢扫频率
        uint32_t last_fast_time;    //上次被设置为快扫模式时间
    } fast2slow_scan;   //快慢扫
    uint16_t sleep_freq;            //低功耗模式下扫描频率，会依据硬件具体情况选择一个接近的值

    TOUCH_HalScanerTable_Type scaner_table;     //扫描器配置表

    void (*run)(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param);    //param为0xA5A5A5A5时表示强制运行算法
    int (*sleep_run)(struct TOUCH_HalDispatch_Type *self);              //返回1表示可以从sleep模式中唤醒
    void (*reset)(struct TOUCH_HalDispatch_Type *self);                 //复位底层硬件模块，并继续上次trig
    uint8_t (*get_scaner)(struct TOUCH_HalDispatch_Type *self);         //获取当前扫描器编号
} TOUCH_HalDispatch_Type;      //Touch的分发器基类类型

typedef struct
{
    uint16_t key_mask;           //置1掩码的按键raw值累加组合生成新的按键
    uint16_t sign_mask;          //符号位掩码，如果为0表示正加，如果为1表示负加
    uint16_t new_key_channelno;  //新生成的按键通道号，从0开始计数，注意和已有的物理按键号不要冲突
    int raw_offset;              //按键raw值累加后-raw_offset为新虚拟按键raw值
} TOUCH_HalMergeKey_Type;

typedef struct
{
#define SLEEP_MASK_NORMAL       0
    uint16_t fast_freq;             //快扫频率
    uint16_t slow_freq;             //慢扫频率
    uint16_t sleep_freq;            //低功耗模式下扫描频率，会依据硬件具体情况选择一个接近的值
    uint16_t scan_firstchannel_waitcycle;       //常规模式下，扫描第一个通道等待的间隔，单位为clock，增加等待时间可降低部分噪音，但会降低采样频率
    uint16_t scan_channel_waitcycle;            //每扫描一个通道等待的间隔，单位为clock，增加等待时间可降低部分噪音，但会降低采样频率
    uint16_t sleep_scan_firstchannel_waitcycle; //低功耗模式下，扫描第一个通道等待的间隔，单位为clock，增加等待时间可降低部分噪音，但会增加功耗
    uint16_t sleep_mask_freq;       //被mask_channel_freq屏蔽的通道扫描频率，单位：sleep模式下每间隔sleep_mask_freq次，扫描一次mask通道，如值为2，表示每间隔2次低功耗采样数据，采集一次mask通道数据
    uint16_t skip_poweron_datas;    //上电时丢弃几个不稳定数据
    uint16_t fast_remove_residues_enable; //快速抗残留使能标志位
#define TOUCH_SCAN_ALL        0     //分发器一次扫描完所有通道
#define TOUCH_SCAN_MANY       1   //分发器一次扫描多个通道，多个由scan_many_num设置
    uint16_t scan_type;     //扫描类型
    uint16_t scan_many_num; //当scan_type配置为TOUCH_SCAN_MANY时，一次扫描几个通道
    uint16_t noise_force_wakeup;     //置1时，检测到噪音时，强制从低功耗唤醒
} TOUCH_HalDispatch_OnlyTouchPara_Type; //仅touch分发器，参数类型
typedef struct
{
    uint8_t cur_scaner;               //当前扫描器
    uint8_t cur_scaner_channel;       //当前扫描器的通道编号
    uint8_t scan_done;                //扫描完1个周期

    uint16_t skip_poweron_cnt;        //跳过上电时不稳定数据计数器

    T_SiData sleep_ondemand_lastdata; //低功耗按需扫描上一次值，这个值需要是低功耗唤醒通道值
    uint32_t sleep_scancnt;           //低功耗扫描计数器

    uint32_t lastSampTime;            //上次采样时间

    struct
    {
        uint8_t fsm;                  //状态机，0表示初始状态
        uint8_t last_scaner;          //上次扫描器
        uint8_t last_scaner_channel;  //上次扫描器的通道编号
    } scan_many;                      //扫描多个通道

    uint8_t skip_lograwdata_normalcnt;//飞低功耗模式下跳过打印原始数据计数器
    uint8_t skip_lograwdata_sleepcnt;   //低功耗模式下跳过打印原始数据计数器
} TOUCH_HalDispatch_OnlyTouchData_Type; //仅touch分发器，运行时临时数据缓冲区
typedef struct
{
    TOUCH_HalDispatch_Type super;       //父类
    TOUCH_HalDispatch_OnlyTouchPara_Type para;      //分发器参数
    TOUCH_HalDispatch_OnlyTouchData_Type data;      //分发器运行时数据缓冲区
} TOUCH_HalDispatch_OnlyTouch_Type;     //分发器中仅touch工作，touch独占adc

extern volatile uint16_t touchHaltMonitorCnt;               //触摸低功耗监控计数器超时
extern T_SiAlgoObject touchAlgoObject;          //触摸算法对象

void Touch_HalDispatch_SetFastScan(TOUCH_HalDispatch_Type *dispatch, uint8_t fast);
#define Touch_HalDispatch_GetFastScan(dispatch)         ((dispatch)->flag.fast_scan)
#define Touch_HalDispatch_SetInSleep(dispatch,sleep)    do{     \
                                                            (dispatch)->flag.in_sleep = (sleep);    \
                                                          }while(0)
#define Touch_HalDispatch_GetInSleep(dispatch)          (((dispatch)->flag.in_sleep) != 0)
int Touch_HalDispatch_GetChannelNum(struct TOUCH_HalDispatch_Type *dispatch);   //获取touch通道总数

TOUCH_HalDispatch_Type *Touch_HalDispatchOnlyTouchCreate(TOUCH_HalDispatch_OnlyTouch_Type *nd, const TOUCH_HalScanerTable_Type *scaner, const TOUCH_HalDispatch_OnlyTouchPara_Type *para);

void Touch_HalDispatch_DiscardSampData(int cnt);    //丢弃非低功耗采样数据，以每次扫描周期计数

#endif
