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

/**
 * @brief  0xBA SID - 读取LED/设备参数配置（Customer LED Configuration GET）
 *         根据命令字段(command)读取LED PN电压、RGB色坐标、白点配置、PWM参数、
 *         版本信息、UUID、芯片寄存器配置、测试值等
 * @param  ptr - UDS请求报文指针（ptr[1]<<8|ptr[2]为子命令）
 * @param  length - 报文长度
 * @note   解析ptr中的命令字段，通过switch-case分发处理各个GET子命令，
 *         最终调用lin_diag_positive_notify()返回数据
 * @retval None
 */
void lin_diag_led_config_get(uint8_t *ptr, uint16_t length)
{
    int16_t typical_temp __attribute__((unused));
    uint16_t duty_value[3] __attribute__((unused));

    monitor_status_e fault_sts __attribute__((unused));
    uint16_t command = ptr[1] << 8 | ptr[2];
    uint16_t data_len = 0;

    switch (command)
    {
        /**
         * @brief  子命令: 读取LED PN电压和温度采样
         * @note   返回: 温度符号(ptr[1], 0正1负) + 温度值(ptr[2..3]) + 3路PN电压(ptr[4..9])
         *         PN电压仅当meas_pn_sample_status_get()采样有效时填充，否则填0
         */
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

        /**
         * @brief  子命令: 读取LED典型PN电压（从存储区加载）
         * @note   从store_generic_data_get()读取存储的PN电压典型值，
         *         温度符号(ptr[1]) + 温度值(ptr[2]) + 3路PN电压大端转换后返回
         */
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

        /**
         * @brief  子命令: 读取LED RGB色坐标和亮度参数
         * @note   返回: 3通道色温(ptr[1..3]) + 6个色坐标xy(ptr[4..15], uint16) +
         *         3通道亮度intensity(ptr[16..21], uint16)
         *         所有uint16数据需大端转换后组包返回
         */
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

        /**
         * @brief  子命令: 读取白点配置参数
         * @note   从store_generic_data_get()加载LED_WHITE_COLOR_PARAM，
         *         所有uint16字段经大端转换后返回，数据长度LED_WHITE_COLOR_SIZE
         */
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

        /**
         * @brief  子命令: 读取LED RGB电流设置值
         * @note   调用pal_led_current_get()获取LED电流值，3通道返回相同值
         */
        case COMMAND_GET_LED_RGB_CURRENT:
        {
            pal_led_current_get(LED_CHANNLE_0, &ptr[1]);
            ptr[3] = ptr[2] = ptr[1];
            data_len = 3;
        }
        break;

        /**
         * @brief  子命令: 读取PWM占空比参数
         * @note   调用pal_led_dutcycle_get()获取3路PWM占空比，
         *         经大端转换后拷贝到响应缓冲区返回
         */
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

        /**
         * @brief  子命令: 读取RGBL计算参数
         * @note   调用cm_get_RGBL_value()获取R/G/B/L值，L亮度经大端转换后返回
         */
        case COMMAND_GET_RGBL_PARAMETER:
        {
            cm_get_RGBL_value(&ptr[1], &ptr[2], &ptr[3], (uint16_t *)&ptr[4]);
            endian_swap_func((uint8_t *)&ptr[4], sizeof(uint16_t));

            data_len = 5;
        }
        break;

        /**
         * @brief  子命令: 读取相对亮度系数
         * @note   从store_generic_data_get()加载LED_RELATIVE_FACTOR_PARAM，
         *         uint32数据经大端转换后返回
         */
        case COMMAND_GET_RELATIVE_FACTOR:
        {
            store_generic_data_get(LED_CHANNLE_0, LED_RELATIVE_FACTOR_PARAM, (uint8_t *)&ptr[1], LED_RELATIVE_FACTOR_SIZE);
            endian_swap_func((uint8_t *)&ptr[1], sizeof(uint32_t));

            data_len = 4;
        }
        break;

        /**
         * @brief  子命令: 读取固件版本信息
         * @note   返回: APP版本(4B) + 编译日期(4B, YYYYMMDD) + 编译时间(4B, HHMMSS) +
         *         Bootloader版本(4B)，所有uint32经大端转换
         */
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

        /**
         * @brief  子命令: 读取芯片UUID
         * @note   调用store_chip_info_get()获取3×uint32 UUID，
         *         每个uint32分别经大端转换后返回
         */
        case COMMAND_GET_UUID:
        {
            store_chip_info_get(CHIP_INFO_UUID, &ptr[1], 3 * sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[1], sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[5], sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[9], sizeof(uint32_t));
            data_len = 12;
        }
        break;

        /**
         * @brief  子命令: 读取静态PN采样状态
         * @note   调用meas_pn_static_sample_status_get()获取LED静态PN采样状态标识
         */
        case COMMAND_GET_STATIC_PN_SAMPLE:
        {
            ptr[1] = (uint8_t)meas_pn_static_sample_status_get(LED_CHANNLE_0);

            data_len = 1;
        }
        break;

        /**
         * @brief  子命令: 读取LED厂商信息
         * @note   返回led_type[20]数组，包含LED型号/厂商等预配置信息
         */
        case COMMAND_GET_LED_VENDOR_INFO:
        {

            for (uint8_t i = 0; i < 20; i++)
            {
                ptr[i + 1] = led_type[i];
            }

            data_len = 20;
        }
        break;

        /**
         * @brief  子命令: 读取LED故障状态
         * @note   调用monitor_detect_status_get()获取故障状态字，高字节在前返回
         */
        case COMMAND_GET_LED_STATUS:
        {
            fault_sts = (monitor_status_e)monitor_detect_status_get();

            ptr[1] = fault_sts >> 8;
            ptr[2] = fault_sts;

            data_len = 2;
        }
        break;

        /**
         * @brief  子命令: 读取寄存器配置
         * @note   从请求中提取寄存器地址(ptr[3..6])，经大端转换后调用store_reg_param_get()读取，
         *         结果再次大端转换后返回
         */
        case COMMAND_GET_REG_CFG:
        {
            endian_swap_func((uint8_t *)&ptr[3],  sizeof(uint32_t));
            store_reg_param_get(&ptr[3], &ptr[1]);
            endian_swap_func((uint8_t *)&ptr[1],  sizeof(uint32_t));

            data_len = 4;
        }
        break;

        /**
         * @brief  子命令: 读取全部模拟信号测试值
         * @note   直接拷贝g_analog_signal结构体(analog_signal_t)到响应缓冲区，
         *         所有uint16数据经大端转换后返回
         * @warning 本case缺少break语句，执行后会落入default分支
         */
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

