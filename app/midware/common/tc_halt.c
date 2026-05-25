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

void HaltInit(void)         //低功耗任务初始化
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

int HaltFilterWakeupRegister(HALT_FILTER_WAKEUP_CALLBACK callback)  //注册低功耗唤醒后过滤函数,1成功，0注册缓冲区已满，最大注册个数见：MAX_FILTER_WAKEUP_CALLBACK_NUM
{
    if (filterWakeupCallback.num >= MAX_FILTER_WAKEUP_CALLBACK_NUM)
    {
        return -1;
    }
    filterWakeupCallback.callback[filterWakeupCallback.num++] = callback;
    return 1;
}

void HaltFilterWakeupClear(void)                        //清空注册的过滤函数
{
    filterWakeupCallback.num = 0;
}

int HaltEnter(uint16_t type, uint16_t v)        //进入低功耗，返回1表示进入低功耗成功，返回-1表示失败
{
    uint32_t param;

    TcTaskClrAllMsg();      //清空所有消息，确保进入低功耗消息可以发送成功
    param = GEN_HALTMSG_PARAM(type, v);
    return TcTaskSendMsg(haltTask, MSG_TASK_PREENTER_HALT, (void *)param);
}

void HaltTimeoutChgPeriod(uint32_t periodTick)        //修改进入低功耗超时定时器的超时周期
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

void HaltTimeoutReset(void)                                         //重置进入低功耗超时定时器的超时时间
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

int HaltMonitorRegister(HALT_MONITOR_CALLBACK callback) //注册halt监控器,1成功，-1注册缓冲区已满，最大注册个数见：MAX_HALT_MONITOR_CALLBACK_NUM
{
    if (haltMonitorCallback.num >= MAX_HALT_MONITOR_CALLBACK_NUM)
    {
        return -1;
    }
    haltMonitorCallback.callback[haltMonitorCallback.num++] = callback;
    return 1;
}

uint8_t IsHaltMode(void)    //是否处在halt模式
{
    return inHaltFlag;
}

static void HaltTask(uint32_t msg, void *param)         //低功耗任务
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

static void LowLevelHaltEnter(uint16_t type, uint16_t v)    //底层进入低功耗
{
    (void)v;

    ll_lpm_mcu_enter((sleep_mode_e)type, 0);
}

static void SystemLowPowerInit(void)                    //降低功耗相关
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

static void HaltMonitorRtcWaveSw(uint8_t sw)        //halt监控器rtc wave开关
{
    (void)sw;

    haltMonitorTimerCount = 0;
}

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

uint16_t GetHaltMode(void)  //获取低功耗模式
{
    return haltmode;
}

void SetHaltMode(uint16_t mode) //设置低功耗模式
{
    haltmode = mode;
}

void TIMER_IRQHandler(void)
{
    if (TIM_LITE->ISR_F.CNT_UDF_INT_STATUS)
    {
        TIM_LITE->ICR_F.CNT_UDF_INT_CLR = 1;

        touchHaltRtcTrigFlag = 1;

        haltMonitorTimerCount += TIM_LITE->INIT_VAL;
        if (haltMonitorTimerCount >= 32768 * 600 / 1000)        //约600ms执行一次
        {
            haltMonitorTimerCount = 0;
            haltMonitorCallback.waveFlag = 1;
            g_TcSystickIntCnt += (SYSTICK_COUNTS * 600);
            TcSystick += 600 * TC_SYSTICK_HZ / 1000;
        }
    }
}
