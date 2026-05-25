#include "tc.h"
#include "tc_log.h"
#include "tcae10_ll_def.h"
#include "tc_usermsg.h"
#include "tc_halt.h"
#include "app.h"
#include "lin_process.h"
#include "lin_snpd.h"
#include "diagnosticIII.h"
#include "lin_task.h"
#include "pal_lin_comm.h"

static const char *TAG = "APP";

static volatile uint8_t linWakeupFlag = 0;
static T_TcTask *linTask = NULL;
static uint8_t linCommSincePowerOn = 0;

static void LinTask(uint32_t msg, void *param);    //Lin任务
static int LinHaltFilterWakeupCallback(void);      //Lin唤醒回调
static void LinHaltMonitorCallback(void);          //Lin低功耗监控器
extern void lin_goto_idle_state(void);
extern void lin_uncd_frame_handle(void);

// 通过LIN打印调试信息
#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
static void Lin_PrintLogInfo(void)
{
    uint8_t udsResponseData[LOG_RAMBUFFER_SIZE + 1];

    if (g_bUDSReadLogInfo)
    {
        const char *buffer;
        int buffLen = TC_LOG_GetRamBuffer(&buffer);

        g_bUDSReadLogInfo = FALSE;
        udsResponseData[0] = LIN_DEBUG_INFO_SID + 0x40;

//        if (buffLen > 0)
//        {
//            printf("buffLen=%d\n", buffLen);
//        }

        if (buffLen <= LOG_RAMBUFFER_SIZE)
        {
            memcpy(&udsResponseData[1], buffer, buffLen);
            ld_send_message(buffLen + 1, (const l_u8 *)udsResponseData);
        }

        TC_LOG_ClrRamBuffer();
    }
}
#endif

void LinInit(void)
{
    if ((linTask = TcTaskCreate("lin", LinTask, NULL, TC_TASK_PRIO_HIGH)) == NULL)   //创建Lin任务
    {
        TC_LOGE(TAG, "LinTask create fail");
    }
    else
    {
        TC_LOGI(TAG, "LinTask create ok");
    }
}

