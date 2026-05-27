/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief  lin dianosticiii source file.
 *
 * @file   diagnosticiii.c
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

#ifdef ENABLE_TEST_MODE
#include "fff_diagnosticIII.h"
#include "fff_tc_log.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "tc_log.h"
#endif

#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
/* PRQA S 3451 2 #3451 - Global identifier '%1s' is intentionally declared in multiple files (e.g., for shared global variable or forward declaration). */
/* PRQA S 3449 1 #3449 - Multiple declarations of external object/function are intentional (e.g., for compatibility or conditional compilation). */
extern uint8_t g_bUDSReadLogInfo;    /**< 诊断日志读取标志（仅LOG_INTERFACE_LIN接口），由SID 0xA0置位，日志读取接口轮询后清零 */
#endif

/*
 * UDS接收缓冲区大小 = 最大队列长度 × 6
 * 用于存储从LIN传输层接收到的诊断请求报文（单帧或多帧）
 */
#define UDS_RECEIVE_BUFFER_SIZE     (MAX_QUEUE_SIZE * 6)    /**< UDS接收缓冲区大小 = 最大队列长度 × 6，用于存储从LIN传输层接收的诊断请求报文（单帧或多帧） */

/* PRQA S 3218 1 #3209 - File scope static variable used in one function, intentional design */
static const char *TAG = "DIAGNOSTICIII";                   /**< 日志标签，用于TC_LOGE等日志宏输出，标识诊断模块来源 */

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1514 1 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/**
 * @brief  UDS数据转储标志
 * @note   由SERVICE_DATA_DUMP(0xB4)服务置位为1，上层应用轮询该标志后执行数据采集
 *        并清零。用于诊断仪请求节点上报ADC原始值、PN结电压、VBAT等模拟信号。
 */
uint8_t g_bUDSDataDumpFlag = 0;

/**
 * @brief  当前接收到的LIN节点地址(NAD)
 * @note   由setUDSNAD()在LIN传输层接收回调中写入，供诊断服务过滤和地址匹配使用。
 *        有效范围0x01~0x7F（单播），0x7E=功能寻址，0x7F=广播。
 *        通过lin_current_rcvd_nad()读取。
 */
static uint8_t current_rcvd_nad;

