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
extern uint8_t g_bUDSReadLogInfo;
#endif

#define UDS_RECEIVE_BUFFER_SIZE     (MAX_QUEUE_SIZE * 6)

/* PRQA S 3218 1 #3209 - File scope static variable used in one function, intentional design */
static const char *TAG = "DIAGNOSTICIII";

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1514 1 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
uint8_t g_bUDSDataDumpFlag = 0; /* UDS数据转储标志, 由SERVICE_DATA_DUMP服务置位, 上层轮询后清零 */
static uint8_t current_rcvd_nad; /* 当前接收到的节点地址(NAD), 由setUDSNAD()设置, 供诊断服务过滤使用 */

/**
 * @brief  LIN诊断服务主调度函数
 *        遍历所有诊断服务标志位, 将接收到的数据分发至对应的LIN标准服务处理函数,
 *        最后调用自定义服务钩子和服务后处理钩子
 * @param  无
 * @note   支持的LIN标准服务包括: ReadByID / SaveConfig / AssignNAD /
 *        AssignFrameID / ConditionalChangeNAD / IOControl / DataDump /
 *        TargetReset / GetTraceability等;
 *        单帧/多帧传输模式通过_TL_FRAME_SUPPORT_宏切换;
 *        未匹配的SID转发至lin_custom_diag_service_handle()处理
 * @retval 无
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_service_handle(void)
{
    uint16_t length;
    uint8_t data[UDS_RECEIVE_BUFFER_SIZE];

    for (uint8_t i = 0; i < (uint8_t)_DIAG_NUMBER_OF_SERVICES_; i++)
    {
        if (lin_diag_services_flag[i]!=0u)
        {
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
            /* get pdu from rx queue */
            ld_receive_message(&length, data);
#else /* Single frame support */

            for (uint8_t index = 2; i < 8; i++)
            {
                data[data_index++] = (*tl_current_rx_pdu_ptr)[2];
            }

            length = (*tl_current_rx_pdu_ptr)[1] & 0x0F
#endif /* End (_TL_FRAME_SUPPORT_ = = _TL_MULTI_FRAME_) */

            switch (lin_diag_services_supported[i])
            {
                case SERVICE_READ_BY_IDENTIFY:/* Mandatory for TL LIN 2.1 & 2.0, Optional for J2602 */
                    lin_diagservice_read_by_identifier(data, length);
                    break;

                case SERVICE_SAVE_CONFIGURATION:
                    lin_diag_save_configuration(data, length);
                    break;

                case SERVICE_ASSIGN_NAD:
#if LIN_PROTOCOL == PROTOCOL_J2602
                    lin_assign_NAD_j2602(data, length);
#else
                    lin_diagservice_assign_nad(lin_initial_NAD, data, length);
#endif
                    break;

#if LIN_PROTOCOL == PROTOCOL_21

                case SERVICE_ASSIGN_FRAME_ID_RANGE:    /* Mandatory for TL LIN 2.1 */
                    lin_diag_assign_frame_id_range(data, length);
                    break;
#endif

#if LIN_PROTOCOL != PROTOCOL_J2602

                case SERVICE_CONDITIONAL_CHANGE_NAD:
                    lin_diag_conditional_change_nad(data, length);
                    break;
#endif

#if ((LIN_PROTOCOL == PROTOCOL_20) || (LIN_PROTOCOL == PROTOCOL_J2602) || (LIN_PROTOCOL == PROTOCOL_21))

                case SERVICE_ASSIGN_FRAME_ID:
                    lin_diag_assign_frame_identifier(data, length);
                    break;
#endif

#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
                case 0xA0:
                    g_bUDSReadLogInfo = 1;
                    break;
#endif

#if LIN_PROTOCOL == PROTOCOL_J2602

                case SERVICE_TARGET_RESET:
                    lin_diag_target_reset(data, length);
                    break;
#endif

                case SERVICE_IO_CONTROL_BY_IDENTIFY:
                    lin_diag_io_control_by_identifier(data, length);
                    break;

                case SERVICE_DATA_DUMP:
                    g_bUDSDataDumpFlag = 1;
                    break;

                case SERVICE_GET_TRACEABILITY_MSG:
                    lin_diag_get_traceability_msg(data, length);
                    break;
#ifdef CFG_LIN_CONFORM_TEST

                case 0xad:
                    diag_0xad_command(data, length);

                    break;

                case 0xae:
                    diag_0xae_command(data, length);

                    break;

                case 0xaf:
                    diag_0xaf_command(data, length);
                    break;
#endif
                default:
                    lin_custom_diag_service_handle(lin_diag_services_supported[i], data, length);
                    break;
            }
