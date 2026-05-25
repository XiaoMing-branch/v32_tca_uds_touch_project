/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   diagnosticIII header file.
 *
 * @file    diagnosticIII.h
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

#ifndef DIAGNOSTICIII_H__
#define DIAGNOSTICIII_H__

#include "test_config.h"
#ifdef ENABLE_TEST_MODE
#include <stdint.h>
#include "fff_lin.h"
#include "fff_linlib.h"
#else
#include <stdint.h>
#include "lin.h"
#include "linlib.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

// added #ifndef NULL because there's a warning says: warning: "NULL" redefined
#ifndef NULL
#define NULL                                    0
#endif

/* PRQA S 1534 ++ #3261 - Unused macro defined for future extension and configuration compatibility */
#define NEGATIVE                                0
#define POSITIVE                                1
#define MaxNumberOfBlockLength                  0x50

#define NEGTIVE_ID                              0x7F
#define SNS                                     0x11            /*serviceNotSupported*/
#define SFNS                                    0x12            /*sub-functionNotSupported*/
#define IMLOIF                                  0x13            /*incorrectMessageLengthOrInvalidFormat*/
#define RTL                                     0x14            /*responseTooLong*/
#define BRR                                     0x21            /*busyRepeatRequest*/
#define CNC                                     0x22            /*conditionsNotCorrect*/
#define RSE                                     0x24            /*requestSequenceError*/
#define NRFSC                                   0x25            /*noResponseFromSubnetComponent*/
#define FPEORA                                  0x26            /*FailurePreventsExecutionOfRequestedAction*/
#define ROOR                                    0x31            /*requestOutOfRange*/
#define SAD                                     0x33            /*securityAccessDenied*/
#define IK                                      0x35            /*invalidKey*/
#define ENOA                                    0x36            /*exceedNumberOfAttempts*/
#define RCRRP                                   0x78            /*requestCorrectlyReceived-ResponsePending*/

#define DATA_DUMP_TEMP                          0x00
#define DATA_DUMP_VBAT                          0x01
#define DATA_DUMP_B_PN                          0x02
#define DATA_DUMP_R_PN                          0x03
#define DATA_DUMP_G_PN                          0x04
#define DATA_DUMP_V_TEMP                        0x05
#define DATA_DUMP_V_VBAT                        0x06
#define DATA_DUMP_V_B_PN                        0x07
#define DATA_DUMP_V_R_PN                        0x08
#define DATA_DUMP_V_G_PN                        0x09

#define COMMAND_GET_LED_RGB_PARAM               0x00
#define COMMAND_SET_LED_RGB_PARAM               0x01
#define COMMAND_GET_LED_PN_VOLT                 0x02
#define COMMAND_GET_LED_TYPICAL_PN_VOLT         0x03
#define COMMAND_SET_LED_TYPICAL_PN_VOLT         0x04
#define COMMAND_SET_LED_RGB_CURRENT             0x05
#define COMMAND_SET_LED_PN_VOLT_TRIGGER         0x06
#define COMMAND_GET_PWM_PARAMETER               0x07
#define COMMAND_SET_RGB_TEMPERATURE_SLOPE       0x08
#define COMMAND_GET_RGB_TEMPERATURE_SLOPE       0x09
#define COMMAND_GET_LED_RGB_PARAM_RAM           0x10
#define COMMAND_SET_LED_PWM_LIGHTING            0x11
#define COMMAND_SET_TEMPERATURE_ADJUST          0x12
#define COMMAND_GET_LED_RGB_CURRENT             0x13
#define COMMAND_SET_LED_LUV_LIGHTING            0x14
#define COMMAND_SET_LED_RGBL_LIGHTING           0x15
#define COMMAND_GET_RGBL_PARAMETER              0x16
#define COMMAND_SET_WHITE_POINT_CONFIG          0x17
#define COMMAND_GET_WHITE_POINT_CONFIG          0x18
#define COMMAND_SET_RELATIVE_FACTOR             0x19
#define COMMAND_GET_RELATIVE_FACTOR             0x20
#define COMMAND_SET_STATIC_PN_SAMPLE            0x21
#define COMMAND_GET_VERSION_INFO                0x22
#define COMMAND_GET_UUID                        0x23
#define COMMAND_GET_STATIC_PN_SAMPLE            0x24
#define COMMAND_SET_LED_CXY_LIGHTING            0x25
#define COMMAND_SET_WHITETEST_LIGHTING          0x26
#define COMMAND_SET_LED_RGB_PARAM_RESET         0x27
#define COMMAND_GET_LED_VENDOR_INFO             0x28
#define COMMAND_GET_LED_STATUS                  0x29
#define COMMAND_SET_REG_CFG                     0x30
#define COMMAND_GET_REG_CFG                     0x31
#define COMMAND_GET_TEST_VAULE                  0x32

//CUSTOMER_NAD_CONFIG
#define CUSTOMER_NAD_ORIGIN_SET                 0x01
#define CUSTOMER_NAD_ORIGIN_GET                 0x02
#define CUSTOMER_NAD_RECOVERY_ORIGIN            0x03
#define CUSTOMER_NAD_SET_FOR_DFU                0x04
#define CUSTOMER_NAD_GET_FOR_DFU                0x05
/* PRQA S 1534 -- */


void lin_diagservice_read_by_identifier(uint8_t *ptr, uint16_t length);
void lin_diag_assign_frame_id_range(uint8_t *ptr, uint16_t length);
void lin_diagservice_assign_nad(uint8_t NAD, uint8_t *ptr, uint16_t length);
void lin_assign_NAD_j2602(uint8_t *ptr, uint16_t length);
void lin_diag_conditional_change_nad(uint8_t *ptr, uint16_t length);
void lin_diag_assign_frame_identifier(uint8_t *ptr, uint16_t length);
void lin_diag_target_reset(uint8_t *ptr, uint16_t length);
void lin_diag_service_handle(void);
void lin_diag_tester_present(uint8_t *ptr, uint16_t length);
void lin_diag_save_configuration(uint8_t *ptr, uint16_t length);
void lin_diag_data_dump_control(uint8_t *ptr, uint16_t length);
void lin_diag_ecu_reset(uint8_t *ptr, uint16_t length);
void lin_diag_snpd(uint8_t *ptr, uint16_t length);
void lin_diag_write_by_identifier(uint8_t *ptr, uint16_t length);
void lin_diag_io_control_by_identifier(uint8_t *ptr, uint16_t length);
void lin_diag_get_traceability_msg(uint8_t *ptr, uint16_t length);
void clear_dtc_info_handle(uint8_t *ptr, uint16_t length);
#ifdef CFG_LIN_CONFORM_TEST
void diag_0xad_command(uint8_t *ptr, uint16_t length);
void diag_0xae_command(uint8_t *ptr, uint16_t length);
void diag_0xaf_command(uint8_t *ptr, uint16_t length);
#endif

void lin_diag_positive_notify(uint8_t sid, uint8_t *data, uint16_t length);
void lin_diag_negative_notify(uint8_t sid, uint8_t resp_value);

void lin_custom_diag_service_handle(uint8_t sid,uint8_t *ptr, uint16_t length);
void lin_diag_service_hook(void);
uint8_t lin_current_rcvd_nad(void);
void LinDiagnosticSessionCheck(void);
#ifdef __cplusplus
}
#endif
#endif /* __DIAGNOSTICIII_H__ */
