/**
*****************************************************************************
* @brief  demo example source file.
* @file   lin_cfg.c
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
*****************************************************************************
*/
#include "fff_lin_cfg.h"
#include "fff_lin.h"

/* Mapping interface with hardware */
const lin_hardware_name lin_virtual_ifc = SLIC;
l_u8 lin_lld_response_buffer[10] = {0};
l_u8 lin_successful_transfer = 0;
l_u8 lin_error_in_response = 0;
l_u8 lin_goto_sleep_flg = 0;
/* Save configuration flag */
l_u8 lin_save_configuration_flg = 0;
lin_word_status_str lin_word_status = {0};
l_u8 lin_current_pid = 0;

l_signal_handle LI0_response_error_signal = LI0_EHIS_FL_ResponseError;

volatile l_u8 buffer_backup_data[8] = {0};

/* definition and initialization of signal array */
l_u8    lin_pFrameBuf[LIN_FRAME_BUF_SIZE] =
{


  0xff /* 0 : 11111111 */ /* start of frame LI0_EHIS_FL_State */

  ,0xff /* 1 : 11111111 */
  ,0xff /* 2 : 11111111 */
  ,0x7f /* 3 : 01111111 */
  ,0x08 /* 4 : 00001000 */
  ,0x44 /* 5 : 01000100 */
  ,0x01 /* 6 : 00000001 */
  ,0x05 /* 7 : 00000101 */


  ,0xff /* 8 : 11111111 */ /* start of frame LI0_EHIS_FR_State */

  ,0xff /* 9 : 11111111 */
  
  ,0xff /* 10 : 11111111 */
  
  ,0x7f /* 11 : 01111111 */
  
  ,0x08 /* 12 : 00001000 */
  
  ,0x44 /* 13 : 01000100 */
  
  ,0x01 /* 14 : 00000001 */
  
  ,0x05 /* 15 : 00000101 */
  

  ,0xff /* 16 : 11111111 */ /* start of frame LI0_EHIS_RL_State */

  ,0xff /* 17 : 11111111 */
  
  ,0xff /* 18 : 11111111 */
  
  ,0x7f /* 19 : 01111111 */
  
  ,0x08 /* 20 : 00001000 */
  
  ,0x44 /* 21 : 01000100 */
  
  ,0x01 /* 22 : 00000001 */
  
  ,0x05 /* 23 : 00000101 */
  

  ,0xff /* 24 : 11111111 */ /* start of frame LI0_EHIS_RR_State */

  ,0xff /* 25 : 11111111 */
  
  ,0xff /* 26 : 11111111 */
  
  ,0x7f /* 27 : 01111111 */
  
  ,0x08 /* 28 : 00001000 */
  
  ,0x44 /* 29 : 01000100 */
  
  ,0x01 /* 30 : 00000001 */
  
  ,0x05 /* 31 : 00000101 */
  

  ,0x03 /* 32 : 00000011 */ /* start of frame LI0_VIU_DWS */

  ,0x00 /* 33 : 00000000 */
  
  ,0x00 /* 34 : 00000000 */
  
  ,0xfc /* 35 : 11111100 */
  
  ,0xff /* 36 : 11111111 */
  
  ,0xff /* 37 : 11111111 */
  
  ,0xff /* 38 : 11111111 */
  
  ,0xff /* 39 : 11111111 */
  
};

/* definition and initialization of signal array */
l_u8    lin_flag_handle_tbl[LIN_FLAG_BUF_SIZE] =
{


  0xFF /* 0: start of flag frame LI0_EHIS_FL_State */

  ,0xFF /* 1: */


  ,0xFF /* 2: start of flag frame LI0_EHIS_FR_State */

  ,0xFF /* 3: */
  

  ,0xFF /* 4: start of flag frame LI0_EHIS_RL_State */

  ,0xFF /* 5: */
  

  ,0xFF /* 6: start of flag frame LI0_EHIS_RR_State */

  ,0xFF /* 7: */
  

  ,0xFF /* 8: start of flag frame LI0_VIU_DWS */

  ,0xFF /* 9: */
  
};

/*************************** Flag set when signal is updated ******************/
/* Diagnostic signal */
l_u8 lin_diag_signal_tbl[16] = {0};
/*****************************event trigger frame*****************************/