/* PRQA S 2987 ++ #2987 - Function call with no side effects is intentional (e.g., access to volatile, debug hook, or intentional no-op). */
            lin_diag_services_flag[i] = 0;
        }
    }
    lin_diag_service_hook();
}
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
/**
 * @brief  设置当前接收到的节点地址(NAD)
 * @param  NAD: 目标节点地址值
 * @note   将传入的NAD值保存至静态变量current_rcvd_nad中,
 *        供后续诊断服务进行地址过滤和路由判断
 * @retval 无
 */
void setUDSNAD(uint8_t NAD)
{
    current_rcvd_nad = NAD;
}
/**
 * @brief  发送UDS肯定响应
 *        组包格式: [SID+0x40][data...], 通过传输层ld_send_message()发送
 * @param  sid:    服务标识符(SID), 响应中会自动加上0x40
 * @param  data:   响应数据缓冲区指针
 * @param  length: 响应数据长度
 * @note   肯定响应报文格式: 首字节为SID|0x40, 后续紧跟响应数据;
 *        若length超过内部缓冲区容量则打印错误日志但不阻塞发送;
 *        实际发送长度为 1 + length 字节
 * @retval 无
 */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diag_positive_notify(uint8_t sid, uint8_t *data, uint16_t length)
{
    l_u8 slave_resp_dat[UDS_RECEIVE_BUFFER_SIZE];

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

    slave_resp_dat[0] = sid + 0x40u;

    for (uint16_t i = 0; i < length; i++)
    {
        slave_resp_dat[1u + i] = data[i];
    }

    ld_send_message((l_u16)(1u + length), (l_u8 *)slave_resp_dat);
}

/**
 * @brief  发送UDS否定响应(NRC)
 *        组包格式: [0x7F][SID][NRC], 通过传输层ld_send_message()发送
 * @param  sid:        请求的服务标识符(SID)
 * @param  resp_value: 否定响应码(NRC), 指示拒绝原因
 * @note   否定响应报文固定3字节: 首字节0x7F(否定响应标识),
 *        第2字节为原始SID, 第3字节为NRC错误码;
 *        常见NRC包括: 0x12(不支持), 0x22(条件错误), 0x31(请求越界)等
 * @retval 无
 */
void lin_diag_negative_notify(uint8_t sid, uint8_t resp_value)
{
    l_u8 slave_resp_dat[UDS_RECEIVE_BUFFER_SIZE];

    slave_resp_dat[0] = RES_NEGATIVE;
    slave_resp_dat[1] = sid;
    slave_resp_dat[2] = resp_value;

    ld_send_message(3, (l_u8 *)slave_resp_dat);
}

/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2071 1 #3269 - Language extension used for compiler and hardware optimization */
/**
 * @brief  自定义诊断服务处理函数(弱符号)
 *        当SID匹配不到任何LIN标准服务时, 由lin_diag_service_handle()调用
 * @param  sid:    自定义服务标识符
 * @param  ptr:    接收到的数据缓冲区指针
 * @param  length: 数据长度
 * @note   声明为__attribute__((weak)), 用户可在其他源文件中重新定义
 *        以实现私有扩展诊断服务; 默认实现为空操作, 参数未使用
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
 * @brief  诊断服务调度完成后钩子函数(弱符号)
 *        在lin_diag_service_handle()遍历完所有服务标志后调用
 * @param  无
 * @note   声明为__attribute__((weak)), 用户可重新定义以实现
 *        服务调度后的自定义处理(如状态上报、日志记录等);
 *        默认实现为空操作
 * @retval 无
 */
__attribute__((weak)) void lin_diag_service_hook(void)
{
}

/**
 * @brief  获取当前接收到的节点地址(NAD)
 * @param  无
 * @note   返回静态变量current_rcvd_nad的值, 该值由setUDSNAD()写入,
 *        用于诊断层判断当前通信目标地址及地址过滤
 * @retval 当前NAD值(uint8_t)
 */
uint8_t lin_current_rcvd_nad(void)
{
	return current_rcvd_nad;
}
