/**
 *****************************************************************************
 * @brief   lin lib header file.
 *
 * @file    linlib.h
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

#ifndef __FFF_LINLIB_H__
#define __FFF_LINLIB_H__

#include "fff.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* For negative response */
#define RES_NEGATIVE                      0x7F      /**< negative response */
#define GENERAL_REJECT                    0x10      /**< Error code raised when request for service not supported comes  */
#define SERVICE_NOT_SUPPORTED             0x11      /**< Error code in negative response for not supported service */
#define SUBFUNCTION_NOT_SUPPORTED         0x12      /**< Error code in negative response for not supported subfunction  */

/* Define wildcards */
#define LD_BROADCAST                      0x7F   /**< NAD */
#define LD_FUNCTIONAL_NAD                 0x7E   /**< functional NAD */
#define LD_ANY_SUPPLIER                   0x7FFF /**< Supplier */
#define LD_ANY_FUNCTION                   0xFFFF /**< Function */
#define LD_ANY_MESSAGE                    0xFFFF /**< Message */

/* Identifiers of node read by identifier service */
#define LIN_PRODUCT_IDENT                 0x00   /**< Node product identifier */
#define SERIAL_NUMBER                     0x01   /**< Serial number */

#ifdef __cplusplus
}
#endif
#endif /* __FFF_LINLIB_H__ */
