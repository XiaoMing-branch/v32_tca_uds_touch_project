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

/** @brief 日志标签 */
const static char *TAG = "TOUCH_HALDISPATCH";

/** @brief 触摸低功耗监控计数器超时，初始化为0 */
volatile uint16_t touchHaltMonitorCnt = 0;

/** @brief 触摸算法对象，初始化为0 */
T_SiAlgoObject touchAlgoObject = {0};

/** @brief 丢弃touch采样数据计数器 */
static int discardTouchSampDataCnt = 0;

/**
 * @brief 将scanner编号和scanner通道号转换为从0开始的全局通道号
 * @param self 触摸分发器实例指针
 * @param scaner 扫描器编号
 * @param scaner_chanel 扫描器内的通道号
 * @return 从0开始的全局通道号
 */
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
/**
 * @brief 计算中值滤波（噪音抑制器）值
 * @param center_filter_table 中值滤波配置表指针
 * @param channel 当前通道号
 * @param cur_data 当前采样数据
 * @param is_sleep 是否为低功耗通道（1表示sleep通道）
 * @return 滤波后的数据
 */
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

/**
 * @brief 判断是否需要运行抗残留数据采集
 * @param self 触摸分发器基类实例指针
 * @param rself 仅触摸分发器实例指针
 * @retval 0 不需要抗残留
 * @retval 1 需要抗残留
 * @note 若低功耗扫描通道和正常运行扫描完全一致时，不需要抗残留；否则需要
 */
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

/**
 * @brief run方法（虚函数），处理定时消息/进入低功耗/从低功耗唤醒
 * @param self 触摸分发器实例指针
 * @param msg 消息类型（MSG_TASK_TIMER / MSG_TASK_ENTER_HALT / MSG_TASK_WAKE_UP）
 * @param param 附加参数，为0xA5A5A5A5时强制运行算法
 */
static void touch_haldispatchonlytouch_run(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param);
/**
 * @brief 扫描多个通道的run方法（虚函数），支持分多次扫描完所有通道
 * @param self 触摸分发器实例指针
 * @param msg 消息类型（MSG_TASK_TIMER / MSG_TASK_ENTER_HALT / MSG_TASK_WAKE_UP）
 * @param param 附加参数，为0xA5A5A5A5时强制运行算法
 */
static void touch_haldispatchonlytouch_run_scanmany(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param);
/**
 * @brief 扫描一轮所有通道
 * @param self 触摸分发器实例指针
 * @param forceEnd 为1时强制扫描完本轮所有通道
 * @retval 1 所有通道扫描结束
 * @retval 0 扫描未完成
 */
static int touch_haldispatchonlytouch_scanmany_channel(struct TOUCH_HalDispatch_Type *self, int forceEnd);
/**
 * @brief sleep_run方法（虚函数），低功耗模式下运行一次触摸扫描
 * @param self 触摸分发器实例指针
 * @retval 1 检测到有效按键，可从低功耗唤醒
 * @retval 0 未检测到有效按键
 */
static int touch_haldispatchonlytouch_sleep_run(struct TOUCH_HalDispatch_Type *self);
/**
 * @brief 复位底层硬件模块，并继续上次trig（虚函数）
 * @param self 触摸分发器实例指针
 */
void touch_haldispatchonlytouch_reset(struct TOUCH_HalDispatch_Type *self);
/**
 * @brief 获取当前扫描器编号（虚函数）
 * @param self 触摸分发器实例指针
 * @return 当前扫描器编号
 */
static uint8_t touch_haldispatchonlytouch_get_scaner(struct TOUCH_HalDispatch_Type *self);

/**
 * @brief 计算低功耗模式下mask通道是否可以被扫描
 * @param self 仅触摸分发器实例指针
 * @retval 1 可以扫描mask通道
 * @retval 0 当前不满足mask通道扫描条件
 */
static int calc_can_scan_mask(TOUCH_HalDispatch_OnlyTouch_Type *self);
/**
 * @brief 预备，调用所有touch_node的low_init进行底层初始化
 * @param self 触摸分发器实例指针
 */
static void touch_haldispatchonlytouch_setup(struct TOUCH_HalDispatch_Type *self);
/**
 * @brief 低功耗模式下重新计算touch值，对touch_data应用偏置、倍采样、噪音抑制等功能
 * @param touch_data 原始触摸数据
 * @param can_scan_mask mask通道是否可扫描标志
 * @param self 触摸分发器基类实例指针
 * @param rself 仅触摸分发器实例指针
 * @return 处理后的触摸数据
 */
static T_SiData recalc_touchdata_insleep(T_SiData touch_data, int can_scan_mask, const struct TOUCH_HalDispatch_Type *self, const TOUCH_HalDispatch_OnlyTouch_Type *rself);
/**
 * @brief 运行低功耗数据抗残留，丢弃几轮采样值
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 */
static void sleep_remove_residues_run(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself);
/**
 * @brief 运行常规数据抗残留，丢弃几轮采样值
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 */
static void normal_remove_residues_run(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself);

