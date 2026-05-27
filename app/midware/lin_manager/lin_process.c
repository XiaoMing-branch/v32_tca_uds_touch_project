/**
 *****************************************************************************
 * @brief   lin process source file.
 *
 * @file    lin_process.c
 * @author  AE/FAE team
 * @date    2024.01.01
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */

#include "lin_hw_cfg.h"
#include "lin_cfg.h"
#include "lin_process.h"
#include "lin_wakeup.h"
#include "lin_snpd.h"
#include "logging.h"
#include "pal_lin_comm.h"

#define MULTI_BYTES_MODE                    /**< 多字节接收模式(使能) */
#undef MULTI_BYTES_MODE                     /**< 取消多字节接收模式(当前未使用) */
#define RX_BYTE_PRINT_DEBUG                 /**< 接收字节打印调试(使能) */
#undef RX_BYTE_PRINT_DEBUG                  /**< 取消接收字节打印调试 */

#define CFG_LIN_DEBUG   0                   /**< LIN调试日志开关, 0=关闭, 1=使能 */

#if 1 == CFG_LIN_DEBUG
    #define LOG_LIN(...)  do{log_debug("[LIN] " __VA_ARGS__);}while(0)           /**< LIN调试日志宏(变参) */
    #define LOG_LIN1(format, args ...)  do{log_debug("[LIN] " format, ## args);}while(0)  /**< LIN调试日志宏(格式化) */
#else
    #define LOG_LIN(format, args ...)       /**< 调试关闭时为空宏, 不产生任何代码 */
#endif

#define LIN_STA_SUCC_TRANSFER           1         /**< LIN状态位掩码: 成功传输, 响应接收/发送完成后置位 */
#define LIN_STA_ERROR_RESP              2         /**< LIN状态位掩码: 响应错误, 接收超时或校验错误时置位 */
#define LIN_STA_BUS_ACTIVITY            4         /**< LIN状态位掩码: 总线活动, 检测到BREAK信号时置位, 进入IDLE时清除 */
#define LIN_STA_FRAME_ERR               8         /**< LIN状态位掩码: 帧错误, STOP位错误时置位 */
#define LIN_STA_CHECKSUM_ERR            16        /**< LIN状态位掩码: 校验和错误, 接收响应数据校验失败时置位 */
#define LIN_STA_READBACK_ERR            32        /**< LIN状态位掩码: 回读错误, TX/RX数据冲突(位错误)时置位 */
#define LIN_STA_PARITY_ERR              64        /**< LIN状态位掩码: 奇偶校验错误, PID奇偶校验失败时置位 */
#define LIN_STA_RESET                   128       /**< LIN状态位掩码: 复位状态 */

#define  SLEEP_AFTER_INIT 0                 /**< 初始化后是否立即进入休眠模式: 0=否, 1=是 */

volatile uint8_t lin_error;                 /**< LIN全局错误码, 记录最近一次错误类型 */
lin_precfg_t lin_cfg;                       /**< LIN协议栈预配置参数结构体 */

static  l_u8 ifc = 0xFF;                   /**< LIN接口句柄, 标识当前使用的LIN硬件接口 */
static  l_u8 state = UNINIT;               /**< LIN状态机当前状态 */
static  lin_status l_status;                /**< LIN状态位结构体, 记录传输结果各标志位 */
static  l_u8 cnt_byte = 0;                 /**< 当前已接收或已发送的响应字节计数 */
static  l_u8 *ptr = 0;                     /**< 响应缓冲区当前读写位置指针 */
static  l_u8 current_id = 0x00;            /**< 当前帧的真实ID(经PID奇偶校验提取后) */
static  l_u8 *response_buffer = 0;         /**< 指向LIN响应数据缓冲区的指针, [0]=长度 */
static  l_u8 pid = 0x80;                   /**< 当前帧的受保护标识符(PID, 含奇偶校验位) */
static  l_u16 frame_timeout_cnt = 0;       /**< 帧超时递减计数器(LIN帧级超时, 含同步段) */
static  l_u16 res_frame_timeout_cnt = 0;   /**< 响应帧超时递减计数器(等待响应数据的最大时间) */
static  l_u16 lin_rcvbreak_cnt = 0;        /**< BREAK信号接收次数统计计数器 */
l_u16 idle_timeout_cnt = 0;                /**< 空闲超时递减计数器(IDLE状态下无通信超时) */

extern const l_u16 lin_max_frame_res_timeout_val[8];    /**< 各数据长度对应的最大响应超时计数值查找表 */
extern l_u8 lin_lld_response_buffer[10];                /**< LIN底层响应数据共享缓冲区(10字节) */

extern void lin_sci_baudrate_update(void);       /**< LIN SCI波特率更新函数(弱定义, 可重写) */
extern void lin_goto_idle_state(void);           /**< 切换LIN状态机到IDLE空闲状态 */
extern void lin_process_break_handle(void);      /**< LIN BREAK信号处理钩子函数(弱定义, 可重写) */

void lin_lld_isr_callback(uint32_t isr);        /**< LIN主中断回调函数声明 */

