/**
*****************************************************************************
* @brief  touch haldispatch source
* @file   touch_haldispatch.c
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
#include "tc.h"
#include "tc_log.h"
#include "si_include.h"
#include "touch_tool.h"
#include "touch_haldispatch.h"
#include "touch_config.h"
#include "si_touch_port.h"
#include "tc_halt.h"
#include "tcae10_ll_def.h"

const static char *TAG = "TOUCH_HALDISPATCH";

volatile uint16_t touchHaltMonitorCnt = 0;      // 触摸低功耗监控计数器超时，初始化为0

T_SiAlgoObject touchAlgoObject = {0};           // 触摸算法对象，初始化为0

static int discardTouchSampDataCnt = 0;  //丢弃touch采样数据

static __INLINE uint16_t touch_haldispatchonlytouch_channelnum(TOUCH_HalDispatch_Type *self, uint8_t scaner, uint8_t scaner_chanel) //将scaner和scaner_chanel转换成从0开始的通道号
{
    uint8_t i;
    uint16_t channel = scaner_chanel;

    for (i = 0; i < scaner; ++i)
    {
        channel += self->scaner_table.scaners[i].channel_len;
    }
    return channel;
}

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
static __INLINE T_SiData calc_center_filter(TOUCH_HalCenterFilter_Type *center_filter_table, uint8_t channel, T_SiData cur_data, int is_sleep)    //计算噪音抑制器值，is_sleep为1表示为sleep通道
{
    T_SiData rt_data = cur_data;
    (void)is_sleep;

    if (center_filter_table[channel].enable != 0)
    {
        if (center_filter_table[channel].buflen == 0)
        {
            center_filter_table[channel].buflen = TOUCH_CENTER_FILTER_BUFLEN;
            for (int i = 0; i < TOUCH_CENTER_FILTER_BUFLEN; ++i)
            {
                center_filter_table[channel].buf[i] = cur_data;
            }
        }

        for (int i = 0; i < TOUCH_CENTER_FILTER_BUFLEN - 1; ++i)
        {
            center_filter_table[channel].buf[i] = center_filter_table[channel].buf[i + 1];
        }
        center_filter_table[channel].buf[TOUCH_CENTER_FILTER_BUFLEN - 1] = cur_data;

        rt_data = CalcMinMaxAvg(TOUCH_CENTER_FILTER_BUFLEN, center_filter_table[channel].buf, center_filter_table[channel].discardMinvNum, center_filter_table[channel].discardMaxvNum);
    }

    return rt_data;
}
#endif

static __INLINE int calc_can_remove_residues(const struct TOUCH_HalDispatch_Type *self, const TOUCH_HalDispatch_OnlyTouch_Type *rself)
{
    int remove_residues_flag = 0;

    if (rself->para.sleep_mask_freq != 0)           //全扫通道一次过后，需要进行一次抗残留数据采集
    {
        //若低功耗扫描通道和正常运行扫描一样时，不需要抗残留，否则需要抗残留
        for (uint32_t i = 0; i < self->scaner_table.scaner_len; ++i)
        {
            if ((self->scaner_table.scaners[i].sleep_channelmask != 0) || self->scaner_table.scaners[i].gainer_table)      //有通道被屏蔽，或者配置了增益表，开启抗残留
            {
                remove_residues_flag = 1;
            }
        }
    }

    return remove_residues_flag;
}

static void touch_haldispatchonlytouch_run(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param);     //run方法，虚函数
static void touch_haldispatchonlytouch_run_scanmany(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param);     //扫描多个的run方法，虚函数
static int touch_haldispatchonlytouch_scanmany_channel(struct TOUCH_HalDispatch_Type *self, int forceEnd);       //所有通道扫描结束，返回1，否则返回0，forceEnd为1表示强制扫描完一轮
static int touch_haldispatchonlytouch_sleep_run(struct TOUCH_HalDispatch_Type *self);                           //sleep_run方法，虚函数
void touch_haldispatchonlytouch_reset(struct TOUCH_HalDispatch_Type *self);                                     //复位底层硬件模块，并继续上次trig，虚函数
static uint8_t touch_haldispatchonlytouch_get_scaner(struct TOUCH_HalDispatch_Type *self);                      //获取当前扫描器编号，虚函数

static int calc_can_scan_mask(TOUCH_HalDispatch_OnlyTouch_Type *self);     //计算sleep下的mask通道是否可以被扫描
static void touch_haldispatchonlytouch_setup(struct TOUCH_HalDispatch_Type *self);   //预备
//低功耗模式下重新计算touch值，对touch_data应用偏置、倍采样、噪音抑制等功能
static T_SiData recalc_touchdata_insleep(T_SiData touch_data, int can_scan_mask, const struct TOUCH_HalDispatch_Type *self, const TOUCH_HalDispatch_OnlyTouch_Type *rself);
static void sleep_remove_residues_run(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself);       //运行低功耗数据抗残留，丢弃几轮采样值
static void normal_remove_residues_run(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself);      //运行常规数据抗残留，丢弃几轮采样值

//static void rescan_sleep_touchdata(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself);    //重新采集一轮低功耗数据

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
    static void double_samp_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);    //清倍采样缓冲区，only_sleep为1表示仅复位sleep数据
    static void noise_avoid_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);    //清噪音抑制器缓冲区，only_sleep为1表示仅复位sleep数据
    static void center_filter_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);  //清中值滤波缓冲区，only_sleep为1表示仅复位sleep数据
    static void low_pass_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);       //清低通滤波器缓冲区，only_sleep为1表示仅复位sleep数据
#endif

#if TOUCH_LAZY_SCAN_ENABLE
    static void touch_lazy_config(TOUCH_HalDispatch_OnlyTouch_Type *self, int scanerid);    //懒扫描配置
    void touch_lazy_scan_completed(int scanerid);       //懒扫描完成
#endif

static void set_sleep_all_ioenable(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself, uint8_t enable); //打开或关闭所有touch通道接地功能
static __INLINE void set_sleep_ioenable_asneed(const TOUCH_HalScanerTable_Type *scaner, uint8_t scaner_index, uint8_t logicchannel) //将有需要的通道配置为正常
{
    if (scaner->scaners[scaner_index].sleep_iodisable_channelmask != 0)
    {
        //所有被扫描的通道都要打开，否则共享的通道在某些情况下会出问题，比如低功耗下共享模式扫描通道12，1接地2不接地，1在接地时，会把共享通道关闭，导致2无法正常工作，除非2使能共享通道
        //if (scaner->scaners[scaner_index].sleep_iodisable_channelmask & (0x1U << logicchannel))
        {
            TOUCH_HalInterface_Type *touch_node = scaner->scaners[scaner_index].touch_node;
            touch_node->set_ioenable(touch_node, scaner->scaners[scaner_index].channel_table[logicchannel].channel, 1);
        }
    }
}
static __INLINE void set_sleep_iodisable_asneed(const TOUCH_HalScanerTable_Type *scaner, uint8_t scaner_index, uint8_t logicchannel) //将有需要的通道配置为接地
{
    if ((scaner->scaners[scaner_index].sleep_iodisable_channelmask & ((uint32_t)0x1U << logicchannel)) != 0)
    {
        TOUCH_HalInterface_Type *touch_node = scaner->scaners[scaner_index].touch_node;
        touch_node->set_ioenable(touch_node, scaner->scaners[scaner_index].channel_table[logicchannel].channel, 0);
    }
}

static void set_all_ioenable(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself, uint8_t enable); //打开或关闭所有touch通道接地功能
static __INLINE void set_ioenable_asneed(const TOUCH_HalScanerTable_Type *scaner, uint8_t scaner_index, uint8_t logicchannel) //将有需要的通道配置为正常
{
    if (scaner->scaners[scaner_index].iodisable_channelmask != 0)
    {
        //所有被扫描的通道都要打开，否则共享的通道在某些情况下会出问题，比如低功耗下共享模式扫描通道12，1接地2不接地，1在接地时，会把共享通道关闭，导致2无法正常工作，除非2使能共享通道
        //if (scaner->scaners[scaner_index].iodisable_channelmask & (0x1U << logicchannel))
        {
            TOUCH_HalInterface_Type *touch_node = scaner->scaners[scaner_index].touch_node;
            touch_node->set_ioenable(touch_node, scaner->scaners[scaner_index].channel_table[logicchannel].channel, 1);
        }
    }
}
static __INLINE void set_iodisable_asneed(const TOUCH_HalScanerTable_Type *scaner, uint8_t scaner_index, uint8_t logicchannel) //将有需要的通道配置为接地
{
    if ((scaner->scaners[scaner_index].iodisable_channelmask & ((uint32_t)0x1U << logicchannel)) != 0)
    {
        TOUCH_HalInterface_Type *touch_node = scaner->scaners[scaner_index].touch_node;
        touch_node->set_ioenable(touch_node, scaner->scaners[scaner_index].channel_table[logicchannel].channel, 0);
    }
}

static void touch_haldispatchonlytouch_ctor(TOUCH_HalDispatch_OnlyTouch_Type *self);         //构造函数
static __INLINE void touch_haldispatchonlytouch_nextchannel(TOUCH_HalDispatch_OnlyTouch_Type *self)
{
    if (++self->data.cur_scaner_channel >= self->super.scaner_table.scaners[self->data.cur_scaner].channel_len)
    {
        self->data.cur_scaner_channel = 0;
        ++self->data.cur_scaner;
    }
};

static __INLINE void delay_clock(uint32_t clk)      //延迟一段时间，单位clock
{
    for (uint32_t i = 0; i < clk; ++i)
    {
        __NOP();
    }
}

void Touch_HalDispatch_SetFastScan(TOUCH_HalDispatch_Type *dispatch, uint8_t fast)
{
    dispatch->flag.fast_scan = fast;
    if (fast != 0)
    {
        dispatch->fast2slow_scan.last_fast_time = TouchGetTime();
    }
}

int Touch_HalDispatch_GetChannelNum(struct TOUCH_HalDispatch_Type *dispatch)   //获取touch通道总数
{
    int channelNum = 0;

    for (int i = 0; i < dispatch->scaner_table.scaner_len; ++i)
    {
        channelNum += dispatch->scaner_table.scaners[i].channel_len;
    }

    return channelNum;
}

TOUCH_HalDispatch_Type *Touch_HalDispatchOnlyTouchCreate(TOUCH_HalDispatch_OnlyTouch_Type *nd, const TOUCH_HalScanerTable_Type *scaner, const TOUCH_HalDispatch_OnlyTouchPara_Type *para)
{
    uint8_t sleep_channelmask_all0 = 1;

    if (para->fast_freq == 0 || para->slow_freq == 0 || para->sleep_freq == 0)
    {
        TC_LOGE(TAG, "illegal freq 0");
    }
    for (uint32_t i = 0; i < scaner->scaner_len; ++i)
    {
        if (!scaner->scaners[i].gainer_table || !scaner->scaners[i].sleep_gainer_table)    //gainer_table和sleep_gainer_table不能为空
        {
            TC_LOGE(TAG, "gainer_table and sleep_gainer_table not == NULL");
        }

        if (scaner->scaners[i].sleep_channelmask)
        {
            sleep_channelmask_all0 = 0;
        }

#if TOUCH_REDUCED_MODE      //精简模式
        if (scaner->scaners[i].double_samp_table)
        {
            TC_LOGE(TAG, "not support double_samp_table in reduced mode");
        }
        if (scaner->scaners[i].noise_avoid_table)
        {
            TC_LOGE(TAG, "not support noise_avoid_table in reduced mode");
        }
        if (scaner->scaners[i].sleep_noise_avoid_table)
        {
            TC_LOGE(TAG, "not support sleep_noise_avoid_table in reduced mode");
        }
        if (scaner->scaners[i].low_pass_table)
        {
            TC_LOGE(TAG, "not support low_pass_table in reduced mode");
        }
#endif
    }

#if TOUCH_SLEEP_MASK_FREQ==0
    if (sleep_channelmask_all0)     //低功耗唤醒通道和扫描通道共用一个touch数据
    {
        for (uint32_t i = 0; i < scaner->scaner_len; ++i)
        {
            if (memcmp(scaner->scaners[i].gainer_table, scaner->scaners[i].sleep_gainer_table, sizeof(*scaner->scaners[i].gainer_table)*scaner->scaners[i].channel_len) != 0)
            {
                TC_LOGE(TAG, "(TOUCH_SLEEP_MASK_FREQ==0 && sleep_channelmask==0) but gainer_table!=sleep_gainer_table");
            }
            if (scaner->scaners[i].iodisable_channelmask != scaner->scaners[i].sleep_iodisable_channelmask)
            {
                TC_LOGE(TAG, "(TOUCH_SLEEP_MASK_FREQ==0 && sleep_channelmask==0) but iodisable_channelmask!=sleep_iodisable_channelmask");
            }
        }
    }
#else
    (void)sleep_channelmask_all0;
#endif

#if !(TOUCH_REDUCED_MODE)           //非精简模式
    //初始化double_samp_table
    double_samp_reset(scaner, 0);
    //初始化noise_avoid_table
    noise_avoid_reset(scaner, 0);
    //初始化center_filter_table
    center_filter_reset(scaner, 0);
    //初始化low_pass_table
    low_pass_reset(scaner, 0);
#endif

    memcpy(&nd->super.scaner_table, scaner, sizeof(*scaner));
    memcpy(&nd->para, para, sizeof(*para));

    nd->super.fast2slow_scan.last_fast_time = TouchGetTime();
    nd->super.fast2slow_scan.fast_freq = nd->para.fast_freq;
    nd->super.fast2slow_scan.slow_freq = nd->para.slow_freq;
    nd->super.sleep_freq = nd->para.sleep_freq;

    if (para->scan_type == TOUCH_SCAN_MANY)
    {
#if TOUCH_REDUCED_MODE  //精简模式
        TC_LOGE(TAG, "not support scan_many_num in reduced mode");
#else
        nd->super.run = touch_haldispatchonlytouch_run_scanmany;
        if (para->scan_many_num <= 0)
        {
            TC_LOGE(TAG, "scan_many_num != 0");
        }
#endif
    }
    else
    {
        nd->super.run = touch_haldispatchonlytouch_run;
    }
    nd->super.sleep_run = touch_haldispatchonlytouch_sleep_run;
    nd->super.reset = touch_haldispatchonlytouch_reset;
    nd->super.get_scaner = touch_haldispatchonlytouch_get_scaner;

    touch_haldispatchonlytouch_ctor(nd);

    return (TOUCH_HalDispatch_Type *)nd;
}

static void touch_haldispatchonlytouch_ctor(TOUCH_HalDispatch_OnlyTouch_Type *self)          //构造函数
{
    uint8_t i, j;
    self->super.flag.fast_scan = 1;
    self->super.flag.in_sleep = 0;

    memset(&self->data, 0x0, sizeof(self->data));
    self->data.scan_done = 1;

    for (i = 0; i < self->super.scaner_table.scaner_len; ++i)           //touch io pinmux配置
    {
#if TOUCH_LAZY_SCAN_ENABLE
        self->super.scaner_table.scaners[i].lazy_scan.waitflag = 0;
#endif
        for (j = 0; j < self->super.scaner_table.scaners[i].channel_len; ++j)
        {
#if TOUCH_LAZY_SCAN_ENABLE
            if (self->super.scaner_table.scaners[i].lazy_scan.channelmask & (0x1 << j))     //懒初始化
            {
                self->super.scaner_table.scaners[i].lazy_scan.waitflag = 1;
                self->super.scaner_table.scaners[i].lazy_scan.begin_timems = TouchGetTime();
                continue;
            }
#endif
            Touch_IOConfig(self->super.scaner_table.scaners[i].channel_table[j].channel);
            if (self->super.scaner_table.scaners[i].channel_table[j].shield.shld_en)
            {
                if (self->super.scaner_table.scaners[i].channel_table[j].shield.shld_sel == SOURCE_FOLLOW_SHIELD || self->super.scaner_table.scaners[i].channel_table[j].shield.shld_sel == SHIELD_PIN_AS_SHIELD)
                {
                    ll_gpio_afio_config(GPIO_PIN_1, (gpio_afio_mux_e)GPIO1_SOFTWARE_INPUT_FUNCTION_CAP_SHIELD);
                }
                else
                {
                    Touch_IOConfig(self->super.scaner_table.scaners[i].channel_table[j].shield.shld_sel);
                }
            }
        }
    }
    touch_haldispatchonlytouch_setup(&self->super);         //父类预备
}

static void touch_haldispatchonlytouch_run(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param)      //run方法
{
    uint16_t i;
    T_SiData touch_data;
    TOUCH_HalInterface_Type *touch_node;
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    if (msg == MSG_TASK_TIMER)              //定时消息
    {
        uint16_t freq = self->fast2slow_scan.fast_freq;         //获取扫描频率
        int timeout = (TouchGetTime() - rself->data.lastSampTime >= SiIFastDiv(1000U, freq)) ? 1 : 0;
        if (rself->data.lastSampTime == 0 || timeout != 0 || ((uint32_t)param) == 0xA5A5A5A5) //开始下一轮扫描
        {
            rself->data.lastSampTime = TouchGetTime();      //记录上一次采样时间

            rself->data.cur_scaner = 0;
            rself->data.cur_scaner_channel = 0;

            delay_clock(rself->para.scan_firstchannel_waitcycle);       //扫描第一个通道延迟一段时间
            while (rself->data.cur_scaner < self->scaner_table.scaner_len)
            {
#if TOUCH_LAZY_SCAN_ENABLE
                if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                        (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
                {
                    if (TouchGetTime() - self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.begin_timems > self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.wait_timems)  //开始扫描
                    {
                        touch_lazy_config(rself, rself->data.cur_scaner);
                    }
                    touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
                    continue;
                }
#endif

                touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
                touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
                set_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
                if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
                {
                    touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].gainer_table[rself->data.cur_scaner_channel]);
                }

                //按需运行抗残留
                if (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table != NULL && (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table[rself->data.cur_scaner_channel] != 0))
                {
                    touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
                    //等待touch采集完成
                    i = 0;
                    while (!(TcTaskGetBitFlag(currentTask) != 0) && i < 0xFFFFU)      //等待采集完成，用TcTaskGetBitFlag判断，不要用Touch_HalInterface_GetReady，否则噪音会增大，具体原因还未分析出
                    {
                        ++i;
                    }
                    TcTaskClrBitFlag(currentTask, TcTaskGetBitFlag(currentTask));       //清任务标记位
                    touch_node->get_data(touch_node);      //获取touch数据
                    delay_clock(rself->para.scan_channel_waitcycle);       //延迟一段时间
                    //普通模式，无功耗限制，进一步运行抗残留
                    touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
                    if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
                    {
                        touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].gainer_table[rself->data.cur_scaner_channel]);
                    }
                }
                //采集数据
                touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
                //等待touch采集完成
                i = 0;
                while (!(TcTaskGetBitFlag(currentTask) != 0) && i < 0xFFFFU)      //等待采集完成，用TcTaskGetBitFlag判断，不要用Touch_HalInterface_GetReady，否则噪音会增大，具体原因还未分析出
                {
                    ++i;
                }
                if (i < 0xFFFFU)
                {
                    TcTaskClrBitFlag(currentTask, TcTaskGetBitFlag(currentTask));       //清任务标记位

                    touch_data = touch_node->get_data(touch_node);      //获取touch数据
                    set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
                    TOUCH_HalCenterFilter_Type *center_filter_table = self->scaner_table.scaners[rself->data.cur_scaner].center_filter_table;
                    if (center_filter_table != NULL)              //噪音抑制器
                    {
                        touch_data = calc_center_filter(center_filter_table, rself->data.cur_scaner_channel, touch_data, 0);
                    }
                    TOUCH_HalLowPassFilter_Type *low_pass_table = self->scaner_table.scaners[rself->data.cur_scaner].low_pass_table;
                    if (low_pass_table != NULL)              //低通滤波器
                    {
                        touch_data = calc_low_pass(low_pass_table, rself->data.cur_scaner_channel, touch_data, 0);
                    }
                    TOUCH_HalDoubleSamp_Type *double_samp_table = self->scaner_table.scaners[rself->data.cur_scaner].double_samp_table;
                    if (double_samp_table != NULL)       //计算倍采样
                    {
                        touch_data = calc_double_samp(double_samp_table, rself->data.cur_scaner_channel, touch_data, 0);    //计算倍采样值
                    }
                    TOUCH_HalNoiseAvoid_Type *noise_avoid_table = self->scaner_table.scaners[rself->data.cur_scaner].noise_avoid_table;
                    if (noise_avoid_table != NULL)              //噪音抑制器
                    {
                        touch_data = calc_noise_avoid(noise_avoid_table, rself->data.cur_scaner_channel, touch_data, 0);
                    }
#endif
                }
                else        //采集超时
                {
                    set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地

                    touch_data = SiAlgoGetRawData(&touchAlgoObject, touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel));           //取上次值作为touch_data

                    TC_LOGW(TAG, "touch node%d timeout", rself->data.cur_scaner);
                    self->reset(self);      //复位并重新触发
                }

                //将touch数据放入算法缓冲区
                i = touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel);
                SiAlgoSetRawData(&touchAlgoObject, i, touch_data);          //数据压入算法缓冲区

                delay_clock(rself->para.scan_channel_waitcycle);       //延迟一段时间
                touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
            }

            if (rself->data.skip_poweron_cnt < rself->para.skip_poweron_datas)
            {
                ++rself->data.skip_poweron_cnt;
            }
            else
            {
#if LOG_RAWDATA
                if (++rself->data.skip_lograwdata_normalcnt > LOG_RAWDATA_NORMAL_INTERVAL)
                {
                    rself->data.skip_lograwdata_normalcnt = 0;
                    TC_LOG_RAWDATA_I16(TC_LOG_RAWDATA_CAPSENSOR_NORMAL, TC_LOGRAWDATA_TOUCH_STATUS_FAST, &SiAlgoGetRawData(&touchAlgoObject, 0), Touch_HalDispatch_GetChannelNum(self)*sizeof(T_SiData));
                }
#endif
                if (discardTouchSampDataCnt > 0)
                {
                    --discardTouchSampDataCnt;
                }
                else
                {
                    SiAlgoProcress(&touchAlgoObject);               //运行Touch算法
                }
            }
        }
    }

    if (msg == MSG_TASK_ENTER_HALT)     //进入低功耗
    {
        Touch_HalDispatch_SetInSleep(self, 1);

        set_all_ioenable(self, rself, 1);           //使能全扫接地的通道
        set_sleep_all_ioenable(self, rself, 0);       //将所有需要接地的touch通道接地

        if (calc_can_remove_residues(self, rself) != 0)       //计算是否需要抗残留
        {
            sleep_remove_residues_run(self, rself);       //运行抗残留，丢弃几轮采样值
        }

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
        //初始化double_samp_table
        double_samp_reset(&self->scaner_table, 1);
        //初始化noise_avoid_table
        noise_avoid_reset(&self->scaner_table, 1);
        //初始化center_filter_table
        center_filter_reset(&self->scaner_table, 1);
        //初始化low_pass_table
        low_pass_reset(&self->scaner_table, 1);
#endif

        rself->data.sleep_scancnt = 0;              //低功耗扫描计数器清零
        rself->data.sleep_ondemand_lastdata = 0;    //按需扫描上次数据为0
        rself->data.skip_lograwdata_sleepcnt = 0;

        touchHaltRtcTrigFlag = 0;

        /*rself->data.cur_scaner = 0;
        rself->data.cur_scaner_channel = 0;
        touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
        TouchRtcTrigConfig(Touch_HalInterfaceTouchId(touch_node), TouchFreq2RtcCycle(self->sleep_freq), 1); */  //开启RTC中断
    }

    if (msg == MSG_TASK_WAKE_UP)        //从低功耗唤醒
    {
        set_sleep_all_ioenable(self, rself, 1);       //将所有需要接地的touch通道不接地
        set_all_ioenable(self, rself, 0);       //禁用全扫接地的通道
        rself->data.lastSampTime = 0;       //立刻采集touch

        Touch_HalDispatch_SetInSleep(self, 0);              //寄存器配置就绪后，才切换

        rself->data.skip_lograwdata_normalcnt = 0;

        /*rself->data.cur_scaner = 0;
        touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
        TouchRtcTrigConfig(Touch_HalInterfaceTouchId(touch_node), TouchFreq2RtcCycle(self->sleep_freq), 0); */  //关闭RTC中断
    }
}