/**********************************  Frame table **********************************/
const lin_frame_struct lin_frame_tbl[LIN_NUM_OF_FRMS] ={

    { LIN_FRM_UNCD, 8, LIN_RES_PUB, 0, 0, 2  , (l_u8*)&LI0_response_error_signal  }

   ,{ LIN_FRM_UNCD, 8, LIN_RES_PUB, 8, 2, 2 , (l_u8*)&LI0_response_error_signal }
  
   ,{ LIN_FRM_UNCD, 8, LIN_RES_PUB, 16, 4, 2 , (l_u8*)&LI0_response_error_signal }
  
   ,{ LIN_FRM_UNCD, 8, LIN_RES_PUB, 24, 6, 2 , (l_u8*)&LI0_response_error_signal }
  
   ,{ LIN_FRM_UNCD, 8, LIN_RES_SUB, 32, 8, 2 , (l_u8*)0 }
  
   ,{ LIN_FRM_DIAG, 8, LIN_RES_SUB, 0, 0, 0 , (l_u8*)0 }
  
   ,{ LIN_FRM_DIAG, 8, LIN_RES_PUB, 0, 0, 0 , (l_u8*)0 }
  
};

/*********************************** Frame flag Initialization **********************/
/*************************** Frame flag for send/receive successfully ***************/
l_bool lin_frame_flag_tbl[LIN_NUM_OF_FRMS] = {0, 0, 0, 0, 0, 0, 0};
/*************************** Frame flag for updating signal in frame ****************/
volatile l_u8 lin_frame_updating_flag_tbl[LIN_NUM_OF_FRMS] = {0, 0, 0, 0, 0, 0, 0};


/**************************** Lin configuration Initialization ***********************/
/* max_response_frame_timeout = round((1.4x(10+Nx10)xTbit)/Tbase_period) + 3 */

const l_u16 lin_max_frame_res_timeout_val[8]={

6, 7, 9, 10, 12, 13, 15, 16

};


l_u8 lin_configuration_RAM[LIN_SIZE_OF_CFG]= {0x00, 0x11, 0x13, 0x14, 0x12, 0x01, 0x3C, 0x3D ,0xFF};


l_u16  lin_configuration_ROM[LIN_SIZE_OF_CFG]= {0x00, 0x11, 0x13, 0x14, 0x12, 0x01, 0x3C, 0x3D ,0xFFFF};

/***************************************** Node Attribute*****************************************/

l_u8 lin_configured_NAD = 0x68;    /*<configured_NAD>*/
l_u8 lin_initial_NAD    =0x68;    /*<initial_NAD>*/
lin_product_id product_id = {0x0000, 0x0000, 0x0000 };  /* {<supplier_id>,<function_id>,<variant>} */
l_signal_handle response_error =  LI0_EHIS_FL_ResponseError;
const l_u8 num_frame_have_esignal = 1;                                 /*number of frame contain error signal*/
l_u16 lin_response_error_byte_offset[1] = {LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError};                  /*<interface_name>_< response_error>*/
l_u8 lin_response_error_bit_offset[1] = {LIN_BIT_OFFSET_LI0_EHIS_FL_ResponseError};                  /*<interface_name>_< response_error>*/


/************************** TL Layer and Diagnostic: SINGLE interface **************************/
/* QUEUE information */
lin_tl_pdu_data tl_tx_queue_data[MAX_QUEUE_SIZE];    /*transmit queue data */
lin_tl_pdu_data tl_rx_queue_data[MAX_QUEUE_SIZE];    /*receive queue data */

lin_transport_layer_queue lin_tl_tx_queue = {
0,                                                /* the first element of queue */
0,                                                /* the last element of queue */
LD_QUEUE_EMPTY,                                   /* status of queue */
0,                                                /* curernt size of queue */
MAX_QUEUE_SIZE,                                   /* size of queue */
tl_tx_queue_data,                                 /* data of queue */
};

