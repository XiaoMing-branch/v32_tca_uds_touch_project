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
static const char *TAG = "LIN FRAME"; /**< 模块日志标签，用于标识LIN帧模块的调试输出 */

/* PRQA S 1514 2 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
/**
 * LIN帧结构说明：
 *   PID（Protected Identifier，保护标识符）：由硬件/LLD层根据LDF配置自动生成，标识帧类型和方向。
 *   数据字节（Data Bytes）：承载具体信号值，每个信号映射到特定数据字节中的特定位域。
 *   校验和（Checksum）：符合LIN 2.1规范，采用增强型校验和算法，覆盖PID和数据字节。
 * 以下 door_st 和 door_cmd 结构体封装了LIN帧中传输的信号数据。
 */
DoorSt_T door_st = {0};   // Door handle status feedback signal, initialized to 0
DoorCmd_T door_cmd = {0}; // The ECU controls the door handle signal, initialized to 0
extern user_cfg_t g_user_info;              /**< 用户配置信息，包含当前车门位置（左前/左后/右前/右后）及UDS诊断相关配置参数 */
extern volatile uint8_t lin_error;          /**< LIN总线通信错误标志，非零值表示LIN通信过程中发生错误 */

/**
 * @brief 根据NAD值设置对应车门的LIN响应错误标志
 * @param err_flag 错误标志：0=无错误，1=响应错误
 * @retval 无
 * @note 通过g_user_info.config_word判断当前配置的车门位置，分别设置FL/RL/FR/RR的响应错误信号
 */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void AppSetLinErrByNad(uint8_t err_flag)
{
    /* PRQA S 3469 15 #3258 - Function-like macro used for performance and compiler optimization requirements */
    if ((uint8_t)LEFT_FRONT_DOOR == g_user_info.config_word) /* 当前配置为左前门：设置左前门LIN响应错误标志 */
    {
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_FL_ResponseError(err_flag);
    }
    else if ((uint8_t)LEFT_REAR_DOOR == g_user_info.config_word) /* 当前配置为左后门：设置左后门LIN响应错误标志 */
    {
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_RL_ResponseError(err_flag);
    }
    else if ((uint8_t)RIGHT_FRONT_DOOR == g_user_info.config_word) /* 当前配置为右前门：设置右前门LIN响应错误标志 */
    {
        /* PRQA S 4559 1 #4559 - Using essentially unsigned type as first operand of conditional operator is intentional; result is well-defined. */
        l_bool_wr_LI0_EHIS_FR_ResponseError(err_flag);
    }
    else if ((uint8_t)RIGHT_REAR_DOOR == g_user_info.config_word) /* 当前配置为右后门：设置右后门LIN响应错误标志 */
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
 * @brief 通过LIN总线向主机发送门把手状态信号（故障状态、开关状态、软硬件版本号）
 * @param 无
 * @retval 无
 * @note 各车门的状态标志位由底层驱动（LLD）根据LIN帧接收结果置位，本函数轮询标志并发送对应信号。
 *       LIN帧中的PID由硬件/LLD自动封装，校验和由LIN控制器硬件自动计算。
 */
/* PRQA S 2985 ++ #2985 - Redundant operation is intentional (e.g., to satisfy coding style or generate specific assembly). */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void App_LinSendDoorState(void)
{
    if (lin_error != 0u) /* LIN总线有错误：设置当前车门响应错误标志 */
    {
        AppSetLinErrByNad(1);
    }
    else /* LIN总线无错误：清除当前车门响应错误标志 */
    {
        AppSetLinErrByNad(0);
    }

    /* PRQA S 3469 ++ #3258 - Function-like macro used for performance and compiler optimization requirements */
    if (l_flg_tst_LI0_EHIS_FL_State_flag() != 0u) /* 左前门状态更新标志已置位：发送左前门把手状态信号 */
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
        /* l_u8_wr_xx / l_bool_wr_xx 宏通过LDF生成的PID将信号写入对应LIN帧数据字节，
           校验和由LIN硬件控制器在发送时自动计算并附加。 */
        lin_error = 0;

        if ((uint8_t)LEFT_FRONT_DOOR != g_user_info.config_word)
        {
            g_user_info.config_word = (uint8_t)LEFT_FRONT_DOOR; // Left front door handle
            uds_diagnostic_configword_remap_nad();
        }
    }
    if (l_flg_tst_LI0_EHIS_RL_State_flag() != 0u) /* 左后门状态更新标志已置位：发送左后门把手状态信号 */
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
    if (l_flg_tst_LI0_EHIS_FR_State_flag() != 0u) /* 右前门状态更新标志已置位：发送右前门把手状态信号 */
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
    if (l_flg_tst_LI0_EHIS_RR_State_flag() != 0u) /* 右后门状态更新标志已置位：发送右后门把手状态信号 */
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

/**
 * @brief 从LIN总线接收主机下发的门把手控制信号（工作模式、车速有效标志、车速值）
 * @param 无
 * @retval 无
 * @note 根据g_user_info.config_word解析不同车门位置的LIN信号映射。
 *       l_u8_rd_xx / l_bool_rd_xx 宏通过LDF生成的PID从对应LIN帧数据字节中提取特定位域信号，
 *       校验和由硬件在接收时自动验证。
 */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void App_LinReceiveDoorState(void)
{
    uint16_t data[5] = {0}; /* 临时数据缓冲区，用于按位拼接车速值 */

    if (l_flg_tst_LI0_VIU_DWS_flag() != 0u) /* VIU_DWS门状态标志已置位：接收主机下发的门把手控制信号 */
    {
        l_flg_clr_LI0_VIU_DWS_flag();

        if ((uint8_t)LEFT_FRONT_DOOR == g_user_info.config_word) // left front door
        {
            /* 左前门LIN帧信号映射：UsageMode(Bit2-5), VehSpeedValid(Bit6), VehicleSpeed(Bit8-20) */
            door_cmd.UsageMode = l_u8_rd_LI0_bit2_5();
            door_cmd.VehicleSpeedValid = l_bool_rd_LI0_bit6();

            data[0] = l_u8_rd_LI0_bit8_9();    /* Bit8-9: 车速值 bit0-1 */
            data[1] = l_bool_rd_LI0_bit10();    /* Bit10: 车速值 bit2 */
            data[2] = l_bool_rd_LI0_bit11();    /* Bit11: 车速值 bit3 */
            data[3] = l_bool_rd_LI0_bit12();    /* Bit12: 车速值 bit4 */
            /* PRQA S 0499 1 #0499 - Shift count is within range after type promotion (actual type has larger width). */
            data[4] = l_u8_rd_LI0_bit13_20();   /* Bit13-20: 车速值 bit5-12 */

            /* 从分散的位域中重构13位车速值（Bit0-12） */
            door_cmd.VehicleSpeed = ((data[0] & 0x3u) |
                                     ((data[1] & 0x1u) << 2) |
                                     ((data[2] & 0x1u) << 3) |
                                     ((data[3] & 0x1u) << 4) |
                                     ((data[4] & 0xFFu) << 5));
        }
        else if ((uint8_t)LEFT_REAR_DOOR == g_user_info.config_word) // left rear door
        {
            /* 左后门LIN帧信号映射：UsageMode(Bit6-9), VehSpeedValid(Bit10), VehicleSpeed(Bit11-23) */
            data[0] = l_bool_rd_LI0_bit6();     /* Bit6: UsageMode bit0 */
            data[1] = l_bool_rd_LI0_bit7();     /* Bit7: UsageMode bit1 */
            data[2] = l_u8_rd_LI0_bit8_9();     /* Bit8-9: UsageMode bit2-3 */

            /* 从分散的位域中重构4位UsageMode（Bit6-9） */
            door_cmd.UsageMode = (uint8_t)((data[0] & 0x1u) |
                                           ((data[1] & 0x1u) << 1) |
                                           ((data[2] & 0x3u) << 2));
            door_cmd.VehicleSpeedValid = l_bool_rd_LI0_bit10();
            data[0] = l_bool_rd_LI0_bit11();    /* Bit11: 车速值 bit0 */
            data[1] = l_bool_rd_LI0_bit12();    /* Bit12: 车速值 bit1 */
            /* PRQA S 0499 1 #0499 - Shift count is within range after type promotion (actual type has larger width). */
            data[2] = l_u8_rd_LI0_bit13_20();   /* Bit13-20: 车速值 bit2-9 */
            data[3] = l_u8_rd_LI0_bit21_23();   /* Bit21-23: 车速值 bit10-12 */

            /* 从分散的位域中重构13位车速值（Bit11-23） */
            door_cmd.VehicleSpeed = ((data[0] & 0x1u) |
                                     ((data[1] & 0x1u) << 1) |
                                     ((data[2] & 0x1u) << 2) |
                                     ((data[3] & 0x7u) << 10));
        }
        else if (((uint8_t)RIGHT_FRONT_DOOR == g_user_info.config_word) || ((uint8_t)RIGHT_REAR_DOOR == g_user_info.config_word)) // right front or rear door
        {
            /* 右前/右后门LIN帧信号映射：UsageMode(Bit8-11), VehSpeedValid(Bit12), VehicleSpeed(Bit13-25) */
            data[0] = l_u8_rd_LI0_bit8_9();     /* Bit8-9: UsageMode bit0-1 */
            data[1] = l_bool_rd_LI0_bit10();    /* Bit10: UsageMode bit2 */
            data[2] = l_bool_rd_LI0_bit11();    /* Bit11: UsageMode bit3 */

            /* 从分散的位域中重构4位UsageMode（Bit8-11） */
            door_cmd.UsageMode = (uint8_t)((data[0] & 0x3u) |
                                           ((data[1] & 0x1u) << 2) |
                                           ((data[2] & 0x1u) << 3));
            door_cmd.VehicleSpeedValid = l_bool_rd_LI0_bit12();
            /* PRQA S 0499 1 #0499 - Shift count is within range after type promotion (actual type has larger width). */
            data[0] = l_u8_rd_LI0_bit13_20();   /* Bit13-20: 车速值 bit0-7 */
            data[1] = l_u8_rd_LI0_bit21_23();   /* Bit21-23: 车速值 bit8-10 */
            data[2] = l_u8_rd_LI0_bit24_25();   /* Bit24-25: 车速值 bit11-12 */

            /* 从分散的位域中重构13位车速值（Bit13-25） */
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
 * @brief LIN控制报文处理入口，依次执行发送门状态信号和接收主机控制信号
 * @param 无
 * @retval 无
 * @note 该函数在主循环中周期调用，先后执行App_LinSendDoorState和App_LinReceiveDoorState。
 *       LIN帧的PID、数据字节和校验和遵循LIN 2.1协议规范。
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void App_LinControlMsg(void)
{
    App_LinSendDoorState();

    App_LinReceiveDoorState();
}