static void touch_haldispatchonlytouch_run_scanmany(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param)     //扫描多个的run方法，虚函数
{
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    if (msg == MSG_TASK_TIMER)              //定时消息
    {
        uint16_t freq = self->fast2slow_scan.fast_freq;         //获取扫描频率
        int timeout = (TouchGetTime() - rself->data.lastSampTime >= SiIFastDiv(1000U, freq)) ? 1 : 0;
        if (rself->data.lastSampTime == 0 || timeout != 0 || rself->data.scan_many.fsm != 0  || ((uint32_t)param) == 0xA5A5A5A5) //开始下一轮扫描
        {
            switch (rself->data.scan_many.fsm)
            {
            case 0:
                rself->data.lastSampTime = TouchGetTime();      //记录上一次采样时间
                rself->data.cur_scaner = 0;
                rself->data.cur_scaner_channel = 0;

                delay_clock(rself->para.scan_firstchannel_waitcycle);       //扫描第一个通道延迟一段时间
                if (touch_haldispatchonlytouch_scanmany_channel(self, 0) != 0)     //扫描结束
                {
                    rself->data.scan_many.fsm = 2;
                }
                else
                {
                    rself->data.scan_many.fsm = 1;
                }

                rself->data.scan_many.last_scaner = rself->data.cur_scaner;
                rself->data.scan_many.last_scaner_channel = rself->data.cur_scaner_channel;
                break;
            case 1:
                rself->data.cur_scaner = rself->data.scan_many.last_scaner;
                rself->data.cur_scaner_channel = rself->data.scan_many.last_scaner_channel;

                if (touch_haldispatchonlytouch_scanmany_channel(self, 0) != 0)     //扫描结束
                {
                    rself->data.scan_many.fsm = 2;
                }

                rself->data.scan_many.last_scaner = rself->data.cur_scaner;
                rself->data.scan_many.last_scaner_channel = rself->data.cur_scaner_channel;
                break;
            case 2:
                if (rself->data.skip_poweron_cnt < rself->para.skip_poweron_datas)
                {
                    ++rself->data.skip_poweron_cnt;
                }
                else
                {
#if LOG_RAWDATA
                    if (++rself->data.skip_lograwdata_normalcnt > LOG_RAWDATA_NORMAL_INTERVAL)
                    {
                        rself->data.skip_lograwdata_normalcnt = 0;
                        TC_LOG_RAWDATA_I16(TC_LOG_RAWDATA_CAPSENSOR_NORMAL, TC_LOGRAWDATA_TOUCH_STATUS_FAST, &SiAlgoGetRawData(&touchAlgoObject, 0), Touch_HalDispatch_GetChannelNum(self)*sizeof(T_SiData));
                    }
#endif
                    if (discardTouchSampDataCnt > 0)
                    {
                        --discardTouchSampDataCnt;
                    }
                    else
                    {
                        SiAlgoProcress(&touchAlgoObject);               //运行Touch算法
                    }
                }
                rself->data.scan_many.fsm = 0;
                break;
            default:
                rself->data.scan_many.fsm = 0;
                break;
            }
        }
    }

    if (msg == MSG_TASK_ENTER_HALT)     //进入低功耗
    {
        if (rself->data.scan_many.fsm != 0)      //强制扫描完一轮
        {
            rself->data.scan_many.fsm = 0;
            (void)touch_haldispatchonlytouch_scanmany_channel(self, 1);
        }

        Touch_HalDispatch_SetInSleep(self, 1);

        set_all_ioenable(self, rself, 1);       //使能全扫接地的通道
        set_sleep_all_ioenable(self, rself, 0);       //将所有需要接地的touch通道接地

        if (calc_can_remove_residues(self, rself) != 0)       //计算是否需要抗残留
        {
            sleep_remove_residues_run(self, rself);       //运行抗残留，丢弃几轮采样值
        }

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
        //初始化double_samp_table
        double_samp_reset(&self->scaner_table, 1);
        //初始化noise_avoid_table
        noise_avoid_reset(&self->scaner_table, 1);
        //初始化center_filter_table
        center_filter_reset(&self->scaner_table, 1);
        //初始化low_pass_table
        low_pass_reset(&self->scaner_table, 1);
#endif

        rself->data.sleep_scancnt = 0;              //低功耗扫描计数器清零
        rself->data.sleep_ondemand_lastdata = 0;    //按需扫描上次数据为0
        rself->data.skip_lograwdata_sleepcnt = 0;

        touchHaltRtcTrigFlag = 0;

        /*rself->data.cur_scaner = 0;
        rself->data.cur_scaner_channel = 0;
        touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
        TouchRtcTrigConfig(Touch_HalInterfaceTouchId(touch_node), TouchFreq2RtcCycle(self->sleep_freq), 1); */  //开启RTC中断
    }

    if (msg == MSG_TASK_WAKE_UP)        //从低功耗唤醒
    {
        set_sleep_all_ioenable(self, rself, 1);       //将所有需要接地的touch通道不接地
        set_all_ioenable(self, rself, 0);       //禁用全扫接地的通道
        rself->data.lastSampTime = 0;       //立刻采集touch

        Touch_HalDispatch_SetInSleep(self, 0);              //寄存器配置就绪后，才切换

        rself->data.skip_lograwdata_normalcnt = 0;

        /*rself->data.cur_scaner = 0;
        touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
        TouchRtcTrigConfig(Touch_HalInterfaceTouchId(touch_node), TouchFreq2RtcCycle(self->sleep_freq), 0); */  //关闭RTC中断
    }
}

