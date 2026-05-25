/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
*****************************************************************************
* @brief  demo example source file.
* @file   lin_frame.c
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
#include "fff_lin_frame.h"
#include "fff_lin_cfg.h"
#include "fff_lin.h"
#include "fff_app.h"
#include "fff_custom_diagnosticIII.h"
#include "fff_tc_log.h"
#include "fff_lin_process.h"
#else
#include "lin_frame.h"
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "lin_cfg.h"
#include "lin.h"
#include "app.h"
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "custom_diagnosticIII.h"
#include "tc_log.h"
#include "lin_process.h"
#endif

/* PRQA S 3207 1 #3207 - Static object is intentionally unused (e.g., used via debugger or inline assembly). */
static const char *TAG = "LIN FRAME";

/* PRQA S 1514 2 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
DoorSt_T door_st = {0};   // Door handle status feedback signal, initialized to 0
DoorCmd_T door_cmd = {0}; // The ECU controls the door handle signal, initialized to 0
extern user_cfg_t g_user_info;
extern volatile uint8_t lin_error;

/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void AppSetLinErrByNad(uint8_t err_flag)
{
    /* PRQA S 3469 15 #3258 - Function-like macro used for performance and compiler optimization requirements */
    if ((uint8_t)LEFT_FRONT_DOOR == g_user_info.config_word)
    {
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_FL_ResponseError(err_flag);
    }
    else if ((uint8_t)LEFT_REAR_DOOR == g_user_info.config_word)
    {
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_RL_ResponseError(err_flag);
    }
    else if ((uint8_t)RIGHT_FRONT_DOOR == g_user_info.config_word)
    {
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_FR_ResponseError(err_flag);
    }
    else if ((uint8_t)RIGHT_REAR_DOOR == g_user_info.config_word)
    {
        /* PRQA S 3469 2 #3469 - Function-like macro usage is intentional (e.g., for type genericity, debug info, or side-effect control); equivalent function would not be suitable. */
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_RR_ResponseError(err_flag);
    }
    else
    {
        (void)0;
    }
}

/**
 * @description: Send signals to the host via LIN
 * @return [void]
 */
