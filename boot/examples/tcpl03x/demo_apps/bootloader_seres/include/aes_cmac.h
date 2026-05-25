/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   aes cmac header file.
 *
 * @file    aes_cmac.h
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

/* PRQA S 1534 ++ #3261 - Unused macro defined for future extension and configuration compatibility */
#ifndef AES_CMAC_H
#define AES_CMAC_H
/*****************************************************************************/
#include "test_config.h"
#ifdef ENABLE_TEST_MODE
#include "fff_system_tcpl03x.h"
#else
#include "system_tcpl03x.h"
#endif
/* PRQA S 1535 13 #3262 - Unused typedef defined for future extension and type consistency */
typedef unsigned long int s32;
typedef signed short int s16;
typedef unsigned char s8;
typedef unsigned long int u32;
typedef unsigned short int u16;
typedef unsigned char u8;

typedef unsigned long int AS_U32;
typedef unsigned short int AS_U16;
typedef unsigned char AS_U08;
typedef signed long int AS_S32;
typedef signed short int AS_S16;
typedef signed char AS_S08;
typedef unsigned char AS_BOOL;

#define AS_NULL ((AS_BOOL)0)
#define AS_TRUE ((AS_BOOL)1)
#define AS_FALSE ((AS_BOOL)0)

#define AS_ENABLE ((AS_BOOL)1)
#define AS_DISABLE ((AS_BOOL)0)

/* MACRO ------------------------------------------------------------*/
#define AS_HINIBBLE(x) ((u8)(x >> 4))
#define AS_LONIBBLE(x) ((u8)(x & 0x0f))

#define AS_LOBYTE(x) ((u8)(x & 0xff))
#define AS_HIBYTE(x) ((u8)((x >> 8) & 0xff))

#define AS_MARKHIBYTE(x) ((u8)((x >> 4) & 0x0f))

#define AS_LOWORD(x) ((u16)(x & 0xffff))
#define AS_HIWORD(x) ((u16)((x >> 16) & 0xffff))
#define AS_MAKEWORD(msb, lsb) ((u16)(((u16)msb) << 8) | lsb)
#define AS_MAKEDWORD(msb, lsb) ((u32)(((u32)msb) << 16) | lsb)
#define AS_MAKE4BtoDWORD(msb, b2, b1, lsb) ((u32)(((u32)msb << 24) | ((u32)b2 << 16) | ((u32)b1 << 8) | lsb))

#define AS_MAKEWORDDHIBYTE(x) ((u8)((x >> 24) & 0x000000ff))
#define AS_MAKEDWORDMIDHIBYTE(x) ((u8)((x >> 16) & 0x000000ff))
#define AS_MAKEDWORDMIDLOBYTE(x) ((u8)((x >> 8) & 0x000000ff))
#define AS_MAKEDWORDLOBYTE(x) ((u8)(x & 0x000000ff))

/*****************************************************************************/
extern s8 gs_aKey[16];
extern void aes_cmac(s8 *key, s8 *input, s32 length, s8 *mac);
extern void Gen_CMACkey(s8 *key);
extern void sha256(const uint8_t *data, uint32_t len, uint8_t digest[32]);
#endif /*_AES-CMAC_H */
/* PRQA S 1534 -- */
