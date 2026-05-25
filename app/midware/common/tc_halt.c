/**
*****************************************************************************
* @brief  tc halt source
* @file   tc_halt.c
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

#include "tcae10_ll_def.h"
#include "tc.h"
#include "tc_log.h"
#include "tc_halt.h"
#include "tc_usermsg.h"

static const char *TAG = "HALT";

#define EMC_TEST_EN     0

#define GEN_HALTMSG_PARAM(type,v)   (((uint32_t)(type)<<16) | (v))
#define PARSE_HALTMSG_PARAM_TYPE(param)     ((uint16_t)((param)>>16))
#define PARSE_HALTMSG_PARAM_VALUE(param)    ((uint16_t)((param)&0xFFFFU))

volatile uint8_t touchHaltRtcTrigFlag = 0;           //触摸低功耗rtc触发计数器
static uint8_t inHaltFlag = 0;  //低功耗标记
static int haltMonitorTimerCount = 0;

static struct
{
    uint8_t num;
    HALT_FILTER_WAKEUP_CALLBACK callback[MAX_FILTER_WAKEUP_CALLBACK_NUM];
} filterWakeupCallback = {0};               //低功耗唤醒后过滤函数

static T_TcTask *haltTask = NULL;
static T_TcTimer *haltTimeoutTimer = NULL;            //低功耗超时定时器

static struct
{
    uint8_t num;
    uint8_t waveFlag;       //检测到RTC wave标志，需要调用回调接口
    HALT_MONITOR_CALLBACK callback[MAX_HALT_MONITOR_CALLBACK_NUM];
} haltMonitorCallback = {0};               //halt监控器回调接口

static uint16_t haltmode = SLEEPWALK_MODE;

static void HaltTask(uint32_t msg, void *param);        //低功耗任务
static void LowLevelHaltEnter(uint16_t type, uint16_t v);   //底层进入低功耗
static void SystemLowPowerInit(void);                   //降低功耗相关

static void HaltMonitorRtcWaveSw(uint8_t sw);       //halt监控器rtc wave开关

static void SetSwdDisableOnDemand(uint8_t disable);      //按需开关swd功能

/**
 * @brief  低功耗任务初始化
 * @note   创建名为"halt"的关键优先级任务，用于管理系统低功耗
 *         进入/退出流程。创建失败时打印错误日志
 */
void HaltInit(void)
{
    if ((haltTask = TcTaskCreate("halt", HaltTask, NULL, TC_TASK_PRIO_CRITICAL)) == NULL)           //创建低功耗任务失败
    {
        TC_LOGE(TAG, "HaltTask create failed");
    }
    else
    {
        TC_LOGI(TAG, "HaltTask create success");
    }
}

/**
 * @brief  注册低功耗唤醒后过滤函数
 * @param  callback - 过滤函数指针，返回1表示有效唤醒，0则继续休眠
 * @retval 1 - 注册成功，-1 - 注册缓冲区已满
 * @note   最多注册MAX_FILTER_WAKEUP_CALLBACK_NUM个过滤函数
 */
int HaltFilterWakeupRegister(HALT_FILTER_WAKEUP_CALLBACK callback)
{
    if (filterWakeupCallback.num >= MAX_FILTER_WAKEUP_CALLBACK_NUM)
    {
        return -1;
    }
    filterWakeupCallback.callback[filterWakeupCallback.num++] = callback;
    return 1;
}

/**
 * @brief  清空所有已注册的低功耗唤醒过滤函数
 */
void HaltFilterWakeupClear(void)
{
    filterWakeupCallback.num = 0;
}

/**
 * @brief  进入低功耗模式
 * @param  type - 低功耗模式类型（见SLEEP_MODE_E）
 * @param  v - 参数，暂未使用，填0
 * @retval 1 - 进入低功耗消息发送成功，-1 - 发送失败
 * @note   先清空所有任务消息确保进入低功耗消息可送达，
 *         封装type/v为32位参数发送MSG_TASK_PREENTER_HALT消息
 */
int HaltEnter(uint16_t type, uint16_t v)
{
    uint32_t param;

    TcTaskClrAllMsg();      //清空所有消息，确保进入低功耗消息可以发送成功
    param = GEN_HALTMSG_PARAM(type, v);
    return TcTaskSendMsg(haltTask, MSG_TASK_PREENTER_HALT, (void *)param);
}

