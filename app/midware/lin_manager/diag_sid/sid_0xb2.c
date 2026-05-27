/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   LIN诊断配置服务 - SID $B2 通过标识符读取数据（ReadByIdentifier）处理源文件
 *
 * @file    sid_0xb2.c
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
#include "fff_tc_log.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "tc_log.h"
#endif

/* PRQA S 1534 1 #3261 - Unused macro defined for future extension and configuration compatibility */
#define LOG_DIAG(...)  //do{log_debug("[DIAG_0xB2] " __VA_ARGS__);}while(0)
/* PRQA S 3218 1 #3209 - File scope static variable used in one function, intentional design */
static const char *TAG = "B2";          /*!< 日志标签，用于TC_LOGI输出调试信息 */
extern l_u8 ld_read_by_id_callout(l_u8 id, l_u8 *data); /*!< 用户自定义ID读取回调函数，由应用层实现 */

/**
 * @brief  SID $B2 通过标识符读取数据（ReadByID）处理函数
 * @param  ptr - UDS请求报文指针，包含Supplier ID、Function ID及要读取的标识符ID
 * @param  length - 报文长度
 * @note   LIN协议读取操作，支持：
 *         - LIN_PRODUCT_IDENT：读取产品ID（Supplier ID + Function ID + Variant）
 *         - SERIAL_NUMBER：暂不支持，返回SUBFUNCTION_NOT_SUPPORTED
 *         - 用户自定义ID范围（LIN_READ_USR_DEF_MIN~MAX）：通过ld_read_by_id_callout()回调处理
 *         执行前先校验Supplier ID和Function ID匹配性。
 *         用户自定义ID回调用data_callout[5]传递数据，自动裁剪尾部0xFF以获取实际数据长度。
 * @retval None (通过 lin_diag_positive_notify / lin_diag_negative_notify 返回响应)
 */
/* PRQA S 2071 2 #3269 - Language extension used for compiler and hardware optimization */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
__WEAK void lin_diagservice_read_by_identifier(uint8_t *ptr, uint16_t length)
{
    (void)length;
    uint8_t id;                         /*!< 要读取的标识符ID，从请求报文Byte[1]获取 */
    uint16_t supid, fid;                /*!< 请求报文中的Supplier ID和Function ID */

    /* 从请求报文中提取Supplier ID和Function ID */
    supid = ((uint16_t)ptr[3] << 8) | ptr[2];
    fid = ((uint16_t)ptr[5] << 8) | ptr[4];

    /* 校验Supplier ID和Function ID是否与本产品匹配 */
    if (((supid != (uint16_t)product_id.supplier_id) && (supid != (uint16_t)LD_ANY_SUPPLIER)) || \
        ((fid != (uint16_t)product_id.function_id) && (fid != (uint16_t)LD_ANY_FUNCTION)))
    {
        /* Supplier ID或Function ID不匹配：重置从节点响应计数，静默退出 */
        tl_slaveresp_cnt = 0;
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
        tl_service_status = LD_SERVICE_IDLE;
#endif /* End (_TL_FRAME_SUPPORT_ = = _TL_MULTI_FRAME_) */
/* PRQA S 3432 3 #3267 - Macro arguments are safely used without unintended operator precedence issues */
/* PRQA S 2742 2 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
/* PRQA S 2880 1 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
        TC_LOGI(TAG, "no reply %x-%x\n", supid, fid);
        return;
    }

    /* 获取要读取的标识符ID */
    id = ptr[1];

    /* 根据标识符ID类型分发处理 */
    switch (ptr[1])
    {
        /* LIN_PRODUCT_IDENT：读取产品标识（Supplier ID + Function ID + Variant） */
        case LIN_PRODUCT_IDENT:
            ptr[1] = (uint8_t)(product_id.supplier_id & 0xFFu);
            ptr[2] = (uint8_t)(product_id.supplier_id >> 8);
            ptr[3] = (uint8_t)(product_id.function_id & 0xFFu);
            ptr[4] = (uint8_t)(product_id.function_id >> 8);
            ptr[5] = product_id.variant;
            lin_diag_positive_notify(ptr[0], &ptr[1], 5);
            break;

        /* SERIAL_NUMBER：序列号读取，当前版本不支持 */
        case SERIAL_NUMBER:
            lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
            break;

        /* 用户自定义ID范围：通过回调函数由应用层处理 */
        default:
            /* 检查ID是否在用户自定义范围内 */
            if ((id >= (uint8_t)LIN_READ_USR_DEF_MIN) && (id <= (uint8_t)LIN_READ_USR_DEF_MAX))
            {
                uint8_t data_callout[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; /*!< 回调输出缓冲区，初始化为全0xFF */
                uint8_t data_len = 0;       /*!< 实际数据长度，从尾部裁剪0xFF后计算 */
                uint8_t retval = ld_read_by_id_callout(id, data_callout); /*!< 调用用户自定义ID读取回调 */

                /* 回调返回正响应：处理回调数据 */
                if (retval == (uint8_t)LD_POSITIVE_RESPONSE)
                {
                    /* 从尾部向前扫描，裁剪末尾的0xFF以确定有效数据长度 */
                    for (uint8_t i = 0; i < 5u; i++)
                    {
                        /* 找到第一个非0xFF字节的位置，确认有效数据长度 */
                        if (data_callout[4u - i] != 0xFFu)
                        {
                            data_len = 5u - i;
                            break;
                        }
                    }

                    /* 有效数据长度 > 0：发送正响应 */
                    if (data_len > 0u)
                    {
                        lin_diag_positive_notify(ptr[0], &data_callout[0], data_len);
                    }
                    else
                    {
                        /* 无有效数据：发送负响应 */
                        lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
                    }
                }
                /* 回调返回负响应：标识符不支持 */
                else if (retval == (uint8_t)LD_NEGATIVE_RESPONSE)
                {
                    lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
                }
                /* 回调返回无响应：静默忽略，不做回答 */
                else if (retval == (uint8_t)LD_ID_NO_RESPONSE)
                {
                    /*Do not answer*/
                }else{(void)0;}
            }
            else
            {
                /* ID不在用户自定义范围内：发送负响应 */
                lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
            }

            break;
    } /* End of switch */
}