static int touch_haldispatchonlytouch_scanmany_channel(struct TOUCH_HalDispatch_Type *self, int forceEnd)    //所有通道扫描结束，返回1，否则返回0
{
    uint16_t i;
    T_SiData touch_data;
    TOUCH_HalInterface_Type *touch_node;
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    for (uint32_t scan_num = 0; (scan_num < rself->para.scan_many_num || forceEnd != 0) && rself->data.cur_scaner < self->scaner_table.scaner_len; ++scan_num)
    {
#if TOUCH_LAZY_SCAN_ENABLE
        if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
        {
            if (TouchGetTime() - self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.begin_timems > self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.wait_timems)  //开始扫描
            {
                touch_lazy_config(rself, rself->data.cur_scaner);
            }
            touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
            continue;
        }
#endif

        touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
        touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
        set_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
        if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
        {
            touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].gainer_table[rself->data.cur_scaner_channel]);
        }

        //按需运行抗残留
        if (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table && (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table[rself->data.cur_scaner_channel] != 0))
        {
            touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
            //等待touch采集完成
            i = 0;
            while (!(TcTaskGetBitFlag(touchTaskHandle) != 0) && i < 0xFFFFU)      //等待采集完成，用TcTaskGetBitFlag判断，不要用Touch_HalInterface_GetReady，否则噪音会增大，具体原因还未分析出
            {
                ++i;
            }
            TcTaskClrBitFlag(touchTaskHandle, TcTaskGetBitFlag(touchTaskHandle));       //清任务标记位
            touch_node->get_data(touch_node);      //获取touch数据
            delay_clock(rself->para.scan_channel_waitcycle);       //延迟一段时间
            //普通模式，无功耗限制，进一步运行抗残留
            touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
            if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
            {
                touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].gainer_table[rself->data.cur_scaner_channel]);
            }
        }
        //采集数据
        touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
        //等待touch采集完成
        i = 0;
        while (!(TcTaskGetBitFlag(touchTaskHandle) != 0) && i < 0xFFFFU)        //等待采集完成，用TcTaskGetBitFlag判断，不要用Touch_HalInterface_GetReady，否则噪音会增大，具体原因还未分析出
        {
            ++i;
        }

        if (i < 0xFFFFU)
        {
            TcTaskClrBitFlag(touchTaskHandle, TcTaskGetBitFlag(touchTaskHandle));       //清任务标记位

            touch_data = touch_node->get_data(touch_node);      //获取touch数据
            set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
            TOUCH_HalCenterFilter_Type *center_filter_table = self->scaner_table.scaners[rself->data.cur_scaner].center_filter_table;
            if (center_filter_table != NULL)              //噪音抑制器
            {
                touch_data = calc_center_filter(center_filter_table, rself->data.cur_scaner_channel, touch_data, 0);
            }
            TOUCH_HalLowPassFilter_Type *low_pass_table = self->scaner_table.scaners[rself->data.cur_scaner].low_pass_table;
            if (low_pass_table != NULL)              //低通滤波器
            {
                touch_data = calc_low_pass(low_pass_table, rself->data.cur_scaner_channel, touch_data, 0);
            }
            TOUCH_HalDoubleSamp_Type *double_samp_table = self->scaner_table.scaners[rself->data.cur_scaner].double_samp_table;
            if (double_samp_table != NULL)       //计算倍采样
            {
                touch_data = calc_double_samp(double_samp_table, rself->data.cur_scaner_channel, touch_data, 0);    //计算倍采样值
            }
            TOUCH_HalNoiseAvoid_Type *noise_avoid_table = self->scaner_table.scaners[rself->data.cur_scaner].noise_avoid_table;
            if (noise_avoid_table != NULL)              //噪音抑制器
            {
                touch_data = calc_noise_avoid(noise_avoid_table, rself->data.cur_scaner_channel, touch_data, 0);
            }
#endif
        }
        else        //采集超时
        {
            set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地

            touch_data = SiAlgoGetRawData(&touchAlgoObject, touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel));           //取上次值作为touch_data

            TC_LOGW(TAG, "touch node%d timeout", rself->data.cur_scaner);
            self->reset(self);      //复位并重新触发
        }

        //将touch数据放入算法缓冲区
        i = touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel);
        SiAlgoSetRawData(&touchAlgoObject, i, touch_data);          //数据压入算法缓冲区

        delay_clock(rself->para.scan_channel_waitcycle);       //延迟一段时间
        touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
    }

    return (!(rself->data.cur_scaner < self->scaner_table.scaner_len)) ? 1 : 0;       //判断扫描是否完成
}

