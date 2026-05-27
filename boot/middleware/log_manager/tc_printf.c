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
void (*log_func)(unsigned char);  /**< 指向当前日志设备的字符输出回调函数 */
static char *outptr;              /**< 指向内存缓冲区的当前写入位置，非空时为 sprintf 模式 */

/**
 * @brief  输出单个字符到日志设备或内存缓冲区
 * @param  c - 待输出的字符
 * @note   若 CONVERT_CR_2_CRLF 使能，'\n' 自动转换为 "\r\n"；
 *         若 outptr 非空则写入内存缓冲区，否则通过 log_func 回调输出
 * @retval 无
 */
void tc_putc(char c)
{
    if (CONVERT_CR_2_CRLF && c == '\n')     /* CONVERT_CR_2_CRLF 使能时将 LF 扩展为 CR+LF */
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
 * @brief  输出以 null 结尾的字符串到日志设备
 * @param  str - 待输出的字符串指针
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
 * @brief  输出字符串到指定设备
 * @param  func - 目标设备的输出回调函数指针
 * @param  str  - 待输出的字符串指针
 * @note   临时切换 log_func 到指定设备，输出完成后恢复原设备
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
 * @brief  核心格式化输出函数（内部调用）
 * @param  fmt - 格式化字符串指针
 * @param  arp - 可变参数列表
 * @note   支持格式：%d/%u（十进制）、%x/%X（十六进制）、%s（字符串）、
 *         %c（字符）、%b（二进制）、%o（八进制）；
 *         支持宽度指定（如 %6d）、左对齐（%-）和零填充（%0）
 * @retval 无
 */
static void tc_vprintf(const char *fmt, va_list arp)
{
    unsigned int r, i, j, w, f;
    unsigned long v;
    char s[16], c, d, *p;

    for (;;)
    {
        c = *fmt++;                 /* Get a char */

        if (!c)
        {
            break;    /* End of format? */
        }

        if (c != '%')               /* Pass through it if not a % sequense */
        {
            tc_putc(c);
            continue;
        }

        f = 0;                      /* f: bit0=零填充, bit1=左对齐, bit2=长整型, bit3=负号 */
        c = *fmt++;                 /* Get first char of the sequense */

        if (c == '0')               /* Flag: '0' padded */
        {
            f = 1;
            c = *fmt++;
        }
        else
        {
            if (c == '-')           /* Flag: left justified */
            {
                f = 2;
                c = *fmt++;
            }
        }

        for (w = 0; c >= '0' && c <= '9'; c = *fmt++)   /* Minimum width */
        {
            w = w * 10 + c - '0';
        }

        if (c == 'l' || c == 'L')   /* Prefix: Size is long int */
        {
            f |= 4;
            c = *fmt++;
        }

        if (!c)
        {
            break;    /* End of format? */
        }

        d = c;

        if (d >= 'a')               /* 将格式字符转为大写以简化 switch 判断 */
        {
            d -= 0x20;
        }

        switch (d)                  /* Type is... */
        {
            case 'S' :                  /* String */
                p = va_arg(arp, char *);

                for (j = 0; p[j]; j++) ;

                while (!(f & 2) && j++ < w)
                {
                    tc_putc(' ');
                }

                tc_puts(p);

                while (j++ < w)
                {
                    tc_putc(' ');
                }

                continue;

            case 'C' :                  /* Character */
                tc_putc((char)va_arg(arp, int));
                continue;

            case 'B' :                  /* Binary */
                r = 2;
                break;

            case 'O' :                  /* Octal */
                r = 8;
                break;

            case 'D' :                  /* Signed decimal */
            case 'U' :                  /* Unsigned decimal */
                r = 10;
                break;

            case 'X' :                  /* Hexdecimal */
                r = 16;
                break;

            default:                    /* Unknown type (passthrough) */
                tc_putc(c);
                continue;
        }

        /* 根据 long 前缀(f&4)或类型(D有符号)获取变参，统一转为 unsigned long */
        v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));

        if (d == 'D' && (v & 0x80000000))    /* 有符号负数：取绝对值并标记负号标志 */
        {
            v = 0 - v;
            f |= 8;
        }

        i = 0;

        do
        {
            d = (char)(v % r);
            v /= r;

            if (d > 9)              /* 10~15 转换为字母 a-f 或 A-F */
            {
                d += (c == 'x') ? 0x27 : 0x07;  /* 小写(x→0x27)或大写(X→0x07)偏移 */
            }

            s[i++] = d + '0';
        }
        while (v && i < sizeof(s));

        if (f & 8)                  /* 负号标志置位时在数字后追加 '-' */
        {
            s[i++] = '-';
        }

        j = i;                      /* 记录数字有效字符数 */
        d = (f & 1) ? '0' : ' ';    /* 填充字符：零填充标志用 '0'，否则用空格 */

        while (!(f & 2) && j++ < w) /* 右对齐时在左侧填充指定字符 */
        {
            tc_putc(d);
        }

        do                          /* 从高位到低位逆序输出数字字符 */
        {
            tc_putc(s[--i]);
        }
        while (i);

        while (j++ < w)             /* 左对齐时在右侧补空格 */
        {
            tc_putc(' ');
        }
    }
}

/**
 * @brief  向默认设备输出格式化字符串
 * @param  fmt - 格式化字符串指针
 * @note   默认设备即 log_func 指定的输出回调
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
 * @brief  向内存缓冲区输出格式化字符串
 * @param  buff - 目标缓冲区指针
 * @param  fmt  - 格式化字符串指针
 * @note   临时切换 outptr 指向 buff，输出完成后追加 '\0' 并恢复
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
 * @brief  向指定设备输出格式化字符串
 * @param  func - 目标设备的输出回调函数指针
 * @param  fmt  - 格式化字符串指针
 * @note   临时切换 log_func 到指定设备，输出完成后恢复原设备
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
