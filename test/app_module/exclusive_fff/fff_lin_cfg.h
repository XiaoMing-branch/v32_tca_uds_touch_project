/**
*****************************************************************************
* @brief  demo example source file.
* @file   lin_cfg.h
* @author AE/FAE team
* @date   28/JUL/2023
*****************************************************************************
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <b>&copy; Copyright (c) 2023 Tinychip Microelectronics Co.,Ltd.</b>
*
******************************************************************************/
#ifndef    __FFF_LIN_CFG_H_
#define    __FFF_LIN_CFG_H_

#include "fff.h"
#include "fff_lin_hw_cfg.h"
/* Define operating mode */
#define _MASTER_MODE_     0
#define _SLAVE_MODE_      1
#define LIN_MODE   _SLAVE_MODE_
/* Define protocol version */
#define PROTOCOL_21       0
#define PROTOCOL_J2602    1
#define PROTOCOL_20       2
#define LIN_PROTOCOL    PROTOCOL_21

#define SCI_ADDR        SCI0_ADDR    /* For slave */

#define LIN_BAUD_RATE    19200    	 /*For slave*/
/**********************************************************************/
/***************          Diagnostic class selection  *****************/
/**********************************************************************/
#define _DIAG_CLASS_I_          0
#define _DIAG_CLASS_II_         1
#define _DIAG_CLASS_III_        2

#define _DIAG_CLASS_SUPPORT_    _DIAG_CLASS_III_

#define MAX_LENGTH_SERVICE 40

#if LIN_DEBUG_EN
#define MAX_QUEUE_SIZE 20
#else
#define MAX_QUEUE_SIZE 7
#endif


#define _DIAG_NUMBER_OF_SERVICES_    27

#define DIAGSRV_SESSION_CONTROL_ORDER    0

#define DIAGSRV_ECU_RESET_ORDER    1

#define DIAGSRV_FAULT_MEMORY_CLEAR_ORDER    2

#define DIAGSRV_FAULT_MEMORY_READ_ORDER    3

#define DIAGSRV_READ_DATA_BY_IDENTIFIER_ORDER    4

#define DIAGSRV_IO_CONTROL_BY_IDENTIFIER_ORDER    5

#define DIAGSRV_ASSIGN_NAD_ORDER    6

#define DIAGSRV_ASSIGN_FRAME_IDENTIFIER_ORDER    7

#define DIAGSRV_READ_BY_IDENTIFIER_ORDER    8

#define DIAGSRV_CONDITIONAL_CHANGE_NAD_ORDER    9

#define DIAGSRV_DATA_DUMP_ORDER    10

#define DIAGSRV_TARGET_RESET_ORDER    11

#define DIAGSRV_SAVE_CONFIGURATION_ORDER    12

#define DIAGSRV_ASSIGN_FRAME_ID_RANGE_ORDER    13

#define DIAGSRV_LED_GET_CONFIG_ORDER    14

#define DIAGSRV_LED_SET_CONFIG_ORDER    15

#define DIAGSRV_SOC_REG_RD_ORDER    16

#define DIAGSRV_SOC_REG_WR_ORDER    17


/**************** FRAME SUPPORT DEFINITION ******************/
#define _TL_SINGLE_FRAME_       0
#define _TL_MULTI_FRAME_        1

#define _TL_FRAME_SUPPORT_      _TL_MULTI_FRAME_

/* frame buffer size */
#define LIN_FRAME_BUF_SIZE			40
#define LIN_FLAG_BUF_SIZE			10

/**********************************************************************/
/***************               Interfaces           *******************/
/**********************************************************************/
typedef enum {
   LI0
}l_ifc_handle;

/**********************************************************************/
/***************               Signals              *******************/
/**********************************************************************/
/* Number of signals */
#define LIN_NUM_OF_SIGS  54
/* List of signals */
typedef enum {

   /* Interface_name = LI0 */

   LI0_EHIS_FL_ResponseError

   , LI0_EHIS_FL_FltSt
  
   , LI0_EHIS_FL_SwtSt
  
   , LI0_EHIS_FL_SW_MinorVersA
  
   , LI0_EHIS_FL_SW_MajorVersA
  
   , LI0_EHIS_FL_HW_PhaVers
  
   , LI0_EHIS_FL_HW_MajorVersB
  
   , LI0_EHIS_FL_HW_MinorVersB
  
   , LI0_EHIS_FL_SN_MajorVersB
  
   , LI0_EHIS_FL_SN_MinorVersB
  
   , LI0_EHIS_FL_SN_SupplierCod
  
   , LI0_EHIS_RL_ResponseError
  
   , LI0_EHIS_RL_FltSt
  
   , LI0_EHIS_RL_SwtSt
  
   , LI0_EHIS_RL_SW_MinorVersA
  
   , LI0_EHIS_RL_SW_MajorVersA
  
   , LI0_EHIS_RL_HW_PhaVers
  
   , LI0_EHIS_RL_HW_MajorVersB
  
   , LI0_EHIS_RL_HW_MinorVersB
  
   , LI0_EHIS_RL_SN_MajorVersB
  
   , LI0_EHIS_RL_SN_MinorVersB
  
   , LI0_EHIS_RL_SN_SupplierCod
  
   , LI0_EHIS_RR_ResponseError
  
   , LI0_EHIS_RR_FltSt
  
   , LI0_EHIS_RR_SwtSt
  
   , LI0_EHIS_RR_SW_MinorVersA
  
   , LI0_EHIS_RR_SW_MajorVersA
  
   , LI0_EHIS_RR_HW_PhaVers
  
   , LI0_EHIS_RR_HW_MajorVersB
  
   , LI0_EHIS_RR_HW_MinorVersB
  
   , LI0_EHIS_RR_SN_MajorVersB
  
   , LI0_EHIS_RR_SN_MinorVersB
  
   , LI0_EHIS_RR_SN_SupplierCod
  
   , LI0_EHIS_FR_ResponseError
  
   , LI0_EHIS_FR_FRtSt
  
   , LI0_EHIS_FR_SwtSt
  
   , LI0_EHIS_FR_SW_MinorVersA
  
   , LI0_EHIS_FR_SW_MajorVersA
  
   , LI0_EHIS_FR_HW_PhaVers
  
   , LI0_EHIS_FR_HW_MajorVersB
  
   , LI0_EHIS_FR_HW_MinorVersB
  
   , LI0_EHIS_FR_SN_MajorVersB
  
   , LI0_EHIS_FR_SN_MinorVersB
  
   , LI0_EHIS_FR_SN_SupplierCod
  
   , LI0_bit2_5
  
   , LI0_bit6
  
   , LI0_bit7
  
   , LI0_bit8_9
  
   , LI0_bit10
  
   , LI0_bit11
  
   , LI0_bit12
  
   , LI0_bit13_20
  
   , LI0_bit21_23
  
   , LI0_bit24_25
  
} l_signal_handle;
/**********************************************************************/
/*****************               Frame             ********************/
/**********************************************************************/
/* Number of frames */
#define LIN_NUM_OF_FRMS  7
/* List of frames */
typedef enum {
/* All frames for master node */

   /* Interface_name = LI0 */

   LI0_EHIS_FL_State

   , LI0_EHIS_FR_State
  
   , LI0_EHIS_RL_State
  
   , LI0_EHIS_RR_State
  
   , LI0_VIU_DWS
  
   , LI0_MasterReq
  
   , LI0_SlaveResp
  
} l_frame_handle;
/**********************************************************************/
/***************             Configuration          *******************/
/**********************************************************************/
/* Size of configuration in ROM and RAM used for interface: LI1 */
#define LIN_SIZE_OF_CFG  9
#define LIN_CFG_FRAME_NUM  5
/*********************************************************************
 * global macros
 *********************************************************************/
