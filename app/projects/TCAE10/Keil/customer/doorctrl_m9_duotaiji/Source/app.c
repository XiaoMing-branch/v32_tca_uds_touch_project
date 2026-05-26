/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
  ******************************************************************************
  * @brief  application main file.
  *
  * @file   app.c
  * @author AE/FAE team
  * @date
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
  *
  ******************************************************************************
  */
#ifdef ENABLE_TEST_MODE
#include "fff_tcae10.h"
#include "fff_tcae10_ll_def.h"
#include "fff_tc_log.h"
#include "fff_tc_halt.h"
#include "fff_misc.h"
#include "fff_tc.h"
#include "fff_app.h"
#include "fff_lin_task.h"
#include "fff_si_touch_port.h"
#include "fff_touch_config.h"
#include "fff_lin_frame.h"
#include "fff_store_manager.h"
#include "fff_diagnosticIII.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "tcae10.h"
#include "tcae10_ll_def.h"
#include "tc_log.h"
#include "tc_halt.h"
#include "misc.h"
#include "tc.h"
#include "app.h"
#include "lin_task.h"
#include "si_touch_port.h"
#include "touch_config.h"
#include "lin_frame.h"
#include "store_manager.h"
#include "diagnosticIII.h"
#endif

/** @brief 日志标签，用于模块日志输出 */
STATIC const char *TAG = "APP";

/** @brief 应用任务处理函数声明 */
static void AppTask(uint32_t msg, void *param);
/** @brief 释放未使用的IO口，防止低功耗漏电 */
static void FreeIoSet(void);
/** @brief 关闭未使用的外设以降低功耗 */
static void FreePerSet(void);

/** @brief 门控GPIO初始化声明 */
static void DoorGpioInit(void);
/** @brief 执行Flash例程27服务 */
extern void SysDoFlashRoutine27Service(void);

/** @brief 门状态结构体全局变量 */
extern DoorSt_T door_st;

/** @brief 处理PWM输出声明 */
static void HandleDoorPwm(void);
/** @brief 启动PWM输出声明 */
static void DoorPwmStart(void);
/** @brief 停止PWM输出声明 */
static void DoorPwmStop(void);
/**
 * @brief PWM控制结构体
 * @note 包含按键掩码、状态变化标志、状态机状态和计时起始时间
 */
static struct
{
    uint8_t keymask;    /**< 按键掩码 */
    uint8_t changed;    /**< 状态变化标志 */
    uint8_t fsm;        /**< 状态机当前状态 */
    uint32_t begin_t;   /**< 计时起始时间(ms) */
} pwmCtrl = {0};

/** @brief 软件版本号字符串 */
extern const char g_seres_app_software_version[21];
/** @brief LIN序列号版本字符串 */
extern const char g_lin_sequence_num_version[24];