/**
 * @brief  LIN SCI波特率更新回调函数(弱定义)
 *        用户可在应用层重写此函数以自定义波特率更新逻辑
 * @param  无
 * @retval 无
 */
__attribute__((weak)) void lin_sci_baudrate_update(void)
{
    //do noting
}

/**
 * @brief  LIN BREAK信号处理钩子函数(弱定义)
 *        用户可在应用层重写此函数以自定义BREAK信号处理行为
 * @param  无
 * @retval 无
 */
__attribute__((weak)) void lin_process_break_handle(void)
{
    //do noting
}

/**
 * @brief  LIN协议栈参数初始化
 *        将配置宏定义的参数填充至lin_cfg全局结构体
 * @param  无
 * @note   初始化内容包括: 波特率、定时基准周期、空闲超时阈值、
 *        帧数量、诊断服务数量、队列深度和N_As/N_Cr超时计数值;
 *        max_idle_timeout根据SLEEP_AFTER_INIT宏决定是否缩放;
 *        n_max_timeout_cnt = 1000 * (1000 / TIME_BASE_PERIOD)
 * @retval 无
 */
void lin_param_init(void)
{
    lin_cfg.lin_baud_rate = LIN_BAUD_RATE;
    lin_cfg.time_base_period = TIME_BASE_PERIOD;
#if SLEEP_AFTER_INIT
    lin_cfg.max_idle_timeout = _MAX_IDLE_TIMEOUT_ / 25;
#else
    lin_cfg.max_idle_timeout = _MAX_IDLE_TIMEOUT_;
#endif
    lin_cfg.lin_num_of_frms = LIN_NUM_OF_FRMS;
    lin_cfg.lin_cfg_frame_num = LIN_CFG_FRAME_NUM;
    lin_cfg.diag_number_of_services = _DIAG_NUMBER_OF_SERVICES_;
    lin_cfg.max_queue_size = MAX_QUEUE_SIZE;
    lin_cfg.n_max_timeout_cnt = ((unsigned short)(1000 * (1000 / TIME_BASE_PERIOD)));
}

/**
 * @brief  初始化LIN硬件SCI接口
 *        调用pal_lin_init()配置LIN总线为从机模式并注册中断回调
 * @param  无
 * @note   使用配置的波特率(LIN_BAUD_RATE)和LIN_BUS_0总线;
 *        中断回调函数为lin_lld_isr_callback(), 处理所有LIN硬件事件;
 *        LIN_MODE_SLV表示从机模式
 * @retval 无
 */
void lin_init_sci(void)
{
    pal_lin_init(LIN_BUS_0, LIN_MODE_SLV, LIN_BAUD_RATE, lin_lld_isr_callback);
}

/**
 * @brief  禁用LIN SCI中断(硬件反初始化)
 *        调用pal_lin_deinit()关闭LIN外设并停止所有中断
 * @param  无
 * @note   实际调用pal_lin_deinit(LIN_BUS_0)完成硬件反初始化,
 *        LIN外设时钟关闭、中断禁用、引脚释放
 * @retval 无
 */
void lin_lld_sci_int_disable(void)
{
    pal_lin_deinit(LIN_BUS_0);
}

/**
 * @brief  LIN SCI完全反初始化
 *        将状态机置为UNINIT并调用pal_lin_deinit()关闭硬件
 * @param  无
 * @note   先重置state = UNINIT, 再调用lin_lld_sci_int_disable()
 *        关闭外设; 反初始化后LIN总线不再响应任何通信
 * @retval 无
 */
void lin_lld_sci_deinit(void)
{
    state = UNINIT;
    lin_lld_sci_int_disable();
}

/**
 * @brief  LIN发送唤醒请求处理
 *        当LIN状态为IDLE或SLEEP时, 调用lin_goto_idle_state()准备唤醒
 * @param  无
 * @note   唤醒操作仅将状态切回IDLE并重置空闲超时计数器;
 *        实际唤醒信号(强制总线空闲>250us)由上层lin_wakeup模块发送
 * @retval 无
 */
void lin_lld_sci_tx_wake_up(void)
{
    if ((state == IDLE) || (state == LIN_SLEEP_MODE))
    {
        /* Set Lin state to idle */
        lin_goto_idle_state();
    }
}

/**
 * @brief  启用LIN SCI中断(占位函数)
 *        当前实现为空操作, 中断在lin_init_sci()中通过pal_lin_init()一并使能
 * @param  无
 * @note   保留此函数接口以供需要时单独控制中断使能;
 *        当前中断使能与硬件初始化绑定, 无需单独调用
 * @retval 无
 */
void lin_lld_sci_int_enable(void)
{
}

/**
 * @brief  忽略当前LIN响应帧
 *        直接调用lin_goto_idle_state()将状态机切回IDLE,
 *        放弃当前正在接收或发送的响应帧
 * @param  无
 * @note   用于协议层在特定条件下需跳过当前响应帧的场景,
 *        如PID不匹配、从机无需应答等情况
 * @retval 无
 */