#define l_bool_rd(SIGNAL) l_bool_rd_##SIGNAL()
#define l_bool_wr(SIGNAL, A) l_bool_wr_##SIGNAL(A)
#define l_u8_rd(SIGNAL) l_u8_rd_##SIGNAL()
#define l_u8_wr(SIGNAL, A) l_u8_wr_##SIGNAL(A)
#define l_u16_rd(SIGNAL) l_u16_rd_##SIGNAL()
#define l_u16_wr(SIGNAL, A) l_u16_wr_##SIGNAL(A)
#define l_bytes_rd(SIGNAL, start, count, data)  l_bytes_rd_##SIGNAL(start, count, data)
#define l_bytes_wr(SIGNAL, start, count, data) l_bytes_wr_##SIGNAL(start, count, data)
#define l_flg_tst(FLAG) l_flg_tst_##FLAG()
#define l_flg_clr(FLAG) l_flg_clr_##FLAG()
#define LIN_TEST_BIT(A,B) ((l_bool)((((A) & (1U << (B))) != 0U) ? 1U : 0U))
#define LIN_SET_BIT(A,B)                      ((A) |= (l_u8) (1U << (B)))
#define LIN_CLEAR_BIT(A,B)               ((A) &= ((l_u8) (~(1U << (B)))))
#define LIN_BYTE_MASK  ((l_u16)(((l_u16)((l_u16)1 << CHAR_BIT)) - (l_u16)1))
#define LIN_FRAME_LEN_MAX                                             10U

/* Returns the low byte of the 32-bit value    */
#define BYTE_0(n)                              ((l_u8)((n) & (l_u8)0xFF))
/* Returns the second byte of the 32-bit value */
#define BYTE_1(n)                        ((l_u8)(BYTE_0((n) >> (l_u8)8)))
/* Returns the third byte of the 32-bit value  */
#define BYTE_2(n)                       ((l_u8)(BYTE_0((n) >> (l_u8)16)))
/* Returns high byte of the 32-bit value       */
#define BYTE_3(n)                       ((l_u8)(BYTE_0((n) >> (l_u8)24)))

/*
 * defines for signal access
 */



#define LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError    3U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_ResponseError    7U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_ResponseError    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_ResponseError    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_ResponseError    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_FltSt    4U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_FltSt    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_FltSt    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_FltSt    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_FltSt    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_SwtSt    4U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_SwtSt    1U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_SwtSt    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SwtSt    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SwtSt    2U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA    4U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_SW_MinorVersA    3U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_SW_MinorVersA    7U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MinorVersA    3U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA    5U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_SW_MajorVersA    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_SW_MajorVersA    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MajorVersA    4U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers    5U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_HW_PhaVers    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_HW_PhaVers    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_PhaVers    5U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB    6U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_HW_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_HW_MajorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MajorVersB    6U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB    6U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_HW_MinorVersB    4U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_HW_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB    0U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MinorVersB    7U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB    7U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_SN_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_SN_MajorVersB    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB    1U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MajorVersB    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB    7U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_SN_MinorVersB    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_SN_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB    1U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MinorVersB    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod    7U
#define LIN_BIT_OFFSET_LI0_EHIS_FL_SN_SupplierCod    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FL_SN_SupplierCod    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod    1U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_SupplierCod    2U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_ResponseError    19U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_ResponseError    7U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_ResponseError    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_ResponseError    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_ResponseError    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_FltSt    20U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_FltSt    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_FltSt    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_FltSt    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_FltSt    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_SwtSt    20U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_SwtSt    1U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_SwtSt    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SwtSt    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SwtSt    2U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA    20U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_SW_MinorVersA    3U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_SW_MinorVersA    7U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MinorVersA    3U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA    21U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_SW_MajorVersA    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_SW_MajorVersA    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MajorVersA    4U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers    21U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_HW_PhaVers    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_HW_PhaVers    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_PhaVers    5U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB    22U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_HW_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_HW_MajorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MajorVersB    6U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB    22U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_HW_MinorVersB    4U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_HW_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB    4U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MinorVersB    7U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB    23U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_SN_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_SN_MajorVersB    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB    5U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MajorVersB    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB    23U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_SN_MinorVersB    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_SN_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB    5U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MinorVersB    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod    23U
#define LIN_BIT_OFFSET_LI0_EHIS_RL_SN_SupplierCod    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RL_SN_SupplierCod    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod    5U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_SupplierCod    2U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError    27U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_ResponseError    7U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_ResponseError    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_ResponseError    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_ResponseError    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_FltSt    28U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_FltSt    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_FltSt    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_FltSt    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_FltSt    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_SwtSt    28U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_SwtSt    1U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_SwtSt    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SwtSt    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SwtSt    2U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA    28U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_SW_MinorVersA    3U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_SW_MinorVersA    7U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MinorVersA    3U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA    29U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_SW_MajorVersA    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_SW_MajorVersA    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MajorVersA    4U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers    29U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_HW_PhaVers    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_HW_PhaVers    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_PhaVers    5U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB    30U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_HW_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_HW_MajorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MajorVersB    6U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB    30U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_HW_MinorVersB    4U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_HW_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB    6U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MinorVersB    7U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB    31U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_SN_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_SN_MajorVersB    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB    7U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MajorVersB    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB    31U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_SN_MinorVersB    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_SN_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB    7U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MinorVersB    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod    31U
#define LIN_BIT_OFFSET_LI0_EHIS_RR_SN_SupplierCod    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_RR_SN_SupplierCod    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod    7U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_SupplierCod    2U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_ResponseError    11U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_ResponseError    7U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_ResponseError    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_ResponseError    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_ResponseError    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_FRtSt    12U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_FRtSt    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_FRtSt    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_FRtSt    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_FRtSt    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_SwtSt    12U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_SwtSt    1U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_SwtSt    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SwtSt    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SwtSt    2U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA    12U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_SW_MinorVersA    3U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_SW_MinorVersA    7U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MinorVersA    3U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA    13U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_SW_MajorVersA    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_SW_MajorVersA    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MajorVersA    4U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers    13U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_HW_PhaVers    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_HW_PhaVers    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_PhaVers    5U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB    14U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_HW_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_HW_MajorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MajorVersB    6U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB    14U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_HW_MinorVersB    4U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_HW_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB    2U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MinorVersB    7U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB    15U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_SN_MajorVersB    0U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_SN_MajorVersB    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB    3U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MajorVersB    0U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB    15U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_SN_MinorVersB    2U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_SN_MinorVersB    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB    3U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MinorVersB    1U


