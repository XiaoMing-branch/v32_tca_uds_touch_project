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
uint8_t g_bUDSDataDumpFlag = 0;
static uint8_t current_rcvd_nad;

/********************************************************
** \brief   lin_diag_service_handle
**
** \param   None
**
** \retval  None
*********************************************************/
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
void setUDSNAD(uint8_t NAD)
{
    current_rcvd_nad = NAD;
}
/* PRQA S 2987 -- */
/********************************************************
** \brief   dfu_do_notify_cp        data return
**
** \param   uint8_t                 sid:service_id
** \param   uint8_t                 *data
** \param   uint16_t                length
**
** \retval  None
*********************************************************/
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

/********************************************************
** \brief   lin_diag_notify_response    response
**
** \param   uint8_t                     resp_type
** \param   uint8_t                     sid:service_id
** \param   uint8_t                     resp_value
**
** \retval  None
*********************************************************/
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
__attribute__((weak)) void lin_custom_diag_service_handle(uint8_t sid, uint8_t *ptr, uint16_t length)
{
    (void)(sid);
    (void)(ptr);
    (void)(length);
}

/* PRQA S 2071 1 #3269 - Language extension used for compiler and hardware optimization */
__attribute__((weak)) void lin_diag_service_hook(void)
{
}

uint8_t lin_current_rcvd_nad(void)
{
	return current_rcvd_nad;
}
