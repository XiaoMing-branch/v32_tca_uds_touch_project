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
#ifdef ENABLE_TEST_MODE
#include "fff_lin_cfg.h"
#include "fff_lin.h"
#else
#include "lin_cfg.h"
#include "lin.h"
#endif

/**
 * @brief 硬件接口映射，指定使用的LIN硬件接口为SLIC
 */
const lin_hardware_name lin_virtual_ifc = SLIC;
/**
 * @brief LLD响应缓冲区，存储底层驱动响应数据
 */
l_u8 lin_lld_response_buffer[10] = {0};
/**
 * @brief 成功传输标志，指示LIN帧是否成功传输
 */
l_u8 lin_successful_transfer = 0;
/**
 * @brief 响应错误标志，指示LIN响应中是否发生错误
 */
l_u8 lin_error_in_response = 0;
/**
 * @brief 休眠标志，指示LIN总线是否进入休眠模式
 */
l_u8 lin_goto_sleep_flg = 0;
/**
 * @brief 配置保存标志，指示是否需要保存当前配置
 */
l_u8 lin_save_configuration_flg = 0;
/**
 * @brief 字状态结构体，记录LIN通信的字状态信息
 */
lin_word_status_str lin_word_status = {0};
/**
 * @brief 当前PID（保护标识符），标识当前正在处理的LIN帧
 */
l_u8 lin_current_pid = 0;

/**
 * @brief LI0响应错误信号句柄，指向LHIS_FL帧的ResponseError信号
 */
l_signal_handle LI0_response_error_signal = LI0_EHIS_FL_ResponseError;

/**
 * @brief 备份缓冲区，用于数据备份（8字节）
 */
volatile l_u8 buffer_backup_data[8] = {0};

/**
 * @brief 信号帧缓冲区数组，存储所有LIN帧的信号数据
 * @note  每个帧占用8字节，依次存放5个状态帧和1个诊断帧
 *        索引0-7:  LI0_EHIS_FL_State（左后输入状态）
 *        索引8-15: LI0_EHIS_FR_State（右后输入状态）
 *        索引16-23: LI0_EHIS_RL_State（左后输出状态）
 *        索引24-31: LI0_EHIS_RR_State（右后输出状态）
 *        索引32-39: LI0_VIU_DWS（门开关状态）
 */
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

/**
 * @brief 帧信号标志处理表，标识每帧中各信号是否已更新
 * @note  每帧占用2字节标志位
 *        索引0-1: LI0_EHIS_FL_State
 *        索引2-3: LI0_EHIS_FR_State
 *        索引4-5: LI0_EHIS_RL_State
 *        索引6-7: LI0_EHIS_RR_State
 *        索引8-9: LI0_VIU_DWS
 */
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

/**
 * @brief 诊断信号标志表，记录诊断信号更新状态（16字节）
 */
l_u8 lin_diag_signal_tbl[16] = {0};
/*****************************event trigger frame*****************************/

/**
 * @brief LIN帧配置表，定义所有帧的协议参数
 * @note  帧配置结构: {帧类型, 数据长度, 响应类型, 帧缓冲区偏移, 标志偏移, 错误信号计数, 错误信号句柄}
 *        索引0: LI0_EHIS_FL_State - 无条件帧，Publisher，偏移0
 *        索引1: LI0_EHIS_FR_State - 无条件帧，Publisher，偏移8
 *        索引2: LI0_EHIS_RL_State - 无条件帧，Publisher，偏移16
 *        索引3: LI0_EHIS_RR_State - 无条件帧，Publisher，偏移24
 *        索引4: LI0_VIU_DWS       - 无条件帧，Subscriber，偏移32
 *        索引5: 诊断请求帧         - 诊断帧，Subscriber
 *        索引6: 诊断响应帧         - 诊断帧，Publisher
 */