#define LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod    15U
#define LIN_BIT_OFFSET_LI0_EHIS_FR_SN_SupplierCod    6U
#define LIN_SIGNAL_SIZE_LI0_EHIS_FR_SN_SupplierCod    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod    3U
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_SupplierCod    2U


#define LIN_BYTE_OFFSET_LI0_bit2_5    32U
#define LIN_BIT_OFFSET_LI0_bit2_5    2U
#define LIN_SIGNAL_SIZE_LI0_bit2_5    4U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit2_5    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit2_5    0U


#define LIN_BYTE_OFFSET_LI0_bit6    32U
#define LIN_BIT_OFFSET_LI0_bit6    6U
#define LIN_SIGNAL_SIZE_LI0_bit6    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit6    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit6    1U


#define LIN_BYTE_OFFSET_LI0_bit7    32U
#define LIN_BIT_OFFSET_LI0_bit7    7U
#define LIN_SIGNAL_SIZE_LI0_bit7    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit7    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit7    2U


#define LIN_BYTE_OFFSET_LI0_bit8_9    33U
#define LIN_BIT_OFFSET_LI0_bit8_9    0U
#define LIN_SIGNAL_SIZE_LI0_bit8_9    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit8_9    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit8_9    3U


#define LIN_BYTE_OFFSET_LI0_bit10    33U
#define LIN_BIT_OFFSET_LI0_bit10    2U
#define LIN_SIGNAL_SIZE_LI0_bit10    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit10    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit10    4U


#define LIN_BYTE_OFFSET_LI0_bit11    33U
#define LIN_BIT_OFFSET_LI0_bit11    3U
#define LIN_SIGNAL_SIZE_LI0_bit11    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit11    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit11    5U


#define LIN_BYTE_OFFSET_LI0_bit12    33U
#define LIN_BIT_OFFSET_LI0_bit12    4U
#define LIN_SIGNAL_SIZE_LI0_bit12    1U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit12    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit12    6U


#define LIN_BYTE_OFFSET_LI0_bit13_20    33U
#define LIN_BIT_OFFSET_LI0_bit13_20    5U
#define LIN_SIGNAL_SIZE_LI0_bit13_20    8U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit13_20    8U
#define LIN_FLAG_BIT_OFFSET_LI0_bit13_20    7U


#define LIN_BYTE_OFFSET_LI0_bit21_23    34U
#define LIN_BIT_OFFSET_LI0_bit21_23    5U
#define LIN_SIGNAL_SIZE_LI0_bit21_23    3U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit21_23    9U
#define LIN_FLAG_BIT_OFFSET_LI0_bit21_23    0U


#define LIN_BYTE_OFFSET_LI0_bit24_25    35U
#define LIN_BIT_OFFSET_LI0_bit24_25    0U
#define LIN_SIGNAL_SIZE_LI0_bit24_25    2U
#define LIN_FLAG_BYTE_OFFSET_LI0_bit24_25    9U
#define LIN_FLAG_BIT_OFFSET_LI0_bit24_25    1U




#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_State             0
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_State              0

#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_State             2
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_State              0

#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_State             4
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_State              0

#define LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_State             6
#define LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_State              0

#define LIN_FLAG_BYTE_OFFSET_LI0_VIU_DWS             8
#define LIN_FLAG_BIT_OFFSET_LI0_VIU_DWS              0


/**********************************************************************/
/***************        Static API Functions        *******************/
/**********************************************************************/
/*
 * the static signal access macros
 */


/* static access macros for signal LI0_EHIS_FL_ResponseError */

 
#define l_bool_rd_LI0_EHIS_FL_ResponseError() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_FL_ResponseError))

#define l_bool_wr_LI0_EHIS_FL_ResponseError(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_FL_ResponseError)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_FL_ResponseError));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_ResponseError);}
/* static access macros for signal LI0_EHIS_FL_FltSt */

 
#define l_bool_rd_LI0_EHIS_FL_FltSt() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_FL_FltSt))

#define l_bool_wr_LI0_EHIS_FL_FltSt(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_FL_FltSt)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_FL_FltSt));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_FltSt);}
 
/* static access macros for signal LI0_EHIS_FL_SwtSt */
 
