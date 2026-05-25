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
 * @brief  根据数据ID获取内部测量数据值
 * @param  id - 数据ID(DATA_DUMP_TEMP/VBAT/B_PN/R_PN/G_PN/V_VBAT等)
 * @note   从g_analog_signal全局结构中读取指定ADC原始数据或计算值
 *         支持温度、电池电压、RGB PN结电压等多种测量数据
 *         返回16位无符号数据(大小端由调用方处理)
 * @retval 对应ID的测量数据值; 无效ID返回0xFFFF
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
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.pn_volt[LED_CHANNEL_0][LED_B], sizeof(g_analog_signal.pn_volt[LED_CHANNEL_0][LED_B]));
            break;

        case DATA_DUMP_V_R_PN:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.pn_volt[LED_CHANNEL_0][LED_R], sizeof(g_analog_signal.pn_volt[LED_CHANNEL_0][LED_R]));
            break;

        case DATA_DUMP_V_G_PN:
            memcpy((uint8_t *)&data, (uint8_t *)&g_analog_signal.pn_volt[LED_CHANNEL_0][LED_G], sizeof(g_analog_signal.pn_volt[LED_CHANNEL_0][LED_G]));
            break;
    }

    return data;
}

/**
 * @brief  SID $B4 DataDump数据转储控制
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   子功能0x10(Slave->Master): 读取ptr[2]和ptr[3]指定的两个数据ID值
 *         进行大小端转换后填入响应报文, 返回5字节数据
 *         子功能0x20(Master->Slave): 仅返回正响应(用户自定义扩展)
 *         其他子功能返回CNC(条件不满足)
 * @retval None
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