#if !(TOUCH_REDUCED_MODE)                   //非精简模式
    /**
     * @brief 清倍采样缓冲区
     * @param scaner 扫描器配置表指针
     * @param only_sleep 为1表示仅复位sleep数据，为0同时复位normal和sleep数据
     */
    static void double_samp_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);
    /**
     * @brief 清噪音抑制器缓冲区
     * @param scaner 扫描器配置表指针
     * @param only_sleep 为1表示仅复位sleep数据，为0同时复位normal和sleep数据
     */
    static void noise_avoid_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);
    /**
     * @brief 清中值滤波缓冲区
     * @param scaner 扫描器配置表指针
     * @param only_sleep 为1表示仅复位sleep数据，为0同时复位normal和sleep数据
     */
    static void center_filter_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);
    /**
     * @brief 清低通滤波器缓冲区
     * @param scaner 扫描器配置表指针
     * @param only_sleep 为1表示仅复位sleep数据，为0同时复位normal和sleep数据
     */
    static void low_pass_reset(const TOUCH_HalScanerTable_Type *scaner, int only_sleep);
#endif

#if TOUCH_LAZY_SCAN_ENABLE
    /**
     * @brief 懒扫描配置，对延迟初始化的通道进行IO配置
     * @param self 仅触摸分发器实例指针
     * @param scanerid 扫描器ID
     */
    static void touch_lazy_config(TOUCH_HalDispatch_OnlyTouch_Type *self, int scanerid);
    /**
     * @brief 懒扫描完成回调（弱函数）
     * @param scanerid 扫描器ID
     */
    void touch_lazy_scan_completed(int scanerid);
#endif

/**
 * @brief 打开或关闭低功耗模式下所有touch通道接地功能
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 * @param enable 1使能接地，0关闭接地
 */
static void set_sleep_all_ioenable(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself, uint8_t enable);
/**
 * @brief 低功耗模式下按需将通道IO配置为正常（使能）状态
 * @param scaner 扫描器配置表指针
 * @param scaner_index 扫描器索引
 * @param logicchannel 逻辑通道号
 * @note 若scaner的sleep_iodisable_channelmask不为0，所有被扫描的通道都要打开，
 *       否则共享通道在某些情况下会出问题（如低功耗下共享模式扫描时）
 */
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
/**
 * @brief 低功耗模式下按需将通道IO配置为接地（禁用）状态
 * @param scaner 扫描器配置表指针
 * @param scaner_index 扫描器索引
 * @param logicchannel 逻辑通道号
 * @note 通过sleep_iodisable_channelmask位掩码判断是否需要接地，
 *       实现特定通道采集完成后自动接地的省电功能
 */
static __INLINE void set_sleep_iodisable_asneed(const TOUCH_HalScanerTable_Type *scaner, uint8_t scaner_index, uint8_t logicchannel) //将有需要的通道配置为接地
{
    if ((scaner->scaners[scaner_index].sleep_iodisable_channelmask & ((uint32_t)0x1U << logicchannel)) != 0)
    {
        TOUCH_HalInterface_Type *touch_node = scaner->scaners[scaner_index].touch_node;
        touch_node->set_ioenable(touch_node, scaner->scaners[scaner_index].channel_table[logicchannel].channel, 0);
    }
}

/**
 * @brief 打开或关闭常规模式下所有touch通道的IO功能
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 * @param enable 1=使能IO（取消接地），0=禁用IO（接地）
 */
static void set_all_ioenable(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself, uint8_t enable); //打开或关闭所有touch通道接地功能
/**
 * @brief 常规模式下按需将通道IO配置为正常（使能）状态
 * @param scaner 扫描器配置表指针
 * @param scaner_index 扫描器索引
 * @param logicchannel 逻辑通道号
 * @note 通过iodisable_channelmask位掩码判断是否需要操作，
 *       所有被扫描的通道都要打开，否则共享通道会有问题
 */
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
/**
 * @brief 常规模式下按需将通道IO配置为接地（禁用）状态
 * @param scaner 扫描器配置表指针
 * @param scaner_index 扫描器索引
 * @param logicchannel 逻辑通道号
 * @note 通过iodisable_channelmask位掩码判断是否需要接地，
 *       实现特定通道采集完成后自动接地的功能
 */
static __INLINE void set_iodisable_asneed(const TOUCH_HalScanerTable_Type *scaner, uint8_t scaner_index, uint8_t logicchannel) //将有需要的通道配置为接地
{
    if ((scaner->scaners[scaner_index].iodisable_channelmask & ((uint32_t)0x1U << logicchannel)) != 0)
    {
        TOUCH_HalInterface_Type *touch_node = scaner->scaners[scaner_index].touch_node;
        touch_node->set_ioenable(touch_node, scaner->scaners[scaner_index].channel_table[logicchannel].channel, 0);
    }
}