#define l_u8_rd_LI0_EHIS_FL_SwtSt() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SwtSt]) >> 1U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FL_SwtSt(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SwtSt] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SwtSt] & 0xf9U) | \
    (((A) << 1U) & 0x06U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SwtSt); \
    }


 
/* static access macros for signal LI0_EHIS_FL_SW_MinorVersA */
 
#define l_u8_rd_LI0_EHIS_FL_SW_MinorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA] + (lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA + 1U] << 8U)) >> 3U) & 0x7fU))


#define l_u8_wr_LI0_EHIS_FL_SW_MinorVersA(A) \
    { \
    buffer_backup_data[4U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_FL_State] |= (1U << 4); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA] & 0x07U) | \
    (((A) << 3U) & 0xf8U)); \
    buffer_backup_data[4U + 1U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA + 1U]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_FL_State] |= (1U << (4 + 1U)); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA + 1U] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA + 1U] & 0xfcU) | \
    (((A) >> 5U) & 0x03U)); \
    lin_frame_updating_flag_tbl[LI0_EHIS_FL_State] &= (~(0x03 << 4)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MinorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_FL_SW_MajorVersA */
 
#define l_u8_rd_LI0_EHIS_FL_SW_MajorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FL_SW_MajorVersA(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MajorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_FL_HW_PhaVers */
 
#define l_u8_rd_LI0_EHIS_FL_HW_PhaVers() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FL_HW_PhaVers(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_PhaVers); \
    }


 
/* static access macros for signal LI0_EHIS_FL_HW_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_FL_HW_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB]) >> 0U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FL_HW_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB] & 0xf0U) | \
    (((A) << 0U) & 0x0fU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FL_HW_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_FL_HW_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB]) >> 4U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FL_HW_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB] & 0x0fU) | \
    (((A) << 4U) & 0xf0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FL_SN_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_FL_SN_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB]) >> 0U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FL_SN_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB] & 0xfcU) | \
    (((A) << 0U) & 0x03U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FL_SN_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_FL_SN_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FL_SN_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FL_SN_SupplierCod */
 
#define l_u8_rd_LI0_EHIS_FL_SN_SupplierCod() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FL_SN_SupplierCod(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_SupplierCod); \
    }


/* static access macros for signal LI0_EHIS_RL_ResponseError */

 
#define l_bool_rd_LI0_EHIS_RL_ResponseError() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_RL_ResponseError))

#define l_bool_wr_LI0_EHIS_RL_ResponseError(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_RL_ResponseError)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_RL_ResponseError));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_ResponseError);}
/* static access macros for signal LI0_EHIS_RL_FltSt */

 
#define l_bool_rd_LI0_EHIS_RL_FltSt() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_RL_FltSt))

#define l_bool_wr_LI0_EHIS_RL_FltSt(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_RL_FltSt)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_RL_FltSt));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_FltSt);}
 
/* static access macros for signal LI0_EHIS_RL_SwtSt */
 
#define l_u8_rd_LI0_EHIS_RL_SwtSt() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SwtSt]) >> 1U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RL_SwtSt(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SwtSt] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SwtSt] & 0xf9U) | \
    (((A) << 1U) & 0x06U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SwtSt); \
    }


 
/* static access macros for signal LI0_EHIS_RL_SW_MinorVersA */
 
#define l_u8_rd_LI0_EHIS_RL_SW_MinorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA] + (lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA + 1U] << 8U)) >> 3U) & 0x7fU))


#define l_u8_wr_LI0_EHIS_RL_SW_MinorVersA(A) \
    { \
    buffer_backup_data[4U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_RL_State] |= (1U << 4); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA] & 0x07U) | \
    (((A) << 3U) & 0xf8U)); \
    buffer_backup_data[4U + 1U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA + 1U]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_RL_State] |= (1U << (4 + 1U)); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA + 1U] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA + 1U] & 0xfcU) | \
    (((A) >> 5U) & 0x03U)); \
    lin_frame_updating_flag_tbl[LI0_EHIS_RL_State] &= (~(0x03 << 4)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MinorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_RL_SW_MajorVersA */
 
#define l_u8_rd_LI0_EHIS_RL_SW_MajorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RL_SW_MajorVersA(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MajorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_RL_HW_PhaVers */
 
#define l_u8_rd_LI0_EHIS_RL_HW_PhaVers() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RL_HW_PhaVers(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_PhaVers); \
    }


 
/* static access macros for signal LI0_EHIS_RL_HW_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_RL_HW_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB]) >> 0U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RL_HW_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB] & 0xf0U) | \
    (((A) << 0U) & 0x0fU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RL_HW_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_RL_HW_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB]) >> 4U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RL_HW_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB] & 0x0fU) | \
    (((A) << 4U) & 0xf0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RL_SN_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_RL_SN_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB]) >> 0U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RL_SN_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB] & 0xfcU) | \
    (((A) << 0U) & 0x03U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RL_SN_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_RL_SN_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RL_SN_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RL_SN_SupplierCod */
 
#define l_u8_rd_LI0_EHIS_RL_SN_SupplierCod() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RL_SN_SupplierCod(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_SupplierCod); \
    }


/* static access macros for signal LI0_EHIS_RR_ResponseError */

 
#define l_bool_rd_LI0_EHIS_RR_ResponseError() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_RR_ResponseError))

#define l_bool_wr_LI0_EHIS_RR_ResponseError(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_RR_ResponseError)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_RR_ResponseError));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_ResponseError);}
/* static access macros for signal LI0_EHIS_RR_FltSt */

 
#define l_bool_rd_LI0_EHIS_RR_FltSt() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_RR_FltSt))

#define l_bool_wr_LI0_EHIS_RR_FltSt(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_RR_FltSt)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_FltSt], \
    LIN_BIT_OFFSET_LI0_EHIS_RR_FltSt));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_FltSt);}
 
/* static access macros for signal LI0_EHIS_RR_SwtSt */
 
#define l_u8_rd_LI0_EHIS_RR_SwtSt() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SwtSt]) >> 1U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RR_SwtSt(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SwtSt] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SwtSt] & 0xf9U) | \
    (((A) << 1U) & 0x06U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SwtSt); \
    }


 