static int touch_haldispatchonlytouch_sleep_run(struct TOUCH_HalDispatch_Type *self)                          //sleep_run方法，虚函数
{
    int can_scan_mask = 0;      //mask通道是否可以被扫描
    uint8_t sleep_channelmask_all0 = 1;
    uint8_t touch_data_valid;
    uint8_t use_wkupdata_as_touchdata;  //使用wkup数据作为touch数据
    T_SiData touch_data;
    T_SiData touch_data_bkup;
    struct
    {
        uint8_t valid;          //1表示touch数据有效
        T_SiData touch_data;
    } touch_data_wkupbufs[SI_CH_MAXNUM];
    uint32_t i;
    TOUCH_HalInterface_Type *touch_node;
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    if (touchHaltRtcTrigFlag != 0)       //准备就绪
    {
        touchHaltRtcTrigFlag = 0;       //清rtc触发标记
        touchHaltMonitorCnt = 0;        //清低功耗超时计数器

        delay_clock(rself->para.sleep_scan_firstchannel_waitcycle);     //等待芯片稳定后，再采集

        rself->data.cur_scaner = 0;
        rself->data.cur_scaner_channel = 0;
        while (rself->data.cur_scaner < self->scaner_table.scaner_len)
        {
            if (self->scaner_table.scaners[rself->data.cur_scaner].sleep_channelmask)
            {
                sleep_channelmask_all0 = 0;
            }

#if TOUCH_LAZY_SCAN_ENABLE
            if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                    (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
            {
                touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
                continue;
            }
#endif

            if ((self->scaner_table.scaners[rself->data.cur_scaner].sleep_channelmask & ((uint32_t)0x1U << rself->data.cur_scaner_channel)) != 0) //通道被屏蔽
            {
                touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
                continue;
            }

            touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;         //调用nextchannel后，更新touch_node
            touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
            set_sleep_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
            if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
            {
                touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].sleep_gainer_table[rself->data.cur_scaner_channel]);
            }

#if TOUCH_SLEEP_MASK_FREQ == 0
            //仅在TOUCH_SLEEP_MASK_FREQ为0时运行抗残留
            if (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table && (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table[rself->data.cur_scaner_channel] != 0))
            {
                touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
                //等待touch采集完成
                i = 0;
                while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
                {
                    ++i;
                }
                touch_node->get_data(touch_node);       //Touch_HalInterface_SetReady(touch_node, 0);           //清就绪标志
                delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
                //进一步运行抗残留
                touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
                if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
                {
                    touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].sleep_gainer_table[rself->data.cur_scaner_channel]);
                }
            }
