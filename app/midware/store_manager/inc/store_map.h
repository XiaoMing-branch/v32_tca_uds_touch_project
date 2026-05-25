/**
 *****************************************************************************
 * @brief   store map header file.
 *
 * @file    store_map.h
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

#ifndef __STORE_MAP_H__
#define __STORE_MAP_H__
#include "lin_cfg.h"


/** @brief  LIN NAD 快速存储地址（存放临时 LIN NAD 值，用于产线校准） */
#define FAST_LIN_NAD_ADDR                   (0x0000F600UL)

/** @brief  系统参数 Flash 基地址（sector 1，0xFA00） */
#define SYSTEM_PARAM_BASE_ADDR              (0x0000FA00UL)

/** @brief  客户参数 Flash 基地址（sector 0，0xF800） */
#define CUSTOMER_PARAM_BASE_ADDR            (0x0000F800UL)

/** @brief  Flash 写入交换区基地址（sector 2，0xFC00，写入时做暂存区用） */
#define FLASH_SWAP_BASE_ADDR                (0x0000FC00UL)

/* sector 0 内部偏移定义 */

/** @brief  系统参数 CRC 校验值大小（单位：字节） */
#define SYSTEM_PARAM_CRC_SIZE       (4)

/** @brief  系统配置参数在 sector 0 内的偏移 */
#define SYSTEM_CFG_OFFSET           (0)

/** @brief  系统配置参数结构体大小（20 字节数据 + 4 字节 CRC） */
#define SYSTEM_CFG_SIZE             (sizeof(sys_cfg_t))

/** @brief  系统配置参数补齐到 4 字节所需的填充大小 */
#define SYSTEM_CFG_RESEVERD         (4-SYSTEM_CFG_SIZE%4)

/** @brief  LIN 配置参数在 sector 0 内的偏移（紧随系统配置之后） */
#define SYSTEM_ID_CFG_OFFSET        (SYSTEM_CFG_OFFSET + SYSTEM_CFG_SIZE + SYSTEM_PARAM_CRC_SIZE + SYSTEM_CFG_RESEVERD)

/** @brief  LIN 配置参数大小 */
#define SYSTEM_ID_CFG_SIZE          (LIN_SIZE_OF_CFG)

/** @brief  LIN 配置参数补齐到 4 字节所需的填充大小 */
#define SYSTEM_ID_CFG_RESEVERD      (4-SYSTEM_ID_CFG_SIZE%4)

/** @brief  系统参数总大小（含 CRC 和对齐填充） */
#define TOTAL_SYSTEM_PARAM_SIZE     (SYSTEM_ID_CFG_OFFSET+SYSTEM_ID_CFG_SIZE+SYSTEM_ID_CFG_RESEVERD+SYSTEM_PARAM_CRC_SIZE)

#endif
