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

#include "fff_diagnosticIII.h"
#include "fff_tc_log.h"


#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
extern uint8_t g_bUDSReadLogInfo;
#endif

#define UDS_RECEIVE_BUFFER_SIZE     (MAX_QUEUE_SIZE * 6)

static const char *TAG = "DIAGNOSTICIII";

uint8_t g_bUDSDataDumpFlag = 0;
static uint8_t current_rcvd_nad;

// DEFINE_FAKE_VOID_FUNC(lin_diagservice_read_by_identifier, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_assign_frame_id_range, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diagservice_assign_nad, uint8_t, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(lin_assign_NAD_j2602, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_conditional_change_nad, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_assign_frame_identifier, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(lin_diag_target_reset, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(lin_diag_service_handle);
// DEFINE_FAKE_VOID_FUNC(lin_diag_tester_present, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_save_configuration, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(lin_diag_data_dump_control, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_ecu_reset, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_snpd, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_write_by_identifier, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_io_control_by_identifier, uint8_t *, uint16_t);
// DEFINE_FAKE_VOID_FUNC(lin_diag_get_traceability_msg, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(clear_dtc_info_handle, uint8_t *, uint16_t);

DEFINE_FAKE_VOID_FUNC(lin_diag_positive_notify, uint8_t, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(lin_diag_negative_notify, uint8_t, uint8_t);
DEFINE_FAKE_VALUE_FUNC(uint8_t, lin_current_rcvd_nad);

#ifdef CFG_LIN_CONFORM_TEST
DEFINE_FAKE_VOID_FUNC(diag_0xad_command, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(diag_0xae_command, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(diag_0xaf_command, uint8_t *, uint16_t);
#endif