#endif
            //采集数据
            touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
            i = 0;
            while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
            {
                ++i;
            }

            if (i < 0xFFFF)
            {
                touch_data = touch_node->get_data(touch_node);      //获取touch数据
                touch_data_bkup = touch_data;
                touch_data_valid = 1;
                set_sleep_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地
                touch_data = recalc_touchdata_insleep(touch_data, can_scan_mask, self, rself);      //对touch_data应用偏置、倍采样、噪音抑制等功能
            }
            else
            {
                touch_data_valid = 0;
                set_sleep_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地
                touch_data = SiAlgoGetRawData(&touchAlgoObject, touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel));           //取上次值作为touch_data
                TC_LOGE(TAG, "touch timeout");
                self->reset(self);      //复位并重新触发
            }

            i = touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel);
            SiAlgoSetRawData(&touchAlgoObject, i, touch_data);      //数据压入算法缓冲区
            touch_data_wkupbufs[i].valid = touch_data_valid;                //备份touch wkup数据
            touch_data_wkupbufs[i].touch_data = touch_data_bkup;

            delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
            touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
        }

        T_SiErrRt wkupFlag = SI_RT_OK;
#if LOG_RAWDATA
        if (++rself->data.skip_lograwdata_sleepcnt > LOG_RAWDATA_SLEEP_INTERVAL)
        {
            rself->data.skip_lograwdata_sleepcnt = 0;
            if (IsHaltMode())
            {
                TC_LOG_RAWDATA_I16(TC_LOG_RAWDATA_CAPSENSOR_SLEEP, TC_LOGRAWDATA_TOUCH_STATUS_SLEEP, &SiAlgoGetRawData(&touchAlgoObject, 0), Touch_HalDispatch_GetChannelNum(self)*sizeof(T_SiData));
            }
            else
            {
                TC_LOG_RAWDATA_I16(TC_LOG_RAWDATA_CAPSENSOR_SLEEP, TC_LOGRAWDATA_TOUCH_STATUS_SLOW, &SiAlgoGetRawData(&touchAlgoObject, 0), Touch_HalDispatch_GetChannelNum(self)*sizeof(T_SiData));
            }
        }