const lin_frame_struct lin_frame_tbl[LIN_NUM_OF_FRMS] ={

    { LIN_FRM_UNCD, 8, LIN_RES_PUB, 0, 0, 2  , (l_u8*)&LI0_response_error_signal  }

   ,{ LIN_FRM_UNCD, 8, LIN_RES_PUB, 8, 2, 2 , (l_u8*)&LI0_response_error_signal }
  
   ,{ LIN_FRM_UNCD, 8, LIN_RES_PUB, 16, 4, 2 , (l_u8*)&LI0_response_error_signal }
  
   ,{ LIN_FRM_UNCD, 8, LIN_RES_PUB, 24, 6, 2 , (l_u8*)&LI0_response_error_signal }
  
   ,{ LIN_FRM_UNCD, 8, LIN_RES_SUB, 32, 8, 2 , (l_u8*)0 }
  
   ,{ LIN_FRM_DIAG, 8, LIN_RES_SUB, 0, 0, 0 , (l_u8*)0 }
  
   ,{ LIN_FRM_DIAG, 8, LIN_RES_PUB, 0, 0, 0 , (l_u8*)0 }
  
};

/**
 * @brief 帧发送/接收成功标志表，每帧对应一个标志位
 * @note  索引0-6分别对应7个LIN帧的收发完成状态
 */
l_bool lin_frame_flag_tbl[LIN_NUM_OF_FRMS] = {0, 0, 0, 0, 0, 0, 0};
/**
 * @brief 帧信号更新标志表，标识每帧中的信号是否正在被更新
 * @note  索引0-6分别对应7个LIN帧的更新状态
 */
volatile l_u8 lin_frame_updating_flag_tbl[LIN_NUM_OF_FRMS] = {0, 0, 0, 0, 0, 0, 0};


/**
 * @brief LIN最大帧响应超时值表（8种波特率对应超时值）
 * @note  计算公式: max_response_frame_timeout = round((1.4x(10+Nx10)xTbit)/Tbase_period) + 3
 *        索引0-7对应不同波特率（20kbps-100kbps步进）的超时重传次数
 */
const l_u16 lin_max_frame_res_timeout_val[8]={
    6, 7, 9, 10, 12, 13, 15, 16
};

/**
 * @brief LIN节点配置参数（RAM区），用于运行时可修改的配置
 * @note  字节含义:
 *        索引0: 保留 (0x00)
 *        索引1: 节点NAD (0x11)
 *        索引2: 配置ID (0x13)
 *        索引3: 诊断PDU响应时间 (0x14)
 *        索引4: 帧响应时间 (0x12)
 *        索引5: 协议版本 (0x01 = LIN 2.1)
 *        索引6-7: 保留/供应商特定 (0x3C, 0x3D)
 *        索引8: 配置状态标志 (0xFF)
 */
l_u8 lin_configuration_RAM[LIN_SIZE_OF_CFG] = {0x00, 0x11, 0x13, 0x14, 0x12, 0x01, 0x3C, 0x3D, 0xFF};

/**
 * @brief LIN节点配置参数（ROM区/默认值），用于出厂默认配置
 * @note  各字节含义同lin_configuration_RAM，最后一个字为0xFFFF表示ROM结束标记
 */
l_u16 lin_configuration_ROM[LIN_SIZE_OF_CFG] = {0x00, 0x11, 0x13, 0x14, 0x12, 0x01, 0x3C, 0x3D, 0xFFFF};

/**
 * @brief LIN节点属性配置
 * @{
 */
/**
 * @brief 已配置的NAD（节点地址），从机节点地址 = 0x68
 * @note  该值由主节点通过诊断配置服务分配，用于节点识别和诊断通信
 */
l_u8 lin_configured_NAD = 0x68;
/**
 * @brief 初始NAD（节点地址），上电默认地址 = 0x68
 * @note  与configured_NAD相同，表示节点在未重新配置前使用此地址
 */
l_u8 lin_initial_NAD = 0x68;
/**
 * @brief LIN产品标识符
 * @note  {supplier_id, function_id, variant}
 *        supplier_id: 供应商ID
 *        function_id: 功能ID
 *        variant:     变体版本号
 */
const lin_product_id product_id = {0x0000, 0x0000, 0x0000};
/**
 * @brief 响应错误信号句柄，指向LI0_EHIS_FL帧的ResponseError信号
 */
l_signal_handle response_error = LI0_EHIS_FL_ResponseError;
/**
 * @brief 包含错误信号的帧数量
 */
