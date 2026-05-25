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

/********************************************************
** \brief   GetDataById
** \param   uint8_t                    id
** \retval  uint16_t
*********************************************************/
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

/********************************************************
** \brief   lin_diag_data_dump_control
** \param   uint8_t*                    ptr
** \param   uint16_t                    length
** \retval  None
*********************************************************/
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