/* static access macros for signal LI0_EHIS_RR_SW_MinorVersA */
 
#define l_u8_rd_LI0_EHIS_RR_SW_MinorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA] + (lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA + 1U] << 8U)) >> 3U) & 0x7fU))


#define l_u8_wr_LI0_EHIS_RR_SW_MinorVersA(A) \
    { \
    buffer_backup_data[4U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_RR_State] |= (1U << 4); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA] & 0x07U) | \
    (((A) << 3U) & 0xf8U)); \
    buffer_backup_data[4U + 1U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA + 1U]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_RR_State] |= (1U << (4 + 1U)); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA + 1U] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA + 1U] & 0xfcU) | \
    (((A) >> 5U) & 0x03U)); \
    lin_frame_updating_flag_tbl[LI0_EHIS_RR_State] &= (~(0x03 << 4)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MinorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_RR_SW_MajorVersA */
 
#define l_u8_rd_LI0_EHIS_RR_SW_MajorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RR_SW_MajorVersA(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MajorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_RR_HW_PhaVers */
 
#define l_u8_rd_LI0_EHIS_RR_HW_PhaVers() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RR_HW_PhaVers(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_PhaVers); \
    }


 
/* static access macros for signal LI0_EHIS_RR_HW_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_RR_HW_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB]) >> 0U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RR_HW_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB] & 0xf0U) | \
    (((A) << 0U) & 0x0fU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RR_HW_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_RR_HW_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB]) >> 4U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RR_HW_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB] & 0x0fU) | \
    (((A) << 4U) & 0xf0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RR_SN_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_RR_SN_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB]) >> 0U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RR_SN_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB] & 0xfcU) | \
    (((A) << 0U) & 0x03U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RR_SN_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_RR_SN_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_RR_SN_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_RR_SN_SupplierCod */
 
#define l_u8_rd_LI0_EHIS_RR_SN_SupplierCod() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_RR_SN_SupplierCod(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_SupplierCod); \
    }


/* static access macros for signal LI0_EHIS_FR_ResponseError */

 
#define l_bool_rd_LI0_EHIS_FR_ResponseError() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_FR_ResponseError))

#define l_bool_wr_LI0_EHIS_FR_ResponseError(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_FR_ResponseError)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_ResponseError], \
    LIN_BIT_OFFSET_LI0_EHIS_FR_ResponseError));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_ResponseError);}
/* static access macros for signal LI0_EHIS_FR_FRtSt */

 
#define l_bool_rd_LI0_EHIS_FR_FRtSt() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_FRtSt], \
    LIN_BIT_OFFSET_LI0_EHIS_FR_FRtSt))

#define l_bool_wr_LI0_EHIS_FR_FRtSt(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_FRtSt], \
    LIN_BIT_OFFSET_LI0_EHIS_FR_FRtSt)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_FRtSt], \
    LIN_BIT_OFFSET_LI0_EHIS_FR_FRtSt));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_FRtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_FRtSt);}
 
/* static access macros for signal LI0_EHIS_FR_SwtSt */
 
#define l_u8_rd_LI0_EHIS_FR_SwtSt() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SwtSt]) >> 1U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FR_SwtSt(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SwtSt] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SwtSt] & 0xf9U) | \
    (((A) << 1U) & 0x06U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SwtSt); \
    }


 
/* static access macros for signal LI0_EHIS_FR_SW_MinorVersA */
 
#define l_u8_rd_LI0_EHIS_FR_SW_MinorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA] + (lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA + 1U] << 8U)) >> 3U) & 0x7fU))


#define l_u8_wr_LI0_EHIS_FR_SW_MinorVersA(A) \
    { \
    buffer_backup_data[4U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_FR_State] |= (1U << 4); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA] & 0x07U) | \
    (((A) << 3U) & 0xf8U)); \
    buffer_backup_data[4U + 1U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA + 1U]; \
    lin_frame_updating_flag_tbl[LI0_EHIS_FR_State] |= (1U << (4 + 1U)); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA + 1U] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA + 1U] & 0xfcU) | \
    (((A) >> 5U) & 0x03U)); \
    lin_frame_updating_flag_tbl[LI0_EHIS_FR_State] &= (~(0x03 << 4)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MinorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_FR_SW_MajorVersA */
 
#define l_u8_rd_LI0_EHIS_FR_SW_MajorVersA() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FR_SW_MajorVersA(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MajorVersA); \
    }


 
/* static access macros for signal LI0_EHIS_FR_HW_PhaVers */
 
#define l_u8_rd_LI0_EHIS_FR_HW_PhaVers() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FR_HW_PhaVers(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_PhaVers); \
    }


 
/* static access macros for signal LI0_EHIS_FR_HW_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_FR_HW_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB]) >> 0U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FR_HW_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB] & 0xf0U) | \
    (((A) << 0U) & 0x0fU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FR_HW_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_FR_HW_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB]) >> 4U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FR_HW_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB] & 0x0fU) | \
    (((A) << 4U) & 0xf0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FR_SN_MajorVersB */
 
#define l_u8_rd_LI0_EHIS_FR_SN_MajorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB]) >> 0U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FR_SN_MajorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB] & 0xfcU) | \
    (((A) << 0U) & 0x03U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MajorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FR_SN_MinorVersB */
 
#define l_u8_rd_LI0_EHIS_FR_SN_MinorVersB() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_EHIS_FR_SN_MinorVersB(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MinorVersB); \
    }


 
/* static access macros for signal LI0_EHIS_FR_SN_SupplierCod */
 
#define l_u8_rd_LI0_EHIS_FR_SN_SupplierCod() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod]) >> 6U) & 0x03U))


#define l_u8_wr_LI0_EHIS_FR_SN_SupplierCod(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod] & 0x3fU) | \
    (((A) << 6U) & 0xc0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_SupplierCod); \
    }


 
/* static access macros for signal LI0_bit2_5 */
 
#define l_u8_rd_LI0_bit2_5() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit2_5]) >> 2U) & 0x0fU))


#define l_u8_wr_LI0_bit2_5(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit2_5] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit2_5] & 0xc3U) | \
    (((A) << 2U) & 0x3cU)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit2_5],\
         LIN_FLAG_BIT_OFFSET_LI0_bit2_5); \
    }