/**
 * @brief 构造函数，初始化触摸分发器
 * @param self 仅触摸分发器实例指针
 */
static void touch_haldispatchonlytouch_ctor(TOUCH_HalDispatch_OnlyTouch_Type *self);
/**
 * @brief 前进到下一个通道，若当前扫描器通道已遍历完则切换到下一个扫描器
 * @param self 仅触摸分发器实例指针
 */
static __INLINE void touch_haldispatchonlytouch_nextchannel(TOUCH_HalDispatch_OnlyTouch_Type *self)
{
    if (++self->data.cur_scaner_channel >= self->super.scaner_table.scaners[self->data.cur_scaner].channel_len)
    {
        self->data.cur_scaner_channel = 0;
        ++self->data.cur_scaner;
    }
};

/**
 * @brief 延迟指定的clock周期数，用于等待触摸采集稳定
 * @param clk 延迟的clock周期数
 * @note 增加等待时间可降低部分噪音，但会降低采样频率
 */
static __INLINE void delay_clock(uint32_t clk)      //延迟一段时间，单位clock
{
    for (uint32_t i = 0; i < clk; ++i)
    {
        __NOP();
    }
}

/**
 * @brief 设置快扫模式
 * @param dispatch 触摸分发器实例指针
 * @param fast 1表示快扫模式，0表示慢扫模式
 * @note 设置为快扫时同时记录快扫起始时间，用于后续快慢扫自动切换
 */
void Touch_HalDispatch_SetFastScan(TOUCH_HalDispatch_Type *dispatch, uint8_t fast)
{
    dispatch->flag.fast_scan = fast;
    if (fast != 0)
    {
        dispatch->fast2slow_scan.last_fast_time = TouchGetTime();
    }
}

/**
 * @brief 获取touch通道总数
 * @param dispatch 触摸分发器实例指针
 * @return touch通道总数
 */
int Touch_HalDispatch_GetChannelNum(struct TOUCH_HalDispatch_Type *dispatch)   //获取touch通道总数
{
    int channelNum = 0;

    for (int i = 0; i < dispatch->scaner_table.scaner_len; ++i)
    {
        channelNum += dispatch->scaner_table.scaners[i].channel_len;
    }

    return channelNum;
}

/**
 * @brief 创建仅触摸分发器实例
 * @param nd 仅触摸分发器实例指针（需预先分配内存）
 * @param scaner 扫描器配置表指针
 * @param para 分发器参数指针
 * @return 触摸分发器基类指针
 * @retval NULL 创建失败（参数校验不通过）
 * @note 该函数会：
 *       - 校验频率参数合法性（fast_freq/slow_freq/sleep_freq不能为0）
 *       - 校验gainer_table和sleep_gainer_table不能为空
 *       - 校验精简模式下不支持倍采样/噪音抑制/低通滤波等功能
 *       - 初始化倍采样/噪音抑制/中值滤波/低通滤波器缓冲区
 *       - 根据scan_type选择不同的run方法（TOUCH_SCAN_ALL或TOUCH_SCAN_MANY）
 */
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

/**
 * @brief 构造函数，初始化触摸分发器运行时数据
 * @param self 仅触摸分发器实例指针
 * @note 执行以下初始化：
 *       - 设置快扫标志和低功耗标志
 *       - 清零运行时数据缓冲区
 *       - 遍历所有扫描器的所有通道，进行IO pinmux配置
 *       - 支持懒扫描延迟初始化（TOUCH_LAZY_SCAN_ENABLE）
 *       - 配置shield引脚
 *       - 调用touch_haldispatchonlytouch_setup进行底层初始化
 */
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

/**
 * @brief run方法（虚函数），处理定时消息/进入低功耗/从低功耗唤醒
 * @param self 触摸分发器实例指针
 * @param msg 消息类型
 *        - MSG_TASK_TIMER: 定时消息，触发通道扫描调度
 *        - MSG_TASK_ENTER_HALT: 进入低功耗消息，切换到低功耗模式
 *        - MSG_TASK_WAKE_UP: 从低功耗唤醒消息，恢复到正常模式
 * @param param 附加参数，为0xA5A5A5A5时强制开始新一轮扫描
 * @note 正常扫描调度流程（MSG_TASK_TIMER）：
 *       - 判断是否超时（距离上次采样时间 >= 采样周期）
 *       - 所有通道依次采集：配置通道->配置增益->抗残留(可选)->采集数据->后处理
 *       - 数据后处理链路：中值滤波->低通滤波->倍采样->噪音抑制
 *       - 采集超时时取上次数据作为当前值并复位硬件
 *       - 采集完成后调用SiAlgoProcress运行触摸算法
 */