__WEAK void lin_customized_operation(void)
{
    ;
}
static void LinTask(uint32_t msg, void *param)    //Lin任务
{
    static T_TcTimer *linTimer = NULL;            //定时器
#if !(LIN_CUSTOM_MASTER_WKUP)
    static uint8_t wkupSlaveFlag = 0;
    static uint32_t wkupSlaveCount = 0;
#endif
    static uint16_t wkupBreakCount = 0;          //唤醒时收到break信号数
    static uint8_t linFirstCountFlag = 0;        //lin首个计数标志
    static uint32_t linLastTick;                 //lin超时计数器用

#if CFG_SUPPORT_LIN_SNPD
    static lin_snpd_context_t  lin_snpd_ctx =
    {
        .timeout = 0,
        .status = {0},
        .enter_func = NULL,
        .exit_func = NULL,
    };
#endif

    if (msg == MSG_TASK_INIT)
    {
        if ((linTimer = TcTimerCreate(TC_TIMER_TYPE_CIRCLE, 1, NULL, currentTask, NULL)) == NULL)
        {
            TC_LOGE(TAG, "appTimer create fail");
        }
        else
        {
            TcTimerStart(linTimer);
        }

        lin_process_init();

#if CFG_SUPPORT_LIN_SNPD || CFG_SUPPORT_DFU_MULT
        /* read nad */
        lin_snpd_nad_read(&lin_configured_NAD);
        /* nvr_pid_read(lin_configuration_RAM,LIN_SIZE_OF_CFG); read pid */
#endif

#ifdef CFG_LIN_CONFORM_TEST
        /* read frame_id */
        lin_snpd_id_read();
#endif

#if CFG_SUPPORT_LIN_SNPD
        lin_snpd_init(&lin_snpd_ctx);
#endif
        lin_customized_operation();
        if (HaltFilterWakeupRegister(LinHaltFilterWakeupCallback) <= 0)       //低功耗唤醒过滤函数注册失败
        {
            TC_LOGE(TAG, "LinHaltFilterWakeupCallback register fail");
        }
        if (HaltMonitorRegister(LinHaltMonitorCallback) <= 0)                 //低功耗监控器注册失败
        {
            TC_LOGE(TAG, "LinHaltMonitorCallback register fail");
        }

#if !(LIN_CUSTOM_MASTER_WKUP)
        wkupSlaveFlag = 1;
        wkupSlaveCount = TcTimeGet();
#endif
        wkupBreakCount = lin_lld_sci_rcv_break_cnt();
        linLastTick = TcTimeGet();
        linFirstCountFlag = 1;
    }

    if (msg == MSG_TASK_TIMER)
    {
        lin_diag_service_handle();
        lin_uncd_frame_handle();
#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
        Lin_PrintLogInfo();
#endif

        uint32_t curTick = TcTimeGet();
        if (linFirstCountFlag)
        {
            lin_lld_sci_timeout(1);
            linFirstCountFlag = 0;
        }
        else
        {
            lin_lld_sci_timeout(curTick - linLastTick);
        }
        linLastTick = curTick;

#if CFG_SUPPORT_LIN_SNPD
        lin_snpd_process_handle();
#endif

#if LOW_POWER_EN
        l_u8 linState = lin_lld_sci_get_state();
        if (linState != LIN_SLEEP_MODE)      //lin刷新低功耗超时定时器
        {
            HaltTimeoutReset();
        }

        if (lin_goto_sleep_flg) //lin命令进入低功耗
        {
            if (LinCanEnterSleep())
            {
                lin_goto_sleep_flg = 0;
                HaltEnter(GetHaltMode(), 0);     //进入低功耗
            }
        }
#endif

        if (lin_lld_sci_rcv_break_cnt() != wkupBreakCount)       //判断lin是否通信
        {
            linCommSincePowerOn = 1;
        }

#if !(LIN_CUSTOM_MASTER_WKUP)
        if (wkupSlaveFlag)      //按需1秒唤醒一次master
        {
            if (lin_lld_sci_rcv_break_cnt() != wkupBreakCount)  //判断lin是否通信
            {
                wkupSlaveFlag = 0;
            }
            else
            {
                if (TcTimeGet() - wkupSlaveCount > 1000 * TC_SYSTICK_HZ / 1000)
                {
                    lin_lld_slave_wakeup();     //发送wakeup信号
                    wkupSlaveCount = TcTimeGet();
                }
            }
        }
#endif
    }

    if (msg == MSG_TASK_ENTER_HALT)
    {
        linWakeupFlag = 0;
        lin_goto_sleep_flg = 0;

#if !(LIN_CUSTOM_MASTER_WKUP)
        wkupSlaveFlag = 0;
#endif

        ll_wakeup_init(WAKEUP_SOUERCE_LIN, WAKEUP_TIME_5, WAKEUP_FILTER_3);
        ASYSCFG->ICR |= ASYSCFG_INT_WAKEUP;
        ll_syscfg_isr_enable(ASYSCFG_INT_WAKEUP, true);
        EnableNvic(AON_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, ENABLE);
    }

    if (msg == MSG_TASK_WAKE_UP)
    {
        ll_syscfg_isr_enable(ASYSCFG_INT_WAKEUP, false);
        EnableNvic(AON_IRQn, TCAE10_DEFAULT_IRQ_LEVEL, DISABLE);
        lin_goto_idle_state();

#if !(LIN_CUSTOM_MASTER_WKUP)
        lin_lld_slave_wakeup();     //发送wakeup信号
        wkupSlaveFlag = 1;
        wkupSlaveCount = TcTimeGet();
#endif
        wkupBreakCount = lin_lld_sci_rcv_break_cnt();
        linLastTick = TcTimeGet();
        linFirstCountFlag = 1;
    }
}

//上电之后lin是否通信过
unsigned char LinCommSincePowerOn(void)
{
    return linCommSincePowerOn;
}

//销毁lin
void LinDestroy(void)
{
    if (linTask)
    {
        TcTaskForceDestroy(linTask);
        linTask = NULL;

        pal_lin_deinit(LIN_BUS_0);

        CRG_CONFIG_UNLOCK();
        CRG->LIN_SCI_CLKRST_CTRL_F.PCLK_EN_LIN_SCI = 0;
        CRG_CONFIG_LOCK();
    }
}

//返回1表示收到lin sleep命令后可以进入低功耗，0表示不可以
__WEAK int32_t LinCanEnterSleep(void)
{
    return 1;
}

/********************************************************
** \brief   lin_uncd_frame_handle
**
** \param   None
**
** \retval  None
*********************************************************/
__attribute__((weak)) void lin_uncd_frame_handle(void)
{
    //do noting
}

static int LinHaltFilterWakeupCallback(void)
{
    if (!linTask)
    {
        return 0;
    }
    return linWakeupFlag;
}

static void LinHaltMonitorCallback(void)
{
    if (!linTask)
    {
        return;
    }

    if (lin_lld_sci_get_state() != LIN_SLEEP_MODE)      //lin协议栈唤醒
    {
        linWakeupFlag = 1;
    }
}

__WEAK void LvdIRQHandler(void)
{

}

void AON_IRQHandler(void)
{
    uint32_t isr = ASYSCFG->ISR;

    ASYSCFG_CONFIG_UNLOCK();

    if (ll_syscfg_isr_get(ASYSCFG_INT_WAKEUP))  //wakeup
    {
        linWakeupFlag = 1;
    }
    if (ll_syscfg_isr_get(ASYSCFG_INT_VSLVD)) // lvd
    {
        LvdIRQHandler();
    }

    ASYSCFG->ICR |= isr;

    ASYSCFG_CONFIG_LOCK();
}
