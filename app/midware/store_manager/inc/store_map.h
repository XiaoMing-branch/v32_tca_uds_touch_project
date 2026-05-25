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


#define FAST_LIN_NAD_ADDR                   (0x0000F600UL) /*存放临时lin nad*/
#define SYSTEM_PARAM_BASE_ADDR              (0x0000FA00UL) /* sector 1*/
#define CUSTOMER_PARAM_BASE_ADDR            (0x0000F800UL) /* sector 0*/
#define FLASH_SWAP_BASE_ADDR                (0x0000FC00UL) /* sector 2，flash写入时做暂存区用 */

/* sector 0*/
#define SYSTEM_PARAM_CRC_SIZE       (4)
#define SYSTEM_CFG_OFFSET           (0)
#define SYSTEM_CFG_SIZE             (sizeof(sys_cfg_t)) //20 Byte Data + 4Byte CR
#define SYSTEM_CFG_RESEVERD         (4-SYSTEM_CFG_SIZE%4)
#define SYSTEM_ID_CFG_OFFSET        (SYSTEM_CFG_OFFSET + SYSTEM_CFG_SIZE + SYSTEM_PARAM_CRC_SIZE + SYSTEM_CFG_RESEVERD)
#define SYSTEM_ID_CFG_SIZE          (LIN_SIZE_OF_CFG)
#define SYSTEM_ID_CFG_RESEVERD      (4-SYSTEM_ID_CFG_SIZE%4)

#define TOTAL_SYSTEM_PARAM_SIZE     (SYSTEM_ID_CFG_OFFSET+SYSTEM_ID_CFG_SIZE+SYSTEM_ID_CFG_RESEVERD+SYSTEM_PARAM_CRC_SIZE)

#endif