/**
 * @brief  LIN诊断服务主调度函数
 * @note   遍历所有诊断服务标志位数组lin_diag_services_flag[]，根据
 *        lin_diag_services_supported[]中注册的SID分发至对应处理函数。
 *        支持的LIN标准服务包括：
 *        - 0xB2 ReadByID：通过标识符读取数据
 *        - 0xB6 SaveConfig：保存配置到Flash
 *        - 0xB0 AssignNAD：分配节点地址
 *        - 0xB7 AssignFrameIDRange：分配帧ID范围（LIN 2.1）
 *        - 0xB3 ConditionalChangeNAD：条件改变节点地址
 *        - 0xB1 AssignFrameID：分配帧标识符
 *        - 0x2F IOControl：I/O控制
 *        - 0xB4 DataDump：数据转储
 *        - 0x32 GetTraceability：获取可追溯信息
 *        - 0xA0 读取日志（LOG_INTERFACE_LIN）
 *        传输层模式：_TL_MULTI_FRAME_多帧 或 _TL_SINGLE_FRAME_单帧
 *        未匹配SID转发至lin_custom_diag_service_handle()弱符号实现
 * @param  无
 * @retval 无
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_service_handle(void)
{
    uint16_t length;                              /**< 从传输层接收的诊断请求PDU数据长度（字节数） */
    uint8_t data[UDS_RECEIVE_BUFFER_SIZE];        /**< 诊断请求PDU数据缓冲区，存储从传输层接收的完整报文帧 */

    for (uint8_t i = 0; i < (uint8_t)_DIAG_NUMBER_OF_SERVICES_; i++)
    {
        if (lin_diag_services_flag[i]!=0u)
        {
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
            /* 多帧模式：从传输层接收队列获取完整PDU */
            ld_receive_message(&length, data);
#else /* Single frame support */
            /* 单帧模式：从当前接收PDU指针中逐字节拷贝数据 */
            for (uint8_t index = 2; i < 8; i++)
            {
                data[data_index++] = (*tl_current_rx_pdu_ptr)[2];
            }

            length = (*tl_current_rx_pdu_ptr)[1] & 0x0F
#endif /* End (_TL_FRAME_SUPPORT_ = = _TL_MULTI_FRAME_) */

            switch (lin_diag_services_supported[i])
            {
                case SERVICE_READ_BY_IDENTIFY:/* Mandatory for TL LIN 2.1 & 2.0, Optional for J2602 */
                    /* SID 0xB2：通过标识符(ProductID/SerialNumber/UserDef)读取节点数据 */
                    lin_diagservice_read_by_identifier(data, length);
                    break;

                case SERVICE_SAVE_CONFIGURATION:
                    /* SID 0xB6：将运行时NAD和帧ID配置持久化存储到Flash */
                    lin_diag_save_configuration(data, length);
                    break;

                case SERVICE_ASSIGN_NAD:
                    /* SID 0xB0：分配NAD节点地址，校验Supplier ID和Function ID后写入新NAD */
#if LIN_PROTOCOL == PROTOCOL_J2602
                    lin_assign_NAD_j2602(data, length);
#else
                    lin_diagservice_assign_nad(lin_initial_NAD, data, length);
#endif
                    break;

#if LIN_PROTOCOL == PROTOCOL_21

                case SERVICE_ASSIGN_FRAME_ID_RANGE:    /* Mandatory for TL LIN 2.1 */
                    /* SID 0xB7：分配连续4个帧ID范围（仅LIN 2.1） */
                    lin_diag_assign_frame_id_range(data, length);
                    break;
#endif

#if LIN_PROTOCOL != PROTOCOL_J2602

                case SERVICE_CONDITIONAL_CHANGE_NAD:
                    /* SID 0xB3：按条件（XOR Invert & AND Mask）改变节点地址 */
                    lin_diag_conditional_change_nad(data, length);
                    break;
#endif

#if ((LIN_PROTOCOL == PROTOCOL_20) || (LIN_PROTOCOL == PROTOCOL_J2602) || (LIN_PROTOCOL == PROTOCOL_21))

                case SERVICE_ASSIGN_FRAME_ID:
                    /* SID 0xB1：分配/删除帧标识符，校验PID奇偶和Supplier/Function ID匹配 */
                    lin_diag_assign_frame_identifier(data, length);
                    break;
#endif

#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
                case 0xA0:
                    /* 专有SID 0xA0：置位日志读取标志，供LIN日志接口读取诊断日志 */
                    g_bUDSReadLogInfo = 1;
                    break;
#endif

#if LIN_PROTOCOL == PROTOCOL_J2602

                case SERVICE_TARGET_RESET:
                    /* SID 0xB5：J2602协议目标复位，将各帧error signal位置为Reset状态 */
                    lin_diag_target_reset(data, length);
                    break;
#endif

                case SERVICE_IO_CONTROL_BY_IDENTIFY:
                    /* SID 0x2F：通过标识符控制I/O（短时控制输出/强制输入值） */
                    lin_diag_io_control_by_identifier(data, length);
                    break;

                case SERVICE_DATA_DUMP:
                    /* SID 0xB4：数据转储请求，置位g_bUDSDataDumpFlag供上层采集数据 */
                    g_bUDSDataDumpFlag = 1;
                    break;

                case SERVICE_GET_TRACEABILITY_MSG:
                    /* SID 0x32：获取可追溯信息（当前为存根实现） */
                    lin_diag_get_traceability_msg(data, length);
                    break;
#ifdef CFG_LIN_CONFORM_TEST

                case 0xad:
                    /* 一致性测试专用SID 0xAD */
                    diag_0xad_command(data, length);

                    break;

                case 0xae:
                    /* 一致性测试专用SID 0xAE */
                    diag_0xae_command(data, length);

                    break;

                case 0xaf:
                    /* 一致性测试专用SID 0xAF */
                    diag_0xaf_command(data, length);
                    break;
#endif
                default:
                    /* 未匹配SID：转入弱符号自定义处理函数，用户可在custom_diagnosticIII.c中重定义 */
                    lin_custom_diag_service_handle(lin_diag_services_supported[i], data, length);
                    break;
            }
/* PRQA S 2987 ++ #2987 - Function call with no side effects is intentional (e.g., access to volatile, debug hook, or intentional no-op). */
            lin_diag_services_flag[i] = 0;
        }
    }
    /* 所有诊断服务调度完成后调用钩子函数（用户可重定义） */
    lin_diag_service_hook();
}
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
/**
 * @brief  设置当前接收到的LIN节点地址(NAD)
 * @param  NAD: 目标节点地址值（范围0x01~0x7F，0x7E=功能寻址，0x7F=广播）
 * @note   由LIN传输层接收回调调用，保存当前诊断请求的目标NAD至静态变量current_rcvd_nad，
 *        供后续诊断服务进行地址过滤和路由判断。
 *        该值可通过lin_current_rcvd_nad()读取。
 * @retval 无
 */