/* static access macros for signal LI0_bit6 */

 
#define l_bool_rd_LI0_bit6() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit6], \
    LIN_BIT_OFFSET_LI0_bit6))

#define l_bool_wr_LI0_bit6(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit6], \
    LIN_BIT_OFFSET_LI0_bit6)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit6], \
    LIN_BIT_OFFSET_LI0_bit6));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit6],\
         LIN_FLAG_BIT_OFFSET_LI0_bit6);}
/* static access macros for signal LI0_bit7 */

 
#define l_bool_rd_LI0_bit7() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit7], \
    LIN_BIT_OFFSET_LI0_bit7))

#define l_bool_wr_LI0_bit7(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit7], \
    LIN_BIT_OFFSET_LI0_bit7)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit7], \
    LIN_BIT_OFFSET_LI0_bit7));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit7],\
         LIN_FLAG_BIT_OFFSET_LI0_bit7);}
 
/* static access macros for signal LI0_bit8_9 */
 
#define l_u8_rd_LI0_bit8_9() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit8_9]) >> 0U) & 0x03U))


#define l_u8_wr_LI0_bit8_9(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit8_9] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit8_9] & 0xfcU) | \
    (((A) << 0U) & 0x03U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit8_9],\
         LIN_FLAG_BIT_OFFSET_LI0_bit8_9); \
    }


/* static access macros for signal LI0_bit10 */

 
#define l_bool_rd_LI0_bit10() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit10], \
    LIN_BIT_OFFSET_LI0_bit10))

#define l_bool_wr_LI0_bit10(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit10], \
    LIN_BIT_OFFSET_LI0_bit10)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit10], \
    LIN_BIT_OFFSET_LI0_bit10));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit10],\
         LIN_FLAG_BIT_OFFSET_LI0_bit10);}
/* static access macros for signal LI0_bit11 */

 
#define l_bool_rd_LI0_bit11() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit11], \
    LIN_BIT_OFFSET_LI0_bit11))

#define l_bool_wr_LI0_bit11(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit11], \
    LIN_BIT_OFFSET_LI0_bit11)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit11], \
    LIN_BIT_OFFSET_LI0_bit11));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit11],\
         LIN_FLAG_BIT_OFFSET_LI0_bit11);}
/* static access macros for signal LI0_bit12 */

 
#define l_bool_rd_LI0_bit12() \
    (LIN_TEST_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit12], \
    LIN_BIT_OFFSET_LI0_bit12))

#define l_bool_wr_LI0_bit12(A) \
    {(A) ? \
    (LIN_SET_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit12], \
    LIN_BIT_OFFSET_LI0_bit12)):\
    (LIN_CLEAR_BIT(lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit12], \
    LIN_BIT_OFFSET_LI0_bit12));\
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit12],\
         LIN_FLAG_BIT_OFFSET_LI0_bit12);}
 
/* static access macros for signal LI0_bit13_20 */
 
#define l_u8_rd_LI0_bit13_20() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20] + (lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20 + 1U] << 8U)) >> 5U) & 0xffU))


#define l_u8_wr_LI0_bit13_20(A) \
    { \
    buffer_backup_data[1U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20]; \
    lin_frame_updating_flag_tbl[LI0_VIU_DWS] |= (1U << 1); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20] & 0x1fU) | \
    (((A) << 5U) & 0xe0U)); \
    buffer_backup_data[1U + 1U] =  lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20 + 1U]; \
    lin_frame_updating_flag_tbl[LI0_VIU_DWS] |= (1U << (1 + 1U)); \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20 + 1U] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit13_20 + 1U] & 0xe0U) | \
    (((A) >> 3U) & 0x1fU)); \
    lin_frame_updating_flag_tbl[LI0_VIU_DWS] &= (~(0x03 << 1)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit13_20],\
         LIN_FLAG_BIT_OFFSET_LI0_bit13_20); \
    }


 
/* static access macros for signal LI0_bit21_23 */
 
#define l_u8_rd_LI0_bit21_23() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit21_23]) >> 5U) & 0x07U))


#define l_u8_wr_LI0_bit21_23(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit21_23] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit21_23] & 0x1fU) | \
    (((A) << 5U) & 0xe0U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit21_23],\
         LIN_FLAG_BIT_OFFSET_LI0_bit21_23); \
    }


 
/* static access macros for signal LI0_bit24_25 */
 
#define l_u8_rd_LI0_bit24_25() \
    ((l_u8)  (((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit24_25]) >> 0U) & 0x03U))


#define l_u8_wr_LI0_bit24_25(A) \
    { \
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit24_25] = \
    (l_u8)((lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_bit24_25] & 0xfcU) | \
    (((A) << 0U) & 0x03U)); \
    LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit24_25],\
         LIN_FLAG_BIT_OFFSET_LI0_bit24_25); \
    }




/* Signal flag APIs */

#define l_flg_tst_LI0_EHIS_FL_ResponseError_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_ResponseError)
#define l_flg_clr_LI0_EHIS_FL_ResponseError_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_ResponseError)

#define l_flg_tst_LI0_EHIS_FL_FltSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_FltSt)
#define l_flg_clr_LI0_EHIS_FL_FltSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_FltSt)

#define l_flg_tst_LI0_EHIS_FL_SwtSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SwtSt)
#define l_flg_clr_LI0_EHIS_FL_SwtSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SwtSt)

#define l_flg_tst_LI0_EHIS_FL_SW_MinorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MinorVersA)
#define l_flg_clr_LI0_EHIS_FL_SW_MinorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MinorVersA)

#define l_flg_tst_LI0_EHIS_FL_SW_MajorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MajorVersA)
#define l_flg_clr_LI0_EHIS_FL_SW_MajorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SW_MajorVersA)

#define l_flg_tst_LI0_EHIS_FL_HW_PhaVers_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_PhaVers)
#define l_flg_clr_LI0_EHIS_FL_HW_PhaVers_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_PhaVers)

#define l_flg_tst_LI0_EHIS_FL_HW_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MajorVersB)
#define l_flg_clr_LI0_EHIS_FL_HW_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MajorVersB)

