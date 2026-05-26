/**
*****************************************************************************
* @brief  touch端口适配层，提供触摸初始化、任务调度、低功耗管理及中断处理
* @file   si_touch_port.c
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

#include "tc.h"
#include "tc_log.h"
#include "tcae10_ll_def.h"
#include "si_include.h"
#include "touch_config.h"
#include "tc_usermsg.h"
#include "si_touch_port.h"
#include "touch_tool.h"
#include "tc_halt.h"
#include "app.h"

/** @brief 日志标签 */
static const char *TAG = "TOUCH_PORT";

/** @brief touch分发器指针 */
TOUCH_HalDispatch_Type *touchDispatch = NULL;

/** @brief touch任务句柄 */
T_TcTask *touchTaskHandle = NULL;

/** @brief 使能采集touch数据标志，1表示开启，0表示关闭 */
static uint8_t enableSamp = 1;

/** @brief 强制从低功耗唤醒标志 */
static uint8_t forceWakeup = 0;

/** @brief 强制设置任务标志位 */
static uint8_t forceSetTaskBitFlag = 0;

/**
 * @brief  触摸任务处理函数
 * @param[in] msg  消息类型
 * @param[in] param 消息参数指针
 */
static void TouchTask(uint32_t msg, void *param);

/**
 * @brief  touch低功耗监控器回调函数
 */
static void TouchHaltMonitorCallback(void);

/**
 * @brief  触摸初始化
 * @note   初始化算法对象、配置参数、使能NVIC中断，并创建触摸检测任务
 */
void TouchInit(void)
{
    SiGetTimeMs = TouchGetTime;

    (void)SiAlgoInit(&touchAlgoObject);       //touch算法对象初始化

    TouchConfig(&touchAlgoObject);

    EnableNvic(TOUCH_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, ENABLE);

    TC_LOGI(TAG, "Touch Algo Version:%d.%d , Comp Time:%s", SiAlgoVersion() / 100, SiAlgoVersion() % 100, SiAlgoCompileTime());    //打印算法版本

    if ((touchTaskHandle = TcTaskCreate("touch", TouchTask, NULL, TC_TASK_PRIO_HIGH)) == NULL)     //创建触摸检测任务
    {
        TC_LOGE(TAG, "TouchTask create fail");
    }
    else
    {
        TC_LOGI(TAG, "TouchTask create ok");
    }
}

/**
 * @brief  低功耗唤醒源过滤回调函数
 * @retval 0 不需要唤醒（touch采集关闭或无需运行）
 * @retval 1 需要唤醒（快扫模式或强制唤醒）
 * @note   当enableSamp为0时，增加延迟等待TIM_LITE指令生效，防止死机
 */
__WEAK int TouchHaltFilterWakeupCallback(void)
{
    if (!enableSamp)     //不采集touch
    {
        for (int i = 0; i < 100; ++i)       //延迟不能删除，增加tc_halt中TIM_LITE->CTRL_F.PAUSE开关之间延迟，因为TIM_LITE时钟是32K，需要等待指令生效，否则会死机
        {
            __NOP();
        }
        return 0;
    }

#if TOUCH_FAST2SLOW_SWITCHTIMEOUT     //快慢扫功能打开
    if (forceWakeup || touchDispatch->sleep_run(touchDispatch))
    {
        Touch_HalDispatch_SetFastScan(touchDispatch, 1);   //快扫模式
        return 1;
    }
#else
    if (forceWakeup || touchDispatch->sleep_run(touchDispatch))
    {
        return 1;
    }
#endif

    return 0;
}

/**
 * @brief  锁定管理低功耗的T_SiObject
 * @note   弱函数，由用户自行实现低功耗对象锁定
 */
__WEAK void TouchHaltLockLpSiObject(void)
{
}

/**
 * @brief  解锁管理低功耗的T_SiObject
 * @note   弱函数，由用户自行实现低功耗对象解锁
 */
__WEAK void TouchHaltUnlockLpSiObject(void)
{
}

/**
 * @brief  锁定常规的T_SiObject
 * @note   弱函数，由用户自行实现常规对象锁定
 */
__WEAK void TouchHaltLockSiObject(void)
{
}

/**
 * @brief  解锁常规的T_SiObject
 * @note   弱函数，由用户自行实现常规对象解锁
 */
