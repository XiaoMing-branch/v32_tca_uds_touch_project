/**
 *****************************************************************************
 * @brief   pal lin communication source file.
 *
 * @file    pal_lin_comm.c
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

#include "fff_pal_func_def.h"
#include "fff_pal_lin_comm.h"
#include "fff_utilities.h"

/**
 * @brief LIN ISR FUNC FLAG
 */
#define LIN_ISR0_BASE_FLAG (SCI_INT_BRK_DET | SCI_INT_SYNC_DET | SCI_INT_SYNC_VAL_ERR |         \
                            SCI_INT_RX_PID_DONE | SCI_INT_RX_STP_ERR | SCI_INT_RX_PTY_CHK_ERR | \
                            SCI_INT_TX_RX_CONF)
#ifdef __TCPL01X__
#define LIN_ISR0_FUNC_FLAG (LIN_ISR0_BASE_FLAG | SCI_INT_TX_DONE)
#else
#define LIN_ISR0_FUNC_FLAG (LIN_ISR0_BASE_FLAG | SCI_INT_TX_DONE | SCI_INT_TX_FIFO_EMPTY)
#endif

#define BIT(A,B)            (((A)>>(B))&0x01)

DEFINE_FAKE_VOID_FUNC(pal_lin_init,lin_bus_e, lin_mode_e, uint32_t, ISR_FUNC_CALLBACK);
DEFINE_FAKE_VOID_FUNC(pal_lin_deinit,lin_bus_e);
DEFINE_FAKE_VOID_FUNC(pal_lin_rx_response,lin_bus_e, uint8_t, uint8_t *, uint8_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_tx_response,lin_bus_e, uint8_t, uint8_t *, uint8_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_tx_4byte,lin_bus_e, uint8_t *, uint8_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_tx_header,lin_bus_e, uint8_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_aa_enter,uint16_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_aa_exit);
DEFINE_FAKE_VOID_FUNC(pal_lin_aa_ready);
DEFINE_FAKE_VALUE_FUNC(uint8_t,pal_lin_parity_calib,lin_parity_type_e,uint8_t);
DEFINE_FAKE_VALUE_FUNC(uint8_t,pal_lin_checksum_calib,uint8_t,uint8_t *);
DEFINE_FAKE_VALUE_FUNC(uint16_t,pal_lin_aa_raw_code_get,uint16_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_abort_handle,lin_bus_e, lin_abort_type_e);
DEFINE_FAKE_VOID_FUNC(pal_lin_read_byte,lin_bus_e, lin_read_type_e, uint8_t *);
DEFINE_FAKE_VOID_FUNC(pal_lin_autobaudrate_check,lin_bus_e);