static void touch_haldispatchonlytouch_run(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param)      //run方法
{
    uint16_t i;
    T_SiData touch_data;
    TOUCH_HalInterface_Type *touch_node;
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    /* 事件流：HW定时器触发 → dispatch层通道扫描 → 数据处理链路 → 应用层算法 */
    if (msg == MSG_TASK_TIMER)              //定时消息
    {
        uint16_t freq = self->fast2slow_scan.fast_freq;         //获取扫描频率
        /* 调度条件：首次运行(lastSampTime==0)/超时(距上次采样≥周期)/强制触发(0xA5A5A5A5) → 执行新一轮采集 */
        int timeout = (TouchGetTime() - rself->data.lastSampTime >= SiIFastDiv(1000U, freq)) ? 1 : 0;
        if (rself->data.lastSampTime == 0 || timeout != 0 || ((uint32_t)param) == 0xA5A5A5A5) /**< 状态转换条件：首次运行/超时/强制触发 → 开始新一轮扫描 */
        {
            rself->data.lastSampTime = TouchGetTime();      //记录上一次采样时间

            rself->data.cur_scaner = 0;
            rself->data.cur_scaner_channel = 0;

            /* 通道采集前等待：等待电源/参考电压稳定后再开始第一个通道的采集 */
            delay_clock(rself->para.scan_firstchannel_waitcycle);       //扫描第一个通道延迟一段时间
            /* 通道扫描调度循环：dispatch层遍历所有扫描器→所有通道→依次采集 */
            while (rself->data.cur_scaner < self->scaner_table.scaner_len)   /**< 通道扫描调度循环：依次遍历所有扫描器的所有通道 */
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
                /** 单通道扫描步骤1：配置当前通道 */
                touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
                /** 单通道扫描步骤2：按需控制IO使能 */
                set_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
                /** 单通道扫描步骤3：配置增益 */
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

                    /** 数据处理链路：中值滤波 → 低通滤波 → 倍采样 → 噪音抑制 */
#if !(TOUCH_REDUCED_MODE)                   //非精简模式
                    /** 步骤1：中值滤波（去毛刺） */
                    TOUCH_HalCenterFilter_Type *center_filter_table = self->scaner_table.scaners[rself->data.cur_scaner].center_filter_table;
                    if (center_filter_table != NULL)              //噪音抑制器
                    {
                        touch_data = calc_center_filter(center_filter_table, rself->data.cur_scaner_channel, touch_data, 0);
                    }
                    /** 步骤2：低通滤波（平滑数据） */
                    TOUCH_HalLowPassFilter_Type *low_pass_table = self->scaner_table.scaners[rself->data.cur_scaner].low_pass_table;
                    if (low_pass_table != NULL)              //低通滤波器
                    {
                        touch_data = calc_low_pass(low_pass_table, rself->data.cur_scaner_channel, touch_data, 0);
                    }
                    /** 步骤3：倍采样计算（提高信噪比） */
                    TOUCH_HalDoubleSamp_Type *double_samp_table = self->scaner_table.scaners[rself->data.cur_scaner].double_samp_table;
                    if (double_samp_table != NULL)       //计算倍采样
                    {
                        touch_data = calc_double_samp(double_samp_table, rself->data.cur_scaner_channel, touch_data, 0);    //计算倍采样值
                    }
                    /** 步骤4：噪音抑制（消除突发噪声） */
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

                /* 事件流：单通道采集完成 → dispatch层将数据推入算法缓冲区 */
                i = touch_haldispatchonlytouch_channelnum(self, rself->data.cur_scaner, rself->data.cur_scaner_channel);
                SiAlgoSetRawData(&touchAlgoObject, i, touch_data);          //数据压入算法缓冲区

                delay_clock(rself->para.scan_channel_waitcycle);       //延迟一段时间
                touch_haldispatchonlytouch_nextchannel(rself);      //下一个通道
            }

            /** 调度决策：上电稳定期跳过前skip_poweron_datas轮数据 */
            if (rself->data.skip_poweron_cnt < rself->para.skip_poweron_datas)
            {
                ++rself->data.skip_poweron_cnt;     //上电未稳定，跳过本轮数据处理
            }
            else
            {
#if LOG_RAWDATA
                /** 按间隔输出原始数据日志 */
                if (++rself->data.skip_lograwdata_normalcnt > LOG_RAWDATA_NORMAL_INTERVAL)
                {
                    rself->data.skip_lograwdata_normalcnt = 0;
                    TC_LOG_RAWDATA_I16(TC_LOG_RAWDATA_CAPSENSOR_NORMAL, TC_LOGRAWDATA_TOUCH_STATUS_FAST, &SiAlgoGetRawData(&touchAlgoObject, 0), Touch_HalDispatch_GetChannelNum(self)*sizeof(T_SiData));
                }
#endif
                /** 若处于数据丢弃期，递减计数器但不运行算法 */
                if (discardTouchSampDataCnt > 0)
                {
                    --discardTouchSampDataCnt;
                }
                else
                {
                    /** 正常运行：将采样数据送入触摸算法处理 */
                    SiAlgoProcress(&touchAlgoObject);               //运行Touch算法
                }
            }
        }
    }

        /* 事件流：系统请求进入低功耗 → dispatch切换到sleep模式，配置sleep IO并运行抗残留 */
        if (msg == MSG_TASK_ENTER_HALT)     /**< 状态转换条件：收到低功耗进入消息 → 切换到低功耗模式 */
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

    if (msg == MSG_TASK_WAKE_UP)        /**< 状态转换条件：收到唤醒消息 → 切换到正常模式 */
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