__WEAK void TouchHaltUnlockSiObject(void)
{
}

/**
 * @brief  touch低功耗监控器回调函数
 * @note   在低功耗模式下周期性检查touch模块运行状态，超时则复位touch
 */
static void TouchHaltMonitorCallback(void)
{
#if WATCH_DOG_EN
    ll_wdg_reload();
#endif
    if (!enableSamp)     //不采集touch
    {
        return;
    }

    if (++touchHaltMonitorCnt > TOUCH_HALT_MONITOR_TIMEOUT)
    {
        TC_LOGW(TAG, "touch node%d timeout", touchDispatch->get_scaner(touchDispatch));
        touchHaltMonitorCnt = 0;

        //有理由怀疑touch模块挂起，需要复位并重新触发
        touchDispatch->reset(touchDispatch);
    }
}

/**
 * @brief  触摸任务处理函数
 * @param[in] msg  消息类型（MSG_TASK_INIT / MSG_TASK_TIMER / MSG_TASK_BITFLAG / MSG_TASK_ENTER_HALT / MSG_TASK_WAKE_UP）
 * @param[in] param 消息参数指针
 * @note   根据消息类型执行初始化、快慢扫切换、低功耗进入/唤醒等操作
 */
static void TouchTask(uint32_t msg, void *param)    //触摸任务
{
#if TOUCH_FAST2SLOW_SWITCHTIMEOUT     //快慢扫功能打开
    static uint8_t is_fast_mode = 1;  //是否处在快扫模式
    static uint32_t lastSlowSampTime = 0; //上次慢扫采样时间
#endif
    static T_TcTimer *touchTimer = NULL;            //触摸定时器

    if (msg == MSG_TASK_INIT)
    {
        if ((touchTimer = TcTimerCreate(TC_TIMER_TYPE_CIRCLE, 1, NULL, currentTask, NULL)) == NULL)
        {
            TC_LOGE(TAG, "touchTimer create fail");
        }
        else
        {
            TcTimerStart(touchTimer);
        }
        if (HaltFilterWakeupRegister(TouchHaltFilterWakeupCallback) <= 0)       //低功耗唤醒过滤函数注册失败
        {
            TC_LOGE(TAG, "TouchHaltFilterWakeupCallback register fail");
        }
        if (HaltMonitorRegister(TouchHaltMonitorCallback) <= 0)                 //低功耗监控器注册失败
        {
            TC_LOGE(TAG, "TouchHaltMonitorCallback register fail");
        }
    }

    if (msg == MSG_TASK_TIMER || msg == MSG_TASK_BITFLAG)
    {
        if (!enableSamp)     //关闭touch采集
        {
            return;
        }

#if TOUCH_FAST2SLOW_SWITCHTIMEOUT     //快慢扫功能打开
        //判断快慢扫状态是否改变
        if (is_fast_mode)       //快扫模式
        {
            if ((Touch_HalDispatch_GetFastScan(touchDispatch) && TouchGetTime() - touchDispatch->fast2slow_scan.last_fast_time >= TOUCH_FAST2SLOW_SWITCHTIMEOUT) ||
                    !Touch_HalDispatch_GetFastScan(touchDispatch)) //切换到慢扫
            {
                is_fast_mode = 0;
                Touch_HalDispatch_SetFastScan(touchDispatch, 0);
                lastSlowSampTime = TouchGetTime();
                TC_LOGD(TAG, "slow mode");

                TouchEnterHaltHook();
                touchDispatch->run(touchDispatch, MSG_TASK_ENTER_HALT, 0);
            }
        }
        else   //慢扫模式
        {
            if (Touch_HalDispatch_GetFastScan(touchDispatch))      //切换到快扫
            {
                is_fast_mode = 1;
                Touch_HalDispatch_SetFastScan(touchDispatch, 1);
                TC_LOGD(TAG, "fast mode");

                TouchWakeupHook();
                touchDispatch->run(touchDispatch, MSG_TASK_WAKE_UP, 0);
            }
        }

        if (is_fast_mode)   //快扫模式
        {
            touchDispatch->run(touchDispatch, msg, param);
        }
        else        //慢扫模式
        {
            if (TouchGetTime() - lastSlowSampTime >= SiIFastDiv(1000, touchDispatch->fast2slow_scan.slow_freq))
            {
                lastSlowSampTime = TouchGetTime();
                touchHaltRtcTrigFlag = 1;
                if (touchDispatch->sleep_run(touchDispatch))    //切换到快扫模式
                {
                    Touch_HalDispatch_SetFastScan(touchDispatch, 1);
                }
            }
        }
#else
        touchDispatch->run(touchDispatch, msg, param);
#endif
    }

    if (msg == MSG_TASK_ENTER_HALT)
    {
        forceWakeup = 0;    //关闭touch强制唤醒

#if TOUCH_FAST2SLOW_SWITCHTIMEOUT     //快慢扫功能打开
        is_fast_mode = 0;
        Touch_HalDispatch_SetFastScan(touchDispatch, 0);    //进入慢扫模式

        touchHaltMonitorCnt = 0;
        TouchEnterHaltHook();
        if (enableSamp)
        {
            touchDispatch->run(touchDispatch, msg, param);
        }
#else
        touchHaltMonitorCnt = 0;
        TouchEnterHaltHook();
        if (enableSamp)
        {
            touchDispatch->run(touchDispatch, msg, param);
        }
#endif
        TouchRtcTrigConfig(touchDispatch->sleep_freq, 1);     //开启RTC中断
    }

    if (msg == MSG_TASK_WAKE_UP)
    {
#if TOUCH_FAST2SLOW_SWITCHTIMEOUT     //快慢扫功能打开
        HaltTimeoutReset();
        TcTaskClrMsg(touchTaskHandle);             //清空自身所有消息缓冲区
#else
        HaltTimeoutReset();
        (void)TcTaskClrMsg(touchTaskHandle);             //清空自身所有消息缓冲区
        TouchWakeupHook();
        if (enableSamp)
        {
            touchDispatch->run(touchDispatch, msg, param);      //在TcTaskClrMsg之后调用
        }
#endif
        TouchRtcTrigConfig(touchDispatch->sleep_freq, 0);     //关闭RTC中断
    }
}

