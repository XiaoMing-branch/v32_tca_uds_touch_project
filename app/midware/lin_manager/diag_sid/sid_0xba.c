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
#include "fff_store_manager.h"
#include "fff_measure.h"
#include "fff_colormixing.h"
#include "fff_version.h"
#include "fff_monitor_manager.h"
#else
#include "diagnosticIII.h"
#include "utilities.h"
#include "store_manager.h"
#include "measure.h"
#include "colormixing.h"
#include "version.h"
#include "monitor_manager.h"
#endif

extern LedCoordinate_t g_led_param;
extern const uint8_t led_type[20];

/********************************************************
** \brief   lin_diag_led_config_get
** \param   uint8_t*                    ptr
** \param   uint16_t                    length
** \retval  None
*********************************************************/
void lin_diag_led_config_get(uint8_t *ptr, uint16_t length)
{
    int16_t typical_temp __attribute__((unused));
    uint16_t duty_value[3] __attribute__((unused));

    monitor_status_e fault_sts __attribute__((unused));
    uint16_t command = ptr[1] << 8 | ptr[2];
    uint16_t data_len = 0;

    switch (command)
    {
        case COMMAND_GET_LED_PN_VOLT:
        {
            ptr[1] = (g_analog_signal.temp > 0) ? 0 : 1;
            ptr[2] = (g_analog_signal.temp > 0) ? (g_analog_signal.temp >> 8) : ((-g_analog_signal.temp) >> 8);
            ptr[3] = (g_analog_signal.temp > 0) ? (g_analog_signal.temp & 0xFF) : ((-g_analog_signal.temp) & 0xFF);

            if (meas_pn_sample_status_get(LED_CHANNLE_0))
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    memcpy(&ptr[4 + 2 * i], (uint8_t *)&g_analog_signal.pnVolt[i], sizeof(uint16_t));
                    endian_swap_func((uint8_t *)&ptr[4 + 2 * i], sizeof(uint16_t));
                }
            }
            else
            {
                memset(&ptr[4], 0x00, 6);
            }

            data_len = 9;
        }
        break;

        case COMMAND_GET_LED_TYPICAL_PN_VOLT:
        {
            store_generic_data_get(LED_CHANNLE_0, LED_PN_VOLT_PARAM, (uint8_t *)&ptr[1], LED_TEMP_PN_VOLT_SIZE);
            memcpy((uint8_t *)&typical_temp, &ptr[1], sizeof(int16_t));
            ptr[1] = (typical_temp > 0) ? 0 : 1;
            ptr[2] = (typical_temp > 0) ? typical_temp : (-typical_temp);

            for (uint8_t i = 0; i < 3; i++)
            {
                endian_swap_func((uint8_t *)&ptr[3 + 2 * i], sizeof(uint16_t));
            }

            data_len = 8;
        }
        break;

        case COMMAND_GET_LED_RGB_PARAM:
        {
            ptr[1] = g_led_param.red.temperature;
            ptr[2] = g_led_param.green.temperature;
            ptr[3] = g_led_param.blue.temperature;

            memcpy(&ptr[4], (uint8_t *)&g_led_param.red.x, sizeof(uint16_t));
            memcpy(&ptr[6], (uint8_t *)&g_led_param.red.y, sizeof(uint16_t));
            memcpy(&ptr[8], (uint8_t *)&g_led_param.green.x, sizeof(uint16_t));
            memcpy(&ptr[10], (uint8_t *)&g_led_param.green.y, sizeof(uint16_t));
            memcpy(&ptr[12], (uint8_t *)&g_led_param.blue.x, sizeof(uint16_t));
            memcpy(&ptr[14], (uint8_t *)&g_led_param.blue.y, sizeof(uint16_t));
            memcpy(&ptr[16], (uint8_t *)&g_led_param.red.intensity, sizeof(uint16_t));
            memcpy(&ptr[18], (uint8_t *)&g_led_param.green.intensity, sizeof(uint16_t));
            memcpy(&ptr[20], (uint8_t *)&g_led_param.blue.intensity, sizeof(uint16_t));

            for (uint8_t i = 0; i < (sizeof(LedCoordinate_t) -3); i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[4 + i], sizeof(uint16_t));
            }

            data_len = 21;
        }
        break;

        case COMMAND_GET_WHITE_POINT_CONFIG:
        {
            store_generic_data_get(LED_CHANNLE_0, LED_WHITE_COLOR_PARAM, (uint8_t *)&ptr[1], LED_WHITE_COLOR_SIZE);

            for (uint8_t i = 0; i < sizeof(CommLedGeneralParam_t); i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[1 + i], sizeof(uint16_t));
            }

            data_len = 10;
        }
        break;

        case COMMAND_GET_LED_RGB_CURRENT:
        {
            pal_led_current_get(LED_CHANNLE_0, &ptr[1]);
            ptr[3] = ptr[2] = ptr[1];
            data_len = 3;
        }
        break;

        case COMMAND_GET_PWM_PARAMETER:
        {
            pal_led_dutcycle_get(LED_CHANNLE_0, duty_value);

            for (uint8_t i = 0; i < 3; i++)
            {
                endian_swap_func((uint8_t *)&duty_value[i], sizeof(uint16_t));
            }

            memcpy(&ptr[1], (uint8_t *)duty_value, 3 * sizeof(int16_t));

            data_len = 6;
        }
        break;

        case COMMAND_GET_RGBL_PARAMETER:
        {
            cm_get_RGBL_value(&ptr[1], &ptr[2], &ptr[3], (uint16_t *)&ptr[4]);
            endian_swap_func((uint8_t *)&ptr[4], sizeof(uint16_t));

            data_len = 5;
        }
        break;

        case COMMAND_GET_RELATIVE_FACTOR:
        {
            store_generic_data_get(LED_CHANNLE_0, LED_RELATIVE_FACTOR_PARAM, (uint8_t *)&ptr[1], LED_RELATIVE_FACTOR_SIZE);
            endian_swap_func((uint8_t *)&ptr[1], sizeof(uint32_t));

            data_len = 4;
        }
        break;

        case COMMAND_GET_VERSION_INFO:
        {

            ptr[1] = APP_VER & 0xFF;
            ptr[2] = (APP_VER >> 8) & 0xFF;
            ptr[3] = (APP_VER >> 16) & 0xFF;
            ptr[4] = (APP_VER >> 24) & 0xFF;
            endian_swap_func((uint8_t *)&ptr[1], sizeof(uint32_t));

            ptr[5] = YEAR_MONTH_DAY & 0xFF;
            ptr[6] = (YEAR_MONTH_DAY >> 8) & 0xFF;
            ptr[7] = (YEAR_MONTH_DAY >> 16) & 0xFF;
            ptr[8] = (YEAR_MONTH_DAY >> 24) & 0xFF;
            endian_swap_func((uint8_t *)&ptr[5], sizeof(uint32_t));

            ptr[9] = HOUR_MIN_SEC & 0xFF;
            ptr[10] = (HOUR_MIN_SEC >> 8) & 0xFF;
            ptr[11] = (HOUR_MIN_SEC >> 16) & 0xFF;
            ptr[12] = (HOUR_MIN_SEC >> 24) & 0xFF;
            endian_swap_func((uint8_t *)&ptr[9], sizeof(uint32_t));

            store_chip_info_get(CHIP_INFO_BOOT_VER, &ptr[13], sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[13], sizeof(uint32_t));

            data_len = 16;
        }
        break;

        case COMMAND_GET_UUID:
        {
            store_chip_info_get(CHIP_INFO_UUID, &ptr[1], 3 * sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[1], sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[5], sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[9], sizeof(uint32_t));
            data_len = 12;
        }
        break;

        case COMMAND_GET_STATIC_PN_SAMPLE:
        {
            ptr[1] = (uint8_t)meas_pn_static_sample_status_get(LED_CHANNLE_0);

            data_len = 1;
        }
        break;

        case COMMAND_GET_LED_VENDOR_INFO:
        {

            for (uint8_t i = 0; i < 20; i++)
            {
                ptr[i + 1] = led_type[i];
            }

            data_len = 20;
        }
        break;

        case COMMAND_GET_LED_STATUS:
        {
            fault_sts = (monitor_status_e)monitor_detect_status_get();

            ptr[1] = fault_sts >> 8;
            ptr[2] = fault_sts;

            data_len = 2;
        }
        break;

        case COMMAND_GET_REG_CFG:
        {
            endian_swap_func((uint8_t *)&ptr[3],  sizeof(uint32_t));
            store_reg_param_get(&ptr[3], &ptr[1]);
            endian_swap_func((uint8_t *)&ptr[1],  sizeof(uint32_t));

            data_len = 4;
        }
        break;

        case COMMAND_GET_TEST_VAULE:
        {
            data_len = sizeof(analog_signal_t);

            memcpy(&ptr[1], (uint8_t *)&g_analog_signal, data_len);

            for (uint8_t i = 0; i < data_len; i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[1 + i], sizeof(uint16_t));
            }
        }

        default:
            break;
    }

    lin_diag_positive_notify(ptr[0], &ptr[1], data_len);
}