void lin_lld_sci_ignore_response(void)
{
    lin_goto_idle_state();
}

/**
 * @brief  设置LIN进入低功耗休眠模式
 *        将状态机切换到LIN_SLEEP_MODE并置位睡眠标志
 * @param  无
 * @note   低功耗模式下LIN接收器保持活动, 可检测唤醒信号(BREAK/WAKEUP);
 *        实际硬件休眠配置(如ECU/AFE深度睡眠)需在HW层实现,
 *        当前仅设置软件状态标志; 检测到BREAK后自动退出休眠
 * @retval 无
 */
void lin_lld_sci_set_low_power_mode(void)
{
    /* Configure Hw code */
    //lin_sleep_mode_set(DEEPSLEEP_MODE);//ecu & afe sleep
    /* Set Lin status = receiving data*/
    state = LIN_SLEEP_MODE;
    lin_goto_sleep_flg = 1;
}

/**
 * @brief  配置LIN响应接收缓冲区并启动硬件接收
 *        设置期望接收长度和缓冲区指针, 调用pal_lin_rx_response()启动接收
 * @param  msg_length: 期望接收的响应数据字节数
 * @note   response_buffer[0]存放长度, cnt_byte清零,
 *        ptr指向缓冲区起始; 硬件接收完成后逐字节触发RX中断;
 *        状态切换至RECV_DATA等待数据到达
 * @retval 无
 */
void lin_lld_sci_rx_response(l_u8 msg_length)
{
    /* Put response length and pointer of response buffer into descriptor */
    *(response_buffer) = msg_length;
    cnt_byte = 0;
    ptr = response_buffer;
    pal_lin_rx_response(LIN_BUS_0, pid, &response_buffer[1], msg_length);
    state = RECV_DATA;
}

/**
 * @brief  发送LIN响应数据到总线
 *        调用pal_lin_tx_response()将PID和响应缓冲区数据发送至LIN总线
 * @param  无
 * @note   response_buffer[0]存放长度, [1..n]存放实际数据;
 *        cnt_byte初始化为实际发送字节数(不超过4字节);
 *        发送成功则状态切换到SEND_DATA, 失败则回到IDLE
 * @retval 无
 */
void lin_lld_sci_tx_response(void)
{
    if (pal_lin_tx_response(LIN_BUS_0, pid,  &response_buffer[1], response_buffer[0]))
    {
        cnt_byte = (response_buffer[0] > 4) ? 4 : response_buffer[0]; /* 硬件FIFO单次最大发送4字节 */
        state = SEND_DATA;
    }
    else
    {
        lin_goto_idle_state(); /* 发送启动失败则回到IDLE, 等待下一帧 */
    }
}
#if CFG_SUPPORT_LIN_MASTER
#if defined (__TCPL03X__) || defined(__TCAE10__)
extern void lin_tl_uncd_master_send(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length);     /**< LIN主机发送函数(传输层) */
extern void lin_tl_uncd_master_receive(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length);  /**< LIN主机接收函数(传输层) */
extern void lin1_lld_isr_callback(uint32_t isr);                                                       /**< LIN1总线中断回调函数 */
/**
 * @brief  LIN主机模式发送数据帧
 *        根据帧ID计算PID后通过LIN1总线发送数据
 * @param  frame_id: 帧ID(0-63), 将自动计算奇偶校验位生成PID
 * @param  buffer: 待发送数据缓冲区指针
 * @param  length: 待发送数据长度(字节)
 * @retval 无
 */
void lin_lld_sci_tx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length)
{
    uint8_t pid = lin_process_parity(frame_id, MAKE_PARITY);
    lin_tl_uncd_master_send(LIN_BUS_1, pid, buffer, length);
}

/**
 * @brief  LIN主机模式请求接收数据帧
 *        根据帧ID计算PID后通过LIN1总线发起帧头, 等待从机响应
 * @param  frame_id: 帧ID(0-63), 将自动计算奇偶校验位生成PID
 * @param  buffer: 接收数据缓冲区指针
 * @param  length: 期望接收的数据长度(字节)
 * @retval 无
 */
void lin_lld_sci_rx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length)
{
    uint8_t pid = lin_process_parity(frame_id, MAKE_PARITY);
    lin_tl_uncd_master_receive(LIN_BUS_1, pid, buffer, length);
}

/**
 * @brief  读取LIN主机模式下接收到的帧数据
 *        获取指定PID对应的接收帧数据及PID值
 * @param  pid: 输出参数, 返回接收帧的受保护标识符
 * @param  buffer: 输出参数, 返回接收到的帧数据缓冲区
 * @retval true - 成功获取到数据; false - 无可用数据
 */
bool lin_lld_sci_rx_data(uint8_t *pid, uint8_t *buffer)
{
    return (lin_tl_uncd_frame_get(LIN_BUS_1, pid, buffer));
}