/**
 * @brief  修改进入低功耗超时定时器的超时周期
 * @param  periodTick - 新的超时周期（tick数）
 * @note   定时器未初始化时打印警告日志
 */
void HaltTimeoutChgPeriod(uint32_t periodTick)
{
    if (!haltTimeoutTimer)
    {
        TC_LOGW(TAG, "haltTimeoutTimer not init");
    }
    else
    {
        TcTimerChgPeriod(haltTimeoutTimer, periodTick);
    }
}

/**
 * @brief  重置进入低功耗超时定时器的超时时间
 * @note   每次有通信活动时调用，延迟进入低功耗
 *         定时器未初始化时打印警告日志
 */
void HaltTimeoutReset(void)
{
    if (!haltTimeoutTimer)
    {
        TC_LOGW(TAG, "haltTimeoutTimer not init");
    }
    else
    {
        TcTimerCntReset(haltTimeoutTimer);
    }
}

/**
 * @brief  注册halt低功耗监控器回调
 * @param  callback - 监控器回调函数指针，低功耗期间周期调用
 * @retval 1 - 注册成功，-1 - 注册缓冲区已满
 * @note   最多注册MAX_HALT_MONITOR_CALLBACK_NUM个监控器
 */
int HaltMonitorRegister(HALT_MONITOR_CALLBACK callback)
{
    if (haltMonitorCallback.num >= MAX_HALT_MONITOR_CALLBACK_NUM)
    {
        return -1;
    }
    haltMonitorCallback.callback[haltMonitorCallback.num++] = callback;
    return 1;
}

/**
 * @brief  判断系统是否处于halt低功耗模式
 * @retval 1 - 处于halt模式，0 - 正常运行
 */
uint8_t IsHaltMode(void)
{
    return inHaltFlag;
}

/**
 * @brief  halt低功耗主任务处理函数
 * @param  msg - 消息类型：
 *         MSG_TASK_INIT - 初始化低功耗配置和超时定时器
 *         MSG_TASK_TIMER - 超时触发自动进入低功耗
 *         MSG_TASK_PREENTER_HALT - 执行进入低功耗流程：
 *           广播MSG_TASK_ENTER_HALT -> 关闭SysTick -> 循环睡眠
 *           直至检测到有效唤醒 -> 广播MSG_TASK_WAKE_UP
 *         MSG_TASK_ENTER_HALT - 进入低功耗时关闭SWD
 *         MSG_TASK_WAKE_UP - 从低功耗唤醒时开启SWD
 * @param  param - PREENTER_HALT时包含type(高16位)和v(低16位)
 */
