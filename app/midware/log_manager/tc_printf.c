/**
 *****************************************************************************
 * @brief   tc_printf source file.
 *
 * @file    tc_printf.c
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

#include <stdarg.h>
#include "tc_printf.h"

/* Pointer to the output stream */
void (*log_func)(unsigned char);
static char *outptr;

/**
 * @brief  输出单个字符
 * @param  c - 待输出的字符
 * @note   若 CONVERT_CR_2_CRLF 使能且字符为 '\n'，则自动先输出 '\r' 实现换行符转换。
 *         支持内存缓冲区模式（outptr 非空时写入缓冲区，否则通过 log_func 回调输出）
 * @retval 无
 */
void tc_putc(char c)
{
    if (CONVERT_CR_2_CRLF && c == '\n')
    {
        tc_putc('\r');    /* CR -> CRLF */
    }

    if (outptr)
    {
        *outptr++ = (unsigned char)c;
        return;
    }

    if (log_func)
    {
        log_func((unsigned char)c);
    }
}

/**
 * @brief  输出以 NULL 结尾的字符串
 * @param  str - 待输出的字符串指针
 * @note   逐字符调用 tc_putc 输出，直到遇到 '\0'
 * @retval 无
 */
void tc_puts(const char *str)
{
    while (*str)
    {
        tc_putc(*str++);
    }
}

/**
 * @brief  通过指定设备函数输出字符串
 * @param  func - 输出函数指针（接收 unsigned char 类型）
 * @param  str  - 待输出的字符串指针
 * @note   临时切换 log_func 为 func，输出完成后恢复原设备
 * @retval 无
 */
void tc_fputs(void(*func)(unsigned char), const char *str)
{
    void (*pf)(unsigned char);

    pf = log_func;     /* Save current output device */
    log_func = func;   /* Switch output to specified device */

    while (*str)        /* Put the string */
    {
        tc_putc(*str++);
    }

    log_func = pf;     /* Restore output device */
}

/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  tc_printf("%d", 1234);            "1234"
    tc_printf("%6d,%3d%%", -200, 5);  "  -200,  5%"
    tc_printf("%-6u", 100);           "100   "
    tc_printf("%ld", 12345678L);      "12345678"
    tc_printf("%04x", 0xA3);          "00a3"
    tc_printf("%08LX", 0x123ABC);     "00123ABC"
    tc_printf("%016b", 0x550F);       "0101010100001111"
    tc_printf("%s", "String");        "String"
    tc_printf("%-4s", "abc");         "abc "
    tc_printf("%4s", "abc");          " abc"
    tc_printf("%c", 'a');             "a"
    tc_printf("%f", 10.0);            <tc_printf lacks floating point support>
*/

/**
 * @brief  核心格式化输出引擎（可变参数处理）
 * @param  fmt - 格式化字符串指针，支持 %d/%u/%x/%X/%s/%c/%b/%o/%ld/%lu 及宽度/填充/对齐
 * @param  arp - 可变参数列表 va_list
 * @note   不支持浮点数格式 %f。支持的格式：
 *         - 标志：0(前导填充零)、-(左对齐)
 *         - 宽度：十进制数字指定最小字段宽度
 *         - 前缀：l/L(长整型)
 *         - 类型：d(有符号十进制)、u(无符号十进制)、x/X(十六进制)、
 *                s(字符串)、c(字符)、b(二进制)、o(八进制)
 * @retval 无
 */