/**
 * @brief  LIN主机硬件接口初始化
 *        配置GPIO引脚和LIN1外设为主机模式
 * @param  无
 * @note   初始化GPIO_PIN_5和GPIO_PIN_2为推挽输出并置高电平;
 *        然后调用pal_lin_init()初始化LIN1总线为主机模式,
 *        注册lin1_lld_isr_callback为中断回调;
 *        仅TCPL03X/TCAE10平台支持此功能
 * @retval 无
 */
void lin_sci_master_init(void)
{
    gpio_config_t gpio_cfg =
    {
        .gpio_pin = GPIO_PIN_5,
        .mode = GPIO_MODE_OUT_PP,
        .pull_mode = GPIO_PULL_UP,
        .pull_down_type = GPIO_PULLDOWN_SW_ONLY,

        .afio = AFIO_MUX_0,
        .trigger_flag = GPIO_TRIGGER_NULL,
    };
    pal_gpio_init(&gpio_cfg, NULL);
    pal_gpio_output(GPIO_PIN_5, true);
    gpio_cfg.gpio_pin = GPIO_PIN_2;
    pal_gpio_init(&gpio_cfg, NULL);
    pal_gpio_output(GPIO_PIN_2, true);

    pal_lin_init(LIN_BUS_1, LIN_MODE_MASTER, LIN_BAUD_RATE, lin1_lld_isr_callback);
}
#endif
#endif

/**
 * @brief  获取LIN当前状态字节
 * @param  无
 * @note   返回l_status.byte, 包含LIN_STA_xxx位掩码的组合,
 *        上层通过判断各标志位获取传输结果
 * @retval LIN状态字节: LIN_STA_SUCC_TRANSFER等标志的位组合
 */
l_u8 lin_lld_sci_get_status(void)
{
    return l_status.byte;
}

/**
 * @brief  获取LIN状态机当前状态
 * @param  无
 * @note   返回state变量, 可能值包括:
 *        UNINIT / IDLE / RECV_SYN / RECV_PID / RECV_DATA /
 *        SEND_DATA / SEND_DATA_COMPLETED / PROC_CALLBACK / LIN_SLEEP_MODE
 * @retval 当前LIN状态机状态枚举值
 */
l_u8 lin_lld_sci_get_state(void)
{
    return state;
}

/**
 * @brief  LIN协议超时处理
 *        处理传输层N_CR/N_AS超时以及状态机帧超时/响应超时/空闲超时
 * @param  cnt_tick: 本次滴答计数值(通常为1, 表示逝去的时间基准周期数)
 * @note   多帧模式(_TL_MULTI_FRAME_)下处理:
 *        - N_CR超时: 从机响应超时, 置状态为LD_N_CR_TIMEOUT
 *        - N_AS超时: 主机请求超时, 置状态为LD_N_AS_TIMEOUT
 *        单帧模式仅处理N_AS超时;
 *        状态机超时:
 *        - IDLE: 空闲超时, 进入LIN_SLEEP_MODE
 *        - SEND_PID/SEND_DATA等: 帧超时, 回到IDLE
 *        - RECV_DATA: 响应超时, 置位错误标志并回调通知上层
 * @retval 无
 */
void lin_lld_sci_timeout(l_u16 cnt_tick)
{
    /* Multi frame support */
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
    /* N_CR超时处理: 从机响应超时(多帧传输层) */
    if (LD_CHECK_N_CR_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_receive_msg_status = LD_N_CR_TIMEOUT;
            tl_rx_msg_status = LD_N_CR_TIMEOUT;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
            tl_diag_state = LD_DIAG_IDLE;
        }
        else
        {
            tl_check_timeout = ((tl_check_timeout > cnt_tick) ? (tl_check_timeout - cnt_tick) : 0);
        }
    }

    /* N_AS超时处理: 主机请求超时(多帧传输层) */
    if (LD_CHECK_N_AS_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_tx_msg_status = LD_N_AS_TIMEOUT;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
            tl_diag_state = LD_DIAG_IDLE;
        }
        else
        {
            tl_check_timeout = ((tl_check_timeout > cnt_tick) ? (tl_check_timeout - cnt_tick) : 0);
        }
    }

#else

    /* Single Frame — 单帧模式仅需处理N_AS超时 */
    if (LD_CHECK_N_AS_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
        }
        else
        {
            tl_check_timeout = ((tl_check_timeout > cnt_tick) ? (tl_check_timeout - cnt_tick) : 0);
        }
    }