static void HaltTask(uint32_t msg, void *param)
{
    uint16_t i;
    uint16_t type;
    uint16_t v;
    uint32_t validWkupFlag;      //有效唤醒标记

    if (msg == MSG_TASK_INIT)
    {
        SystemLowPowerInit();               //降低功耗相关

        if ((haltTimeoutTimer = TcTimerCreate(TC_TIMER_TYPE_CIRCLE, ENTER_HALT_TIMEOUT_MS, NULL, currentTask, NULL)) != NULL)
        {
            TcTimerStart(haltTimeoutTimer);
        }
        else
        {
            TC_LOGE(TAG, "haltTimeoutTimer create fail");
        }
    }

    if (msg == MSG_TASK_TIMER)      //超时，准备进入低功耗
    {
        (void)HaltEnter(GetHaltMode(), 0);
    }

    if (msg == MSG_TASK_PREENTER_HALT)                                  //进入低功耗
    {
        TC_LOGD(TAG, "enter halt");
        inHaltFlag = 1;
#if LOG_SYSTEM_STATUS
        TC_LOG_SYSTEM_STATUS(TC_LOG_SYSTEM_HALT);
#endif

        TcTimerStop(haltTimeoutTimer);          //停止超时定时器
        SysTick_Switch(0);                      //关闭systick
        TcTaskBroadcastHandleMsg(MSG_TASK_ENTER_HALT, param);   //广播进入低功耗消息
        HaltMonitorRtcWaveSw(1);                //开启halt监控器

        type = PARSE_HALTMSG_PARAM_TYPE((uint32_t)param);
        v = PARSE_HALTMSG_PARAM_VALUE((uint32_t)param);

#if TOUCH_USE_EXT_LDO
#else
        ASYSCFG_CONFIG_UNLOCK();
        ASYSCFG->LDO12C_CTRL_F.LDO12C_EN = 0;
        ASYSCFG->LDO12C_CTRL_F.LDO12C_WKUP_EN = 1;
        ASYSCFG_CONFIG_LOCK();
#endif

        do
        {
            ASYSCFG_CONFIG_UNLOCK();
#if TOUCH_USE_EXT_LDO
#else
            ASYSCFG->LDO12C_CTRL_F.LDO12C_HW_EN_CLR = 0x1;
#endif
            ASYSCFG->LDO15_CTRL_F.LDO15_DL_IBASE_SEL = 0x4;
            ASYSCFG_CONFIG_LOCK();

#if LOG_LOCAL_LEVEL != TC_LOG_NONE
            PrintEnterSleep();      //关闭打印口
#endif

            //进入睡眠前开启Timelite 具体配置需要app中指定
            TIM_LITE->CTRL_F.PAUSE = 0;     //TIMERLITE_RESUME();
            LowLevelHaltEnter(type, v);
            //暂停Timelite 防止在触发外设同时app对外设进行配置
            TIM_LITE->CTRL_F.PAUSE = 1;     //TIMERLITE_PAUSE();

#if LOG_LOCAL_LEVEL != TC_LOG_NONE
            PrintWakeup();  //开启打印口
#endif

            ASYSCFG_CONFIG_UNLOCK();
            ASYSCFG->LDO15_CTRL_F.LDO15_DL_IBASE_SEL = 0x0;
            ASYSCFG_CONFIG_LOCK();

            if (filterWakeupCallback.num != 0)                   //已经注册过滤函数
            {
                validWkupFlag = 0;
                for (i = 0; i < filterWakeupCallback.num; ++i)
                {
                    validWkupFlag |= (uint32_t)filterWakeupCallback.callback[i]();
                }
            }
            else                                                                    //未注册过滤函数
            {
                validWkupFlag = 1;
            }

            if (haltMonitorCallback.waveFlag != 0 && haltMonitorCallback.num != 0)      //运行halt监控器
            {
                haltMonitorCallback.waveFlag = 0;
                for (i = 0; i < haltMonitorCallback.num; i++)
                {
                    haltMonitorCallback.callback[i]();
                }
            }
        }
        while (validWkupFlag == 0);             //循环进入低功耗，直到检测到有效唤醒标记，或者未注册有效唤醒标记
        HaltMonitorRtcWaveSw(0);            //关闭halt监控器
        TcTaskBroadcastHandleMsg(MSG_TASK_WAKE_UP, param);  //广播唤醒消息
        SysTick_Switch(1);                  //打开systick
        TcTimerStart(haltTimeoutTimer);     //重启超时定时器
        haltMonitorCallback.waveFlag = 0;

        TC_LOGD(TAG, "wake up");
        inHaltFlag = 0;
#if LOG_SYSTEM_STATUS
        TC_LOG_SYSTEM_STATUS(TC_LOG_SYSTEM_NORMAL);
#endif
    }

    if (msg == MSG_TASK_ENTER_HALT)     //进入低功耗
    {
        SetSwdDisableOnDemand(1);
    }

    if (msg == MSG_TASK_WAKE_UP)          //从低功耗唤醒
    {
        SetSwdDisableOnDemand(0);
    }
}

/**
 * @brief  底层MCU进入低功耗睡眠
 * @param  type - 睡眠模式类型（SLEEP_MODE_E）
 * @param  v - 保留参数，暂未使用
 * @note   调用ll_lpm_mcu_enter进入指定睡眠模式
 */
static void LowLevelHaltEnter(uint16_t type, uint16_t v)
{
    (void)v;

    ll_lpm_mcu_enter((sleep_mode_e)type, 0);
}

/**
 * @brief  系统低功耗初始化配置
 * @note   配置各LDO的上电唤醒周期和偏置电流：
 *         - LDO15_DL：设置偏置电流0x0，使能硬件开关
 *         - LDO33_DL：设置偏置电流0x0，使能硬件开关
 *         - 缩短唤醒周期以快速响应
 */