/**
 * @brief 设置硬件和软件版本信息到door_st结构体
 * @note 从Flash读取硬件版本号，从软件版本字符串和LIN序列号字符串中解析各版本字段，
 *       包括：软件主版本号/次版本号、硬件主版本号/次版本号/阶段版本号、
 *       LIN节点主序列号/次序列号/供应商代码
 * @retval 无
 */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void SetHwSwVersion(void)
{
    uint8_t hard_version[8] = {0};

/* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    ll_flash_read(FLASH_TYPE_NVM, 0x00003908u, (uint8_t *)hard_version, sizeof(hard_version));

    door_st.SW_MajorVersA = ((uint8_t)g_seres_app_software_version[10] - 0x30u); // Software major version number
    door_st.SW_MinorVersA = ((((uint8_t)g_seres_app_software_version[15] - 0x30u) * 10u) + ((uint8_t)g_seres_app_software_version[16] - 0x30u)) & 0x7Fu; // Software subversion number

    door_st.HW_MajorVersB = (hard_version[5] - 0x30u);    // Hardware Main Version Number
    door_st.HW_MinorVersB = (hard_version[7] - 0x30u);    // Hardware sub-version number
    // Hardware stage version number
    if (hard_version[3] == (uint8_t)'A')
    {
        door_st.HW_PhaVers = 0x1;
    }
    else if (hard_version[3] == (uint8_t)'B')
    {
        door_st.HW_PhaVers = 0x2;
    }
    else if (hard_version[3] == (uint8_t)'C')
    {
        door_st.HW_PhaVers = 0x3;
    }
    else
    {
        door_st.HW_PhaVers = 0x0;
    }
    // LIN Node: Master Serial Number A/B/C
    if (g_lin_sequence_num_version[8] == 'A')
    {
        door_st.SN_MajorVersB = 0x1;
    }
    else if (g_lin_sequence_num_version[8] == 'B')
    {
        door_st.SN_MajorVersB = 0x2;
    }
    else if (g_lin_sequence_num_version[8] == 'C')
    {
        door_st.SN_MajorVersB = 0x3;
    }
    else
    {
        door_st.SN_MajorVersB = 0x0;
    }
    //LIN Node: Subsequence Number (1~9)
    door_st.SN_MinorVersB = ((uint8_t)g_lin_sequence_num_version[3] - 0x30u) & 0xFu;
    // LIN Node: Supplier Code
    if ((g_lin_sequence_num_version[17] == '3') && (g_lin_sequence_num_version[18] == '1') &&
        (g_lin_sequence_num_version[19] == '9') && (g_lin_sequence_num_version[20] == '7'))
    {
        door_st.SN_SupplierCod = 0x0;
    }
}

/**
 * @brief 系统主函数，执行硬件初始化并创建应用任务
 * @note 系统初始化流程：
 *       1. Flash重映射配置（APP_MATCH_BOOT模式）或延时等待
 *       2. 门控GPIO初始化
 *       3. 看门狗初始化（WATCH_DOG_EN使能时）
 *       4. 日志接口初始化（DEBUG_PRINT_EN && LOG_INTERFACE_UART）
 *       5. 小OS端口初始化(TcPortInit)
 *       6. 使能全局中断
 *       7. 低功耗任务初始化（LOW_POWER_EN使能时）
 *       8. 触摸功能初始化（TOUCH_FUNC_EN使能时）
 *       9. 存储管理器初始化
 *       10. LIN初始化（LIN_FUNC_EN使能时）
 *       11. 创建应用任务AppTask（优先级TC_TASK_PRIO_MID）
 *       12. 进入任务调度循环(TcTaskExec)
 * @retval 无（死循环，永不返回）
 */
/* PRQA S 1503 2 #3214 - Unused function defined for future extension and module completeness */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void TcMain(void)
{
#if defined APP_MATCH_BOOT
    ll_syscfg_remap_config(FLASH_APP_ADDR, true);
#else
    delay1ms(5000);
#endif

    DoorGpioInit();

#if WATCH_DOG_EN
    WdgInit();
#endif

#if ((DEBUG_PRINT_EN == 1) && (LOG_INTERFACE_TYPE == LOG_INTERFACE_UART))
    TC_LOG_Init(1000000);
    TC_LOG_SetPin(PRINT_GPIO6);
#endif

    TcPortInit();      //little os port initialization

    __enable_irq();

#if LOW_POWER_EN
    HaltInit();                             //Low-power task initialization
#endif

#if TOUCH_FUNC_EN
    TouchInit();
#endif
    store_manager_init();
#if LIN_FUNC_EN
    LinInit();          //Lin Initialization
#endif

/* PRQA S 2880 5 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
/* PRQA S 2742 4 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
/* PRQA S 1036 3 #1036 - Comma before ## in variadic macro (GNU extension) is used to swallow comma when no variable args, supported by compiler. */
/* PRQA S 1035 2 #1035 - Macro with variable arguments called without variable arguments (GNU extension), accepted by toolchain. */
/* PRQA S 3432 1 #3267 - Macro arguments are safely used without unintended operator precedence issues */
    TC_LOGI(TAG, "duotaiji m9 door ctrl project");

    if (TcTaskCreate("app", &AppTask, NULL, TC_TASK_PRIO_MID)==NULL)   //Create APP Task
    {
/* PRQA S 2880 6 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
/* PRQA S 2742 5 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
/* PRQA S 2741 4 #2741 - The controlling expression is constant true by design (e.g., debug log enabled). */
/* PRQA S 1036 3 #1036 - Comma before ## in variadic macro (GNU extension) is used to swallow comma when no variable args, supported by compiler. */
/* PRQA S 1035 2 #1035 - Macro with variable arguments called without variable arguments (GNU extension), accepted by toolchain. */
/* PRQA S 3432 1 #3267 - Macro arguments are safely used without unintended operator precedence issues */
        TC_LOGE(TAG, "AppTask create fail");
    }
    else
    {
/* PRQA S 2880 5 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
/* PRQA S 2742 4 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
/* PRQA S 1036 3 #1036 - Comma before ## in variadic macro (GNU extension) is used to swallow comma when no variable args, supported by compiler. */
/* PRQA S 1035 2 #1035 - Macro with variable arguments called without variable arguments (GNU extension), accepted by toolchain. */
/* PRQA S 3432 1 #3267 - Macro arguments are safely used without unintended operator precedence issues */
        TC_LOGI(TAG, "AppTask create ok");
    }

    while (1)
    {
        TcTaskExec();
    }
}

/**
 * @brief 应用任务处理函数
 * @param msg 消息类型
 *        - MSG_TASK_INIT：任务初始化，创建循环定时器、设置版本信息、初始化GPIO
 *        - MSG_TASK_TIMER：定时触发，执行LIN控制、诊断会话检查和PWM处理
 *        - MSG_TASK_ENTER_HALT：进入低功耗，恢复默认会话模式并关闭LDO5V
 *        - MSG_TASK_WAKE_UP：唤醒，重新使能LDO5V并调整低功耗超时
 * @param param 参数指针（当前未使用）
 * @note 使用TcTimerCreate创建2ms周期循环定时器驱动定时任务
 * @retval 无
 */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
static void AppTask(uint32_t msg, void *param)    //App Task
{
    (void)param;
    static T_TcTimer *appTimer = NULL;            //Timer

    if (msg == (uint32_t)MSG_TASK_INIT)
    {appTimer = TcTimerCreate(TC_TIMER_TYPE_CIRCLE, 2, NULL, currentTask, NULL);
        if (appTimer == NULL)
        {
/* PRQA S 2880 6 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
/* PRQA S 2742 5 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
/* PRQA S 2741 4 #2741 - The controlling expression is constant true by design (e.g., debug log enabled). */
/* PRQA S 1036 3 #1036 - Comma before ## in variadic macro (GNU extension) is used to swallow comma when no variable args, supported by compiler. */
/* PRQA S 1035 2 #1035 - Macro with variable arguments called without variable arguments (GNU extension), accepted by toolchain. */
/* PRQA S 3432 1 #3267 - Macro arguments are safely used without unintended operator precedence issues */
            TC_LOGE(TAG, "appTimer create fail");
        }
        else
        {
            TcTimerStart(appTimer);
        }

        SetHwSwVersion();
        DoorGpioInit();

#if DEBUG_PRINT_EN && LOG_INTERFACE_TYPE==LOG_INTERFACE_UART
        PWM->LED_CTRL_F.LED_LDO5V_EN = 1;
#endif
    }

    if (msg == (uint32_t)MSG_TASK_TIMER)
    {
#if WATCH_DOG_EN
        ll_wdg_reload();
#endif

#if LIN_FUNC_EN
        App_LinControlMsg();
#endif
        LinDiagnosticSessionCheck();
        SysDoFlashRoutine27Service();
        HandleDoorPwm();
    }

    if (msg == (uint32_t)MSG_TASK_ENTER_HALT)
    {
        session_mode = SESSION_MODE_DEFAULT;
/* PRQA S 0662 2 #0662 - Accessing member of unnamed struct/union for hardware register definition. */
/* PRQA S 0306 1 #3271 - Cast between object pointer and integer for hardware address access */
        PWM->LED_CTRL_F.LED_LDO5V_EN = (uint32_t)DISABLE;     //Ensure LDO5 is turned off
    }

    if (msg == (uint32_t)MSG_TASK_WAKE_UP)
    {
/* PRQA S 0662 2 #0662 - Accessing member of unnamed struct/union for hardware register definition. */
/* PRQA S 0306 1 #3271 - Cast between object pointer and integer for hardware address access */
        PWM->LED_CTRL_F.LED_LDO5V_EN = (uint32_t)ENABLE;     //Ensure LDO5 is turned off

#if LOW_POWER_EN
        HaltTimeoutChgPeriod(5000);
#endif
    }
}

/**
 * @brief 触摸按键回调函数
 * @param keyNo 按键编号
 * @param status 按键状态（SI_KEY_PRESS按下 / SI_KEY_RELEASE释放）
 * @note 按键按下时设置pwmCtrl.keymask为1，释放时清0，并标记changed标志触发PWM状态机
 * @retval 无
 */
/* PRQA S 1503 2 #3214 - Unused function defined for future extension and module completeness */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
void TouchKeyCallback(uint8_t keyNo, T_SiKeyStatus status)
{
    (void)keyNo;
    if (status == SI_KEY_PRESS)
    {
        pwmCtrl.keymask = 1;
    }
    else if (status == SI_KEY_RELEASE)
    {
        pwmCtrl.keymask = 0;
    }
    else
    {
        return;
    }

    pwmCtrl.changed = 1;
}

/**
 * @brief 关闭未使用的外设模块以降低功耗
 * @note 关闭未使用的外设时钟，减少系统运行时的功耗
 * @retval 无
 */
/* PRQA S 3219 1 #3254 - Unused static function, reserved for future extension */
static void FreePerSet(void)       //Turn off unused peripherals to reduce power consumption
{
}

/**
 * @brief 释放未使用的IO口配置
 * @note 将未使用的IO口设置为合适状态，防止低功耗模式下的漏电
 * @retval 无
 */
/* PRQA S 3219 1 #3254 - Unused static function, reserved for future extension */
static void FreeIoSet(void)     //No need to set IO, prevents low-power leakage
{
}

/**
 * @brief 门控GPIO初始化配置
 * @note 配置PWM时钟使能、LDO5V电源使能，并将UNLOCK_PIN初始化为推挽输出模式
 * @retval 无
 */
STATIC void DoorGpioInit(void)
{
    //PWM CLK CFG
/* PRQA S 0662 5 #0662 - Accessing member of unnamed struct/union for hardware register definition. */
/* PRQA S 0306 4 #3271 - Cast between object pointer and integer for hardware address access */
/* PRQA S 3469 3 #3258 - Function-like macro used for performance and compiler optimization requirements */
    CRG_CONFIG_UNLOCK();
    CRG->PWM_CLKRST_CTRL_F.PCLK_EN_PWM = (uint32_t)ENABLE;
    CRG_CONFIG_LOCK();

/* PRQA S 0662 2 #0662 - Accessing member of unnamed struct/union for hardware register definition. */
/* PRQA S 0306 1 #3271 - Cast between object pointer and integer for hardware address access */
    PWM->LED_CTRL_F.LED_LDO5V_EN = (uint32_t)ENABLE;        //Turn it on only when you need to send a wave

    ll_gpio_output(UNLOCK_PIN, false);
    gpio_config_t cfg =
    {
        .gpio_pin = UNLOCK_PIN,
        .mode = GPIO_MODE_OUT_PP,
        .pull_mode = GPIO_PULL_NONE,
        .afio = AFIO_MUX_3,
        .trigger_flag = GPIO_TRIGGER_NULL
    };
    ll_gpio_init(&cfg, NULL);
    ll_gpio_output(UNLOCK_PIN, false);
}

/**
 * @brief 处理门控PWM输出状态机
 * @note 状态机包含4个状态：
 *       0-等待按键变化触发PWM输出
 *       1-等待最小信号时间(OPEN_DOOR_MIN_TIMEMS)
 *       2-正常响应阶段，超时或按键释放时停止PWM
 *       default-异常处理
 * @retval 无
 */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
static void HandleDoorPwm(void)   //Handle PWM output
{
    switch (pwmCtrl.fsm)
    {
    case 0:
        if (pwmCtrl.changed!=0u)
        {
            pwmCtrl.changed = 0;
            if (pwmCtrl.keymask!=0u)
            {
                DoorPwmStart();     //On
                door_st.SwtSt = 1;

/* PRQA S 3469 1 #3258 - Function-like macro used for performance and compiler optimization requirements */
                pwmCtrl.begin_t = TcTimeGet();
                pwmCtrl.fsm = 1;
            }
            else
            {
                //Off pwm
                DoorPwmStop();
/* PRQA S 3469 1 #3258 - Function-like macro used for performance and compiler optimization requirements */
                pwmCtrl.begin_t = TcTimeGet();              //Used for LinCanEnterSleep
                door_st.SwtSt = 2;
            }
        }
        break;
    case 1:             //Wait for the minimum signal time
/* PRQA S 3469 1 #3258 - Function-like macro used for performance and compiler optimization requirements */
        if ((TcTimeGet() - pwmCtrl.begin_t) >= (uint16_t)OPEN_DOOR_MIN_TIMEMS)
        {
            pwmCtrl.fsm = 2;
        }
        break;
    case 2:     //Normal response
/* PRQA S 3469 1 #3258 - Function-like macro used for performance and compiler optimization requirements */
        if (((TcTimeGet() - pwmCtrl.begin_t) >= (uint16_t)OPEN_DOOR_MAX_TIMEMS) || (pwmCtrl.changed!=0u))
        {
            //Off pwm
            DoorPwmStop();
/* PRQA S 3469 1 #3258 - Function-like macro used for performance and compiler optimization requirements */
            pwmCtrl.begin_t = TcTimeGet();              //Used for LinCanEnterSleep
            door_st.SwtSt = 2;
            pwmCtrl.fsm = 0;
            return;
        }
        break;
default:
(void)0;
break;
    }
}

/**
 * @brief 启动PWM输出，驱动门把手电机
 * @note 使能LDO5V电源并拉高UNLOCK_PIN以输出PWM驱动信号
 * @retval 无
 */
STATIC void DoorPwmStart(void)
{
/* PRQA S 0662 2 #0662 - Accessing member of unnamed struct/union for hardware register definition. */
/* PRQA S 0306 1 #3271 - Cast between object pointer and integer for hardware address access */
    PWM->LED_CTRL_F.LED_LDO5V_EN = (uint32_t)ENABLE;
    ll_gpio_output(UNLOCK_PIN, true);
}

/**
 * @brief 停止PWM输出，关闭门把手电机驱动
 * @note 将UNLOCK_PIN拉低以停止PWM信号输出
 * @retval 无
 */
STATIC void DoorPwmStop(void)
{
    ll_gpio_output(UNLOCK_PIN, false);
}

/**
 * @brief 检查LIN是否可进入休眠模式
 * @note 根据PWM停止后的空闲时间判断是否允许进入低功耗模式
 * @retval 1 可以进入低功耗模式
 * @retval 0 不可进入低功耗模式
 */
//Returning 1 indicates that after receiving the lin sleep command, it can enter low power mode, and 0 indicates that it cannot.
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
int32_t LinCanEnterSleep(void)
{
/* PRQA S 3469 1 #3258 - Function-like macro used for performance and compiler optimization requirements */
    return ((TcTimeGet() - pwmCtrl.begin_t) >= 5000u)?(int32_t)1:(int32_t)0;
}
