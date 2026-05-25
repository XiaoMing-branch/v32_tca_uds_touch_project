/**
*****************************************************************************
* @brief  demo example source file.
* @file   lin_frame.h
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
#ifndef __LIN_FRAME_H__
#define __LIN_FRAME_H__

#include "fff.h"
#include "fff_base_types.h"

typedef struct
{
    uint8_t FltSt;          // ����״̬
    uint8_t SwtSt;          // ����״̬
    uint8_t SW_MinorVersA;  // �����ΰ汾��
    uint8_t SW_MajorVersA;  // �������汾��
    uint8_t HW_PhaVers;     // Ӳ���׶ΰ汾��
    uint8_t HW_MinorVersB;  // Ӳ���ΰ汾��
    uint8_t HW_MajorVersB;  // Ӳ�����汾��
    uint8_t SN_MinorVersB;  // �����к�B
    uint8_t SN_MajorVersB;  // �����к�B
    uint8_t SN_SupplierCod; // ��Ӧ�̴���
} DoorSt_T;

typedef struct
{

    uint8_t UsageMode;          // 整车用户模式
    uint8_t VehicleSpeedValid;  // 车速有效位
    uint16_t VehicleSpeed;      // 车速
} DoorCmd_T;

typedef enum
{
    SW_INVALID_ST = 0,
    SW_PUSH_ST,
    SW_IDLE_ST,
} T_SWSt;

typedef enum
{
    NORMAL_ST = 0,
    FAULT_ST,
} T_FaultSt;   

extern DoorCmd_T door_cmd; // ECU对门把手控制信号

DECLARE_FAKE_VOID_FUNC(App_LinControlMsg);
//void App_LinControlMsg(void);
#endif  //__LIN_FRAME_H__