void setUDSNAD(uint8_t NAD)
{
    current_rcvd_nad = NAD;
}
/**
 * @brief  发送UDS肯定响应（Positive Response）
 * @param  sid:    请求的服务标识符(SID)，响应首字节为 sid + 0x40
 * @param  data:   响应数据缓冲区指针，将被拼接在SID+0x40之后
 * @param  length: 响应数据长度（字节数），不含SID+0x40首字节
 * @note   肯定响应报文格式: [SID|0x40][data0][data1]...[dataN]
 *        实际发送总长度 = 1 + length 字节
 *        SID|0x40规则：UDS协议定义正响应SID = 请求SID的第6位置1，
 *        例如请求SID 0x22 → 正响应SID 0x62
 *        若length超过(UDS_RECEIVE_BUFFER_SIZE - 1)则打印溢出日志
 * @retval 无
 */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diag_positive_notify(uint8_t sid, uint8_t *data, uint16_t length)
{
    l_u8 slave_resp_dat[UDS_RECEIVE_BUFFER_SIZE]; /**< 从机正响应数据缓冲区，byte[0]=SID|0x40(正响应SID)，byte[1..N]=响应负载数据 */

    if (length > (sizeof(slave_resp_dat) - 1u))
    {
/* PRQA S 2741 6 #2741 - The controlling expression is constant true by design (e.g., debug log enabled). */
/* PRQA S 2880 5 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
/* PRQA S 2742 4 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
/* PRQA S 1036 3 #1036 - Comma before ## in variadic macro (GNU extension) is used to swallow comma when no variable args, supported by compiler. */
/* PRQA S 1035 2 #1035 - Macro with variable arguments called without variable arguments (GNU extension), accepted by toolchain. */
/* PRQA S 3432 1 #3267 - Macro arguments are safely used without unintended operator precedence issues */
        TC_LOGE(TAG, "lin positive notify overflow");
    }

    /* 构造正响应SID = 请求SID | 0x40 */
    slave_resp_dat[0] = sid + 0x40u;

    for (uint16_t i = 0; i < length; i++)
    {
        slave_resp_dat[1u + i] = data[i];
    }

    /* 通过传输层发送响应（总长度 = 1字节SID + length字节数据） */
    ld_send_message((l_u16)(1u + length), (l_u8 *)slave_resp_dat);
}