/**
 * @brief 扫描多个通道的run方法（虚函数），将一轮扫描拆分为多次定时任务完成
 * @param self 触摸分发器实例指针
 * @param msg 消息类型
 *        - MSG_TASK_TIMER: 定时消息，继续通道扫描
 *        - MSG_TASK_ENTER_HALT: 进入低功耗消息，强制完成本轮扫描后切换模式
 *        - MSG_TASK_WAKE_UP: 从低功耗唤醒消息，恢复到正常模式
 * @param param 附加参数，为0xA5A5A5A5时强制开始新一轮扫描
 * @note 扫描多通道模式通过状态机(fsm)分步执行：
 *       - fsm=0: 开始新一轮扫描，扫描first_channel个通道
 *       - fsm=1: 继续扫描下一批通道，直到所有通道扫描完成
 *       - fsm=2: 所有通道扫描完成，运行触摸算法，回到fsm=0
 *       进入低功耗时若fsm!=0，先强制完成本轮扫描再切换模式
 */
static void touch_haldispatchonlytouch_run_scanmany(struct TOUCH_HalDispatch_Type *self, uint32_t msg, void *param)     //扫描多个的run方法，虚函数
{
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    /* 事件流（分批模式）：HW定时器触发 → FSM状态机分步调度 → 分批采集 → 全部完成后→算法处理 */
    if (msg == MSG_TASK_TIMER)              //定时消息
    {
        uint16_t freq = self->fast2slow_scan.fast_freq;         //获取扫描频率
        /* 调度条件：首次运行/超时/FSM未完成/强制触发 → 继续或开始新一轮分批扫描 */
        int timeout = (TouchGetTime() - rself->data.lastSampTime >= SiIFastDiv(1000U, freq)) ? 1 : 0;
        if (rself->data.lastSampTime == 0 || timeout != 0 || rself->data.scan_many.fsm != 0  || ((uint32_t)param) == 0xA5A5A5A5) /**< 状态转换条件：首次运行/超时/fsm未完成/强制触发 → 继续或开始新一轮扫描 */
        {
            /* FSM状态机调度：分步完成所有通道采集，避免单次任务耗时过长 */
            switch (rself->data.scan_many.fsm)
            {
            case 0:     /**< 状态：开始新一轮扫描，采集第一批通道 */
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
            case 1:     /**< 状态：继续采集下一批通道，从上次断点处恢复 */
                rself->data.cur_scaner = rself->data.scan_many.last_scaner;
                rself->data.cur_scaner_channel = rself->data.scan_many.last_scaner_channel;

                if (touch_haldispatchonlytouch_scanmany_channel(self, 0) != 0)     //扫描结束
                {
                    rself->data.scan_many.fsm = 2;
                }

                rself->data.scan_many.last_scaner = rself->data.cur_scaner;
                rself->data.scan_many.last_scaner_channel = rself->data.cur_scaner_channel;
                break;
            /* 事件流：FSM全部通道采集完毕 → dispatch层将所有批次数据送入算法缓冲区 → 应用层处理 */
            case 2:     /**< 状态：本轮所有通道采集完成，运行触摸算法 */
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
                rself->data.scan_many.fsm = 0;      /**< 状态转换条件：算法处理完成 → 回到初始状态，等待下一轮 */
                break;
            default:
                rself->data.scan_many.fsm = 0;      /**< 状态转换条件：非法状态 → 复位到初始状态 */
                break;
            }
        }
    }

    /* 事件流（分批模式）：系统请求进入低功耗 → FSM强制完成本轮扫描 → 切换到sleep模式 */
    if (msg == MSG_TASK_ENTER_HALT)     /**< 状态转换条件：收到低功耗进入消息 → 强制完成本轮扫描后切换到低功耗模式 */
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

    if (msg == MSG_TASK_WAKE_UP)        /**< 状态转换条件：收到唤醒消息 → 切换到正常模式 */
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

/**
 * @brief 扫描一轮通道（分批模式），每批扫描scan_many_num个通道
 * @param self 触摸分发器实例指针
 * @param forceEnd 为1时强制扫描完本轮所有通道（用于进入低功耗前的收尾）
 * @retval 1 所有通道扫描结束
 * @retval 0 扫描未完成，需要后续继续扫描
 * @note 该函数与touch_haldispatchonlytouch_run配合实现分时多通道扫描，
 *       通过scan_many_num控制每次采集的通道数量，
 *       避免单次任务执行时间过长导致系统响应延迟。
 *       状态通过cur_scaner/cur_scaner_channel保持，下次调用继续。
 */
static int touch_haldispatchonlytouch_scanmany_channel(struct TOUCH_HalDispatch_Type *self, int forceEnd)    //所有通道扫描结束，返回1，否则返回0
{
    uint16_t i;
    T_SiData touch_data;
    TOUCH_HalInterface_Type *touch_node;
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    /** 分批通道扫描调度：每次采集scan_many_num个通道（forceEnd时采集全部剩余通道） */
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
        /** 分批扫描-步骤1：配置当前通道 */
        touch_node->set_channel(touch_node, &self->scaner_table.scaners[rself->data.cur_scaner].channel_table[rself->data.cur_scaner_channel]);
        /** 分批扫描-步骤2：按需控制IO使能 */
        set_ioenable_asneed(&self->scaner_table, rself->data.cur_scaner, rself->data.cur_scaner_channel);       //按需开启touch功能
        /** 分批扫描-步骤3：配置增益 */
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

            /** 数据处理链路：中值滤波 → 低通滤波 → 倍采样 → 噪音抑制 */
#if !(TOUCH_REDUCED_MODE)                   //非精简模式
            /** 步骤1：中值滤波（去毛刺） */
            TOUCH_HalCenterFilter_Type *center_filter_table = self->scaner_table.scaners[rself->data.cur_scaner].center_filter_table;
            if (center_filter_table != NULL)              //噪音抑制器
            {
                touch_data = calc_center_filter(center_filter_table, rself->data.cur_scaner_channel, touch_data, 0);
            }
            /** 步骤2：低通滤波（平滑数据） */
            TOUCH_HalLowPassFilter_Type *low_pass_table = self->scaner_table.scaners[rself->data.cur_scaner].low_pass_table;
            if (low_pass_table != NULL)              //低通滤波器
            {
                touch_data = calc_low_pass(low_pass_table, rself->data.cur_scaner_channel, touch_data, 0);
            }
            /** 步骤3：倍采样计算（提高信噪比） */
            TOUCH_HalDoubleSamp_Type *double_samp_table = self->scaner_table.scaners[rself->data.cur_scaner].double_samp_table;
            if (double_samp_table != NULL)       //计算倍采样
            {
                touch_data = calc_double_samp(double_samp_table, rself->data.cur_scaner_channel, touch_data, 0);    //计算倍采样值
            }
            /** 步骤4：噪音抑制（消除突发噪声） */
            TOUCH_HalNoiseAvoid_Type *noise_avoid_table = self->scaner_table.scaners[rself->data.cur_scaner].noise_avoid_table;
            if (noise_avoid_table != NULL)              //噪音抑制器
            {
                touch_data = calc_noise_avoid(noise_avoid_table, rself->data.cur_scaner_channel, touch_data, 0);
            }
#endif
        }
        else        /**< 采集超时处理：使用上次有效数据并复位硬件 */
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

/**
 * @brief 低功耗模式下运行一次触摸扫描（虚函数）
 * @param self 触摸分发器实例指针
 * @retval 1 检测到有效按键或噪音强制唤醒条件满足，可从低功耗唤醒
 * @retval 0 未检测到有效按键，继续保持低功耗
 * @note 低功耗扫描调度流程：
 *       1. RTC触发就绪后，依次扫描非屏蔽的低功耗通道
 *       2. 对每个通道采集数据，应用偏置/倍采样/噪音抑制等处理
 *       3. 运行算法检测按键（SiAlgoProcress）
 *       4. 若检测到有效按键(wkupFlag==SI_RT_WKUP)或噪音强制唤醒，返回1
 *       5. 否则根据sleep_mask_freq判断是否需要全扫mask通道
 *       - 全扫阶段：对正常通道进行完整扫描，更新完整touch数据
 *       - 支持use_wkupdata_as_touchdata优化：若低功耗唤醒通道和全扫通道使用同一份数据，
 *         则直接用已采集的wkup数据，避免重复扫描
 */
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
        uint8_t valid;          /**< 1表示touch数据有效 */
        T_SiData touch_data;    /**< 缓存低功耗唤醒时采集的原始touch数据 */
    } touch_data_wkupbufs[SI_CH_MAXNUM];    /**< 低功耗唤醒数据备份缓冲区，用于use_wkupdata_as_touchdata优化 */
    uint32_t i;
    TOUCH_HalInterface_Type *touch_node;
    TOUCH_HalDispatch_OnlyTouch_Type *rself = (TOUCH_HalDispatch_OnlyTouch_Type *)self;

    if (touchHaltRtcTrigFlag != 0)       /**< 状态转换条件：RTC定时触发就绪 → 执行低功耗扫描 */
    {
        touchHaltRtcTrigFlag = 0;       //清rtc触发标记
        touchHaltMonitorCnt = 0;        //清低功耗超时计数器

        delay_clock(rself->para.sleep_scan_firstchannel_waitcycle);     //等待芯片稳定后，再采集

        rself->data.cur_scaner = 0;
        rself->data.cur_scaner_channel = 0;
        /** 低功耗通道扫描调度：采集所有非屏蔽的低功耗唤醒通道 */
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

            if ((self->scaner_table.scaners[rself->data.cur_scaner].sleep_channelmask & ((uint32_t)0x1U << rself->data.cur_scaner_channel)) != 0) /**< 通道被sleep_channelmask屏蔽 → 跳过该通道，不采集 */
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
        /* 事件流：低功耗通道扫描完成 → dispatch层将数据送入算法缓冲区 → 运行算法检测唤醒按键 */
        wkupFlag = SiAlgoProcress(&touchAlgoObject);        //处理低功耗object

        if (wkupFlag == SI_RT_WKUP || (rself->para.noise_force_wakeup && SiNoiseIsDetect4(&touchAlgoObject)))     /**< 状态转换条件：检测到有效按键或噪音 → 返回1唤醒系统 */
        {
            TC_LOGD(TAG, "touch wake up");
            return 1;
        }

        can_scan_mask = calc_can_scan_mask(rself);      //判断是否能扫描mask通道
        if (can_scan_mask != 0)     /**< 状态转换条件：达到mask通道扫描间隔 → 执行全扫 */ 
        {
            rself->data.sleep_scancnt = 0;
        }
        else                        /**< 状态转换条件：未到mask通道扫描间隔 → 跳过全扫，返回0保持睡眠 */
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

        if (use_wkupdata_as_touchdata)      /**< 状态分支：使用已采集的wkup数据作为touch数据（避免重复扫描） */
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
        else                            /**< 状态分支：重新采集全扫通道数据（非wkup复用路径） */
        {
            rself->data.cur_scaner = 0;
            rself->data.cur_scaner_channel = 0;
            /** 全扫通道扫描调度：遍历所有扫描器，采集所有非屏蔽通道 */
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

        /** 低功耗全扫完成后，切换算法对象锁：锁定低功耗算法对象，解锁常规算法对象 */
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
        /** 全扫数据就绪，运行触摸算法检测按键 */
        SiAlgoProcress(&touchAlgoObject);
        /** 恢复算法对象锁状态 */
        TouchHaltUnlockLpSiObject();            //解锁低功耗
        TouchHaltLockSiObject();                //锁定常规
    }

    return 0;
}

