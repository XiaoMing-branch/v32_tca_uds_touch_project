/**
 *****************************************************************************
 * @brief   pal lin communication source file.
 *
 * @file    pal_lin_comm.c
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

#include "pal_func_def.h"
#include "pal_lin_comm.h"
#include "utilities.h"
/**
 * @brief LIN ISR FUNC FLAG
 */
#define LIN_ISR0_BASE_FLAG (SCI_INT_BRK_DET | SCI_INT_SYNC_DET | SCI_INT_SYNC_VAL_ERR |         \
                            SCI_INT_RX_PID_DONE | SCI_INT_RX_STP_ERR | SCI_INT_RX_PTY_CHK_ERR | \
                            SCI_INT_TX_RX_CONF)
#ifdef __TCPL01X__
#define LIN_ISR0_FUNC_FLAG (LIN_ISR0_BASE_FLAG | SCI_INT_TX_DONE)
#else
#define LIN_ISR0_FUNC_FLAG (LIN_ISR0_BASE_FLAG | SCI_INT_TX_DONE | SCI_INT_TX_FIFO_EMPTY)
#endif

#define BIT(A,B)            (((A)>>(B))&0x01)

/**
 * @brief  LIN通信初始化
 * @param  bus - LIN总线号
 * @param  mode - LIN模式(主机/从机)
 * @param  baudrate - 通信波特率
 * @param  callback - 中断回调函数指针
 * @retval 无
 */
void pal_lin_init(lin_bus_e bus, lin_mode_e mode, uint32_t baudrate, ISR_FUNC_CALLBACK callback)
{
    sci_config_t config =
    {
        .clk_cfg = {
            .clk_source = FCLK_SRC_48M,
            .fclk_div = 0,
        },
        .isr_cfg = {
#ifdef MULTI_BYTES_MODE
            .isr = LIN_ISR0_FUNC_FLAG | SCI_INT_RX_DONE | SCI_INT_RX_FIFO_FULL,
#else
            .isr = LIN_ISR0_FUNC_FLAG | SCI_INT_RX_1BYTE_DONE,
#endif
#ifdef __TCPL01X__
            .isr1 = 0,
#endif
            .isr_enable = true,
            .priority = 3,
        },
        .mode = LIN_MODE_SLV == mode ? SCI_MODE_LIN_S : SCI_MODE_LIN_M,
        .baudrate = baudrate,
    };
    ll_sci_init((ll_sci_bus_e)bus, &config, callback);
    ll_sci_isr_enable((ll_sci_bus_e)bus, true);
}

/**
 * @brief  LIN通信去初始化
 * @param  bus - LIN总线号
 * @retval 无
 */
void pal_lin_deinit(lin_bus_e bus)
{
    ll_sci_deinit((ll_sci_bus_e)bus);
}

/**
 * @brief  LIN PID奇偶校验计算或检查
 * @param  type - 奇偶校验类型(生成/校验)
 * @param  pid - 待处理的PID值
 * @retval 校验模式:0xFF=失败,低6位PID=成功; 生成模式:含校验位的PID
 */
uint8_t pal_lin_parity_calib(lin_parity_type_e type, uint8_t pid)
{
    uint8_t ret;

    uint8_t parity = (((BIT(pid, 0)^BIT(pid, 1)^BIT(pid, 2)^BIT(pid, 4)) << 6) |
                      ((~(BIT(pid, 1)^BIT(pid, 3)^BIT(pid, 4)^BIT(pid, 5))) << 7));

    if (LIN_PARITY_CHECK == type)
    {
        if ((pid & 0xC0) != parity)
        {
            ret = 0xFF;
        }
        else
        {
            ret = (uint8_t)(pid & 0x3F);
        }
    }
    else
    {
        ret = (uint8_t)(pid | parity);
    }

    return (ret);
}


/**
 * @brief  LIN校验和计算(经典/增强型)
 * @param  pid - 帧ID(用于判断校验类型)
 * @param  buffer - 8字节数据缓冲区
 * @retval 计算得到的校验和
 */
uint8_t pal_lin_checksum_calib(uint8_t pid, uint8_t *buffer)
{
     uint8_t init_sum = ((0x3C != pid) && (0x7D != pid)) ? pid : 0;

     return (checksum_calculate_func(init_sum, buffer, 8));
}

#if CFG_SUPPORT_LIN_SNPD
/**
 * @brief  LIN自动寻址功能进入
 * @param  aa_cur_th - 自动寻址电流阈值
 * @retval 无
 */
void pal_lin_aa_enter(uint16_t *aa_cur_th)
{
#if CFG_SUPPROT_LINSNPD_EXT_RES
    bool use_ext_res = true;
#else
    bool use_ext_res = false;
#endif

    /* *adc ctrl config* */
    ll_adc_lin_aa_enable(LIN_AA_STYPE_STEPS_4, true);
    /* *IBIAS Control* */
    ll_bias_control_enable(true);
    /* *adc ctrl config backup* */
    ll_lin_aa_ready_set(LL_SCI_BUS_1, false);

    ll_lin_aa_enable(LL_SCI_BUS_1, LIN_AA_STYPE_STEPS_4, use_ext_res, aa_cur_th);
}