static void SystemLowPowerInit(void)
{
    ASYSCFG_CONFIG_UNLOCK();
    ASYSCFG->WAIT_CYCLE_F.PUP_CYCLE_LDO15_DL_HW_EN = 0x4;   //可以比4小
    ASYSCFG->WAIT_CYCLE_F.PUP_CYCLE_LDO33_DL_HW_EN = 0x0;
    ASYSCFG->WAIT_CYCLE_F.PUP_CYCLE_HFCLK = 0x0;                //可以比4小

    ASYSCFG->LDO15_CTRL_F.LDO15_DL_IBASE_SEL = 0x0;
    ASYSCFG->LDO15_CTRL_F.LDO15_DL_SW_ENB = 0x1;

    ASYSCFG->LDO33_CTRL_F.LDO33_DL_IBASE_SEL = 0x0;
    ASYSCFG->LDO33_CTRL_F.LDO33_DL_SW_ENB = 0x1;
    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief  halt监控器RTC wave开关控制
 * @param  sw - 开关（当前实现仅复位haltMonitorTimerCount）
 * @note   开启时复位计数值，由TIMER_IRQHandler周期性递增，
 *         达到阈值后触发haltMonitorCallback回调
 */
static void HaltMonitorRtcWaveSw(uint8_t sw)
{
    (void)sw;

    haltMonitorTimerCount = 0;
}

/**
 * @brief  按需开关SWD调试功能
 * @param  disable - 1关闭SWD，0开启SWD
 * @note   当前实现为空（代码被注释），预留用于在低功耗
 *         进入/退出时配置SWD引脚以降低功耗
 */
static void SetSwdDisableOnDemand(uint8_t disable)
{
//#if (DISABLE_SWD_AFTER_POWERON_TIMEMS != 0)
//    static uint8_t mayEnable = 1;

//    //GPIOB_22, GPIOB_23,        // SWDIO SWCLK
//    if (disable != 0)
//    {
//        GPIO_SET_AFMODE(GPIOB_22, AF6);
//        GPIO_SET_AFMODE(GPIOB_23, AF6);
//        if (mayEnable != 0)
//        {
//            TC_LOGI(TAG, "swd disable");
//        }
//    }
//    else
//    {
//        if (mayEnable != 0)
//        {
//            GPIO_SET_AFMODE(GPIOB_22, AF0);
//            GPIO_SET_AFMODE(GPIOB_23, AF0);
//            TC_LOGI(TAG, "swd enable");
//        }
//    }

//    if (mayEnable != 0)      //超时后锁定mayEnable
//    {
//        if (TcTimeGet() - TC_SYSTICK_INIT_VALUE >= DISABLE_SWD_AFTER_POWERON_TIMEMS * TC_SYSTICK_HZ / 1000)
//        {
//            mayEnable = 0;
//        }
//    }
//#endif
}

/**
 * @brief  获取当前低功耗模式
 * @retval 当前低功耗模式类型（默认SLEEPWALK_MODE）
 */
uint16_t GetHaltMode(void)
{
    return haltmode;
}

/**
 * @brief  设置低功耗模式
 * @param  mode - 低功耗模式类型（SLEEP_MODE_E中的值）
 */
void SetHaltMode(uint16_t mode)
{
    haltmode = mode;
}

/**
 * @brief  TIM_LITE定时器中断处理函数
 * @note   处理计数器下溢中断，完成以下工作：
 *         1. 置位触摸低功耗RTC触发标志touchHaltRtcTrigFlag
 *         2. 累积计时，每约600ms触发一次haltMonitorCallback回调
 *         3. 同步补偿系统嘀嗒计数器（TcSystick）
 */
void TIMER_IRQHandler(void)
{
    if (TIM_LITE->ISR_F.CNT_UDF_INT_STATUS)
    {
        TIM_LITE->ICR_F.CNT_UDF_INT_CLR = 1;

        touchHaltRtcTrigFlag = 1;

        haltMonitorTimerCount += TIM_LITE->INIT_VAL;
        if (haltMonitorTimerCount >= 32768 * 600 / 1000)        /* 约600ms执行一次 */
        {
            haltMonitorTimerCount = 0;
            haltMonitorCallback.waveFlag = 1;
            g_TcSystickIntCnt += (SYSTICK_COUNTS * 600);
            TcSystick += 600 * TC_SYSTICK_HZ / 1000;
        }
    }
}