/**
 * @brief 复位底层硬件模块，并继续上次trig（虚函数）
 * @param self 触摸分发器实例指针
 * @note 在采集超时时调用，先复位触摸模块再重新配置底层
 */
void touch_haldispatchonlytouch_reset(struct TOUCH_HalDispatch_Type *self)                                     //复位底层硬件模块，并继续上次trig，虚函数
{
    Touch_Reset();        //复位模块
    touch_haldispatchonlytouch_setup(self);     //预备
}

/**
 * @brief 获取当前扫描器编号（虚函数）
 * @param self 触摸分发器实例指针
 * @return 当前扫描器编号
 */
static uint8_t touch_haldispatchonlytouch_get_scaner(struct TOUCH_HalDispatch_Type *self)                      //获取当前扫描器编号，虚函数
{
    return ((TOUCH_HalDispatch_OnlyTouch_Type *)self)->data.cur_scaner;
}

/**
 * @brief 预备，调用所有touch_node的low_init进行底层初始化
 * @param self 触摸分发器实例指针
 * @note 遍历所有扫描器，对每个touch_node执行low_init初始化底层硬件
 */
static void touch_haldispatchonlytouch_setup(struct TOUCH_HalDispatch_Type *self)              //预备
{
    uint8_t i;

    for (i = 0; i < self->scaner_table.scaner_len; ++i)               //调用touch_node的low_init
    {
        self->scaner_table.scaners[i].touch_node->low_init(self->scaner_table.scaners[i].touch_node);
    }
}