#endif
        wkupFlag = SiAlgoProcress(&touchAlgoObject);        //处理低功耗object

        if (wkupFlag == SI_RT_WKUP || (rself->para.noise_force_wakeup && SiNoiseIsDetect4(&touchAlgoObject)))                             //检测到有效按键，或噪音强制唤醒
        {
            TC_LOGD(TAG, "touch wake up");
            return 1;
        }

        can_scan_mask = calc_can_scan_mask(rself);      //判断是否能扫描mask通道
        if (can_scan_mask != 0)
        {
            rself->data.sleep_scancnt = 0;
        }
        else
        {
            ++rself->data.sleep_scancnt;
            return 0;
        }

        //执行全扫流程
        use_wkupdata_as_touchdata = 0;
#if TOUCH_SLEEP_MASK_FREQ == 0
        use_wkupdata_as_touchdata = sleep_channelmask_all0;
#else
        (void)sleep_channelmask_all0;
#endif

        if (!use_wkupdata_as_touchdata)
        {
            set_sleep_all_ioenable(self, rself, 1);
            set_all_ioenable(self, rself, 0);       //禁用全扫接地的通道
        }
        if (!use_wkupdata_as_touchdata && calc_can_remove_residues(self, rself) != 0)       //计算是否需要抗残留
        {
            normal_remove_residues_run(self, rself);       //运行抗残留，丢弃几轮采样值
        }

        if (use_wkupdata_as_touchdata)      //用wkup数据作为touch数据
        {
            rself->data.cur_scaner = 0;
            rself->data.cur_scaner_channel = 0;
            while (rself->data.cur_scaner < self->scaner_table.scaner_len)
            {
#if TOUCH_LAZY_SCAN_ENABLE
                if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                        (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
                {
                    if (TouchGetTime() - self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.begin_timems > self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.wait_timems)  //开始扫描
                    {
                        touch_lazy_config(rself, rself->data.cur_scaner);
                    }
                    touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
                    continue;
                }
#endif

                i = touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel);
                if (touch_data_wkupbufs[i].valid)
                {
                    touch_data = touch_data_wkupbufs[i].touch_data;      //使用wkup数据
                    touch_data = recalc_touchdata_insleep(touch_data, can_scan_mask, self, rself);      //对touch_data应用偏置、倍采样、噪音抑制等功能
                }
                else
                {
                    touch_data = SiAlgoGetRawData(&touchAlgoObject, touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel));           //取上次值作为touch_data
                }
                SiAlgoSetRawData(&touchAlgoObject, i, touch_data);      //数据压入算法缓冲区

                touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
            }
        }
        else
        {
            rself->data.cur_scaner = 0;
            rself->data.cur_scaner_channel = 0;
            while (rself->data.cur_scaner < self->scaner_table.scaner_len)
            {
#if TOUCH_LAZY_SCAN_ENABLE
                if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                        (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
                {
                    if (TouchGetTime() - self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.begin_timems > self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.wait_timems)  //开始扫描
                    {
                        touch_lazy_config(rself, rself->data.cur_scaner);
                    }
                    touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
                    continue;
                }
#endif

                touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;         //调用nextchannel后，更新touch_node
                touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
                set_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
                if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
                {
                    touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].gainer_table[rself->data.cur_scaner_channel]);
                }

                //按需运行抗残留
                if (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table && (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table[rself->data.cur_scaner_channel] != 0))
                {
                    touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
                    //等待touch采集完成
                    i = 0;
                    while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
                    {
                        ++i;
                    }
                    touch_node->get_data(touch_node);       //Touch_HalInterface_SetReady(touch_node, 0);           //清就绪标志
                    delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
                    //进一步运行抗残留
                    touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
                    if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
                    {
                        touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].gainer_table[rself->data.cur_scaner_channel]);
                    }
                }
                //采集数据
                touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
                i = 0;
                while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
                {
                    ++i;
                }

                if (i < 0xFFFF)
                {
                    touch_data = touch_node->get_data(touch_node);      //获取touch数据
                    set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地
                    touch_data = recalc_touchdata_insleep(touch_data, can_scan_mask, self, rself);      //对touch_data应用偏置、倍采样、噪音抑制等功能
                }
                else
                {
                    set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);      //按需接地
                    touch_data = SiAlgoGetRawData(&touchAlgoObject, touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel));           //取上次值作为touch_data
                    TC_LOGE(TAG, "touch timeout");
                    self->reset(self);      //复位并重新触发
                }

                i = touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel);
                SiAlgoSetRawData(&touchAlgoObject, i, touch_data);      //数据压入算法缓冲区

                delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
                touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
            }
        }

        TouchHaltLockLpSiObject();              //锁定低功耗
        TouchHaltUnlockSiObject();              //解锁常规
#if LOG_RAWDATA
        if (IsHaltMode())
        {
            TC_LOG_RAWDATA_I16(TC_LOG_RAWDATA_CAPSENSOR_NORMAL, TC_LOGRAWDATA_TOUCH_STATUS_SLEEP, &SiAlgoGetRawData(&touchAlgoObject, 0), Touch_HalDispatch_GetChannelNum(self)*sizeof(T_SiData));
        }
        else
        {
            TC_LOG_RAWDATA_I16(TC_LOG_RAWDATA_CAPSENSOR_NORMAL, TC_LOGRAWDATA_TOUCH_STATUS_SLOW, &SiAlgoGetRawData(&touchAlgoObject, 0), Touch_HalDispatch_GetChannelNum(self)*sizeof(T_SiData));
        }
#endif
        SiAlgoProcress(&touchAlgoObject);
        TouchHaltUnlockLpSiObject();            //解锁低功耗
        TouchHaltLockSiObject();                //锁定常规
    }

    return 0;
}

void touch_haldispatchonlytouch_reset(struct TOUCH_HalDispatch_Type *self)                                     //复位底层硬件模块，并继续上次trig，虚函数
{
    Touch_Reset();        //复位模块
    touch_haldispatchonlytouch_setup(self);     //预备
}

static uint8_t touch_haldispatchonlytouch_get_scaner(struct TOUCH_HalDispatch_Type *self)                      //获取当前扫描器编号，虚函数
{
    return ((TOUCH_HalDispatch_OnlyTouch_Type *)self)->data.cur_scaner;
}

static void touch_haldispatchonlytouch_setup(struct TOUCH_HalDispatch_Type *self)              //预备
{
    uint8_t i;

    for (i = 0; i < self->scaner_table.scaner_len; ++i)               //调用touch_node的low_init
    {
        self->scaner_table.scaners[i].touch_node->low_init(self->scaner_table.scaners[i].touch_node);
    }
}