lin_transport_layer_queue lin_tl_rx_queue = {
0,                                                /* the first element of queue */
0,                                                /* the last element of queue */
LD_QUEUE_EMPTY,                                   /* status of queue */
0,                                                /* curernt size of queue */
MAX_QUEUE_SIZE,                                   /* size of queue */
tl_rx_queue_data,                                 /* data of queue */
};
/* message information in transmit queue */
l_u16 tl_rx_msg_index = 0;                                /* index of message in queue */
l_u16 tl_rx_msg_size = 0;                                 /* Size of message in queue */
/* message information in receive queue */
l_u16 tl_tx_msg_index = 0;                                /* index of message in queue */
l_u16 tl_tx_msg_size = 0;                                 /* Size of message in queue */
lin_last_cfg_result tl_last_cfg_result = {0};               /* Status of the last configuration service in LIN 2.0, J2602 */
l_u8 tl_last_RSID = 0;                                    /* RSID of the last node configuration service */
l_u8 tl_ld_error_code = 0;                                /* Error code in case of positive response */
l_u8 tl_no_of_pdu = 0;                                    /* number of received pdu */
l_u8 tl_frame_counter = 0;                                /* frame counter in received message */
lin_message_timeout_type tl_check_timeout_type = 0;       /* timeout type */
l_u16 tl_check_timeout = 0;                               /* timeout counter*/
l_u8 *tl_ident_data = 0;                                  /* To store address of RAM area contain response */
lin_diagnostic_state tl_diag_state   =  LD_DIAG_IDLE;
lin_service_status tl_service_status =  LD_SERVICE_IDLE ; /* service status */
lin_message_status tl_receive_msg_status;             /* receive message status */
lin_message_status tl_rx_msg_status;                  /* cooked rx status */
lin_message_status tl_tx_msg_status;                  /* cooked tx status */










/****************************Support SID Initialization ***********************/

l_u8 lin_diag_services_supported[_DIAG_NUMBER_OF_SERVICES_] = {0x10,0x11,0x14,0x19,0x22,0x27,0x28,0x2E,0x2F,0x31,0x3E,0x34,0x36,0x37,0x85,0xA0,
                                                                     0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
                                                                     0xBA,0xBB,0xBC};
l_u8 lin_diag_services_flag[_DIAG_NUMBER_OF_SERVICES_] = {0,0,0,0,0,0,0,0,
                                                          0,0,0,0,0,0,0,0,
                                                          0,0,0};
l_u8 tl_slaveresp_cnt = 0;
/*This ld_read_by_id_callout() function is used when the master node transmits a read by
 identifier request with an identifier in the user defined area (id from 32 to 63).
 The driver will call this function when such request is received.
 * id: the identifier in the user defined area (32 to 63)
 * data: pointer points to a data area with 5 bytes, used to give the positive response.
  Driver uses 0xFF "do not care value" for unassigned data values.
  Data length in PCI is (1 + number of assigned meaningful data values).
  Driver will take as data for all data before and including the last value in the frame that is different from 0xFF.
  PCI is 0x02-0x06, so data should have at least one value different from 0xFF.
  For example, a response frame, (NAD) (PCI) (0xF2) (0xFF) (0x00) (0xFF) (0xFF) (0xFF),
  PCI will be 0x03, since in this case driver takes all data before 0x00 and 0x00 as meaningful data,
  and values after 0x00 are do not care value.
 * return: LD_NEGATIVE_RESPONSE Respond with a negative response.
           LD_POSTIVE_RESPONSE Respond with a positive response.
           LD_ID_NO_RESPONSE The slave node will not answer.
 */
// l_u8 ld_read_by_id_callout(l_u8 id, l_u8 *data)
// {
//     l_u8 retval = LD_NEGATIVE_RESPONSE;
//     /* Following code is an example - Real implementation is application-dependent */
//     /* This example implement with ID = 32 - LIN_READ_USR_DEF_MIN */
//     if (id == LIN_READ_USR_DEF_MIN)
//     {
//       /* id received is user defined 32 */
//       data[0] = (l_u8) (id + 1);    /* Data user define */
//       data[1] = (l_u8) (id + 2);    /* Data user define */
//       data[2] = (l_u8) (id + 3);    /* Data user define */
//       data[3] = (l_u8) (id + 4);    /* Data user define */
//       data[4] = (l_u8) (id + 5);    /* Data user define */
//       retval = LD_POSITIVE_RESPONSE;
//     }
//     else
//     {
//       /* other identifiers, respond with negative response by default*/
//     }
//     return retval;
// }
DEFINE_FAKE_VALUE_FUNC(l_u8,ld_read_by_id_callout, l_u8, l_u8*);


