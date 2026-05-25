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

#include "diagnosticIII.h"
#include "utilities.h"
#include "store_manager.h"
#include "measure.h"
#include "colormixing.h"
#include "version.h"
#include "monitor_manager.h"
#include "led_control.h"

extern cm_led_param_t g_led_param[LED_CHANNEL_MAX];
extern const uint8_t led_type[20];

/**
 * @brief  SID $BA LED配置读取(Boot版本GET)
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   支持多种LED相关参数读取:
 *         - PN结电压/温度数据
 *         - RGB LED参数(色温/色坐标/亮度)
 *         - 白点配置/电流值/PWM参数/RGBL值
 *         - 相对因子/版本信息/UUID/静态PN采样
 *         - LED型号/故障状态/寄存器配置/测试原始数据
 * @retval None
 */
void lin_diag_led_config_get(uint8_t *ptr, uint16_t length)
{
    int16_t typical_temp __attribute__((unused));
    uint16_t duty_value[3] __attribute__((unused));
    monitor_status_e fault_sts __attribute__((unused));
    led_type_e rgb __attribute__((unused));
    led_channel_e channel __attribute__((unused));
    color_rgbl_t color_rgbl __attribute__((unused));
    uint16_t command = ptr[1] << 8 | ptr[2];
    uint16_t data_len = 0;

    switch (command)
    {
        case COMMAND_GET_LED_PN_VOLT:
        {
            channel = ptr[3];
            ptr[1] = (g_analog_signal.temp > 0) ? 0 : 1;
            ptr[2] = (g_analog_signal.temp > 0) ? (g_analog_signal.temp >> 8) : ((-g_analog_signal.temp) >> 8);
            ptr[3] = (g_analog_signal.temp > 0) ? (g_analog_signal.temp & 0xFF) : ((-g_analog_signal.temp) & 0xFF);

            if (meas_pn_sample_status_get(channel))
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    memcpy(&ptr[4 + 2 * i], (uint8_t *)&g_analog_signal.pn_volt[channel][i], sizeof(uint16_t));
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
            channel = ptr[3];
            store_generic_data_get(channel, LED_PN_VOLT_PARAM, (uint8_t *)&ptr[1], LED_TEMP_PN_VOLT_SIZE);
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
            channel = ptr[3];

            for (rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
            {
                ptr[1 + rgb] = g_led_param[channel].color[rgb].led_temp;
                memcpy(&ptr[4 + 4 * rgb], (uint8_t *)&g_led_param[channel].color[rgb].xy, 2 * sizeof(uint16_t));
                memcpy(&ptr[16 + 2 * rgb], (uint8_t *)&g_led_param[channel].color[rgb].intensity, sizeof(uint16_t));
            }

            for (uint8_t i = 0; i < (sizeof(cm_led_param_t) -3); i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[4 + i], sizeof(uint16_t));
            }

            data_len = 21;
        }
        break;

        case COMMAND_GET_WHITE_POINT_CONFIG:
        {
            channel = ptr[3];
            store_generic_data_get(channel, LED_WHITE_COLOR_PARAM, (uint8_t *)&ptr[1], LED_WHITE_COLOR_SIZE);

            for (uint8_t i = 0; i < sizeof(cm_white_point_param_t); i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[1 + i], sizeof(uint16_t));
            }

            data_len = 10;
        }
        break;

        case COMMAND_GET_LED_RGB_CURRENT:
        {
            channel = ptr[3];
            led_ctrl_current_get(channel, &ptr[1]);
            data_len = 3;
        }
        break;

        case COMMAND_GET_PWM_PARAMETER:
        {
            channel = ptr[3] >= LED_CHANNEL_MAX ? LED_CHANNEL_0 : ptr[3];
            led_ctrl_duty_get(channel, duty_value);

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
            channel = ptr[3];
            cm_get_RGBL_value(channel,  &color_rgbl);
            ptr[1] = color_rgbl.r;
            ptr[2] = color_rgbl.g;
            ptr[3] = color_rgbl.b;
            ptr[4] = color_rgbl.level >> 8;
            ptr[5] = color_rgbl.level & 0xFF;
            data_len = 5;
        }
        break;

        case COMMAND_GET_RELATIVE_FACTOR:
        {
            channel = LED_CHANNEL_0;
            store_generic_data_get(channel, LED_RELATIVE_FACTOR_PARAM, (uint8_t *)&ptr[1], LED_RELATIVE_FACTOR_SIZE);
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
            channel = ptr[4];
            ptr[1] = (uint8_t)meas_pn_static_sample_status_get(LED_CHANNEL_0) ||
                     (uint8_t)meas_pn_static_sample_status_get(LED_CHANNEL_1);

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

        case COMMAND_GET_TEST_VALUE:
        {
            data_len = sizeof(analog_signal_t);

            memcpy(&ptr[1], (uint8_t *)&g_analog_signal, data_len);

            for (uint8_t i = 0; i < data_len; i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[1 + i], sizeof(uint16_t));
            }
        }
        break;

        default:
            break;
    }

    lin_diag_positive_notify(ptr[0], &ptr[1], data_len);
}