const l_u8 num_frame_have_esignal = 1;
/**
 * @brief 响应错误信号在各帧中的字节偏移表
 * @note  索引0: LI0_EHIS_FL_State帧中的ResponseError字节偏移
 */
l_u16 lin_response_error_byte_offset[1] = {LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError};
/**
 * @brief 响应错误信号在各帧中的位偏移表
 * @note  索引0: LI0_EHIS_FL_State帧中的ResponseError位偏移
 */
l_u8 lin_response_error_bit_offset[1] = {LIN_BIT_OFFSET_LI0_EHIS_FL_ResponseError};
/** @} */


/**
 * @defgroup TL_Layer TL层与诊断模块
 * @brief 传输层（Transport Layer）和诊断通信相关数据结构，单接口模式
 * @{
 */
/**
 * @brief 发送队列数据缓冲区
 */
lin_tl_pdu_data tl_tx_queue_data[MAX_QUEUE_SIZE];

/**
 * @brief 接收队列数据缓冲区
 */
lin_tl_pdu_data tl_rx_queue_data[MAX_QUEUE_SIZE];

/**
 * @brief LIN传输层发送队列
 * @note  包含队列头、尾指针、状态、当前大小、最大大小及数据缓冲区
 */
lin_transport_layer_queue lin_tl_tx_queue = {
    0,              /* 队列头索引 */
    0,              /* 队列尾索引 */
    LD_QUEUE_EMPTY, /* 队列状态: 空 */
    0,              /* 当前队列大小 */
    MAX_QUEUE_SIZE, /* 队列最大容量 */
    tl_tx_queue_data, /* 队列数据缓冲区 */
};

/**
 * @brief LIN传输层接收队列
 * @note  包含队列头、尾指针、状态、当前大小、最大大小及数据缓冲区
 */
lin_transport_layer_queue lin_tl_rx_queue = {
    0,              /* 队列头索引 */
    0,              /* 队列尾索引 */
    LD_QUEUE_EMPTY, /* 队列状态: 空 */
    0,              /* 当前队列大小 */
    MAX_QUEUE_SIZE, /* 队列最大容量 */
    tl_rx_queue_data, /* 队列数据缓冲区 */
};

/**
 * @brief 接收队列中的消息索引
 */
l_u16 tl_rx_msg_index = 0;

/**
 * @brief 接收队列中的消息大小
 */
l_u16 tl_rx_msg_size = 0;

/**
 * @brief 发送队列中的消息索引
 */
l_u16 tl_tx_msg_index = 0;

/**
 * @brief 发送队列中的消息大小
 */
l_u16 tl_tx_msg_size = 0;

/**
 * @brief 上一次节点配置服务（LIN 2.0 / J2602）的执行结果
 */
lin_last_cfg_result tl_last_cfg_result = {0};

/**
 * @brief 上一次节点配置服务的RSID（请求服务ID）
 */
l_u8 tl_last_RSID = 0;

/**
 * @brief 诊断错误码，用于肯定响应中的错误指示
 */
l_u8 tl_ld_error_code = 0;

/**
 * @brief 已接收的PDU数量
 */
l_u8 tl_no_of_pdu = 0;

/**
 * @brief 接收消息中的帧计数器
 */
l_u8 tl_frame_counter = 0;

/**
 * @brief 超时检测类型
 */
lin_message_timeout_type tl_check_timeout_type = 0;

/**
 * @brief 超时计数器
 */
l_u16 tl_check_timeout = 0;

/**
 * @brief RAM数据区域指针，用于存储诊断响应数据
 */
l_u8 *tl_ident_data = 0;

/**
 * @brief 诊断状态，标识当前诊断状态机所处阶段
 */
lin_diagnostic_state tl_diag_state = LD_DIAG_IDLE;

/**
 * @brief 服务状态，标识当前服务处理状态
 */
lin_service_status tl_service_status = LD_SERVICE_IDLE;

/**
 * @brief 接收消息状态
 */
lin_message_status tl_receive_msg_status;

/**
 * @brief 处理后的接收消息状态
 */
lin_message_status tl_rx_msg_status;

/**
 * @brief 处理后的发送消息状态
 */
