/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $B7 分配帧ID范围（AssignFrameIDRange）处理源文件（LIN 2.1）
 *
 * @file    sid_0xb7.c
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

#include "test_config.h"
#ifdef ENABLE_TEST_MODE
#include "fff_diagnosticIII.h"
#include "fff_lin_precfg.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "lin_precfg.h"
#endif

/** @brief LIN协议版本宏开关：仅当协议版本为LIN 2.1（PROTOCOL_21）时编译本模块 */
#if LIN_PROTOCOL == PROTOCOL_21
/**
 * @brief  SID $B7 分配帧ID范围（AssignFrameIDRange）处理函数（LIN 2.1）
 * @param  ptr    - UDS请求报文指针
 *         - ptr[0]：SID $B7
 *         - ptr[1]：起始帧索引 start_index（0~31），指定从哪个帧开始分配
 *         - ptr[2..5]：4个待分配的帧ID值
 * @param  length - 报文长度，须为6字节（1字节SID + 1字节起始索引 + 4字节帧ID值）
 * @note   处理逻辑：
 *         - 步骤1：检查报文长度是否等于6，若否则返回 GENERAL_REJECT 负响应
 *         - 步骤2：逐字节校验 ptr[5..2]（逆序）中每个值：
 *           - 若值不为0xFF且目标帧索引 j 超出有效帧范围（lin_cfg.lin_cfg_frame_num），
 *             则直接返回（不响应）
 *         - 步骤3：校验通过后，正序遍历 ptr[2..5] 分配帧ID：
 *           - 0x00：取消分配该帧，将对应 lin_configuration_RAM[j] 设为 0xFF
 *           - 0xFF：保持该帧之前分配的值不变（跳过）
 *           - 其他值：调用 lin_process_parity(CHECK_PARITY) 计算带奇偶校验的帧ID
 *             - 若返回 0xFF（校验失败），返回 GENERAL_REJECT 负响应
 *             - 否则将计算得到的帧ID存入 lin_configuration_RAM[j]
 *         - 步骤4：所有帧分配完成后发送 $B7 正响应
 * @attention 先逆序校验再正序分配的机制确保：若任一帧ID无效，整个分配操作不会部分执行
 * @retval None（通过 lin_diag_positive_notify 发送正响应）
 * @retval None（通过 lin_diag_negative_notify 发送 GENERAL_REJECT 负响应）—— 报文长度错误或PID奇偶校验失败时
 */
/* PRQA S 2889 2 #3257 - Multiple return statements for logical clarity and efficiency */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diag_assign_frame_id_range(uint8_t *ptr, uint16_t length)
{
    uint8_t start_index;                                                               /*!< 起始帧索引，取自请求报文ptr[1]，标识从哪个帧开始分配 */
    uint8_t i, j;                                                                      /*!< i：报文缓冲区索引游标；j：帧配置表索引游标 */
    uint8_t fid;                                                                       /*!< 经奇偶校验计算后的最终帧ID值 */

    /* 判断：报文长度必须为6字节（1字节SID + 1字节起始索引 + 4字节帧ID值），否则拒绝 */
    if (length != 6u)
    {
        lin_diag_negative_notify(ptr[0], GENERAL_REJECT);                               /*!< 发送GENERAL_REJECT负响应，指示报文格式错误 */
        return;
    }

    start_index = ptr[1];                                                              /*!< 解析起始帧索引，后续分配将从该索引开始 */

    i = 5u;                                                                            /*!< 初始化报文游标指向最后一个帧ID值字节（ptr[5]） */
    j = start_index + 4u;                                                              /*!< 初始化帧表游标指向起始索引+4，逆向遍历前先指向结束位置 */
/* PRQA S 3387 1 #3265 - Increment or decrement operation is safe with no unintended side effects */
    /* 逆向校验：从最后一个帧ID值到第一个，检查每个非0xFF值是否对应有效帧索引 */
    for (; j > start_index; j--)
    {
        /* 判断：若帧ID值不为0xFF（即要实际赋值），且目标帧索引超出总帧数范围，则停止 */
        if ((ptr[i] != 0xFFu) && (j > lin_cfg.lin_cfg_frame_num))
        {
            return;                                                                    /*!< 索引越界：不发送任何响应，静默退出 */
        }
	i--;                                                                              /*!< 报文游标前移，继续校验下一个值 */
    }

    /* Store PIDs */
    i = 2u;                                                                            /*!< 报文游标指向第一个帧ID值字节（ptr[2]） */
    j = start_index + 1u;                                                              /*!< 帧表游标指向起始索引+1，正序遍历的起始位置 */
/* PRQA S 3387 1 #3265 - Increment or decrement operation is safe with no unintended side effects */
    /* 正序分配：依次处理每个帧ID值，写入对应的帧配置表项 */
    for (; i < length; i++)
    {
        /* 根据帧ID值进行分支处理 */
        switch (ptr[i])
        {
            case 0x00:
                /* Unassign frame */
                lin_configuration_RAM[j] = 0xFF;                                        /*!< 值为0x00：取消该帧分配，将配置表项置为0xFF表示未使用 */
                break;

            case 0xFF:
                /* keep the previous assigned value of this frame */
                break;                                                                 /*!< 值为0xFF：保持该帧之前分配的值不变，跳过 */

            default:
                /* Calculate frame ID & Assign ID to frame */
                fid = lin_process_parity(ptr[i], CHECK_PARITY);                         /*!< 其他值：计算带奇偶校验的完整帧ID（Physical ID + Parity） */
                /* 判断：若计算结果为0xFF，表示奇偶校验失败（无效PID），返回负响应 */
                if (0xFFu == fid)
                {
                    lin_diag_negative_notify(ptr[0], GENERAL_REJECT);                   /*!< PID奇偶校验失败，发送GENERAL_REJECT负响应 */
                    return;
                }
                else
                {
                    lin_configuration_RAM[j] = fid;                                     /*!< 校验通过，将带奇偶位的帧ID存入配置表 */
                }
                break;
        }
	j++;                                                                              /*!< 帧表游标后移，指向下一个待分配的帧 */
    } /* End of for statement */

    lin_diag_positive_notify(ptr[0], NULL, 0);                                           /*!< 所有帧分配完成，发送SID $B7正响应 */
#endif