#endif /* END (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)    */

    /* 状态机超时处理: 根据当前状态分别处理帧超时/响应超时/空闲超时 */
    switch (state)
    {
    case IDLE:
        /* 空闲超时: 超过max_idle_timeout未检测到总线活动则进入休眠 */
        if (idle_timeout_cnt == 0)
        {
            /* Trigger callback */
            CALLBACK_HANDLER(ifc, LIN_LLD_BUS_ACTIVITY_TIMEOUT, 0xFF);
            /* goback to IDLE, reset max idle timeout */
            idle_timeout_cnt = lin_cfg.max_idle_timeout;
            /* Set state to sleep mode */
            state = LIN_SLEEP_MODE;
        }
        else
        {
            idle_timeout_cnt = ((idle_timeout_cnt > cnt_tick) ? (idle_timeout_cnt - cnt_tick) : 0);
        }

        break;

    case SEND_PID:        /* Master */
    case RECV_SYN:
    case RECV_PID:
    case SEND_DATA:
    case SEND_DATA_COMPLETED:
        /* 帧头/数据发送超时: frame_timeout_cnt归零则回到IDLE */
        /* timeout send has occurred - change state of the node and inform core */
        if (0 == frame_timeout_cnt)
        {
            lin_goto_idle_state();
        }
        else
        {
            frame_timeout_cnt = ((frame_timeout_cnt > cnt_tick) ? (frame_timeout_cnt - cnt_tick) : 0);
        }

        break;

    case RECV_DATA:
        /* 响应接收超时: 等待从机响应数据超过res_frame_timeout_cnt */
        /* timeout receive has occurred - change state of the node and inform core */
        if (res_frame_timeout_cnt == 0)
        {
            if (cnt_byte > 0)
            {
                lin_error = errRXTIMEOUT;
                /* set lin status: error_in_response */
                l_status.byte |= LIN_STA_ERROR_RESP;
                /* Trigger callback */
                CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_NODATA_TIMEOUT, current_id);

            }

            {
                pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_RX); /* 终止当前接收操作 */
            }

            lin_goto_idle_state();
        }
        else
        {
            res_frame_timeout_cnt = ((res_frame_timeout_cnt > cnt_tick) ? (res_frame_timeout_cnt - cnt_tick) : 0);
        }

        break;

    case PROC_CALLBACK:
        break;

    default:
        break;
    }
}

/**
 * @brief  切换LIN状态机到IDLE空闲状态
 *        清除总线活动标志, 重置空闲超时计数器, 设置state = IDLE
 * @param  无
 * @note   所有状态在完成/错误/超时后均回到此状态, 是状态机的统一出口;
 *        清除LIN_STA_BUS_ACTIVITY标志, 设置最大空闲超时;
 *        IDLE状态下持续检测BREAK/WAKEUP信号以启动新帧
 * @retval 无
 */
void lin_goto_idle_state(void)
{
    /* set lin status: ~bus_activity */
    l_status.byte &= ~LIN_STA_BUS_ACTIVITY;
    /* Set max idle timeout */
    idle_timeout_cnt = lin_cfg.max_idle_timeout;
    state = IDLE;
}

/**
 * @brief  LIN SCI错误中断服务函数(占位)
 *        当前实现为空操作, 错误事件在lin_lld_isr_callback()中统一处理
 * @param  无
 * @retval 无
 */
void lin_lld_sci_err_isr(void)
{
}

/**
 * @brief  LIN SCI接收中断服务函数(占位)
 *        当前实现为空操作, 接收事件在lin_lld_isr_callback()中统一处理
 * @param  无
 * @retval 无
 */
void lin_lld_sci_rx_isr(void)
{
}

/**
 * @brief  LIN主中断回调函数
 *        处理LIN硬件触发的所有中断事件, 推进LIN状态机运行
 * @param  isr: 中断状态标志位掩码, 由硬件中断寄存器直接映射
 * @note   按固定优先级依次判断各中断位; 核心状态机流转:
 *        BREAK→RECV_SYN→SYNC→RECV_PID→PID_OK→RECV_DATA/SEND_DATA→PROC_CALLBACK→IDLE;
 *        错误/超时路径: 任意状态→(错误处理)→IDLE;
 *        处理的中断类型: WAKEUP / BREAK / STOP位错误 / SYNC /
 *        PID完成 / RX字节 / TX完成 / TX-RX冲突 / FIFO空 /
 *        SYNC值错误 / PID奇偶校验错 / FIFO溢出 / 接收超时
 * @retval 无
 */