lin_message_status tl_tx_msg_status;
/** @} */










/**
 * @brief UDS诊断支持的SID（服务标识符）列表
 * @note  支持的诊断服务包括:
 *        0x10 - 诊断会话控制 (DiagnosticSessionControl)
 *        0x11 - 电子控制单元复位 (ECUReset)
 *        0x14 - 清除诊断信息 (ClearDiagnosticInformation)
 *        0x19 - 读取诊断信息 (ReadDTCInformation)
 *        0x22 - 按标识符读取数据 (ReadDataByIdentifier)
 *        0x27 - 安全访问 (SecurityAccess)
 *        0x28 - 通信控制 (CommunicationControl)
 *        0x2E - 按标识符写入数据 (WriteDataByIdentifier)
 *        0x2F - 按地址读写数据 (InputOutputControlByIdentifier)
 *        0x31 - 例程控制 (RoutineControl)
 *        0x3E - 保活 (TesterPresent)
 *        0x34 - 请求下载 (RequestDownload)
 *        0x36 - 传输数据 (TransferData)
 *        0x37 - 请求传输退出 (RequestTransferExit)
 *        0x85 - 控制DTC设置 (ControlDTCSetting)
 *        0xA0 - 会话切换
 *        0xB0-0xB7, 0xBA-0xBC - 供应商特定服务
 */
const l_u8 lin_diag_services_supported[_DIAG_NUMBER_OF_SERVICES_] = {0x10, 0x11, 0x14, 0x19, 0x22, 0x27, 0x28, 0x2E, 0x2F, 0x31, 0x3E, 0x34, 0x36, 0x37, 0x85, 0xA0,
                                                                     0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
                                                                     0xBA, 0xBB, 0xBC};
/**
 * @brief UDS诊断服务启用标志表，标识各SID是否被允许执行
 * @note  与lin_diag_services_supported一一对应，0=禁用，1=启用
 */
l_u8 lin_diag_services_flag[_DIAG_NUMBER_OF_SERVICES_] = {0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                          0, 0, 0, 0, 0, 0, 0, 0,
                                                          0, 0, 0};

/**
 * @brief 从节点响应计数器，记录从节点发送的响应帧数量
 */
l_u8 tl_slaveresp_cnt = 0;

/**
 * @brief  按标识符读取数据回调函数
 * @note   当主节点发送按标识符读取请求，且标识符落在用户定义区域（ID 32~63）时，
 *         驱动层将回调此函数以获取响应数据。
 *         数据格式: data指向5字节缓冲区，用于构造肯定响应。
 *         驱动层将0xFF视为"无关值"，PCI长度 = 1 + 有效数据值个数。
 *         例如响应帧 (NAD)(PCI)(0xF2)(0xFF)(0x00)(0xFF)(0xFF)(0xFF)，
 *         PCI为0x03，驱动将0xF2和0x00作为有效数据。
 * @param  id   用户定义区域的标识符（范围: 32~63）
 * @param  data 指向5字节数据缓冲区的指针，用于存放肯定响应数据
 * @retval LD_NEGATIVE_RESPONSE  响应否定响应码
 * @retval LD_POSITIVE_RESPONSE  响应肯定响应码
 * @retval LD_ID_NO_RESPONSE     从节点不响应
 */
l_u8 ld_read_by_id_callout(l_u8 id, l_u8 *data)
{
    l_u8 retval = LD_NEGATIVE_RESPONSE;
    /* Following code is an example - Real implementation is application-dependent */
    /* This example implement with ID = 32 - LIN_READ_USR_DEF_MIN */
    if (id == LIN_READ_USR_DEF_MIN)
    {
      /* id received is user defined 32 */
      data[0] = (l_u8) (id + 1);    /* Data user define */
      data[1] = (l_u8) (id + 2);    /* Data user define */
      data[2] = (l_u8) (id + 3);    /* Data user define */
      data[3] = (l_u8) (id + 4);    /* Data user define */
      data[4] = (l_u8) (id + 5);    /* Data user define */
      retval = LD_POSITIVE_RESPONSE;
    }
    else
    {
      /* other identifiers, respond with negative response by default*/
    }
    return retval;
}