/**
 * @brief 低功耗模式下重新计算touch值，对touch_data应用偏置、倍采样、噪音抑制等功能
 * @param touch_data 原始触摸采样数据
 * @param can_scan_mask mask通道是否可扫描的标志（1=全扫，0=仅低功耗唤醒通道）
 * @param self 触摸分发器基类实例指针
 * @param rself 仅触摸分发器实例指针
 * @return 处理后的触摸数据
 * @note 数据处理链路：
 *       - 若有sleep_offset_table且为全扫模式，先应用偏置补偿
 *       - 中值滤波（全扫用center_filter_table，非全扫用sleep_center_filter_table）
 *       - 低通滤波
 *       - 倍采样计算
 *       - 噪音抑制（全扫用noise_avoid_table，非全扫用sleep_noise_avoid_table）
 */
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

/**
 * @brief 运行低功耗数据抗残留，丢弃几轮采样值以消除上电/模式切换后的残留电荷
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 * @note 抗残留流程：
 *       - 若启用快速抗残留(fast_remove_residues_enable)，先定位最后一个待扫描通道
 *       - 从起始通道开始，遍历所有非屏蔽的低功耗通道
 *       - 对每个通道执行一次采集但不保存数据（丢弃），共REMOVE_RESIDUES_LOSS_CNT轮
 *       - 低功耗模式下使用sleep_gainer_table配置增益
 *       - 采集完成后按需接地
 */