/* PRQA S 2985 ++ #2985 - Redundant operation is intentional (e.g., to satisfy coding style or generate specific assembly). */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void App_LinSendDoorState(void)
{
    if (lin_error != 0u)
    {
        AppSetLinErrByNad(1);
    }
    else
    {
        AppSetLinErrByNad(0);
    }

    /* PRQA S 3469 ++ #3258 - Function-like macro used for performance and compiler optimization requirements */
    if (l_flg_tst_LI0_EHIS_FL_State_flag() != 0u)
    {
        l_flg_clr_LI0_EHIS_FL_State_flag();
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_FL_FltSt(door_st.FltSt);
        l_u8_wr_LI0_EHIS_FL_SwtSt(door_st.SwtSt);
        /* PRQA S 4543 6 #4543 - Non-negative signed constant used as left operand of shift operator is intentional; shift result is well-defined. */
        /* PRQA S 4542 5 #4542 - Non-negative signed constant used as operand of bitwise operator is intentional; value is safe and bitwise operation is well-defined. */
        /* PRQA S 4532 4 #4532 - Using essentially signed type as operand of bitwise operator is intentional; the signed value is non-negative or bit pattern is intended. */
        /* PRQA S 4434 3 #4434 - Conversion from signed to unsigned on assignment is intentional; the signed value is non-negative or wrap-around behavior is expected. */
        /* PRQA S 1851 2 #1851 - Implicit conversion from unsigned to signed in bitwise operation is intentional and safe. */
        /* PRQA S 1840 1 #1840 - Implicit conversion of non-negative signed constant to unsigned is intentional and safe. */
        l_u8_wr_LI0_EHIS_FL_SW_MinorVersA(door_st.SW_MinorVersA);
        l_u8_wr_LI0_EHIS_FL_SW_MajorVersA(door_st.SW_MajorVersA);
        l_u8_wr_LI0_EHIS_FL_HW_PhaVers(door_st.HW_PhaVers);
        l_u8_wr_LI0_EHIS_FL_HW_MajorVersB(door_st.HW_MajorVersB);
        l_u8_wr_LI0_EHIS_FL_HW_MinorVersB(door_st.HW_MinorVersB);
        l_u8_wr_LI0_EHIS_FL_SN_MajorVersB(door_st.SN_MajorVersB);
        l_u8_wr_LI0_EHIS_FL_SN_MinorVersB(door_st.SN_MinorVersB);
        l_u8_wr_LI0_EHIS_FL_SN_SupplierCod(door_st.SN_SupplierCod);
        lin_error = 0;

        if ((uint8_t)LEFT_FRONT_DOOR != g_user_info.config_word)
        {
            g_user_info.config_word = (uint8_t)LEFT_FRONT_DOOR; // Left front door handle
            uds_diagnostic_configword_remap_nad();
        }
    }
    if (l_flg_tst_LI0_EHIS_RL_State_flag() != 0u)
    {
        l_flg_clr_LI0_EHIS_RL_State_flag();
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_RL_FltSt(door_st.FltSt);
        l_u8_wr_LI0_EHIS_RL_SwtSt(door_st.SwtSt);
        /* PRQA S 4543 6 #4543 - Non-negative signed constant used as left operand of shift operator is intentional; shift result is well-defined. */
        /* PRQA S 4542 5 #4542 - Non-negative signed constant used as operand of bitwise operator is intentional; value is safe and bitwise operation is well-defined. */
        /* PRQA S 4532 4 #4532 - Using essentially signed type as operand of bitwise operator is intentional; the signed value is non-negative or bit pattern is intended. */
        /* PRQA S 4434 3 #4434 - Conversion from signed to unsigned on assignment is intentional; the signed value is non-negative or wrap-around behavior is expected. */
        /* PRQA S 1851 2 #1851 - Implicit conversion from unsigned to signed in bitwise operation is intentional and safe. */
        /* PRQA S 1840 1 #1840 - Implicit conversion of non-negative signed constant to unsigned is intentional and safe. */
        l_u8_wr_LI0_EHIS_RL_SW_MinorVersA(door_st.SW_MinorVersA);
        l_u8_wr_LI0_EHIS_RL_SW_MajorVersA(door_st.SW_MajorVersA);
        l_u8_wr_LI0_EHIS_RL_HW_PhaVers(door_st.HW_PhaVers);
        l_u8_wr_LI0_EHIS_RL_HW_MajorVersB(door_st.HW_MajorVersB);
        l_u8_wr_LI0_EHIS_RL_HW_MinorVersB(door_st.HW_MinorVersB);
        l_u8_wr_LI0_EHIS_RL_SN_MajorVersB(door_st.SN_MajorVersB);
        l_u8_wr_LI0_EHIS_RL_SN_MinorVersB(door_st.SN_MinorVersB);
        l_u8_wr_LI0_EHIS_RL_SN_SupplierCod(door_st.SN_SupplierCod);
        lin_error = 0;

        if ((uint8_t)LEFT_REAR_DOOR != g_user_info.config_word)
        {
            g_user_info.config_word = (uint8_t)LEFT_REAR_DOOR; // Left rear door handle
            uds_diagnostic_configword_remap_nad();
        }
    }
    if (l_flg_tst_LI0_EHIS_FR_State_flag() != 0u)
    {
        l_flg_clr_LI0_EHIS_FR_State_flag();
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_FR_FRtSt(door_st.FltSt);
        l_u8_wr_LI0_EHIS_FR_SwtSt(door_st.SwtSt);
        /* PRQA S 4543 6 #4543 - Non-negative signed constant used as left operand of shift operator is intentional; shift result is well-defined. */
        /* PRQA S 4542 5 #4542 - Non-negative signed constant used as operand of bitwise operator is intentional; value is safe and bitwise operation is well-defined. */
        /* PRQA S 4532 4 #4532 - Using essentially signed type as operand of bitwise operator is intentional; the signed value is non-negative or bit pattern is intended. */
        /* PRQA S 4434 3 #4434 - Conversion from signed to unsigned on assignment is intentional; the signed value is non-negative or wrap-around behavior is expected. */
        /* PRQA S 1851 2 #1851 - Implicit conversion from unsigned to signed in bitwise operation is intentional and safe. */
        /* PRQA S 1840 1 #1840 - Implicit conversion of non-negative signed constant to unsigned is intentional and safe. */
        l_u8_wr_LI0_EHIS_FR_SW_MinorVersA(door_st.SW_MinorVersA);
        l_u8_wr_LI0_EHIS_FR_SW_MajorVersA(door_st.SW_MajorVersA);
        l_u8_wr_LI0_EHIS_FR_HW_PhaVers(door_st.HW_PhaVers);
        l_u8_wr_LI0_EHIS_FR_HW_MajorVersB(door_st.HW_MajorVersB);
        l_u8_wr_LI0_EHIS_FR_HW_MinorVersB(door_st.HW_MinorVersB);
        l_u8_wr_LI0_EHIS_FR_SN_MajorVersB(door_st.SN_MajorVersB);
        l_u8_wr_LI0_EHIS_FR_SN_MinorVersB(door_st.SN_MinorVersB);
        l_u8_wr_LI0_EHIS_FR_SN_SupplierCod(door_st.SN_SupplierCod);
        lin_error = 0;

        if ((uint8_t)RIGHT_FRONT_DOOR != g_user_info.config_word)
        {
            g_user_info.config_word = (uint8_t)RIGHT_FRONT_DOOR; // Right front door handle
            uds_diagnostic_configword_remap_nad();
        }
    }
    if (l_flg_tst_LI0_EHIS_RR_State_flag() != 0u)
    {
        l_flg_clr_LI0_EHIS_RR_State_flag();
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_RR_FltSt(door_st.FltSt);
        l_u8_wr_LI0_EHIS_RR_SwtSt(door_st.SwtSt);
        /* PRQA S 4543 6 #4543 - Non-negative signed constant used as left operand of shift operator is intentional; shift result is well-defined. */
        /* PRQA S 4542 5 #4542 - Non-negative signed constant used as operand of bitwise operator is intentional; value is safe and bitwise operation is well-defined. */
        /* PRQA S 4532 4 #4532 - Using essentially signed type as operand of bitwise operator is intentional; the signed value is non-negative or bit pattern is intended. */
        /* PRQA S 4434 3 #4434 - Conversion from signed to unsigned on assignment is intentional; the signed value is non-negative or wrap-around behavior is expected. */
        /* PRQA S 1851 2 #1851 - Implicit conversion from unsigned to signed in bitwise operation is intentional and safe. */
        /* PRQA S 1840 1 #1840 - Implicit conversion of non-negative signed constant to unsigned is intentional and safe. */
        l_u8_wr_LI0_EHIS_RR_SW_MinorVersA(door_st.SW_MinorVersA);
        l_u8_wr_LI0_EHIS_RR_SW_MajorVersA(door_st.SW_MajorVersA);
        l_u8_wr_LI0_EHIS_RR_HW_PhaVers(door_st.HW_PhaVers);
        l_u8_wr_LI0_EHIS_RR_HW_MajorVersB(door_st.HW_MajorVersB);
        l_u8_wr_LI0_EHIS_RR_HW_MinorVersB(door_st.HW_MinorVersB);
        l_u8_wr_LI0_EHIS_RR_SN_MajorVersB(door_st.SN_MajorVersB);
        l_u8_wr_LI0_EHIS_RR_SN_MinorVersB(door_st.SN_MinorVersB);
        l_u8_wr_LI0_EHIS_RR_SN_SupplierCod(door_st.SN_SupplierCod);
        lin_error = 0;

        if ((uint8_t)RIGHT_REAR_DOOR != g_user_info.config_word)
        {
            g_user_info.config_word = (uint8_t)RIGHT_REAR_DOOR; // Right rear door handle
            uds_diagnostic_configword_remap_nad();
        }
    }
}