void lin_lld_isr_callback(uint32_t isr)
{
    uint8_t tmp_byte;
    uint8_t len;
#ifdef MULTI_BYTES_MODE
    uint8_t chk;
#endif

#if defined (__TCPL01X__)

    /****************************************************************************
    *** 0. WAKEUP 检测 — 总线唤醒事件
    *** 当LIN总线从休眠状态检测到唤醒信号时触发,
    *** 将状态机从LIN_SLEEP_MODE切换至IDLE以准备接收下一帧
    ****************************************************************************/
    if (0 != (isr & LIN_INT_WAKEUP_DET_FLAG))
    {
#if SLEEP_AFTER_INIT
        max_idle_timeout = _MAX_IDLE_TIMEOUT_;
#endif

        if (LIN_SLEEP_MODE == state)
        {
            lin_goto_idle_state();
        }
    }

#endif

    /****************************************************************************
    *** 1. BREAK 检测 — LIN帧起始同步信号
    *** BREAK是LIN帧的起始标志(至少13位显性电平);
    *** 触发后复位帧超时计数器, 激活总线活动标志, 状态推进至RECV_SYN;
    *** 若当前处于休眠模式则先唤醒回到IDLE再重新等待下一帧
    ****************************************************************************/
    if (0 != (isr & LIN_INT_BREAK_DET_FLAG))
    {
        ++lin_rcvbreak_cnt;
        //lin_conflict_err =0 ;
        lin_process_break_handle();
#if SLEEP_AFTER_INIT
        max_idle_timeout = _MAX_IDLE_TIMEOUT_;
#endif
        frame_timeout_cnt = lin_max_frame_res_timeout_val[6]; /* 设置帧超时: 索引6对应最大响应字节数(含校验和)的超时值 */

        if (LIN_SLEEP_MODE == state)
        {
            lin_goto_idle_state(); /* 休眠状态下检测到BREAK则唤醒, 回到IDLE等待完整帧 */
            return;
        }

        l_status.byte = LIN_STA_BUS_ACTIVITY;
        state = RECV_SYN;
    }

    /****************************************************************************
    *** 2. STOP位错误检测
    *** 当LIN总线上检测到STOP位错误(应为隐性电平但读到显性)时触发;
    *** 仅在RECV_DATA/SEND_DATA/SEND_DATA_COMPLETED状态下处理,
    *** 中止当前收发操作, 通知上层帧错误, 回到IDLE
    ****************************************************************************/
    if (0 != (isr & LIN_INT_STOP_BIT_ERROR_FLAG))
    {
        LOG_LIN("LIN_INT_STOP_BIT_ERROR_FLAG\r\n"); 

        //if (((state == RECV_DATA) || (state == SEND_DATA)) && (tl_slaveresp_cnt != 0))
        if ((state == RECV_DATA) || (state == SEND_DATA) || (state == SEND_DATA_COMPLETED))
        {
            if (state == RECV_DATA)
            {
                //rx_abort
                pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_RX);
            }
            else if (state == SEND_DATA)
            {
                //tx_abort
                pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_TX);
            }
            lin_error = errSTOP;
            CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_FRAME_ERR, current_id);
            lin_goto_idle_state();
        }
    }

    /****************************************************************************
    *** 3. SYNC域检测 — 自动波特率校准
    *** SYNC字段为0x55(交替的0和1), 用于从机同步波特率;
    *** 调用pal_lin_autobaudrate_check()校准, 状态推进至RECV_PID
    ****************************************************************************/
    if (0 != (isr & LIN_INT_SYNC_DET_FLAG))
    {
        pal_lin_autobaudrate_check(LIN_BUS_0);
        state = RECV_PID;
    }

    /****************************************************************************
    *** 4. PID接收完成 — 地址识别与过滤
    *** 读取接收到的PID(受保护标识符), 校验奇偶位并提取真实帧ID;
    *** 校验通过则通过回调通知上层PID匹配结果, 设置响应超时;
    *** 校验失败则置位奇偶错误标志, 回调通知上层, 回IDLE
    ****************************************************************************/
    if (0 != (isr & LIN_INT_RX_PID_DONE_FLAG))
    {

        if (state == RECV_PID)
        {

            pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_PID, &tmp_byte);
            /* checkparity and extrait PID */
            current_id = lin_process_parity(tmp_byte, CHECK_PARITY);
            /* Keep the PID */
            pid = tmp_byte;

            if (current_id != 0xFF)
            {
                CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_PID_OK, current_id); /* PID校验通过, 通知上层当前帧ID */

                res_frame_timeout_cnt = lin_max_frame_res_timeout_val[*(response_buffer) - 1]; /* 根据响应数据长度设置响应超时 */
            }
            else
            {
                lin_error = errPID;
                l_status.byte |= LIN_STA_PARITY_ERR;
                /* trigger callback */
                CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_PID_ERR, 0xFF); /* PID奇偶校验失败, 通知上层错误 */
                lin_goto_idle_state();
            }
        }