//低功耗模式下重新计算touch值，对touch_data应用偏置、倍采样、噪音抑制等功能
static T_SiData recalc_touchdata_insleep(T_SiData touch_data, int can_scan_mask, const struct TOUCH_HalDispatch_Type *self, const TOUCH_HalDispatch_OnlyTouch_Type *rself)
{
    T_SiData rt_data = touch_data;

    if ((can_scan_mask != 0) && self->scaner_table.scaners[rself->data.cur_scaner].sleep_offset_table)    //补偿touch数据
    {
        rt_data += (T_SiData)(self->scaner_table.scaners[rself->data.cur_scaner].sleep_offset_table[rself->data.cur_scaner_channel]);
    }

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
    TOUCH_HalCenterFilter_Type *center_filter_table = NULL;
    if (can_scan_mask != 0)          //全扫
    {
        center_filter_table = self->scaner_table.scaners[rself->data.cur_scaner].center_filter_table;
    }
    else
    {
        center_filter_table = self->scaner_table.scaners[rself->data.cur_scaner].sleep_center_filter_table;
    }
    if (center_filter_table != NULL)              //中值滤波
    {
        rt_data = calc_center_filter(center_filter_table, rself->data.cur_scaner_channel, rt_data, ((can_scan_mask == 0) ? 1 : 0));
    }
    TOUCH_HalLowPassFilter_Type *low_pass_table = self->scaner_table.scaners[rself->data.cur_scaner].low_pass_table;
    if (low_pass_table != NULL)              //低通滤波器
    {
        rt_data = calc_low_pass(low_pass_table, rself->data.cur_scaner_channel, rt_data, ((can_scan_mask == 0) ? 1 : 0));
    }
    TOUCH_HalDoubleSamp_Type *double_samp_table = self->scaner_table.scaners[rself->data.cur_scaner].double_samp_table;
    if (double_samp_table != NULL)       //计算倍采样
    {
        rt_data = calc_double_samp(double_samp_table, rself->data.cur_scaner_channel, rt_data, ((can_scan_mask == 0) ? 1 : 0));  //计算倍采样值
    }
    TOUCH_HalNoiseAvoid_Type *noise_avoid_table = NULL;
    if (can_scan_mask != 0)          //全扫
    {
        noise_avoid_table = self->scaner_table.scaners[rself->data.cur_scaner].noise_avoid_table;
    }
    else
    {
        noise_avoid_table = self->scaner_table.scaners[rself->data.cur_scaner].sleep_noise_avoid_table;
    }
    if (noise_avoid_table != NULL)              //噪音抑制器
    {
        rt_data = calc_noise_avoid(noise_avoid_table, rself->data.cur_scaner_channel, rt_data, ((can_scan_mask == 0) ? 1 : 0));
    }
#endif

    return rt_data;
}

static void sleep_remove_residues_run(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself)       //运行低功耗数据抗残留，丢弃几轮采样值
{
#define REMOVE_RESIDUES_LOSS_CNT        1           //抗残留丢弃几次数据

    uint8_t begin_cur_scaner = 0;                   //cur_scaner起始值
    uint8_t begin_cur_scaner_channel = 0;           //cur_scaner_channel起始值
    TOUCH_HalInterface_Type *touch_node = NULL;

    //使用快速抗残留功能，查询最后一个待扫描通道
    if (rself->para.fast_remove_residues_enable != 0)
    {
        rself->data.cur_scaner = 0;
        rself->data.cur_scaner_channel = 0;

        while (rself->data.cur_scaner < self->scaner_table.scaner_len)
        {
            if ((self->scaner_table.scaners[rself->data.cur_scaner].sleep_channelmask & ((uint32_t)0x1U << rself->data.cur_scaner_channel)) != 0) //通道被屏蔽
            {
                touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
                continue;
            }

            begin_cur_scaner = rself->data.cur_scaner;
            begin_cur_scaner_channel = rself->data.cur_scaner_channel;

            touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
        }
    }

    for (int loss_cnt = 0; loss_cnt < REMOVE_RESIDUES_LOSS_CNT; ++loss_cnt)
    {
        rself->data.cur_scaner = begin_cur_scaner;
        rself->data.cur_scaner_channel = begin_cur_scaner_channel;

        while (rself->data.cur_scaner < self->scaner_table.scaner_len)
        {
#if TOUCH_LAZY_SCAN_ENABLE
            if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                    (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
            {
                touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
                continue;
            }
#endif

            if ((self->scaner_table.scaners[rself->data.cur_scaner].sleep_channelmask & ((uint32_t)0x1U << rself->data.cur_scaner_channel)) != 0) //通道被屏蔽
            {
                touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
                continue;
            }

            touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
            touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
            set_sleep_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
            if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
            {
                touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].sleep_gainer_table[rself->data.cur_scaner_channel]);
            }

            Touch_HalInterface_SetReady(touch_node, 0);           //清就绪标志
            //不再针对每个通道单独运行抗残留
            touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);

            uint32_t i = 0;
            while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
            {
                ++i;
            }
            if (i >= 0xFFFF)
            {
                TC_LOGE(TAG, "touch remove residues timeout");
            }
            set_sleep_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需接地

            touch_node->get_data(touch_node);       //Touch_HalInterface_SetReady(touch_node, 0);           //清就绪标志

            delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
            touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
        }
    }
}

static void normal_remove_residues_run(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself)       //运行常规数据抗残留，丢弃几轮采样值
{
    uint8_t begin_cur_scaner = 0;                   //cur_scaner起始值
    uint8_t begin_cur_scaner_channel = 0;           //cur_scaner_channel起始值
    TOUCH_HalInterface_Type *touch_node = NULL;

    //使用快速抗残留功能，查询最后一个待扫描通道
    if (rself->para.fast_remove_residues_enable != 0)
    {
        rself->data.cur_scaner = 0;
        rself->data.cur_scaner_channel = 0;

        while (rself->data.cur_scaner < self->scaner_table.scaner_len)
        {
            begin_cur_scaner = rself->data.cur_scaner;
            begin_cur_scaner_channel = rself->data.cur_scaner_channel;

            touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
        }
    }

    for (int loss_cnt = 0; loss_cnt < REMOVE_RESIDUES_LOSS_CNT; ++loss_cnt)
    {
        rself->data.cur_scaner = begin_cur_scaner;
        rself->data.cur_scaner_channel = begin_cur_scaner_channel;

        while (rself->data.cur_scaner < self->scaner_table.scaner_len)
        {
#if TOUCH_LAZY_SCAN_ENABLE
            if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                    (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
            {
                touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
                continue;
            }
#endif

            touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
            touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
            set_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
            if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
            {
                touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].gainer_table[rself->data.cur_scaner_channel]);
            }

            Touch_HalInterface_SetReady(touch_node, 0);           //清就绪标志
            //不再针对每个通道单独运行抗残留
            touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);

            uint32_t i = 0;
            while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
            {
                ++i;
            }
            if (i >= 0xFFFF)
            {
                TC_LOGE(TAG, "touch remove residues timeout");
            }
            set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需接地

            touch_node->get_data(touch_node);       //Touch_HalInterface_SetReady(touch_node, 0);           //清就绪标志

            delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
            touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
        }
    }
}

//static void rescan_sleep_touchdata(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself)    //重新采集一轮低功耗数据
//{
//    T_SiData touch_data;
//    uint32_t i;
//    TOUCH_HalInterface_Type *touch_node = NULL;

//    rself->data.cur_scaner = 0;
//    rself->data.cur_scaner_channel = 0;

//    while (rself->data.cur_scaner < self->scaner_table.scaner_len)
//    {
//        if ((self->scaner_table.scaners[rself->data.cur_scaner].sleep_channelmask & ((uint32_t)0x1U << rself->data.cur_scaner_channel)) != 0) //通道被屏蔽
//        {
//            touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
//            continue;
//        }

//        touch_node = self->scaner_table.scaners[rself->data.cur_scaner].touch_node;
//        touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
//        set_sleep_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
//        if (self->scaner_table.scaners[rself->data.cur_scaner].gainer_table != NULL)        //配置增益器
//        {
//            touch_node->set_gainer(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].sleep_gainer_table[rself->data.cur_scaner_channel]);
//        }