#define l_flg_tst_LI0_EHIS_FL_HW_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MinorVersB)
#define l_flg_clr_LI0_EHIS_FL_HW_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_HW_MinorVersB)

#define l_flg_tst_LI0_EHIS_FL_SN_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MajorVersB)
#define l_flg_clr_LI0_EHIS_FL_SN_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MajorVersB)

#define l_flg_tst_LI0_EHIS_FL_SN_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MinorVersB)
#define l_flg_clr_LI0_EHIS_FL_SN_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_MinorVersB)

#define l_flg_tst_LI0_EHIS_FL_SN_SupplierCod_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_SupplierCod)
#define l_flg_clr_LI0_EHIS_FL_SN_SupplierCod_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FL_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FL_SN_SupplierCod)

#define l_flg_tst_LI0_EHIS_RL_ResponseError_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_ResponseError)
#define l_flg_clr_LI0_EHIS_RL_ResponseError_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_ResponseError)

#define l_flg_tst_LI0_EHIS_RL_FltSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_FltSt)
#define l_flg_clr_LI0_EHIS_RL_FltSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_FltSt)

#define l_flg_tst_LI0_EHIS_RL_SwtSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SwtSt)
#define l_flg_clr_LI0_EHIS_RL_SwtSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SwtSt)

#define l_flg_tst_LI0_EHIS_RL_SW_MinorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MinorVersA)
#define l_flg_clr_LI0_EHIS_RL_SW_MinorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MinorVersA)

#define l_flg_tst_LI0_EHIS_RL_SW_MajorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MajorVersA)
#define l_flg_clr_LI0_EHIS_RL_SW_MajorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SW_MajorVersA)

#define l_flg_tst_LI0_EHIS_RL_HW_PhaVers_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_PhaVers)
#define l_flg_clr_LI0_EHIS_RL_HW_PhaVers_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_PhaVers)

#define l_flg_tst_LI0_EHIS_RL_HW_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MajorVersB)
#define l_flg_clr_LI0_EHIS_RL_HW_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MajorVersB)

#define l_flg_tst_LI0_EHIS_RL_HW_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MinorVersB)
#define l_flg_clr_LI0_EHIS_RL_HW_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_HW_MinorVersB)

#define l_flg_tst_LI0_EHIS_RL_SN_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MajorVersB)
#define l_flg_clr_LI0_EHIS_RL_SN_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MajorVersB)

#define l_flg_tst_LI0_EHIS_RL_SN_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MinorVersB)
#define l_flg_clr_LI0_EHIS_RL_SN_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_MinorVersB)

#define l_flg_tst_LI0_EHIS_RL_SN_SupplierCod_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_SupplierCod)
#define l_flg_clr_LI0_EHIS_RL_SN_SupplierCod_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RL_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RL_SN_SupplierCod)

#define l_flg_tst_LI0_EHIS_RR_ResponseError_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_ResponseError)
#define l_flg_clr_LI0_EHIS_RR_ResponseError_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_ResponseError)

#define l_flg_tst_LI0_EHIS_RR_FltSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_FltSt)
#define l_flg_clr_LI0_EHIS_RR_FltSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_FltSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_FltSt)

#define l_flg_tst_LI0_EHIS_RR_SwtSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SwtSt)
#define l_flg_clr_LI0_EHIS_RR_SwtSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SwtSt)

#define l_flg_tst_LI0_EHIS_RR_SW_MinorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MinorVersA)
#define l_flg_clr_LI0_EHIS_RR_SW_MinorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MinorVersA)

#define l_flg_tst_LI0_EHIS_RR_SW_MajorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MajorVersA)
#define l_flg_clr_LI0_EHIS_RR_SW_MajorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SW_MajorVersA)

#define l_flg_tst_LI0_EHIS_RR_HW_PhaVers_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_PhaVers)
#define l_flg_clr_LI0_EHIS_RR_HW_PhaVers_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_PhaVers)

#define l_flg_tst_LI0_EHIS_RR_HW_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MajorVersB)
#define l_flg_clr_LI0_EHIS_RR_HW_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MajorVersB)

#define l_flg_tst_LI0_EHIS_RR_HW_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MinorVersB)
#define l_flg_clr_LI0_EHIS_RR_HW_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_HW_MinorVersB)

#define l_flg_tst_LI0_EHIS_RR_SN_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MajorVersB)
#define l_flg_clr_LI0_EHIS_RR_SN_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MajorVersB)

#define l_flg_tst_LI0_EHIS_RR_SN_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MinorVersB)
#define l_flg_clr_LI0_EHIS_RR_SN_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_MinorVersB)

#define l_flg_tst_LI0_EHIS_RR_SN_SupplierCod_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_SupplierCod)
#define l_flg_clr_LI0_EHIS_RR_SN_SupplierCod_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_RR_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_RR_SN_SupplierCod)

#define l_flg_tst_LI0_EHIS_FR_ResponseError_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_ResponseError)
#define l_flg_clr_LI0_EHIS_FR_ResponseError_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_ResponseError],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_ResponseError)

#define l_flg_tst_LI0_EHIS_FR_FRtSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_FRtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_FRtSt)
#define l_flg_clr_LI0_EHIS_FR_FRtSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_FRtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_FRtSt)

#define l_flg_tst_LI0_EHIS_FR_SwtSt_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SwtSt)
#define l_flg_clr_LI0_EHIS_FR_SwtSt_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SwtSt],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SwtSt)

#define l_flg_tst_LI0_EHIS_FR_SW_MinorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MinorVersA)
#define l_flg_clr_LI0_EHIS_FR_SW_MinorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MinorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MinorVersA)

#define l_flg_tst_LI0_EHIS_FR_SW_MajorVersA_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MajorVersA)
#define l_flg_clr_LI0_EHIS_FR_SW_MajorVersA_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SW_MajorVersA],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SW_MajorVersA)

#define l_flg_tst_LI0_EHIS_FR_HW_PhaVers_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_PhaVers)
#define l_flg_clr_LI0_EHIS_FR_HW_PhaVers_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_PhaVers],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_PhaVers)

#define l_flg_tst_LI0_EHIS_FR_HW_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MajorVersB)
#define l_flg_clr_LI0_EHIS_FR_HW_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MajorVersB)