/**
 * @brief  发送UDS否定响应（Negative Response / NRC）
 * @param  sid:        请求的服务标识符(SID)
 * @param  resp_value: 否定响应码(NRC)，指示拒绝原因
 * @note   否定响应报文固定3字节: [0x7F][SID][NRC]
 *        首字节0x7F = 否定响应标识（UDS协议固定值）
 *        第2字节 = 原始请求SID
 *        第3字节 = NRC错误码
 * 
 *        常用NRC码（定义在diagnosticIII.h）：
 *        NRC_0x10(GENERAL_REJECT)     - 通用拒绝
 *        NRC_0x11(SNS)                - 服务不支持（请求的SID未实现）
 *        NRC_0x12(SFNS)               - 子功能不支持
 *        NRC_0x13(IMLOIF)             - 报文长度错误或格式无效
 *        NRC_0x14(RTL)                - 响应太长
 *        NRC_0x21(BRR)                - 忙/重复请求
 *        NRC_0x22(CNC)                - 条件不满足（如会话/安全/电压条件）
 *        NRC_0x24(RSE)                - 请求顺序错误
 *        NRC_0x25(NRFSC)              - 子网组件无响应
 *        NRC_0x26(FPEORA)             - 故障阻止执行请求动作
 *        NRC_0x31(ROOR)               - 请求超出范围（DID/数据/参数无效）
 *        NRC_0x33(SAD)                - 安全访问被拒绝（未解锁或密钥错误）
 *        NRC_0x35(IK)                 - 无效密钥（安全访问密钥不匹配）
 *        NRC_0x36(ENOA)               - 超过尝试次数（安全访问锁定）
 *        NRC_0x78(RCRRP)              - 请求正确接收-响应待定（P2超时延长）
 * @retval 无
 */
void lin_diag_negative_notify(uint8_t sid, uint8_t resp_value)
{
    l_u8 slave_resp_dat[UDS_RECEIVE_BUFFER_SIZE]; /**< 从机负响应数据缓冲区，byte[0]=0x7F(否定响应标识)，byte[1]=原始请求SID，byte[2]=NRC错误码 */

    slave_resp_dat[0] = RES_NEGATIVE;   /* 固定前缀 0x7F */
    slave_resp_dat[1] = sid;            /* 原始请求SID */
    slave_resp_dat[2] = resp_value;     /* NRC错误码 */

    ld_send_message(3, (l_u8 *)slave_resp_dat);
}

/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2071 1 #3269 - Language extension used for compiler and hardware optimization */
/**
 * @brief  自定义诊断服务处理函数（弱符号，可重定义）
 *         当SID匹配不到任何LIN标准服务时，由lin_diag_service_handle()的default分支调用
 * @param  sid:    未匹配的自定义服务标识符(SID)
 * @param  ptr:    接收到的数据缓冲区指针
 * @param  length: 数据长度（字节数）
 * @note   声明为__attribute__((weak))，用户可在其他源文件（如custom_diagnosticIII.c）中
 *        重新定义以实现私有扩展诊断服务（如0x22/0x2E/0x27/0x2F等UDS服务）。
 *        默认实现为空操作，所有参数未使用。
 * @retval 无
 */
__attribute__((weak)) void lin_custom_diag_service_handle(uint8_t sid, uint8_t *ptr, uint16_t length)
{
    (void)(sid);
    (void)(ptr);
    (void)(length);
}

/* PRQA S 2071 1 #3269 - Language extension used for compiler and hardware optimization */
/**
 * @brief  诊断服务调度完成后钩子函数（弱符号，可重定义）
 *         在lin_diag_service_handle()遍历完所有服务标志并清零后调用
 * @param  无
 * @note   声明为__attribute__((weak))，用户可重新定义以实现服务调度后的自定义处理，
 *        如状态上报、日志记录、P2超时刷新等。
 *        默认实现为空操作。
 * @retval 无
 */
__attribute__((weak)) void lin_diag_service_hook(void)
{
}

/**
 * @brief  获取当前接收到的LIN节点地址(NAD)
 * @param  无
 * @note   返回静态变量current_rcvd_nad的值，该值由setUDSNAD()在传输层接收回调中写入。
 *        用于诊断层判断当前通信目标地址，配合lin_initial_NAD/lin_configured_NAD
 *        进行地址过滤和路由选择。
 * @retval 当前NAD值(uint8_t)，范围0x01~0x7F
 */
uint8_t lin_current_rcvd_nad(void)
{
	return current_rcvd_nad;
}