#ifdef CFG_LIN_CONFORM_TEST
//        extern uint8_t lin_slave_wakeup_flag;
//        lin_slave_wakeup_flag = 0;
#endif
    }

    /****************************************************************************
    *** 5. RX单字节接收完成 — 数据接收与校验和验证
    *** 每接收一个字节触发一次; RECV_DATA状态下逐字节存入缓冲区;
    *** 接收完全部字节后进行校验和(checksum)验证:
    ***   - 成功: 置位SUCC_TRANSFER标志, 回调LIN_LLD_RX_COMPLETED
    ***   - 失败: 置位CHECKSUM_ERR标志, 回调LIN_LLD_CHECKSUM_ERR
    *** 休眠模式下检测到特定字节(0xF0/0xE0/0xC0/0x80/0x00)时唤醒总线
    ****************************************************************************/
    if (0 != (isr & LIN_INT_RX_1BYTE_FLAG))
    {
        pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_FIFO, &tmp_byte);

        switch (state)
        {
        case RECV_DATA:
            ptr++;
            *(ptr) = tmp_byte;

            /* Check bytes received fully */
#ifdef RX_BYTE_PRINT_DEBUG
            LOG_LIN(" %02x", tmp_byte);
#endif

            if (cnt_byte == (response_buffer[0]))
            {
#ifdef RX_BYTE_PRINT_DEBUG
                LOG_LIN("\r\n");
#endif
                /* rx_abort */
                pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_RX);

                /* checksum checking */
                if (lin_checksum(response_buffer, pid) == tmp_byte) /* 计算校验和与最后一字节(tmp_byte)比对 */
                {
                    /*******************************************/
                    /***  RX Buffer Full - Checksum OK       ***/
                    /*******************************************/
                    /* set lin status: successful_transfer */
                    l_status.byte |= LIN_STA_SUCC_TRANSFER;
                    /* disable RX interrupt */
                    state = PROC_CALLBACK;
                    /* trigger callback */
                    CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_RX_COMPLETED, current_id); /* 通知上层接收完成 */

                    /* enable RX interrupt */
                    if (LIN_SLEEP_MODE != state)
                    {
                        lin_goto_idle_state(); /* 非休眠状态回到IDLE, 等待下一帧 */
                    }
                }
                else
                {
                    lin_error = errCHECKSUM;
                    /* set lin status: error_in_response, checksum_error */
                    l_status.byte |= (LIN_STA_ERROR_RESP | LIN_STA_CHECKSUM_ERR);
                    LOG_LIN("err no = %x\r\n", l_status.byte);
                    /* trigger callback */
                    CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_CHECKSUM_ERR, current_id); /* 通知上层校验和错误 */
                    lin_goto_idle_state();
                }

            }

            cnt_byte++;
            break;

        case LIN_SLEEP_MODE:
            if ((tmp_byte == 0xF0) || (tmp_byte == 0xE0) || (tmp_byte == 0xC0) || (tmp_byte == 0x80) || (tmp_byte == 0x00))
            {
                /* Set idle timeout again */
                lin_goto_idle_state();
            }

            break;

        default:
            break;
        }
    }

    /****************************************************************************
    *** 6. TX发送完成 — 响应数据发送结束
    *** 从机响应数据已全部发送至总线;
    *** 置位SUCC_TRANSFER标志, 通过回调通知上层LIN_LLD_TX_COMPLETED,
    *** 更新波特率(若需要), 回到IDLE
    ****************************************************************************/
    if (0 != (isr & LIN_INT_TX_DONE_FLAG))
    {
        if (state == SEND_DATA)
        {
            l_status.byte |= LIN_STA_SUCC_TRANSFER;
            state = PROC_CALLBACK;
            CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_TX_COMPLETED, current_id);
            lin_sci_baudrate_update();
            lin_goto_idle_state();
        }
    }

    /****************************************************************************
    *** 7. TX/RX冲突(位错误) — 总线竞争检测
    *** 发送时读取到的总线电平与发送值不一致, 表明存在总线冲突;
    *** 中止当前发送操作, 回调通知上层LIN_LLD_READBACK_ERR, 回到IDLE
    ****************************************************************************/
    if (0 != (isr & LIN_INT_TX_RX_CONFLICT_FLAG))
    {
        if (state == SEND_DATA)
        {
            lin_error = errREADBACK;
            LOG_LIN("LIN_INT_TX_RX_CONFLICT_FLAG\r\n");

            /* tx_abort */
            pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_TX);
            CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_READBACK_ERR, current_id);
            lin_goto_idle_state();
        }
    }

#if defined (__TCPL03X__) || defined(__TCAE10__)

    /****************************************************************************
    *** 8. TX FIFO空中断 — 批量发送(支持4字节模式)
    *** 当发送FIFO为空时触发, 用于连续发送剩余数据;
    *** 每次最多发送4字节, cnt_byte累计已发字节数;
    *** 仅TCPL03X/TCAE10平台支持
    ****************************************************************************/
    if (0 != (isr & LIN_INT_TX_FIFO_EMPTY_FLAG))
    {
        if (state == SEND_DATA)
        {
            if (cnt_byte <= response_buffer[0])
            {
                if (lin_error != errREADBACK || lin_error != errSTOP)
                {
                    len = ((response_buffer[0] - cnt_byte) <= 4) ? (response_buffer[0] - cnt_byte) : 4;
                    pal_lin_tx_4byte(LIN_BUS_0, &response_buffer[cnt_byte + 1], len);
                    cnt_byte += len;
                }
            }
        }
    }

#endif

    /****************************************************************************
    *** 9. SYNC值错误 — 波特率偏差超出容限
    *** 检测到的SYNC字段(0x55)实际脉冲宽度超出允许误差范围,
    *** 置位错误标志并回到IDLE
    ****************************************************************************/
    if (0 != (isr & LIN_INT_SYNC_VALUE_ERROR_FLAG))
    {
        lin_error = errSYNC;
        LOG_LIN("sync value err\r\n");
        lin_goto_idle_state();
    }

    /****************************************************************************
    *** 10. PID奇偶校验错误 — 地址传输错误
    *** 硬件检测到PID的奇偶校验位与计算值不符,
    *** 置位PID错误标志供上层查询
    ****************************************************************************/
    if (0 != (isr & LIN_INT_RX_CHKPTY_ERROR_FLAG))
    {
        lin_error = errPID;
        LOG_LIN("pid check err\r\n");
    }