/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void App_LinReceiveDoorState(void)
{
    uint16_t data[5] = {0};

    if (l_flg_tst_LI0_VIU_DWS_flag() != 0u)
    {
        l_flg_clr_LI0_VIU_DWS_flag();

        if ((uint8_t)LEFT_FRONT_DOOR == g_user_info.config_word) // left front door
        {
            door_cmd.UsageMode = l_u8_rd_LI0_bit2_5();
            door_cmd.VehicleSpeedValid = l_bool_rd_LI0_bit6();

            data[0] = l_u8_rd_LI0_bit8_9();
            data[1] = l_bool_rd_LI0_bit10();
            data[2] = l_bool_rd_LI0_bit11();
            data[3] = l_bool_rd_LI0_bit12();
            /* PRQA S 0499 1 #0499 - Shift count is within range after type promotion (actual type has larger width). */
            data[4] = l_u8_rd_LI0_bit13_20();

            door_cmd.VehicleSpeed = ((data[0] & 0x3u) |
                                     ((data[1] & 0x1u) << 2) |
                                     ((data[2] & 0x1u) << 3) |
                                     ((data[3] & 0x1u) << 4) |
                                     ((data[4] & 0xFFu) << 5));
        }
        else if ((uint8_t)LEFT_REAR_DOOR == g_user_info.config_word) // left rear door
        {
            data[0] = l_bool_rd_LI0_bit6();
            data[1] = l_bool_rd_LI0_bit7();
            data[2] = l_u8_rd_LI0_bit8_9();

            door_cmd.UsageMode = (uint8_t)((data[0] & 0x1u) |
                                           ((data[1] & 0x1u) << 1) |
                                           ((data[2] & 0x3u) << 2));
            door_cmd.VehicleSpeedValid = l_bool_rd_LI0_bit10();
            data[0] = l_bool_rd_LI0_bit11();
            data[1] = l_bool_rd_LI0_bit12();
            /* PRQA S 0499 1 #0499 - Shift count is within range after type promotion (actual type has larger width). */
            data[2] = l_u8_rd_LI0_bit13_20();
            data[3] = l_u8_rd_LI0_bit21_23();
            door_cmd.VehicleSpeed = ((data[0] & 0x1u) |
                                     ((data[1] & 0x1u) << 1) |
                                     ((data[2] & 0x1u) << 2) |
                                     ((data[3] & 0x7u) << 10));
        }
        else if (((uint8_t)RIGHT_FRONT_DOOR == g_user_info.config_word) || ((uint8_t)RIGHT_REAR_DOOR == g_user_info.config_word)) // right front or rear door
        {
            data[0] = l_u8_rd_LI0_bit8_9();
            data[1] = l_bool_rd_LI0_bit10();
            data[2] = l_bool_rd_LI0_bit11();
            door_cmd.UsageMode = (uint8_t)((data[0] & 0x3u) |
                                           ((data[1] & 0x1u) << 2) |
                                           ((data[2] & 0x1u) << 3));
            door_cmd.VehicleSpeedValid = l_bool_rd_LI0_bit12();
            /* PRQA S 0499 1 #0499 - Shift count is within range after type promotion (actual type has larger width). */
            data[0] = l_u8_rd_LI0_bit13_20();
            data[1] = l_u8_rd_LI0_bit21_23();
            data[2] = l_u8_rd_LI0_bit24_25();
            door_cmd.VehicleSpeed = ((data[0] & 0xFFu) |
                                     ((data[1] & 0x7u) << 8) |
                                     ((data[2] & 0x2u) << 11));
        }
        else
        {
            (void)0;
        }
    }
}
/* PRQA S 3469 -- */
/* PRQA S 2985 -- */

/**
 * @description: Receive signals sent by the host and send signals to the host
 * @return [void]
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void App_LinControlMsg(void)
{
    App_LinSendDoorState();

    App_LinReceiveDoorState();
}
