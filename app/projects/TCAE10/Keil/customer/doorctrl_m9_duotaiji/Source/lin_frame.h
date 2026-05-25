/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
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
#ifndef LIN_FRAME_H__
#define LIN_FRAME_H__

#include "test_config.h"

#ifdef ENABLE_TEST_MODE
#include "fff_base_types.h"
#else
#include "base_types.h"
#endif

typedef struct
{
    uint8_t FltSt;          // 
    uint8_t SwtSt;          // 
    uint8_t SW_MinorVersA;  // 
    uint8_t SW_MajorVersA;  // 
    uint8_t HW_PhaVers;     // 
    uint8_t HW_MinorVersB;  // 
    uint8_t HW_MajorVersB;  // 
    uint8_t SN_MinorVersB;  // 
    uint8_t SN_MajorVersB;  // 
    uint8_t SN_SupplierCod; // 
} DoorSt_T;

typedef struct
{

    uint8_t UsageMode;          // Vehicle User Mode
    uint8_t VehicleSpeedValid;  // Effective vehicle speed digits
    uint16_t VehicleSpeed;      // Vehicle speed
} DoorCmd_T;

typedef enum
{
    SW_INVALID_ST = 0,
    SW_PUSH_ST,
    SW_IDLE_ST,
/* PRQA S 1535 1 #3262 - Unused typedef defined for future extension and type consistency */
} T_SWSt;

typedef enum
{
    NORMAL_ST = 0,
    FAULT_ST,
/* PRQA S 1535 1 #3262 - Unused typedef defined for future extension and type consistency */
} T_FaultSt;   //

extern DoorCmd_T door_cmd; // ECU control signal for the door handle
void App_LinControlMsg(void);
#endif  //__LIN_FRAME_H__