#ifndef MULTI_BYTES_MODE

    /****************************************************************************
    *** 11. RX FIFO满 — 接收缓冲区溢出警告
    *** 仅单字节模式(非MULTI_BYTES_MODE)下使能此中断;
    *** 指示接收FIFO已满, 可能丢失数据
    ****************************************************************************/
    if (0 != (isr & LIN_INT_RX_FIFO_FULL_FLAG))
    {
        lin_error = errRXFULL;
        LOG_LIN("AFE_INT_RX_FIFO_FULL\r\n");
    }

#endif

    /****************************************************************************
    *** 12. RX FIFO溢出 — 硬件接收FIFO数据丢失
    *** FIFO已满且新数据到达, 旧数据被覆盖;
    *** 置位溢出错误标志
    ****************************************************************************/
    if (0 != (isr & LIN_INT_RX_FIFO_OVF_FLAG))
    {
        lin_error = errRXOVF;
        LOG_LIN("AFE_INT_RX_FIFO_OVF_ERR\r\n");
    }

#if defined (__TCPL01X__)

    /****************************************************************************
    *** 13. RX接收超时 — 字节间隙超时
    *** 接收过程中相邻字节间隔超过LIN规范允许时间;
    *** 仅TCPL01X平台支持此中断
    ****************************************************************************/
    if (0 != (isr & LIN_INT_RX_TIMEOUT_FLAG))
    {
        lin_error = errRXTIMEOUT;
        LOG_LIN("AFE_INT_RX_TIMEOUT\r\n");
    }

#endif

#if CFG_SUPPORT_LIN_SNPD

    /* SNPD(从机节点位置检测)自动地址分配完成中断 */
    if (0 != (isr & LIN_INT_AUTOADDR_DONE_FLAG))
    {
        //LOG_LIN("AFE_INT_AUTO_ADDR_DONE\r\n");
    }

    /* SNPD从机选择事件: 被主机选中, 置位选择状态标志 */
    if (0 != (isr & LIN_INT_SLV_SELECT_FLAG))
    {
        lin_snpd_status_set(LIN_AA_STATUS_SELECT, 1);
    }

#endif
}

/**
 * @brief  LIN硬件初始化入口
 *        保存接口句柄、初始化响应缓冲区、调用参数初始化和SCI初始化,
 *        最后进入IDLE状态
 * @param  iii: LIN接口句柄, 标识当前LIN硬件接口
 * @note   初始化顺序:
 *        1. 保存接口句柄(ifc)
 *        2. 关联响应缓冲区(response_buffer)
 *        3. lin_param_init() — 加载配置参数
 *        4. lin_init_sci() — 初始化硬件SCI
 *        5. lin_goto_idle_state() — 进入空闲状态
 * @retval 无
 */
void lin_lld_sci_init(l_ifc_handle iii)
{
    (void)ifc; /* 抑制ifc全局变量未使用的编译警告 */

    ifc = (l_u8)iii;
    response_buffer = lin_lld_response_buffer;
    lin_param_init();
    lin_init_sci();

    /* Enter IDLE state */
    lin_goto_idle_state();
}

/**
 * @brief  LIN协议栈定时器初始化函数(占位)
 *        当前实现为空操作, 定时器由系统层(l_sys_init)统一初始化
 * @param  无
 * @retval 无
 */
void lin_lld_timer_TCPL_init(void)
{
}

#if defined (__TCPL03X__) || defined(__TCAE10__)
/**
 * @brief  LIN从机主动唤醒函数
 *        通过LL层发送8位BREAK信号唤醒LIN总线主机
 * @param  无
 * @note   仅TCPL03X/TCAE10平台支持此功能;
 *        调用ll_lin_ctrl_brk_tx(LL_SCI_BUS_1, 8)在LIN1总线上发送BREAK;
 *        用于从机需要主动请求通信的场景
 * @retval 无
 */
void lin_lld_slave_wakeup(void)
{
    ll_lin_ctrl_brk_tx(LL_SCI_BUS_1, 8);
}
#endif
/**
 * @brief  LIN协议栈整体初始化
 *        依次调用系统层、接口层和传输层初始化函数完成协议栈启动
 * @param  无
 * @note   初始化链: l_sys_init() → l_ifc_init() → ld_init();
 *        l_sys_init(): 系统定时器和资源初始化;
 *        l_ifc_init(): LIN接口层初始化, 会调用lin_lld_sci_init();
 *        ld_init(): 诊断传输层初始化
 * @retval 无
 */
void lin_process_init(void)
{
    l_sys_init();
    l_ifc_init();
    ld_init();
}

/**
 * @brief  获取LIN总线BREAK信号接收次数
 *        返回自上次复位以来检测到的BREAK信号总数
 * @param  无
 * @retval BREAK信号接收计数值(lin_rcvbreak_cnt)
 */
l_u16 lin_lld_sci_rcv_break_cnt(void)
{
    return lin_rcvbreak_cnt;
}