//        //按需运行抗残留
//        if (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table && (self->scaner_table.scaners[rself->data.cur_scaner].remove_residues_enable_table[rself->data.cur_scaner_channel] != 0))
//        {
//            touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
//            i = 0;
//            while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
//            {
//                ++i;
//            }
//            touch_node->get_data(touch_node);       //Touch_HalInterface_SetReady(touch_node, 0);           //清就绪标志
//            delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
//        }
//        touch_node->trig(touch_node, TOUCH_TRIG_SOFTWARE, 1);
//        i = 0;
//        while (!(Touch_HalInterface_GetReady(touch_node) != 0) && i < 0xFFFF)      //等待采集完成
//        {
//            ++i;
//        }

//        if (i < 0xFFFF)
//        {
//            touch_data = touch_node->get_data(touch_node);      //获取touch数据
//            set_sleep_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需接地
//            touch_data = recalc_touchdata_insleep(touch_data, 0, self, rself);      //对touch_data应用偏置、倍采样、噪音抑制等功能
//        }
//        else
//        {
//            set_sleep_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需接地
//            touch_data = SiAlgoGetRawData(&touchAlgoObject, touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel));           //取上次值作为touch_data
//            TC_LOGE(TAG, "touch timeout");
//        }

//        i = touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel);
//        SiAlgoSetRawData(&touchAlgoObject, i, touch_data);      //数据压入算法缓冲区

//        delay_clock(self->scaner_table.scaners[rself->data.cur_scaner].sleep_scan_channel_waitcycle);       //延迟一段时间
//        touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
//    }
//}

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
static void double_samp_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep)    //清倍采样缓冲区
{
    for (size_t i = 0; i < scaner->scaner_len; ++i)
    {
        //初始化double_samp_table
        if (scaner->scaners[i].double_samp_table != NULL)
        {
            for (size_t j = 0; j < scaner->scaners[i].channel_len; ++j)
            {
                scaner->scaners[i].double_samp_table[j].dummy1 = 0;
                if (!(only_sleep != 0))
                {
                    scaner->scaners[i].double_samp_table[j].dummy2 = 0;
                }
            }
        }
    }
}

static void noise_avoid_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep)    //清噪音抑制器缓冲区
{
    for (uint32_t i = 0; i < scaner->scaner_len; ++i)
    {
        //初始化noise_avoid_table
        if (scaner->scaners[i].noise_avoid_table != NULL)
        {
            for (uint32_t j = 0; j < scaner->scaners[i].channel_len; ++j)
            {
                scaner->scaners[i].noise_avoid_table[j].dummy1 = 0;
                if (!(only_sleep != 0))
                {
                    scaner->scaners[i].noise_avoid_table[j].dummy2 = 0;
                }
            }
        }
        //初始化sleep_noise_avoid_table
        if (scaner->scaners[i].sleep_noise_avoid_table != NULL)
        {
            for (uint32_t j = 0; j < scaner->scaners[i].channel_len; ++j)
            {
                scaner->scaners[i].sleep_noise_avoid_table[j].dummy1 = 0;
                if (!(only_sleep != 0))
                {
                    scaner->scaners[i].sleep_noise_avoid_table[j].dummy2 = 0;
                }
            }
        }
    }
}

static void center_filter_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep)  //清中值滤波缓冲区，only_sleep为1表示仅复位sleep数据
{
    for (uint32_t i = 0; i < scaner->scaner_len; ++i)
    {
        if (!only_sleep)
        {
            //初始化center_filter_table
            if (scaner->scaners[i].center_filter_table != NULL)
            {
                for (uint32_t j = 0; j < scaner->scaners[i].channel_len; ++j)
                {
                    scaner->scaners[i].center_filter_table[j].buflen = 0;
                }
            }
        }
        //初始化sleep_center_filter_table
        if (scaner->scaners[i].sleep_center_filter_table != NULL)
        {
            for (uint32_t j = 0; j < scaner->scaners[i].channel_len; ++j)
            {
                scaner->scaners[i].sleep_center_filter_table[j].buflen = 0;
            }
        }
    }
}

static void low_pass_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep)    //清低通滤波器缓冲区
{
    for (uint32_t i = 0; i < scaner->scaner_len; ++i)
    {
        //初始化low_pass_table
        if (scaner->scaners[i].low_pass_table != NULL)
        {
            for (uint32_t j = 0; j < scaner->scaners[i].channel_len; ++j)
            {
                scaner->scaners[i].low_pass_table[j].dummy1 = 0;
                if (!(only_sleep != 0))
                {
                    scaner->scaners[i].low_pass_table[j].dummy2 = 0;
                }
            }
        }
    }
}
#endif

static void set_sleep_all_ioenable(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself, uint8_t enable) //打开或关闭所有touch通道接地功能
{
    rself->data.cur_scaner = 0;
    rself->data.cur_scaner_channel = 0;

    while (rself->data.cur_scaner < self->scaner_table.scaner_len)
    {
#if TOUCH_LAZY_SCAN_ENABLE
        if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
        {
            touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
            continue;
        }
#endif
        if (enable != 0)
        {
            set_sleep_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);
        }
        else
        {
            set_sleep_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);
        }

        touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
    }
}

static void set_all_ioenable(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself, uint8_t enable) //打开或关闭所有touch通道接地功能
{
    rself->data.cur_scaner = 0;
    rself->data.cur_scaner_channel = 0;

    while (rself->data.cur_scaner < self->scaner_table.scaner_len)
    {
#if TOUCH_LAZY_SCAN_ENABLE
        if (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.waitflag &&
                (self->scaner_table.scaners[rself->data.cur_scaner].lazy_scan.channelmask & (0x1 << rself->data.cur_scaner_channel))) //懒扫描
        {
            touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
            continue;
        }
#endif
        if (enable != 0)
        {
            set_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);
        }
        else
        {
            set_iodisable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);
        }

        touch_haldispatchonlytouch_nextchannel(rself);      //扫描下一个通道
    }
}

#if TOUCH_LAZY_SCAN_ENABLE
static void touch_lazy_config(TOUCH_HalDispatch_OnlyTouch_Type *self, int scanerid)     //懒扫描配置
{
    for (int j = 0; j < self->super.scaner_table.scaners[scanerid].channel_len; ++j)
    {
        if (self->super.scaner_table.scaners[scanerid].lazy_scan.channelmask & (0x1 << j))     //懒初始化
        {
            Touch_IOConfig(self->super.scaner_table.scaners[scanerid].channel_table[j].channel);
            if (self->super.scaner_table.scaners[scanerid].channel_table[j].shield.shld_en)
            {
                if (self->super.scaner_table.scaners[scanerid].channel_table[j].shield.shld_sel == SOURCE_FOLLOW_SHIELD || self->super.scaner_table.scaners[scanerid].channel_table[j].shield.shld_sel == SHIELD_PIN_AS_SHIELD)
                {
                    ll_gpio_afio_config(GPIO_PIN_1, (gpio_afio_mux_e)GPIO1_SOFTWARE_INPUT_FUNCTION_CAP_SHIELD);
                }
                else
                {
                    Touch_IOConfig(self->super.scaner_table.scaners[scanerid].channel_table[j].shield.shld_sel);
                }
            }
        }
    }

    self->super.scaner_table.scaners[scanerid].lazy_scan.waitflag = 0;

    touch_lazy_scan_completed(scanerid);
}

__WEAK void touch_lazy_scan_completed(int scanerid)     //懒扫描完成
{
    (void)scanerid;
}
#endif

static int calc_can_scan_mask(TOUCH_HalDispatch_OnlyTouch_Type *self)     //计算sleep下的mask通道是否可以被扫描
{
    int can_scan_mask = 1;

    if (self->para.sleep_mask_freq != 0)  //判断是否能扫描mask通道
    {
        if (self->data.sleep_scancnt < self->para.sleep_mask_freq)
        {
            can_scan_mask = 0;
        }
    }
    return can_scan_mask;
}

void Touch_HalDispatch_DiscardSampData(int cnt) //丢弃非低功耗采样数据，以每次扫描周期计数
{
    discardTouchSampDataCnt = cnt;
}