#define l_flg_tst_LI0_EHIS_FR_HW_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MinorVersB)
#define l_flg_clr_LI0_EHIS_FR_HW_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_HW_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_HW_MinorVersB)

#define l_flg_tst_LI0_EHIS_FR_SN_MajorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MajorVersB)
#define l_flg_clr_LI0_EHIS_FR_SN_MajorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MajorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MajorVersB)

#define l_flg_tst_LI0_EHIS_FR_SN_MinorVersB_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MinorVersB)
#define l_flg_clr_LI0_EHIS_FR_SN_MinorVersB_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_MinorVersB],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_MinorVersB)

#define l_flg_tst_LI0_EHIS_FR_SN_SupplierCod_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_SupplierCod)
#define l_flg_clr_LI0_EHIS_FR_SN_SupplierCod_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_EHIS_FR_SN_SupplierCod],\
         LIN_FLAG_BIT_OFFSET_LI0_EHIS_FR_SN_SupplierCod)

#define l_flg_tst_LI0_bit2_5_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit2_5],\
         LIN_FLAG_BIT_OFFSET_LI0_bit2_5)
#define l_flg_clr_LI0_bit2_5_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit2_5],\
         LIN_FLAG_BIT_OFFSET_LI0_bit2_5)

#define l_flg_tst_LI0_bit6_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit6],\
         LIN_FLAG_BIT_OFFSET_LI0_bit6)
#define l_flg_clr_LI0_bit6_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit6],\
         LIN_FLAG_BIT_OFFSET_LI0_bit6)

#define l_flg_tst_LI0_bit7_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit7],\
         LIN_FLAG_BIT_OFFSET_LI0_bit7)
#define l_flg_clr_LI0_bit7_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit7],\
         LIN_FLAG_BIT_OFFSET_LI0_bit7)

#define l_flg_tst_LI0_bit8_9_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit8_9],\
         LIN_FLAG_BIT_OFFSET_LI0_bit8_9)
#define l_flg_clr_LI0_bit8_9_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit8_9],\
         LIN_FLAG_BIT_OFFSET_LI0_bit8_9)

#define l_flg_tst_LI0_bit10_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit10],\
         LIN_FLAG_BIT_OFFSET_LI0_bit10)
#define l_flg_clr_LI0_bit10_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit10],\
         LIN_FLAG_BIT_OFFSET_LI0_bit10)

#define l_flg_tst_LI0_bit11_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit11],\
         LIN_FLAG_BIT_OFFSET_LI0_bit11)
#define l_flg_clr_LI0_bit11_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit11],\
         LIN_FLAG_BIT_OFFSET_LI0_bit11)

#define l_flg_tst_LI0_bit12_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit12],\
         LIN_FLAG_BIT_OFFSET_LI0_bit12)
#define l_flg_clr_LI0_bit12_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit12],\
         LIN_FLAG_BIT_OFFSET_LI0_bit12)

#define l_flg_tst_LI0_bit13_20_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit13_20],\
         LIN_FLAG_BIT_OFFSET_LI0_bit13_20)
#define l_flg_clr_LI0_bit13_20_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit13_20],\
         LIN_FLAG_BIT_OFFSET_LI0_bit13_20)

#define l_flg_tst_LI0_bit21_23_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit21_23],\
         LIN_FLAG_BIT_OFFSET_LI0_bit21_23)
#define l_flg_clr_LI0_bit21_23_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit21_23],\
         LIN_FLAG_BIT_OFFSET_LI0_bit21_23)

#define l_flg_tst_LI0_bit24_25_flag() \
         LIN_TEST_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit24_25],\
         LIN_FLAG_BIT_OFFSET_LI0_bit24_25)
#define l_flg_clr_LI0_bit24_25_flag() \
         LIN_CLEAR_BIT(lin_flag_handle_tbl[LIN_FLAG_BYTE_OFFSET_LI0_bit24_25],\
         LIN_FLAG_BIT_OFFSET_LI0_bit24_25)



/* Frame flag APIs */

   /* Interface_name = LI0 */

 #define l_flg_tst_LI0_EHIS_FL_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_FL_State]
 #define l_flg_clr_LI0_EHIS_FL_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_FL_State] = 0

 #define l_flg_tst_LI0_EHIS_FR_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_FR_State]
 #define l_flg_clr_LI0_EHIS_FR_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_FR_State] = 0

 #define l_flg_tst_LI0_EHIS_RL_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_RL_State]
 #define l_flg_clr_LI0_EHIS_RL_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_RL_State] = 0

 #define l_flg_tst_LI0_EHIS_RR_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_RR_State]
 #define l_flg_clr_LI0_EHIS_RR_State_flag() \
          lin_frame_flag_tbl[LI0_EHIS_RR_State] = 0

 #define l_flg_tst_LI0_VIU_DWS_flag() \
          lin_frame_flag_tbl[LI0_VIU_DWS]
 #define l_flg_clr_LI0_VIU_DWS_flag() \
          lin_frame_flag_tbl[LI0_VIU_DWS] = 0

 #define l_flg_tst_LI0_MasterReq_flag() \
          lin_frame_flag_tbl[LI0_MasterReq]
 #define l_flg_clr_LI0_MasterReq_flag() \
          lin_frame_flag_tbl[LI0_MasterReq] = 0

 #define l_flg_tst_LI0_SlaveResp_flag() \
          lin_frame_flag_tbl[LI0_SlaveResp]
 #define l_flg_clr_LI0_SlaveResp_flag() \
          lin_frame_flag_tbl[LI0_SlaveResp] = 0



/* INTERFACE MANAGEMENT */

#define l_ifc_init_LI0() l_ifc_init(LI0)



#define l_ifc_wake_up_LI0() l_ifc_wake_up(LI0)



#define l_ifc_rx_LI0() l_ifc_rx(LI0)



#define l_ifc_tx_LI0() l_ifc_tx(LI0)



#define l_ifc_aux_LI0() l_ifc_aux(LI0)



#define l_ifc_read_status_LI0() l_ifc_read_status(LI0)


#endif    /* _LIN_CFG_H_ */
