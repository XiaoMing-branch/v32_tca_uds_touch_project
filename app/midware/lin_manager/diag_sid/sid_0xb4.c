/**
 *****************************************************************************
 * @brief   lin dianosticiii source file.
 *
 * @file    diagnosticiii.c
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
#include "fff_utilities.h"
#include "fff_measure.h"
#include "fff_colormixing.h"h"
#else
#include "diagnosticIII.h"
#include "utilities.h"
#include "measure.h"
#include "colormixing.h"
#endif

/**
 * @brief  根据数据转储ID获取模拟信号数据
 * @param  id - 数据转储ID
 * @note   支持的ID映射：
 *         - DATA_DUMP_TEMP/VBAT/B_PN/R_PN/G_PN：从g_analog_signal.adc_raw_data[]读取ADC原始值
 *         - DATA_DUMP_V_VBAT：读取VBAT电压值
 *         - DATA_DUMP_V_TEMP：读取温度电压值
 *         - DATA_DUMP_V_B_PN/V_R_PN/V_G_PN：读取RGB各通道PN结电压
 *         默认返回0xFFFF。
 * @retval 读取到的16位数据
 */
static uint16_t GetDataById(uint8_t id)
{
    uint16_t data = 0xffff;

    switch (id)
    {
        case DATA_DUMP_TEMP:
        case DATA_DUMP_VBAT:
        case DATA_DUMP_B_PN:
        case DATA_DUMP_R_PN:
        case DATA_DUMP_G_PN:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.adc_raw_data[id], sizeof(g_analog_signal.adc_raw_data[id]));
            break;

        case DATA_DUMP_V_VBAT:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.vb, sizeof(g_analog_signal.vb));
            break;

        case DATA_DUMP_V_TEMP:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.temp, sizeof(g_analog_signal.temp));
            break;

        case DATA_DUMP_V_B_PN:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.pnVolt[RGB_BLUE], sizeof(g_analog_signal.pnVolt[RGB_BLUE]));
            break;

        case DATA_DUMP_V_R_PN:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.pnVolt[RGB_RED], sizeof(g_analog_signal.pnVolt[RGB_RED]));
            break;

        case DATA_DUMP_V_G_PN:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.pnVolt[RGB_GREEN], sizeof(g_analog_signal.pnVolt[RGB_GREEN]));
            break;
    }

    return data;
}

/**
 * @brief  SID $B4 数据转储控制（DataDump）处理函数
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   方向控制：
 *         - 0x10 = 从机→主机（Slave to Master）：返回指定ID的ADC原始数据（温度、VBAT、PN电压）
 *         - 0x20 = 主机→从机（Master to Slave）：存根，仅返回正响应（留用户扩展）
 *         数据通过endian_swap_func()进行大小端转换后填入响应报文。
 * @retval None (通过 lin_diag_positive_notify / lin_diag_negative_notify 返回)
 */
void lin_diag_data_dump_control(uint8_t *ptr, uint16_t length)
{
    uint16_t data1, data2;

    switch (ptr[1])
    {
        case 0x10 :  /*ptr direction s->m*/
            data1 = GetDataById(ptr[2]);
            data2 = GetDataById(ptr[3]);
            endian_swap_func((uint8_t *)&data1, sizeof(uint16_t));
            endian_swap_func((uint8_t *)&data2, sizeof(uint16_t));
            memcpy(&ptr[2], (uint8_t *)&data1, sizeof(uint16_t));
            memcpy(&ptr[4], (uint8_t *)&data1, sizeof(uint16_t));

            lin_diag_positive_notify(ptr[0], &ptr[1], 5);
            break;

        case 0x20 :  /*ptr direction m->s*/
            /*add user code*/
            lin_diag_positive_notify(ptr[0], &ptr[1], 2);
            break;

        default :
            lin_diag_negative_notify(ptr[0], CNC);
            break;
    }
}