static void tc_vprintf(const char *fmt, va_list arp)
{
    unsigned int r, i, j, w, f;
    unsigned long v;
    char s[16], c, d, *p;

    for (;;)
    {
        c = *fmt++;                 /* 获取下一个字符 */

        if (!c)
        {
            break;    /* 字符串结束 */
        }

        if (c != '%')               /* 非 % 直接原样输出 */
        {
            tc_putc(c);
            continue;
        }

        f = 0;
        c = *fmt++;                 /* 取 % 后的第一个字符 */

        if (c == '0')               /* 标志：前导填充 '0' */
        {
            f = 1;
            c = *fmt++;
        }
        else
        {
            if (c == '-')           /* 标志：左对齐 */
            {
                f = 2;
                c = *fmt++;
            }
        }

        for (w = 0; c >= '0' && c <= '9'; c = *fmt++)   /* 解析最小宽度 */
        {
            w = w * 10 + c - '0';
        }

        if (c == 'l' || c == 'L')   /* 前缀：长整型 long */
        {
            f |= 4;
            c = *fmt++;
        }

        if (!c)
        {
            break;    /* 格式字符串意外结束 */
        }

        d = c;

        if (d >= 'a')               /* 小写转大写，统一处理 */
        {
            d -= 0x20;
        }

        switch (d)                  /* 根据格式类型分发处理 */
        {
            case 'S' :                  /* 字符串 %s */
                p = va_arg(arp, char *);

                for (j = 0; p[j]; j++) ;   /* 计算字符串长度 */

                while (!(f & 2) && j++ < w) /* 右对齐时输出前导空格 */
                {
                    tc_putc(' ');
                }

                tc_puts(p);                 /* 输出字符串本体 */

                while (j++ < w)             /* 左对齐时输出后置空格 */
                {
                    tc_putc(' ');
                }

                continue;

            case 'C' :                  /* 字符 %c */
                tc_putc((char)va_arg(arp, int));
                continue;

            case 'B' :                  /* 二进制 %b */
                r = 2;
                break;

            case 'O' :                  /* 八进制 %o */
                r = 8;
                break;

            case 'D' :                  /* 有符号十进制 %d */
            case 'U' :                  /* 无符号十进制 %u */
                r = 10;
                break;

            case 'X' :                  /* 十六进制 %x/%X */
                r = 16;
                break;

            default:                    /* 未知格式符，原样输出 */
                tc_putc(c);
                continue;
        }

        /* 获取待转换的数值参数 */
        v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));

        if (d == 'D' && (v & 0x80000000))   /* 负数处理：取绝对值并标记负号 */
        {
            v = 0 - v;
            f |= 8;
        }

        i = 0;

        do                                   /* 按进制 r 转换数值为字符串（逆序存放） */
        {
            d = (char)(v % r);
            v /= r;

            if (d > 9)                       /* 十六进制时，10~15 转为 'a'-'f' 或 'A'-'F' */
            {
                d += (c == 'x') ? 0x27 : 0x07;
            }

            s[i++] = d + '0';
        }
        while (v && i < sizeof(s));

        if (f & 8)                           /* 负数标记：添加负号 */
        {
            s[i++] = '-';
        }

        j = i;
        d = (f & 1) ? '0' : ' ';            /* 填充字符：零填充或空格填充 */

        while (!(f & 2) && j++ < w)          /* 右对齐填充 */
        {
            tc_putc(d);
        }

        do                                   /* 输出转换后的数字字符串（正序） */
        {
            tc_putc(s[--i]);
        }
        while (i);

        while (j++ < w)                      /* 左对齐后置填充 */
        {
            tc_putc(' ');
        }
    }
}

/**
 * @brief  格式化输出到默认设备（UART）
 * @param  fmt - 格式化字符串
 * @param  ... - 可变参数，与格式化字符串对应
 * @note   封装 tc_vprintf，使用 log_func 指向的默认输出设备
 * @retval 无
 */
void tc_printf(const char *fmt,  ...)
{
    va_list arp;


    va_start(arp, fmt);
    tc_vprintf(fmt, arp);
    va_end(arp);
}

/**
 * @brief  格式化输出到内存缓冲区（类似 sprintf）
 * @param  buff - 输出缓冲区指针
 * @param  fmt  - 格式化字符串
 * @param  ...  - 可变参数，与格式化字符串对应
 * @note   通过将 outptr 指向 buff 实现从设备输出到内存的切换，
 *         输出完成后自动追加 '\0' 并恢复输出指针
 * @retval 无
 */
void tc_sprintf(char *buff, const char *fmt, ...)
{
    va_list arp;


    outptr = buff;      /* Switch destination for memory */

    va_start(arp, fmt);
    tc_vprintf(fmt, arp);
    va_end(arp);

    *outptr = 0;        /* Terminate output string with a \0 */
    outptr = 0;         /* Switch destination for device */
}

/**
 * @brief  格式化输出到指定设备（类似 fprintf）
 * @param  func - 输出函数指针（接收 unsigned char 类型）
 * @param  fmt  - 格式化字符串
 * @param  ...  - 可变参数，与格式化字符串对应
 * @note   临时切换 log_func 为目标函数，输出完成后恢复原输出设备
 * @retval 无
 */
void tc_fprintf(void(*func)(unsigned char), const char *fmt,  ...)
{
    va_list arp;
    void (*pf)(unsigned char);

    pf = log_func;     /* Save current output device */
    log_func = func;   /* Switch output to specified device */

    va_start(arp, fmt);
    tc_vprintf(fmt, arp);
    va_end(arp);

    log_func = pf;     /* Restore output device */
}