/**
 * @brief  LIN自动寻址功能退出
 * @param  无
 * @retval 无
 */
void pal_lin_aa_exit(void)
{
    /* *disable Lin aa* */
    ll_lin_aa_disable(LL_SCI_BUS_1);
}

/**
 * @brief  LIN自动寻址就绪设置
 * @param  无
 * @retval 无
 */
void pal_lin_aa_ready(void)
{
    /* *Enable Lin aa* */
    ll_lin_aa_ready_set(LL_SCI_BUS_1, true);
}

/**
 * @brief  获取LIN自动寻址ADC原始码值
 * @param  bufffer - ADC数据缓冲区
 * @param  length - 缓冲区长度
 * @retval 实际获取的原始码值个数
 */
uint16_t pal_lin_aa_raw_code_get(uint16_t *bufffer, uint16_t length)
{
    uint8_t len = ll_adc_fifo_get(bufffer, length);
    return len;
}
#endif

/**
 * @brief  LIN接收响应数据
 * @param  bus - LIN总线号
 * @param  pid - 帧ID
 * @param  buffer - 接收数据缓冲区
 * @param  msg_length - 消息长度
 * @retval 无
 */
void pal_lin_rx_response(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t msg_length)
{
    ll_lin_receive((ll_sci_bus_e)bus, pid, buffer, msg_length);
}

/**
 * @brief  LIN发送4字节数据(多字节模式下使用)
 * @param  bus - LIN总线号
 * @param  buffer - 待发送数据缓冲区
 * @param  msg_length - 消息长度
 * @retval 无
 */
void pal_lin_tx_4byte(lin_bus_e bus, uint8_t *buffer, uint8_t msg_length)
{
    ll_sci_transmit((ll_sci_bus_e)bus, buffer, msg_length);
}

/**
 * @brief  LIN发送响应数据
 * @param  bus - LIN总线号
 * @param  pid - 帧ID
 * @param  buffer - 待发送数据缓冲区
 * @param  msg_length - 消息长度
 * @retval true - 发送成功, false - 发送失败
 */
bool pal_lin_tx_response(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t msg_length)
{
    if (LL_OK == ll_lin_transmit((ll_sci_bus_e)bus, pid, buffer, msg_length))
    {
        return true;
    }

    return false;
}

/**
 * @brief  LIN总线中止处理(清除发送/接收FIFO及中止标志)
 * @param  bus - LIN总线号
 * @param  type - 中止类型(发送/接收/两者)
 * @retval 无
 */
void pal_lin_abort_handle(lin_bus_e bus, lin_abort_type_e type)
{
    ll_sci_clear_type_e clear_type = SCI_CLEAR_NULL;

    if (type & LIN_ABORT_TYPE_TX)
    {
        clear_type |= (SCI_CLEAR_TX_ABORT | SCI_CLEAR_TX_FIFO);
    }

    if (type & LIN_ABORT_TYPE_RX)
    {
        clear_type |= (SCI_CLEAR_RX_ABORT | SCI_CLEAR_RX_FIFO);
    }

    ll_sci_state_clear((ll_sci_bus_e)bus, (ll_sci_clear_type_e)clear_type);
}

/**
 * @brief  LIN读取一个字节(PID或FIFO数据)
 * @param  bus - LIN总线号
 * @param  type - 读取类型(PID/FIFO)
 * @param  byte - 输出读取的字节值
 * @retval 无
 */
void pal_lin_read_byte(lin_bus_e bus, lin_read_type_e type, uint8_t *byte)
{
    if (LIN_READ_TYPE_PID == type)
    {
        ll_lin_pid_read((ll_sci_bus_e)bus, byte);
    }
    else
    {
        ll_lin_read_byte((ll_sci_bus_e)bus, byte);
    }
}

/**
 * @brief  LIN自动波特率检测与校准
 * @param  bus - LIN总线号
 * @note   当检测到的自动波特率与配置波特率偏差超过14%时，复位LIN控制器
 * @retval 无
 */
void pal_lin_autobaudrate_check(lin_bus_e bus)
{
#ifdef __TCPL03X__
    uint32_t baud;
    uint32_t auto_baud;
    uint32_t diff_baud;

    ll_lin_baudrate_read(LL_SCI_BUS_1, &baud);
    ll_lin_auto_baudrate_read(LL_SCI_BUS_1, &auto_baud);

    diff_baud = (auto_baud >= baud) ? (auto_baud - baud) : (baud - auto_baud);

    if (diff_baud > (baud * 14 / 100))
    {
        ll_lin_ctrl_glben(LL_SCI_BUS_1, false);
        ll_lin_ctrl_glben(LL_SCI_BUS_1, true);
    }

#endif
}
