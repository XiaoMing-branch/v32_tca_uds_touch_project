/**
*****************************************************************************
* @brief  tc type header
* @file   tc_type.h
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

#ifndef TC_TYPE_H__
#define TC_TYPE_H__

#include <stdint.h>

/**
 * @brief  CPU状态寄存器保存类型
 * @note   用于临界区保护时保存/恢复PRIMASK寄存器状态
 */
typedef uint32_t TC_CPU_SR;

#endif