/**
 * @brief  使能touch采集
 * @param[in] enable 使能标志，1表示开启，0表示关闭
 */
void TouchEnableSamp(uint8_t enable)
{
    enableSamp = enable;
}

/**
 * @brief  强制将touch从低功耗唤醒
 * @note   设置forceWakeup标志，下次唤醒过滤时将触发唤醒
 */
void TouchForceWakeup(void)
{
    forceWakeup = 1;
}

/**
 * @brief  强制touch扫描并运行一次算法
 * @note   切换到touch任务上下文中执行一次算法运行，执行后恢复原任务上下文
 */
void TouchForceRunAlgoOnce(void)
{
    T_TcTask *bkupContex = currentTask;
    currentTask = touchTaskHandle;      //切换到touchtask环境中执行
    touchDispatch->run(touchDispatch, MSG_TASK_TIMER, (void *)0xA5A5A5A5);
    currentTask = bkupContex;

#if WATCH_DOG_EN
    ll_wdg_reload();
#endif
}

/**
 * @brief  touch中断处理函数
 * @note   处理采样溢出和触发完成中断，非低功耗或强制设置时置位任务标志
 */
void TOUCH_IRQHandler()
{
    if (CAPTOUCH->ISR & CAPTOUCH_SAMP_OVF)
    {
        CAPTOUCH->ICR |= CAPTOUCH_SAMP_OVF;
        TC_LOGE(TAG, "captouch samp wait time overflow");
    }

    if (CAPTOUCH->ISR & CAPTOUCH_TRIG_DONE)
    {
        CAPTOUCH->ICR |= CAPTOUCH_TRIG_DONE;

        if (!Touch_HalDispatch_GetInSleep(touchDispatch) || forceSetTaskBitFlag)       //非低功耗或强制设置
        {
            TcTaskSetBitFlag(touchTaskHandle, 0x1U);
        }
        Touch_HalInterface_SetReady(touchDispatch->scaner_table.scaners[touchDispatch->get_scaner(touchDispatch)].touch_node, 1);   //设置转换完成标志位
    }
}

/**
 * @brief  touch中断强制设置MSG_TASK_BITFLAG
 * @param[in] force 强制标志，1表示强制设置，0表示不强制
 */
void TouchSetTaskBitForce(uint8_t force)
{
    forceSetTaskBitFlag = force;
}
