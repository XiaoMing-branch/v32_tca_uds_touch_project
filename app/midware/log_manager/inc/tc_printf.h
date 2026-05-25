/**
 *****************************************************************************
 * @brief   tc_printf header file.
 *
 * @file    tc_printf.h
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

#ifndef __TC_PRINTF_H__
#define __TC_PRINTF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  \n 自动转换为 \r\n
 * @note   设置为 1 时，输出 '\n' 前自动补 '\r'，用于兼容终端换行
 */
#define CONVERT_CR_2_CRLF        1

/**
 * @brief  注册日志输出回调函数
 * @param  func - 输出函数指针（接收 unsigned char 类型）
 */
#define log_out_func(func) log_func = (void(*)(unsigned char))(func)

/** @brief  当前输出函数指针 */
extern void (*log_func)(unsigned char);

/**
 * @brief  输出单个字符
 * @param  c - 待输出的字符
 */
void tc_putc(char c);

/**
 * @brief  输出以 NULL 结尾的字符串
 * @param  str - 字符串指针
 */
void tc_puts(const char *str);

/**
 * @brief  通过指定设备函数输出字符串
 * @param  func - 输出函数指针
 * @param  str  - 字符串指针
 */
void tc_fputs(void (*func)(unsigned char), const char *str);

/**
 * @brief  格式化输出到默认设备
 * @param  fmt - 格式化字符串
 * @param  ... - 可变参数
 */
void tc_printf(const char *fmt, ...);

/**
 * @brief  格式化输出到内存缓冲区
 * @param  buff - 输出缓冲区
 * @param  fmt  - 格式化字符串
 * @param  ...  - 可变参数
 */
void tc_sprintf(char *buff, const char *fmt, ...);

/**
 * @brief  格式化输出到指定设备
 * @param  func - 输出函数指针
 * @param  fmt  - 格式化字符串
 * @param  ...  - 可变参数
 */
void tc_fprintf(void (*func)(unsigned char), const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif /* __TC_PRINTF_H__ */