static void sleep_remove_residues_run(struct TOUCH_HalDispatch_Type *self, TOUCH_HalDispatch_OnlyTouch_Type *rself)       //运行低功耗数据抗残留，丢弃几轮采样值
{
#define REMOVE_RESIDUES_LOSS_CNT        1           /**< 抗残留丢弃次数，每次丢弃1轮采样数据 */

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

/**
 * @brief 运行常规模式数据抗残留，丢弃几轮采样值以消除上次残留电荷影响
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 * @note 与sleep_remove_residues_run区别：
 *       - 使用常规gainer_table（而非sleep_gainer_table）配置增益
 *       - 使用set_ioenable_asneed/set_iodisable_asneed（常规模式IO控制）
 *       - 不会跳过被sleep_channelmask屏蔽的通道
 */
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
/**
 * @brief 清倍采样缓冲区
 * @param scaner 扫描器配置表指针
 * @param only_sleep 为1时仅复位sleep数据(dummy1)，为0时同时复位normal(dummy1)和sleep(dummy2)数据
 */
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

/**
 * @brief 清噪音抑制器缓冲区
 * @param scaner 扫描器配置表指针
 * @param only_sleep 为1时仅复位sleep数据(dummy1)，为0时同时复位normal(dummy1)和sleep(dummy2)数据
 * @note 同时复位noise_avoid_table和sleep_noise_avoid_table两组缓冲区
 */
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

/**
 * @brief 清中值滤波缓冲区
 * @param scaner 扫描器配置表指针
 * @param only_sleep 为1时仅复位sleep数据(sleep_center_filter_table)，
 *                   为0时同时复位normal(center_filter_table)和sleep数据
 */
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

/**
 * @brief 清低通滤波器缓冲区
 * @param scaner 扫描器配置表指针
 * @param only_sleep 为1时仅复位sleep数据(dummy1)，为0时同时复位normal(dummy1)和sleep(dummy2)数据
 */
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

/**
 * @brief 打开或关闭低功耗模式下所有touch通道的IO功能
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 * @param enable 1=使能IO（取消接地），0=禁用IO（接地）
 * @note 遍历所有扫描器的所有通道，根据sleep_iodisable_channelmask
 *       配置是否需要操作IO。用于进入/退出低功耗模式时整体切换通道IO状态。
 */
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

/**
 * @brief 打开或关闭常规模式下所有touch通道的IO功能
 * @param self 触摸分发器实例指针
 * @param rself 仅触摸分发器实例指针
 * @param enable 1=使能IO（取消接地），0=禁用IO（接地）
 * @note 遍历所有扫描器的所有通道，根据iodisable_channelmask
 *       配置是否需要操作IO。用于常规模式下批量切换通道IO状态。
 */
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
/**
 * @brief 懒扫描配置，对延迟初始化的通道进行IO pinmux配置
 * @param self 仅触摸分发器实例指针
 * @param scanerid 扫描器ID
 * @note 当通道的lazy_scan等待时间到达后调用此函数：
 *       - 对所有标记为懒初始化的通道执行Touch_IOConfig
 *       - 若通道配置了shield，同步配置shield引脚
 *       - 清除lazy_scan.waitflag标志
 *       - 回调touch_lazy_scan_completed通知上层
 */
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

/**
 * @brief 懒扫描完成回调（弱函数，可被用户覆盖）
 * @param scanerid 扫描器ID
 */
__WEAK void touch_lazy_scan_completed(int scanerid)     //懒扫描完成
{
    (void)scanerid;
}
#endif

/**
 * @brief 计算低功耗模式下mask通道是否可以被扫描
 * @param self 仅触摸分发器实例指针
 * @retval 1 可以扫描mask通道（已到达sleep_mask_freq间隔）
 * @retval 0 不满足扫描条件，继续等待
 * @note 当sleep_mask_freq不为0时，每sleep_mask_freq次低功耗扫描中，
 *       仅最后1次允许扫描mask通道，其余次数仅采集非屏蔽通道数据。
 *       该机制在保证低功耗的同时，定期更新被屏蔽通道的数据。
 */
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

/**
 * @brief 丢弃非低功耗采样数据（以上电扫描周期计数）
 * @param cnt 需要丢弃的采样数据次数
 * @note 在touch上电初始化阶段调用，用于跳过前几轮不稳定的采样数据，
 *       直到cnt递减至0后才开始正常处理算法
 */
void Touch_HalDispatch_DiscardSampData(int cnt) //丢弃非低功耗采样数据，以每次扫描周期计数
{
    discardTouchSampDataCnt = cnt;
}
