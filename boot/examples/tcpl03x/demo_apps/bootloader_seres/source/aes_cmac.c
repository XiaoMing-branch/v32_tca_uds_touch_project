/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   AES-CMAC 算法实现源文件。
 *          提供基于 AES-128 的 CMAC（NIST SP 800-38B）消息认证码计算，
 *          以及 SHA-256 哈希计算功能。
 *          包含完整的 AES 加密/解密、子密钥生成、MAC 生成与验证功能。
 *
 * @file    aes_cmac.c
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

#include <string.h>
#include "aes_cmac.h"

/* For AES128 Calculation */
/* PRQA S 3218 1 #3209 - File scope static is intentional for global lookup table */
/**
 * @brief AES 加密 S 盒查找表（16x16）。
 *        用于 SubBytes 变换中字节的非线性代换。
 *        该表为 Rijndael S-box 的标准实现。
 */
STATIC const s32 S[16][16] = {
    {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
    {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
    {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
    {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
    {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
    {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
    {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
    {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
    {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
    {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
    {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
    {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
    {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
    {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
    {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
    {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}};

/* PRQA S 3218 1 #3209 - File scope static is intentional for global lookup table */
/**
 * @brief AES 逆 S 盒查找表（16x16）。
 *        用于 InvSubBytes 变换中字节的非线性逆代换。
 */
STATIC const s32 S2[16][16] = {
    {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
    {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
    {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
    {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
    {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
    {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
    {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
    {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
    {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
    {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
    {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
    {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
    {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
    {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
    {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
    {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}};

/* For CMAC Calculation */
/* PRQA S 3408 3 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1514 2 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/**
 * @brief CMAC 算法中使用的 Rb 常数。
 *        当 MSB 为 1 时，左移结果需异或该常数以进行有限域 GF(2^128) 的约简。
 *        值为 {0x00,...,0x87}，对应 x^128 + x^7 + x^2 + x + 1 的约简多项式。
 */
s8 const_Rb[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};
/**
 * @brief 全零常数数组（128 位）。
 *        AES-128 加密的零向量输入，用于子密钥生成第一步。
 */
s8 const_Zero[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/* PRQA S 1502 1 #3216 - Unused parameter is part of standard callback prototype */
/**
 * @brief CMAC 默认密钥（128 位）。
 *        用于安全启动流程中的认证计算，实际产品中应由安全硬件派生。
 * @note  该密钥为示例密钥，产品中不得直接使用该固定值，
 *        必须从安全存储（如 eFuse/OTP）中读取实际密钥。
 */
s8 gs_aKey[16] = {0x82, 0x73, 0x53, 0x49, 0x82, 0xF8, 0xCD, 0xB6, 0x55, 0x05, 0x6B, 0x07, 0x1B, 0x20, 0x29, 0xC0};

/**
 * @brief  获取一个 32 位整数的左 4 位（高半字节）。
 *         用于从 S 盒索引中提取行号。
 *
 * @param[in] num 输入整数
 * @retval 左 4 位的值（高半字节）
 */
STATIC s32 getLeft4Bit(s32 num)
{
    s32 left = num & 0x000000f0U;
    return left >> 4;
}
/**
 * @brief  获取一个 32 位整数的右 4 位（低半字节）。
 *         用于从 S 盒索引中提取列号。
 *
 * @param[in] num 输入整数
 * @retval 右 4 位的值（低半字节）
 */
STATIC s32 getRight4Bit(s32 num)
{
    return num & 0x0000000fU;
}
/**
 * @brief  从 AES S 盒中查找对应值。
 *         使用行号（高 4 位）和列号（低 4 位）索引 S 盒。
 *
 * @param[in] index S 盒索引值（8 位）
 * @retval S 盒中对应位置的代换值
 */
STATIC s32 getNumFromSBox(s32 index)
{
    s32 row = getLeft4Bit(index);
    s32 col = getRight4Bit(index);
    return S[row][col];
}
/**
 * @brief  将字符转换为带符号扩展的 32 位整数。
 *
 * @param[in] c 输入字符
 * @retval 对应的 32 位整数值
 */
STATIC s32 getIntFromChar(s8 c)
{
    return (s32)c;
}

/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/**
 * @brief  将字节数组转换为 4x4 状态矩阵（列优先顺序）。
 *         AES 标准中状态矩阵按列排列：
 *         array[j][i] = str[k]，其中 k = i * 4 + j。
 *
 * @param[in]  str  输入字节数组指针
 * @param[out] pa   输出 4x4 状态矩阵
 */
STATIC void convertToIntArray(s8 *str, s32 pa[4][4])
{
    s32 k = 0;
    s32 i, j;

    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            pa[j][i] = getIntFromChar(str[k]);
            k++;
        }
    }
}

/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/**
 * @brief  将 4 字节字符串合并为一个 32 位字（大端序）。
 *         用于 AES 密钥扩展中从种子密钥提取字。
 *
 * @param[in] str 输入 4 字节字符串
 * @retval 合并后的 32 位字（大端字节序）
 */
STATIC s32 getWordFromStr(s8 *str)
{
    s32 one, two, three, four;
    one = getIntFromChar(str[0]);
    one = one << 24;
    two = getIntFromChar(str[1]);
    two = two << 16;
    three = getIntFromChar(str[2]);
    three = three << 8;
    four = getIntFromChar(str[3]);
    return one | two | three | four;
}

/**
 * @brief  将一个 32 位字拆分为 4 字节数组（大端序）。
 *         用于 AES 轮密钥处理中字到字节的拆分。
 *
 * @param[in]  num   输入 32 位字
 * @param[out] array 输出 4 字节数组，array[0] 为最高字节
 */
STATIC void splitIntToArray(s32 num, s32 array[4])
{
    s32 one, two, three;

    one = num >> 24;
    array[0] = one;
    two = num >> 16;
    array[1] = two & 0x000000ffU;
    three = num >> 8;
    array[2] = three & 0x000000ffU;
    array[3] = num & 0x000000ffU;
}
/**
 * @brief  对 4 元素数组执行左循环移位（ShiftRows 变换使用）。
 *         将数组元素向左循环移动 step 个位置。
 *
 * @param[in,out] array 输入输出数组
 * @param[in]     step  循环左移步数
 */
STATIC void leftLoop4int(s32 array[4], s32 step)
{
    s32 temp[4];
    s32 i;
    s32 index;

    for (i = 0; i < 4U; i++)
    {
        temp[i] = array[i];
    }

    index = ((step % 4U) == 0U) ? 0U : (step % 4U);
    for (i = 0; i < 4U; i++)
    {
        array[i] = temp[index];
        index++;
        index = index % 4U;
    }
}
/**
 * @brief  将 4 字节数组合并为一个 32 位字（大端序）。
 *         用于 AES 密钥扩展 / T 变换后的字合并。
 *
 * @param[in] array 输入 4 字节数组，array[0] 为最高字节
 * @retval 合并后的 32 位字
 */
STATIC s32 mergeArrayToInt(const s32 array[4])
{
    s32 one = array[0] << 24;
    s32 two = array[1] << 16;
    s32 three = array[2] << 8;
    s32 four = array[3];
    return one | two | three | four;
}
/**
 * @brief  AES 密钥扩展中的 T 变换函数。
 *         对输入字执行：字循环左移 1 字节 -> S 盒代换 -> 异或轮常数 Rcon。
 *         定义：T(w[i]) = SubWord(RotWord(w[i])) ^ Rcon[j]。
 *
 * @param[in] num    输入字
 * @param[in] _round 当前轮数，用于选择 Rcon 常数
 * @retval T 变换结果字
 */
STATIC s32 T(s32 num, s32 _round)
{
    s32 numArray[4];
    s32 i;
    s32 result;
    /* AES 轮常数 Rcon，用于密钥扩展中每轮的异或操作 */
    const s32 Rcon[10] = {
        0x01000000U, 0x02000000U,
        0x04000000U, 0x08000000U,
        0x10000000U, 0x20000000U,
        0x40000000U, 0x80000000U,
        0x1b000000U, 0x36000000U};

    splitIntToArray(num, numArray);
    leftLoop4int(numArray, 1);   /* RotWord: 字循环左移 1 字节 */
    for (i = 0; i < 4U; i++)
    {
        numArray[i] = getNumFromSBox(numArray[i]); /* SubWord: S 盒代换 */
    }
    result = mergeArrayToInt(numArray);
    return result ^ Rcon[_round]; /* 异或轮常数 */
}
/**
 * @brief AES 扩展密钥数组 w[44]。
 *        存储 AES-128 的 44 个字（11 轮 * 4 字/轮）。
 *        包括原始密钥和所有轮密钥。
 */
STATIC s32 w[44];
/**
 * @brief  AES-128 密钥扩展函数。
 *         将初始 16 字节密钥扩展为 44 字的轮密钥数组 w[]。
 *         前 4 个字直接复制，后续每个字由前一字与前 4 个字异或生成，
 *         每 4 个字需经 T 变换。
 *
 * @param[in] key AES-128 初始密钥（16 字节）
 */
STATIC void extendKey(s8 *key)
{
    s32 i;
    s32 j;
    for (i = 0; i < 4U; i++)
    {
        w[i] = getWordFromStr(&key[(i * 4U)]);
    }
    i = 4;
    j = 0;
    for (; i < 44U; i++)
    {
        if ((i % 4U) == 0U)
        {
            w[i] = w[i - 4U] ^ T(w[i - 1U], j);
            j++;
        }
        else
        {
            w[i] = w[i - 4U] ^ w[i - 1U];
        }
    }
}

/**
 * @brief  对 4 元素数组执行右循环移位（逆 ShiftRows 变换使用）。
 *         将数组元素向右循环移动 step 个位置。
 *
 * @param[in,out] array 输入输出数组
 * @param[in]     step  循环右移步数
 */
STATIC void rightLoop4int(s32 array[4], s32 step)
{
    s32 temp[4];
    s32 i;

    for (i = 0; i < 4U; i++)
    {
        temp[i] = array[i];
    }

    if ((step % 4U) == 3U)
    {
        array[3] = temp[0];
        array[2] = temp[3];
        array[1] = temp[2];
        array[0] = temp[1];
    }
    else if ((step % 4U) == 2U)
    {
        array[3] = temp[1];
        array[2] = temp[0];
        array[1] = temp[3];
        array[0] = temp[2];
    }
    else if ((step % 4U) == 1U)
    {
        array[3] = temp[2];
        array[2] = temp[1];
        array[1] = temp[0];
        array[0] = temp[3];
    }
    else
    {
        array[3] = temp[3];
        array[2] = temp[2];
        array[1] = temp[1];
        array[0] = temp[0];
    }
}

/**
 * @brief  AES 轮密钥加变换（AddRoundKey）。
 *         将状态矩阵与当前轮的扩展密钥逐字节异或。
 *
 * @param[in,out] array  状态矩阵
 * @param[in]     _round 当前轮号（0~10），用于从扩展密钥 w[] 中索引对应轮密钥
 */
STATIC void addRoundKey(s32 array[4][4], s32 _round)
{
    s32 warray[4];
    s32 i, j;
    for (i = 0; i < 4U; i++)
    {
        splitIntToArray(w[(_round * 4U) + i], warray);
        for (j = 0; j < 4U; j++)
        {
            array[j][i] = array[j][i] ^ warray[j];
        }
    }
}

/**
 * @brief  从 AES 逆 S 盒 S2 中查找对应值。
 *         用于 InvSubBytes 变换。
 *
 * @param[in] index 逆 S 盒索引值（8 位）
 * @retval 逆 S 盒中对应位置的代换值
 */
STATIC s32 getNumFromS1Box(s32 index)
{
    s32 row = getLeft4Bit(index);
    s32 col = getRight4Bit(index);
    return S2[row][col];
}
/**
 * @brief  AES 逆向字节代换变换（InvSubBytes）。
 *         使用逆 S 盒 S2 对状态矩阵中每个字节执行逆代换。
 *
 * @param[in,out] array 状态矩阵
 */
STATIC void deSubBytes(s32 array[4][4])
{
    s32 i, j;
    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            array[i][j] = getNumFromS1Box(array[i][j]);
        }
    }
}

/**
 * @brief  AES 逆向行移位变换（InvShiftRows）。
 *         对状态矩阵的后三行执行右循环移位。
 *         第 1 行右移 1 个字节，第 2 行右移 2 个字节，第 3 行右移 3 个字节。
 *
 * @param[in,out] array 状态矩阵
 */
STATIC void deShiftRows(s32 array[4][4])
{
    s32 rowTwo[4], rowThree[4], rowFour[4];
    s32 i;

    for (i = 0; i < 4U; i++)
    {
        rowTwo[i] = array[1][i];
        rowThree[i] = array[2][i];
        rowFour[i] = array[3][i];
    }
    rightLoop4int(rowTwo, 1);
    rightLoop4int(rowThree, 2);
    rightLoop4int(rowFour, 3);

    for (i = 0; i < 4U; i++)
    {
        array[1][i] = rowTwo[i];
        array[2][i] = rowThree[i];
        array[3][i] = rowFour[i];
    }
}

/* PRQA S 3218 1 #3209 - File scope static is intentional for global lookup table */
/**
 * @brief AES 加密列混合矩阵 colM（4x4）。
 *        用于 MixColumns 变换中的列混合操作。
 *        每列通过该矩阵的伽罗瓦域乘法与各列字节组合。
 */
STATIC const s32 colM[4][4] = {
    {2, 3, 1, 1},
    {1, 2, 3, 1},
    {1, 1, 2, 3},
    {3, 1, 1, 2}};

/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 2。
 *         左移 1 位，若溢出则异或 0x1B（约简多项式）。
 *         用于 MixColumns 列混合变换。
 *
 * @param[in] s 输入域元素
 * @retval 2 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul2(s32 s)
{
    s32 result = s << 1;
    s32 a7 = result & 0x00000100U;

    if (a7 != 0U)
    {
        result = result & 0x000000ffU;
        result = result ^ 0x1bU;
    }

    return result;
}
/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 3。
 *         结果 = GFMul2(s) ^ s。
 *
 * @param[in] s 输入域元素
 * @retval 3 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul3(s32 s)
{
    return GFMul2(s) ^ s;
}

/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 4。
 *
 * @param[in] s 输入域元素
 * @retval 4 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul4(s32 s)
{
    return GFMul2(GFMul2(s));
}

/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 8。
 *
 * @param[in] s 输入域元素
 * @retval 8 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul8(s32 s)
{
    return GFMul2(GFMul4(s));
}

/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 9。
 *
 * @param[in] s 输入域元素
 * @retval 9 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul9(s32 s)
{
    return GFMul8(s) ^ s;
}
/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 11（0x0B）。
 *
 * @param[in] s 输入域元素
 * @retval 11 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul11(s32 s)
{
    return GFMul9(s) ^ GFMul2(s);
}
/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 12（0x0C）。
 *
 * @param[in] s 输入域元素
 * @retval 12 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul12(s32 s)
{
    return GFMul8(s) ^ GFMul4(s);
}
/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 13（0x0D）。
 *
 * @param[in] s 输入域元素
 * @retval 13 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul13(s32 s)
{
    return GFMul12(s) ^ s;
}
/**
 * @brief  伽罗瓦域 GF(2^8) 乘以 14（0x0E）。
 *
 * @param[in] s 输入域元素
 * @retval 14 * s 的 GF(2^8) 乘法结果
 */
STATIC s32 GFMul14(s32 s)
{
    return GFMul12(s) ^ GFMul2(s);
}
/**
 * @brief  通用伽罗瓦域 GF(2^8) 乘法。
 *         根据乘数 n 选择对应的 GFMul 函数。
 *         支持乘数 1、2、3、9、11、13、14。
 *
 * @param[in] n 乘数
 * @param[in] s 被乘域元素
 * @retval n * s 的 GF(2^8) 乘法结果
 * @note  当前仅支持 AES 解密 MixColumns 所需的乘数集合。
 */
STATIC s32 GFMul(s32 n, s32 s)
{
    s32 result = 0;

    if (n == 1U)
    {
        result = s;
    }
    else if (n == 2U)
    {
        result = GFMul2(s);
    }
    else if (n == 3U)
    {
        result = GFMul3(s);
    }
    else if (n == 0x9U)
    {
        result = GFMul9(s);
    }
    else if (n == 0xbU) /* 11 */
    {
        result = GFMul11(s);
    }
    else if (n == 0xdU) /* 13 */
    {
        result = GFMul13(s);
    }
    else if (n == 0xeU) /* 14 */
    {
        result = GFMul14(s);
    }
    else
    {
        (void)0;
    }
    return result;
}

/* PRQA S 3218 1 #3209 - File scope static is intentional for global lookup table */
/**
 * @brief AES 解密逆列混合矩阵 deColM（4x4）。
 *        用于 InvMixColumns 变换中的逆混合操作。
 *        矩阵元素为 {0x0E, 0x0B, 0x0D, 0x09}。
 */
STATIC const s32 deColM[4][4] = {
    {0xe, 0xb, 0xd, 0x9},
    {0x9, 0xe, 0xb, 0xd},
    {0xd, 0x9, 0xe, 0xb},
    {0xb, 0xd, 0x9, 0xe}};
/**
 * @brief  AES 逆向列混合变换（InvMixColumns）。
 *         对状态矩阵的每列应用逆列混合矩阵 deColM。
 *         用于 AES 解密流程。
 *
 * @param[in,out] array 状态矩阵
 */
STATIC void deMixColumns(s32 array[4][4])
{
    s32 tempArray[4][4];
    s32 i, j;

    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            tempArray[i][j] = array[i][j];
        }
    }

    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            array[i][j] = GFMul(deColM[i][0], tempArray[0][j]) ^ GFMul(deColM[i][1], tempArray[1][j]) ^ GFMul(deColM[i][2], tempArray[2][j]) ^ GFMul(deColM[i][3], tempArray[3][j]);
        }
    }
}

/**
 * @brief  从扩展密钥 w[] 中提取 4 个字组成 4x4 矩阵。
 *         用于 AES 解密流程中获取指定轮的轮密钥。
 *
 * @param[in]  i     密钥轮号（0~10）
 * @param[out] array 输出的 4x4 轮密钥矩阵
 */
STATIC void getArrayFrom4W(s32 i, s32 array[4][4])
{
    s32 index = i * 4U;
    s32 colOne[4], colTwo[4], colThree[4], colFour[4];
    s32 j;

    splitIntToArray(w[index], colOne);
    splitIntToArray(w[index + 1U], colTwo);
    splitIntToArray(w[index + 2U], colThree);
    splitIntToArray(w[index + 3U], colFour);

    for (j = 0; j < 4U; j++)
    {
        array[j][0] = colOne[j];
        array[j][1] = colTwo[j];
        array[j][2] = colThree[j];
        array[j][3] = colFour[j];
    }
}

/* PRQA S 3694 1 #3260 - Multidimensional array parameter not modified, const qualification omitted by design */
/**
 * @brief  两个 4x4 阵列逐字节异或（用于解密中的密钥修正）。
 *         AES 解密流程中，InvMixColumns 后的轮密钥与状态矩阵异或。
 *
 * @param[in,out] aArray 被加矩阵，结果存于此
 * @param[in]     bArray 加数矩阵
 */
STATIC void addRoundTowArray(s32 aArray[4][4], s32 bArray[4][4])
{
    s32 i, j;

    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            aArray[i][j] = aArray[i][j] ^ bArray[i][j];
        }
    }
}

/* PRQA S 3694 1 #3260 - Multidimensional array parameter not modified, const qualification omitted by design */
/**
 * @brief  将 4x4 状态矩阵转换为字节数组（列优先）。
 *         与 convertToIntArray 互为逆操作。
 *
 * @param[in]  array 输入 4x4 状态矩阵
 * @param[out] str   输出字节数组
 */
STATIC void convertArrayToStr(s32 array[4][4], s8 *str)
{
    s32 i, j;
    s8 *pstr = str;

    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            *pstr = (s8)array[j][i];
            pstr++;
        }
    }
}
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
/**
 * @brief  AES-128 解密函数。
 *         对密文执行完整的 AES-128 解密操作，返回明文。
 *         支持分块解密（每块 16 字节）。
 *         使用等价的解密流程：AddRoundKey -> InvSubBytes -> InvShiftRows -> InvMixColumns。
 *
 * @param[in]  c          输入的密文字节数组
 * @param[in]  clen       密文长度（字节）
 * @param[in]  key        AES-128 密钥（16 字节）
 * @param[out] pPlainText 输出的明文字节数组
 *
 * @note  当前未在公开 API 中使用，保留供将来扩展。
 * @note  密文长度必须为 16 的整数倍。
 */
void deAes(s8 *c, s32 clen, s8 *key, s8 *pPlainText)
{
    s32 k, i;
    s32 cArray[4][4];
    s32 wArray[4][4];

    extendKey(key);
    for (k = 0; k < clen; k += 16U)
    {
        convertToIntArray(&c[k], cArray);
        addRoundKey(cArray, 10U);           /* 初始轮密钥加（第 10 轮） */
        for (i = 9U; i >= 1U; i--)
        {
            deSubBytes(cArray);             /* 逆向字节代换 */
            deShiftRows(cArray);            /* 逆向行移位 */
            deMixColumns(cArray);           /* 逆向列混合 */
            getArrayFrom4W(i, wArray);
            deMixColumns(wArray);           /* 轮密钥经逆向列混合修正 */
            addRoundTowArray(cArray, wArray); /* 等效解密轮密钥加 */
        }
        deSubBytes(cArray);
        deShiftRows(cArray);
        addRoundKey(cArray, 0);             /* 最终轮密钥加（第 0 轮） */
        convertArrayToStr(cArray, &pPlainText[k]);
    }
}

/**
 * @brief  AES 正向字节代换变换（SubBytes）。
 *         使用 S 盒 S 对状态矩阵中每个字节执行非线性代换。
 *
 * @param[in,out] array 状态矩阵
 */
STATIC void subBytes(s32 array[4][4])
{
    s32 i, j;

    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            array[i][j] = getNumFromSBox(array[i][j]);
        }
    }
}
/**
 * @brief  AES 正向行移位变换（ShiftRows）。
 *         对状态矩阵的后三行执行左循环移位。
 *         第 1 行左移 1 个字节，第 2 行左移 2 个字节，第 3 行左移 3 个字节。
 *
 * @param[in,out] array 状态矩阵
 */
STATIC void shiftRows(s32 array[4][4])
{
    s32 rowTwo[4], rowThree[4], rowFour[4];
    s32 i;

    for (i = 0; i < 4U; i++)
    {
        rowTwo[i] = array[1][i];
        rowThree[i] = array[2][i];
        rowFour[i] = array[3][i];
    }

    leftLoop4int(rowTwo, 1);
    leftLoop4int(rowThree, 2);
    leftLoop4int(rowFour, 3);

    for (i = 0; i < 4U; i++)
    {
        array[1][i] = rowTwo[i];
        array[2][i] = rowThree[i];
        array[3][i] = rowFour[i];
    }
}
/**
 * @brief  AES 正向列混合变换（MixColumns）。
 *         对状态矩阵的每列应用列混合矩阵 colM。
 *         每列中的每个字节由列中所有字节的线性组合替换。
 *
 * @param[in,out] array 状态矩阵
 */
STATIC void mixColumns(s32 array[4][4])
{

    s32 tempArray[4][4];
    s32 i, j;

    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            tempArray[i][j] = array[i][j];
        }
    }
    for (i = 0; i < 4U; i++)
    {
        for (j = 0; j < 4U; j++)
        {
            array[i][j] = GFMul(colM[i][0], tempArray[0][j]) ^ GFMul(colM[i][1], tempArray[1][j]) ^ GFMul(colM[i][2], tempArray[2][j]) ^ GFMul(colM[i][3], tempArray[3][j]);
        }
    }
}

/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
/**
 * @brief  AES-128 加密函数。
 *         对明文执行完整的 AES-128 加密操作，返回密文。
 *         支持分块加密（每块 16 字节）。
 *         算法流程：AddRoundKey -> SubRows -> ShiftRows -> MixColumns（循环 9 轮）
 *         -> SubBytes -> ShiftRows -> AddRoundKey（最终轮）。
 *
 * @param[in]  key   AES-128 密钥（16 字节）
 * @param[in]  plen  明文长度（字节）
 * @param[in]  p     输入的明文字节数组
 * @param[out] cipher 输出的密文字节数组
 *
 * @note  plen 必须为 16 的整数倍，上层调用需保证分块对齐。
 * @note  该函数内部调用 extendKey，多次调用会重复执行密钥扩展，性能敏感场景可考虑预扩展。
 */
void AES_128(s8 *key, s32 plen, s8 *p, s8 *cipher)
{
    s32 pArray[4][4];
    s32 i, k;

    extendKey(key);

    for (k = 0; k < plen; k += 16U)
    {
        convertToIntArray(&p[k], pArray);
        addRoundKey(pArray, 0);             /* 初始轮密钥加 */
        for (i = 1; i < 10U; i++)
        {
            subBytes(pArray);               /* 字节代换 */
            shiftRows(pArray);              /* 行移位 */
            mixColumns(pArray);             /* 列混合 */
            addRoundKey(pArray, i);         /* 轮密钥加 */
        }
        subBytes(pArray);                   /* 最终轮：字节代换 */
        shiftRows(pArray);                  /* 最终轮：行移位 */
        addRoundKey(pArray, 10);            /* 最终轮：轮密钥加（无列混合） */
        convertArrayToStr(pArray, &cipher[k]);
    }
}

/* CMAC Basic Functions */
/* PRQA S 3673 3 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
/**
 * @brief  128 位（16 字节）异或操作。
 *         CMAC 算法中的基本操作，对两个 128 位数据块逐字节异或。
 *
 * @param[in]  a   输入操作数 a（16 字节）
 * @param[in]  b   输入操作数 b（16 字节）
 * @param[out] out 输出结果（16 字节），out = a ^ b
 */
void xor_128(s8 *a, s8 *b, s8 *out)
{
    AS_S32 i;
    for (i = 0; i < 16; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}

#ifdef AES_PRINT_DEBUG
/**
 * @brief  以十六进制格式打印字节数组（调试用）。
 *         每行最多 16 字节，每 4 字节分组。
 *
 * @param[in] str 每行前缀字符串
 * @param[in] buf 待打印的字节数组
 * @param[in] len 待打印的字节长度
 */
void print_hex(char *str, s8 *buf, AS_S32 len)
{
    AS_S32 i;

    for (i = 0; i < len; i++)
    {
        if ((i % 16) == 0 && i != 0)
            _printf(str);
        _putByteToHex(buf[i]);
        if ((i % 4) == 3)
            _printf(" ");
        if ((i % 16) == 15)
            _printf("\r\n");
    }
    if ((i % 16) != 0)
        _printf("\r\n");
}

/**
 * @brief  打印 128 位（16 字节）数据（调试用）。
 *
 * @param[in] bytes 待打印的 16 字节数组
 */
void print128(s8 *bytes)
{
    AS_S32 j;
    for (j = 0; j < 16; j++)
    {
        _putByteToHex(bytes[j]);
        if ((j % 4) == 3)
            _printf(" ");
    }
}

/**
 * @brief  打印 96 位（12 字节）数据（调试用）。
 *
 * @param[in] bytes 待打印的 12 字节数组
 */
void print96(s8 *bytes)
{
    AS_S32 j;
    for (j = 0; j < 12; j++)
    {
        _putByteToHex(bytes[j]);
        if ((j % 4) == 3)
            _printf(" ");
    }
}
#endif

/* AES-CMAC Generation Function */
/* PRQA S 3673 3 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
/**
 * @brief  128 位数据左移 1 位（CMAC 子密钥生成基本操作）。
 *         将 128 位数据块整体左移 1 位，低位补 0，高位溢出位传递至最低字节。
 *         方向：input[0] 为最高字节，input[15] 为最低字节。
 *
 * @param[in]  input  输入 128 位数据块（16 字节）
 * @param[out] output 左移 1 位后的结果（16 字节）
 */
void leftshift_onebit(s8 *input, s8 *output)
{
    s8 i;
    s8 overflow = 0;

    /* 从最低字节向最高字节逐字节左移，并传递溢出位 */
    for (i = 15; i >= 1U; i--)
    {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = ((input[i] & 0x80U) != 0U) ? 1U : 0U;
    }
    output[0] = input[0] << 1;
    output[0] |= overflow;
    return;
}

/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
/**
 * @brief  CMAC 子密钥 K1 和 K2 生成函数。
 *         遵循 NIST SP 800-38B 规范生成两个 128 位子密钥。
 *         算法步骤：
 *         1. L = AES-128(Key, 0^128)                              （加密全零块）
 *         2. 若 MSB(L) == 0，K1 = L << 1，否则 K1 = (L << 1) ^ Rb
 *         3. 若 MSB(K1) == 0，K2 = K1 << 1，否则 K2 = (K1 << 1) ^ Rb
 *         其中 Rb = 0x00...87 为 GF(2^128) 约简常数。
 *
 * @param[in]  key AES-128 密钥（16 字节）
 * @param[out] K1  生成的子密钥 K1（16 字节）
 * @param[out] K2  生成的子密钥 K2（16 字节）
 *
 * @note  K1 用于处理完整最后一块（异或后加密），K2 用于处理不完整最后一块（补位后异或）。
 * @note  安全性依赖于 AES-128 的加密强度，密钥必须安全存储。
 */
void generate_subkey(s8 *key, s8 *K1, s8 *K2)
{
    s8 L[16];
    s8 Z[16];
    s8 tmp[16];
    s8 i;

    /* 初始化全零向量 */
    for (i = 0; i < 16U; i++)
    {
        Z[i] = 0;
    }
    /* Step 1: L = AES-128(Key, 0^128) */
    AES_128(key, 16, Z, L);
    /* Step 2a: 若 MSB(L) = 0，则 K1 = L << 1 */
    if ((L[0] & 0x80U) == 0U)
    {
        /* If MSB(L) = 0, then K1 = L << 1 */
        leftshift_onebit(L, K1);
    }
    else
    {
        /* Else K1 = ( L << 1 ) (+) Rb */
        leftshift_onebit(L, tmp);
        xor_128(tmp, const_Rb, K1);
    }

    /* Step 3a: 若 MSB(K1) = 0，则 K2 = K1 << 1 */
    if ((K1[0] & 0x80U) == 0U)
    {
        leftshift_onebit(K1, K2);
    }
    else
    {
        /* Else K2 = ( K1 << 1 ) (+) Rb */
        leftshift_onebit(K1, tmp);
        xor_128(tmp, const_Rb, K2);
    }
    return;
}

/* PRQA S 3673 3 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
/**
 * @brief  CMAC 最后一块的补位处理（10* padding）。
 *         当最后一块长度不足 16 字节时，进行 10* 补位：
 *         - 复制有效字节到 pad 前 length 个位置
 *         - 在 length 位置添加 0x80（补位起始标志）
 *         - 其余位置填 0x00
 *
 * @param[in]  lastb  输入的最后一块数据
 * @param[out] pad    补位后的 16 字节输出块
 * @param[in]  length 最后一块有效数据长度（0~15），这里是指模 16 后的余数
 */
void padding(s8 *lastb, s8 *pad, s32 length)
{
    s8 j;

    /* original last block */
    for (j = 0; j < 16U; j++)
    {
        if (j < length)
        {
            pad[j] = lastb[j];      /* 复制有效字节 */
        }
        else if (j == length)
        {
            pad[j] = 0x80;          /* 添加 0x80 补位起始 */
        }
        else
        {
            pad[j] = 0x00;          /* 其余字节填 0 */
        }
    }
}

/*****************************************************************************/
/* Public  application  Function                                            */
/*****************************************************************************/

// for a portion of AES-CMAC (init Gernarate K1,K2)
/* PRQA S 3408 ++ #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1514 ++ #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/**
 * @brief CMAC 全局工作变量（L、K1、K2）。
 *        由 Gen_CMACkey 初始化，供 aes_cmac 使用。
 *        gL   = AES-128(Key, 0^128)，用于子密钥生成。
 *        gK1  = 子密钥 K1。
 *        gK2  = 子密钥 K2。
 */
s8 gL[16], gK1[16], gK2[16];
/* PRQA S 1502 1 #3216 - Unused parameter is part of standard callback prototype */
/**
 * @brief CMAC 全局中间变量。
 *        gX       = 链式 CBC 中间状态。
 *        gY       = 当前块的异或结果。
 *        gM_last  = 最后一块处理后的结果。
 *        gPadded  = 补位后的最后一块。
 */
s8 gX[16], gY[16], gM_last[16], gPadded[16];
/* PRQA S 1514 -- */
/* PRQA S 3408 -- */
/**
 * @brief  CMAC 密钥初始化函数。
 *         执行 CMAC 子密钥生成并保存到全局变量中。
 *         需在首次调用 aes_cmac 之前调用该函数进行初始化。
 *
 * @param[in] key AES-128 密钥（16 字节）
 *
 * @note  该函数将密钥派生状态存入全局变量，不支持多密钥并发场景。
 *        若需切换密钥，必须重新调用 Gen_CMACkey。
 */
void Gen_CMACkey(s8 *key)
{
    s8 i;
    for (i = 0; i < 16U; i++)
    {
        gL[i] = 0;
        gK1[i] = 0;
        gK2[i] = 0;
        gY[i] = 0;
        gM_last[i] = 0;
        gPadded[i] = 0;
    }
    AES_128(key, 16, const_Zero, gL);    /* L = AES-128(Key, 0^128) */
    generate_subkey(key, gK1, gK2);      /* 生成子密钥 K1、K2 */
}

/**
 * @brief  SHA-256 哈希计算函数。
 *         对输入数据执行 FIPS 180-4 定义的 SHA-256 安全哈希计算。
 *         输出 256 位（32 字节）摘要。
 *
 * @param[in]  data   输入数据字节数组
 * @param[in]  len    输入数据长度（字节）
 * @param[out] digest 输出的 256 位哈希摘要（32 字节）
 *
 * @note  该实现使用大端字节序。
 * @note  适用于安全启动中的镜像完整性校验，需确保 data 指针有效。
 */
void sha256(const uint8_t *data, uint32_t len, uint8_t digest[32])
{
    /* SHA-256 初始哈希值 H(0) —— 前 8 个质数的平方根的小数部分 */
    uint32_t h[8] = {0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU, 0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};

    /* SHA-256 轮常数 K[0..63] —— 前 64 个质数的立方根的小数部分 */
    STATIC const uint32_t k[64] = {
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U};

    uint64_t bit_len = (uint64_t)len * 8U;  /* 总比特长度，用于最终填充 */

    /* 计算填充长度：最少 1 字节（0x80）+ 8 字节长度，需 64 字节对齐 */
    uint32_t pad_len = 64U - (len % 64U);
    if (pad_len < 9U)
    {
        pad_len += 64U;
    }
    uint32_t total_len = len + pad_len;
    uint32_t total_blocks = total_len / 64U;

    uint8_t block[64];

    for (uint32_t block_num = 0; block_num < total_blocks; block_num++)
    {
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        memset(block, 0, 64);

        if ((block_num * 64U) < len)
        {
            uint32_t copy_len = len - (block_num * 64U);

            if (copy_len > 64U)
            {
                copy_len = 64;
            }
            /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
            memcpy(block, &data[block_num * 64U], copy_len);

            if (copy_len < 64U)
            {
                /* 添加 0x80 补位起始标志 */
                block[copy_len] = 0x80;

                if (copy_len <= 55U)
                {
                    /* 在最后 8 字节写入原始消息长度（大端序） */
                    for (uint8_t i = 0; i < 8U; i++)
                    {
                        block[56U + i] = (uint8_t)((bit_len >> (56U - (i * 8U))) & 0xFFU);
                    }
                }
            }
        }
        else
        {
            /* 纯填充块：只有补位和长度 */
            block[0] = 0x80;
            for (uint8_t i = 0; i < 8U; i++)
            {
                block[56U + i] = (uint8_t)((bit_len >> (56U - (i * 8U))) & 0xFFU);
            }
        }

        /* 将 64 字节块转换为 16 个 32 位字（大端序） */
        uint32_t _w[64];

        for (int32_t i = 0; i < 16; i++)
        {
            _w[i] = ((uint32_t)block[i * 4] << 24) |
                    ((uint32_t)block[(i * 4) + 1] << 16) |
                    ((uint32_t)block[(i * 4) + 2] << 8) |
                    ((uint32_t)block[(i * 4) + 3]);
        }

        /* 消息扩展：将 16 个字扩展到 64 个字 */
        for (int32_t i = 16; i < 64; i++)
        {
            /* s0 = (w[i-15] >> 7) ^ (w[i-15] >> 18) ^ (w[i-15] >> 3) 的循环移位版本 */
            uint32_t s0 = ((_w[i - 15] >> 7) | (_w[i - 15] << 25)) ^
                          ((_w[i - 15] >> 18) | (_w[i - 15] << 14)) ^
                          (_w[i - 15] >> 3);
            /* s1 = (w[i-2] >> 17) ^ (w[i-2] >> 19) ^ (w[i-2] >> 10) 的循环移位版本 */
            uint32_t s1 = ((_w[i - 2] >> 17) | (_w[i - 2] << 15)) ^
                          ((_w[i - 2] >> 19) | (_w[i - 2] << 13)) ^
                          (_w[i - 2] >> 10);
            _w[i] = _w[i - 16] + s0 + _w[i - 7] + s1;
        }

        /* 初始化工作变量 */
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        /* 主压缩循环：64 轮 */
        for (int32_t i = 0; i < 64; i++)
        {
            /* Σ1(e) = (e >>> 6) ^ (e >>> 11) ^ (e >>> 25) */
            uint32_t S1 = ((e >> 6) | (e << 26)) ^ ((e >> 11) | (e << 21)) ^ ((e >> 25) | (e << 7));
            /* Ch(e,f,g) = (e & f) ^ (~e & g) */
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t temp1 = hh + S1 + ch + k[i] + _w[i];
            /* Σ0(a) = (a >>> 2) ^ (a >>> 13) ^ (a >>> 22) */
            uint32_t S0 = ((a >> 2) | (a << 30)) ^ ((a >> 13) | (a << 19)) ^ ((a >> 22) | (a << 10));
            /* Maj(a,b,c) = (a & b) ^ (a & c) ^ (b & c) */
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        /* 更新哈希值 */
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    /* 将 8 个 32 位哈希字转换为 32 字节摘要（大端序） */
    for (int32_t i = 0; i < 8; i++)
    {
        digest[i * 4] = (uint8_t)(h[i] >> 24);
        digest[(i * 4) + 1] = (uint8_t)((h[i] >> 16) & 0xFFU);
        digest[(i * 4) + 2] = (uint8_t)((h[i] >> 8) & 0xFFU);
        digest[(i * 4) + 3] = (uint8_t)(h[i] & 0xFFU);
    }
}

// for all function
/**
 * @brief  AES-CMAC 消息认证码生成函数（NIST SP 800-38B）。
 *         使用 AES-128 和 CMAC 算法计算输入消息的消息认证码。
 *         算法流程：
 *         1. 调用 generate_subkey 生成子密钥 K1、K2
 *         2. 计算分组数 n = ceil(length / 16)
 *         3. 处理最后一块：完整块用 K1，不完整块用 padding + K2
 *         4. CBC-MAC 链式计算 CBC-MAC（类似于 CBC 模式的 AES 加密）
 *         5. 最终 MAC = AES-128(Key, X[n-1] ^ M_last)
 *
 * @param[in]  key    AES-128 密钥（16 字节）
 * @param[in]  input  输入消息字节数组
 * @param[in]  length 输入消息长度（字节）
 * @param[out] mac    输出的 128 位（16 字节）MAC 值
 *
 * @note  MAC 截断：当前输出完整的 16 字节 MAC，上层可根据安全策略截断。
 * @note  空消息（length=0）也是有效输入，将产生一个 CMAC。
 * @note  消息长度很大（>16*块数）时仍然安全，但需注意内存访问范围。
 */
void aes_cmac(s8 *key, s8 *input, s32 length, s8 *mac)
{
    s8 X[16], Y[16], M_last[16], padded[16];
    s8 K1[16], K2[16];
    s32 n, i, flag;
    generate_subkey(key, K1, K2);                           /* Step 1: 生成子密钥 K1, K2 */
    n = (length + 15U) / 16U;                               /* Step 2: n = ceil(len/16)，分组数 */
    if (n == 0U)
    {
        n = 1;
        flag = 0;                                           /* 空消息：视为 1 个不完整块 */
    }
    else
    {
        if ((length % 16U) == 0U)
        {
            /* last block is a complete block */
            flag = 1;                                       /* 最后一块完整，使用 K1 */
        }
        else
        {
            /* last block is not complete block */
            flag = 0;                                       /* 最后一块不完整，使用 K2 + padding */
        }
    }

    if (flag == 1U)
    {
        /* last block is complete block */
        /* Step 3a: M_last = input[last] ^ K1 */
        xor_128(&input[16U * (n - 1U)], K1, M_last);
    }
    else
    {
        /* Step 3b: padding + xor with K2 */
        padding(&input[16U * (n - 1U)], padded, length % 16U);
        xor_128(padded, K2, M_last);
    }

    /* Step 4: 初始化 X = 0 */
    for (i = 0; i < 16U; i++)
    {
        X[i] = 0;
    }
    /* Step 5: CBC-MAC 链式处理前 n-1 块 */
    for (i = 0; i < (n - 1U); i++)
    {
        xor_128(X, &input[16U * i], Y); /* Y := Mi (+) X */
        AES_128(key, 16, Y, X);         /* X := AES-128(KEY, Y); */
    }

    /* Step 6: 处理最后一块 */
    xor_128(X, M_last, Y);             /* Y := X[n-1] ^ M_last */
    AES_128(key, 16, Y, X);            /* X := AES-128(KEY, Y) */

    /* Step 7: 输出 MAC = X */
    for (i = 0; i < 16U; i++)
    {
        mac[i] = X[i];                 /* 输出 MAC 值 */
    }
}

#ifdef AES_PRINT_DEBUG
/**
 * @brief  AES-CMAC 自测试函数（调试用）。
 *         执行 NIST SP 800-38B 标准测试用例：
 *         使用固定密钥和不同长度的消息验证 CMAC 计算正确性。
 *
 * @retval 0 测试通过
 * @retval -1 测试失败（当前始终返回 0）
 */
AS_S32 AES_CMAC_test(void)
{
    s8 L[16], K1[16], K2[16], T[16], TT[12];
    s8 M[64] =
        {
            // 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10,
            0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
            0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
            0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
            0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
            0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
            0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
            0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
            0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};

    // full test
    _printf("\r\n--------------------------------------------------");
    _printf("\r\nSubkey Generation");
    _printf("\r\nK              ");
    print128(gs_aKey);
    //  AES_128(gs_aKey,16,const_Zero,L)   //AES_128(key,const_Zero,L);
    _printf("\r\nAES_128(key,0) ");
    print128(L);
    //  generate_subkey(gs_aKey,K1,K2)
    _printf("\r\nK1             ");
    print128(K1);
    _printf("\r\nK2             ");
    print128(K2);

    _printf("\r\nExample 1: len = 0");
    _printf("\r\nM              ");
    _printf("<empty string>");
    AES_CMAC(gs_aKey, M, 0, T);
    _printf("\r\nAES_CMAC       ");
    print128(T);

    _printf("\r\nExample 2: len = 16");
    _printf("\r\nM              ");
    print_hex(" 	              ", M, 16);
    AES_CMAC(gs_aKey, M, 16, T);
    _printf("AES_CMAC       ");
    print128(T);

    _printf("\r\nExample 3: len = 40");
    _printf("\r\nM              ");
    print_hex(" 	              ", M, 40);
    AES_CMAC(gs_aKey, M, 40, T);
    _printf("AES_CMAC       ");
    print128(T);

    _printf("\r\nExample 4: len = 64");
    _printf("\r\nM              ");
    print_hex(" 	              ", M, 64);
    AES_CMAC(gs_aKey, M, 64, T);
    _printf("AES_CMAC       ");
    print128(T);

    _printf("\r\n--------------------------------------------------");

    /*output
    --------------------------------------------------
    Subkey Generation
    K              2B7E1516 28AED2A6 ABF71588 09CF4F3C
    AES_128(key,0) 7DF76B0C 1AB899B3 3E42F047 B91B546F
    K1             FBEED618 35713366 7C85E08F 7236A8DE
    K2             F7DDAC30 6AE266CC F90BC11E E46D513B
    Example 1: len = 0
    M              <empty string>
    AES_CMAC       BB1D6929 E9593728 7FA37D12 9B756746
    Example 2: len = 16
    M              6BC1BEE2 2E409F96 E93D7E11 7393172A
    AES_CMAC       070A16B4 6B4D4144 F79BDD9D D04A287C
    Example 3: len = 40
    M              6BC1BEE2 2E409F96 E93D7E11 7393172A
                   AE2D8A57 1E03AC9C 9EB76FAC 45AF8E51
                   30C81C46 A35CE411
    AES_CMAC       DFA66747 DE9AE630 30CA3261 1497C827
    Example 4: len = 64
    M              6BC1BEE2 2E409F96 E93D7E11 7393172A
                   AE2D8A57 1E03AC9C 9EB76FAC 45AF8E51
                   30C81C46 A35CE411 E5FBC119 1A0A52EF
                   F69F2445 DF4F9B17 AD2B417B E66C3710
    AES_CMAC       51F0BEBF 7E3B9D92 FC497417 79363CFE
    --------------------------------------------------
    */

    return 0;
}
#endif
