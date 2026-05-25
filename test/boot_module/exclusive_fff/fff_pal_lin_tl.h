#ifndef __FFF_PAL_LIN_TL_H__
#define __FFF_PAL_LIN_TL_H__

#include <stdint.h>
#include <stdbool.h>
#include "fff.h"
#include "fff_pal_lin_comm.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define POSITIVE 1
#define NEGATIVE 0

#define LIN_BROADCAST_NAD                     0x7Fu   /**< NAD */
#define LIN_FUNCTION_NAD                      0x7Eu
#define LIN_RES_NEGATIVE                      0x7Fu      /**< negative response */

/**
 * @brief lin negative response
 */
#define RES_NEGATIVE                                0x7Fu      /* negative response */
#define GENERAL_REJECT                              0x10u      /*Error code raised when request for service not supported comes  */
#define SERVICE_NOT_SUPPORTED                       0x11u      /*not supported service */
#define SUBFUNCTION_NOT_SUPPORTED                   0x12u      /*not supported subfunction  */
#define IMLOIF                                      0x13u      /*incorrect Message LengtOr InvalidFormat*/
#define RESPONSE_TOO_LONG                           0x14u      /*responseTooLong*/
#define BUSY_REPEAT_REQUEST                         0x21u      /*busyRepeatRequest*/
#define CONDITION_NOT_CORRECT                       0x22u      /*conditionsNotCorrect*/
#define REQUEST_SEQUEENCE_ERROR                     0x24u      /*requestSequenceError*/
#define NRFSC                                       0x25u      /*no Response From Subnet Component*/
#define FPEORA                                      0x26u      /*Failure Prevents Execution Of Requested Action*/
#define REQUEST_OUT_RANGE                           0x31u      /*request Out Of Range*/
#define SECURITY_ACCESS_DENIED                      0x33u      /*security Access Denied*/
#define INVALID_KEY                                 0x35u      /*invalid Key*/
#define ENOA                                        0x36u      /*exceed Number Of Attempts*/
#define REQUIREDTIMEDELAY_NOTEXPIRED                0x37u     
#define DOWNLOAD_REJECTED                           0x70u      /*Upload/download request has been rejected*/
#define TRANSFER_DATA_PAUSE                         0x71u      /*Data transmission is paused*/
#define RCRRP                                       0x78u      /*request Correctly Received-Response Pending*/
#define GENERAL_PROGRAM_FAILURE                     0x72u      /*general Programming Failure*/
#define BLOCK_SEQUENCE_COUNT_ERR                    0x73u      /*block sequence counter error*/
#define SUBFUNCTION_NOTSUPPORTED_INACTIVESESSION    0x7Eu
#define SERVICENOTSUPPORTED_INACTIVESESSION         0x7Fu

/*-------------enum and struct---------------------------*/
/**
  * @brief  lin event type enumeration
  */
typedef enum
{
    LIN_EVENT_PID_OK,               /**< LIN_EVENT_PID_OK */
    LIN_EVENT_TX_COMPLETED,         /**< LIN_EVENT_TX_COMPLETED */
    LIN_EVENT_RX_COMPLETED,         /**< LIN_EVENT_RX_COMPLETED */
    LIN_EVENT_PID_ERR,              /**< LIN_EVENT_PID_ERR */
    LIN_EVENT_CHECKSUM_ERR,         /**< LIN_EVENT_CHECKSUM_ERR */

    LIN_EVENT_SYNC_VALUE_ERR,       /**< LIN_EVENT_SYNC_VALUE_ERR */
    LIN_EVENT_RX_PTY_CHK_ERR,       /**< LIN_EVENT_RX_PTY_CHK_ERR */
    LIN_EVENT_RX_TIMEOUT,           /**< LIN_EVENT_RX_TIMEOUT */
    LIN_EVENT_TX_RX_CONF,           /**< LIN_EVENT_TX_RX_CONF */
    LIN_EVENT_TX_PID_DONE,          /**< LIN_EVENT_TX_PID_DONE */
    LIN_EVENT_RX_BYTE,          /**< LIN_EVENT_RX_BYTE */
} lin_event_type_e;

/**
  * @brief  lin bus state enumeration
  */
typedef enum
{
    LIN_BUS_IDLE           = 0,
    LIN_BUS_SEND          = 1,
    LIN_BUS_RECV,
    LIN_BUS_ERROR,
} lin_bus_state_e;

/**
  * @brief  lin transport layer data
  */
typedef uint8_t lin_tl_data[8];

/**
  * @brief  lin packet struct
  */
typedef struct
{
    uint8_t id;
    uint8_t len;
    uint8_t buff[8];
} lin_packet_t;

/**
  * @brief  lin  transport layer queue struct
  */
typedef struct
{
    bool            ready;
    uint8_t         header;           /* the first element of queue */
    uint8_t         tail;             /* the last element of queue */
    uint16_t        frame_index;      /* the length of data */
    uint8_t         frame_byte;       /* the byte cnt of a frame */
    uint8_t         pid;              /* the pid of a frame */
    lin_tl_data     *tl_data;         /* the ptr of lin data */
} lin_tl_queue_t;

/**
  * @brief  lin  recv context struct
  */
typedef struct
{
    uint8_t         nad;
    uint8_t         pci;
    uint8_t         sid;
    uint16_t        remain_length;
    uint16_t        total_length;
    uint8_t         frame_index;
} lin_recv_context_t;

/*-------------Function port--------------------------*/
// DECLARE_FAKE_VOID_FUNC()
// DECLARE_FAKE_VALUE_FUNC()

DECLARE_FAKE_VOID_FUNC(lin_tl_init);
DECLARE_FAKE_VALUE_FUNC(bool,lin_uds_send,uint8_t,uint8_t *,uint16_t);
DECLARE_FAKE_VALUE_FUNC(bool,lin_uds_negative_response,uint8_t,uint8_t,uint8_t);
DECLARE_FAKE_VALUE_FUNC(bool,lin_uds_receive,uint8_t,uint8_t *,uint16_t *);
DECLARE_FAKE_VALUE_FUNC(bool,lin_tl_uncd_frame_get,lin_bus_e,uint8_t *,uint8_t *);

// void lin_tl_init(void);
// bool lin_uds_send(uint8_t nad, uint8_t *data, uint16_t length);
// bool lin_uds_negative_response(uint8_t nad, uint8_t sid, uint8_t error_code);
// bool lin_uds_receive(uint8_t nad, uint8_t *data, uint16_t *length);
// bool lin_tl_uncd_frame_get(lin_bus_e bus, uint8_t *pid, uint8_t *buffer);


#ifdef __cplusplus
}
#endif
#endif
