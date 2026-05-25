/**
 *****************************************************************************
 * @brief   dfu_uds_manager source file.
 *
 * @file    dfu_uds_manager.c
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

#include "dfu_uds_manager.h"
#include "pal_systick.h"
#include "pal_lin_comm.h"
#include "pal_lin_tl.h"
#include "pal_store.h"
#if defined (__TCPL01X__)
#include "pal_log.h"
#endif
#include "utilities.h"
#include "logging.h"

#define CFG_SUPPORT_COMMUCATION   0
#define CFG_SUPPORT_DEBUG   0

#if 1 == CFG_SUPPORT_DEBUG
#define LOG_DFU(...)  do{log_debug("[DFU] " __VA_ARGS__);}while(0)
#else
#define LOG_DFU(...)
#endif

#if defined (__TCPL01X__)
#define SYSTEM_PARAM_BASE_ADDR      (0x00800400UL)
const uint8_t security_code[4] = { 0x041, 0x31, 0x12, 0x01 };
const uint8_t boot_version[4] = { 2, 1, 0, 0 };
#elif defined (__TCPL03X__)
#define SYSTEM_PARAM_BASE_ADDR      (0x0000FA00UL)
#if defined (TCAE10X)
const uint8_t security_code[4] = { 0x040, 0x31, 0x12, 0x02 };
#else
const uint8_t security_code[4] = { 0x041, 0x31, 0x12, 0x02 };
#endif
const uint8_t boot_version[4] = { 3, 0, 3, 0 };
#endif

extern void lin_lld_isr_callback(uint32_t isr);

STATIC dfu_manager_context_t dfu_ctx = { 0 };

STATIC ServiceUDS_TypeDef uds_request_info =
{
    .sessionMode = DEFALUT_SESSION,
    .requsetMode = REQUEST_ID_ERROR,
    .securityLevel = SECURITY_LEVEL0
};

STATIC const ServiceUDS_TypeDef ServiceUDS[] =
{
    { 0x11u, DEFALUT_SESSION | PROGRAM_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, },               /*reset mcu*/
    { 0x10u, DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, }, /*session control*/
#if 1==CFG_SUPPORT_COMMUCATION
    { 0x28u, DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, },
#endif
    { 0x87u, DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, },
    { 0x27u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL0, },                                               /*security access*/
    { 0x2Eu, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },
    { 0x31u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                               /*routine control*/
    { 0x34u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                               /*request download*/
    { 0x36u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                               /*transfer data*/
    { 0x37u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                               /*request transfer exit*/
};
#define SERVICE_UDS_NUM    (sizeof(ServiceUDS) / sizeof(ServiceUDS_TypeDef))

#define LIN_BAUD_RATE    19200UL       /*For slave*/

STATIC uint32_t lin_baud_rate = LIN_BAUD_RATE;
STATIC uint8_t  lin_configured_NAD = 0x01;

typedef void (*FUNC_PTR)(void);

/********************************************************
** \brief   led_indicate_init
**
** \param   None
**
** \retval  None
** \note    Direct operation register, reduce code flash
*********************************************************/
STATIC void led_indicate_init(void)
{
#if (defined (__TCPL01X__) || defined (__TCPL03X__))
#if defined (__TCPL01X__)
#if !CFG_SUPPORT_DEBUG
    pal_log_init(115200);
#endif
#elif defined (__TCPL03X__)
    CRG_CONFIG_UNLOCK();
    CRG->PWM_CLKRST_CTRL_F.PCLK_EN_PWM = 1;
    CRG->PWM_CLKRST_CTRL_F.FCLK_EN_PWM = 1;
    CRG->PWM_CLKRST_CTRL_F.FCLK_SEL_PWM = 0;
    CRG->PWM_CLKRST_CTRL_F.FCLK_DIV_PWM = 0;

    CRG->ADC_CLKRST_CTRL_F.PCLK_EN_ADC = 1;

    CRG_CONFIG_LOCK();

    ll_gpio_afio_config(GPIO_PIN_6, AFIO_MUX_0);
    ll_gpio_afio_config(GPIO_PIN_7, AFIO_MUX_0);
    ll_gpio_afio_config(GPIO_PIN_8, AFIO_MUX_0);

    PWM->CNT_CFG = 0x0000FFFFUL;

    PWM->CTRL = 0x00001104UL;

    PWM->CH_CTRL = 0x00001500UL;

    ADC->CTRL0_F.VREFBUF_EN = 1;  /* 5V out same as tcpl01x adc bias  */

    PWM->LED_LC0_CTRL = 0x00000000UL;
    PWM->LED_LC1_CTRL = 0x00000000UL;
    PWM->LED_LC1_CTRL = 0x00000000UL;

    /* disable led */
    PWM->LED_CTRL_F.LED_LDO5V_EN = 1;

    while (PWM->LED_CTRL_F.LED_LDO_RDY == 0);

    PWM->LED_CTRL_F.LED_EN = 1;

    /* enable pwm */
    PWM->LED_CTRL = 0x00100013UL;
    PWM->CH_CTRL = 0x00001507UL;
    PWM->CNT_CTRL = 0x00000001UL;

#endif
#endif
}

/********************************************************
** \brief   led_indicate_deinit
**
** \param   None
**
** \retval  None
** \note    Direct operation register, reduce code flash
*********************************************************/
STATIC void led_indicate_deinit(void)
{
#if (defined (__TCPL01X__) || defined (__TCPL03X__))
#if defined (__TCPL01X__)
#if !CFG_SUPPORT_DEBUG
    pal_log_deinit();
#endif
#elif defined (__TCPL03X__)

    ADC->CTRL0_F.VREFBUF_EN = 0;  /* 5V out same as tcpl01x adc bias  */
    CRG_CONFIG_UNLOCK();
    CRG->PWM_CLKRST_CTRL_F.RST_PWM = 1;
    __NOP();
    __NOP();
    CRG->PWM_CLKRST_CTRL_F.RST_PWM = 0;
    __NOP();
    __NOP();
    CRG_CONFIG_LOCK();
#endif
#endif
}

/********************************************************
** \brief   led_indicate_toggle
**
** \param   None
**
** \retval  None
*********************************************************/
STATIC void led_indicate_toggle(void)
{
#if (defined (__TCPL01X__) || defined (__TCPL03X__))
#if defined (__TCPL01X__)
#if !CFG_SUPPORT_DEBUG
    PRINT_UART->TX_DATA = (uint8_t)0x55;
#endif
#elif defined (__TCPL03X__)
    STATIC bool toggle_flag = true;
    uint32_t value = toggle_flag ? 1000 : 0;

    PWM->CH0_PWM_CFG_F.HT0 = value;
    PWM->CH1_PWM_CFG_F.HT1 = value;
    PWM->CH2_PWM_CFG_F.HT2 = value;
    toggle_flag = !toggle_flag;

#endif
#endif
}

/********************************************************
** \brief   JumpToApp
**
** \param   None
**
** \retval  None
*********************************************************/
STATIC void JumpToApp(void)
{
    NVIC_DisableIRQ(TIMER_IRQn);
#if defined (__TCPL01X__)
    NVIC_DisableIRQ(AFE_INT_IRQn);
#elif defined (__TCPL03X__)
    NVIC_DisableIRQ(LINSCI_IRQn);
#endif
    NVIC_DisableIRQ(SysTick_IRQn);
    pal_lin_deinit(LIN_BUS_0);
#ifdef CFG_SUPPORT_DEBUG
    logging_deinit();
#endif
    FUNC_PTR pAppFunc = (FUNC_PTR) * (uint32_t *)(dfu_ctx.dfu_info.image_addr + 4);
    __set_MSP(*(uint32_t *)dfu_ctx.dfu_info.image_addr);
    pAppFunc();
}

/********************************************************
** \brief   queue_lin_empty
**
** \param   None
**
** \retval  uint8_t
*********************************************************/
STATIC uint8_t queue_lin_empty(void)
{
    return ((dfu_ctx.queue_list.head == dfu_ctx.queue_list.tail) ? 1 : 0);
}

/********************************************************
** \brief   system_cfg_load
**
** \param   None
**
** \retval  uint8_t
*********************************************************/
STATIC uint8_t system_cfg_load(void)
{
    uint32_t nad;

    pal_store_read(STORE_TYPE_SEL, SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&nad, sizeof(uint32_t));

    if (0xFF == (nad & 0xFF) || 0 == (nad & 0xFF))
    {
        return (0x01);
    }

    return ((uint8_t)nad);
}

/********************************************************
** \brief   dfu_image_erase
**
** \param   None
**
** \retval  uint8_t
*********************************************************/
STATIC uint8_t dfu_image_erase(void)
{
    if (pal_store_erase(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, FLASH_DFU_INFO_SIZE + FLASH_APP_IMAGE_SIZE))
    {
        return DFU_MSG_SUCCESS;
    }

    return DFU_MSG_ERASE_ERROR;
}

/********************************************************
** \brief   dfu_image_program
**
** \param   uint32_t            addr
** \param   packet_unit_t*      buffer
** \param   uint32_t            length
**
** \retval  uint8_t
*********************************************************/
STATIC uint8_t dfu_image_program(uint32_t addr, packet_unit_t *packet, uint16_t length)
{
    if (addr < FLASH_APP_ADDR || addr >= FLASH_APP_END_ADDR)
    {
        return DFU_MSG_PROGRA_ERROR;
    }

    uint8_t res =  DFU_MSG_SUCCESS;
    pal_store_write(FLASH_TYPE_NVM, addr, (uint8_t *)packet->data, length);
    uint32_t val;

    for (uint32_t i = 0; i < DFU_PROGRAM_WORDS; i++, addr += 4)
    {
        pal_store_read(FLASH_TYPE_NVM, addr, (uint8_t *)&val, sizeof(uint32_t));
        dfu_ctx.write_crc = crc32_calculate_func(dfu_ctx.write_crc, (uint8_t *)&val, sizeof(uint32_t));
    }

    if (packet->crc32 != dfu_ctx.write_crc)
    {
        LOG_DFU("crc32=0x%08x   crc=0x%08x\n", packet->crc32, dfu_ctx.write_crc);
        pal_store_erase(FLASH_TYPE_NVM, addr, DFU_PROGRAM_LENGTH);
        res = DFU_MSG_PROGRA_ERROR;
    }

    return (res);
}

/********************************************************
** \brief   last_dfu_info_get
**
** \param   last_dfu_info_t*        info
**
** \retval  uint8_t
*********************************************************/
STATIC uint8_t last_dfu_info_get(last_dfu_info_t *info)
{
    pal_store_read(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)info, sizeof(last_dfu_info_t));

    if (DFU_INFO_MAGIC != info->magic || DFU_MSG_SUCCESS != info->reason)
    {
        return DFU_MSG_ERROR;
    }

    return DFU_MSG_SUCCESS;
}

/********************************************************
** \brief   last_dfu_info_update
**
** \param   last_dfu_info_t*        info
**
** \retval  uint8_t
*********************************************************/
STATIC uint8_t last_dfu_info_update(last_dfu_info_t *info)
{
    pal_store_write(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)info, sizeof(last_dfu_info_t));
    return (DFU_MSG_SUCCESS);
}

/********************************************************
** \brief   dfu_process_exit
**
** \param   uint8_t     reason
**
** \retval  None
*********************************************************/
STATIC void dfu_process_exit(uint8_t reason)
{
    if (DFU_MSG_SUCCESS == reason)
    {
        dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC;
        dfu_ctx.dfu_info.reason = reason;
        memcpy((uint8_t *)&dfu_ctx.dfu_info.version, (uint8_t *)boot_version, sizeof(uint32_t));
        memcpy((uint8_t *)dfu_ctx.dfu_info.time, (uint8_t *)__TIME__, strlen(__TIME__));
        last_dfu_info_update(&dfu_ctx.dfu_info);
    }
    else if (DFU_MSG_TIMEOUT == reason)
    {
        memset(&dfu_ctx, 0, sizeof(dfu_ctx));
        dfu_ctx.boot_state = BOOT_STATE_UPGRADE;
    }

    led_indicate_deinit();
}

/********************************************************
** \brief   dfu_do_notify_cp
**
** \param   uint8_t     sid
** \param   uint8_t     sub_func
** \param   uint8_t*    data
** \param   uint16_t    length
**
** \retval  None
*********************************************************/
STATIC void dfu_do_notify_cp(uint8_t sid, uint8_t sub_func, uint8_t *data, uint16_t length)
{
    uint8_t response[20];
    uint8_t len = 2 + length;

    response[0] = sid + 0x40;
    response[1] = sub_func;

    for (uint16_t i = 0; i < length; i++)
    {
        response[2 + i] = data[i];
    }

    lin_uds_send(lin_configured_NAD, response, len);
}

/********************************************************
** \brief   dfu_do_notify_response
**
** \param   uint8_t     resp_type
** \param   uint8_t     sid
** \param   uint8_t     resp_value
**
** \retval  None
*********************************************************/
STATIC void dfu_do_notify_response(uint8_t resp_type, uint8_t sid, uint8_t resp_value)
{
    if (POSITIVE == resp_type)
    {
        dfu_do_notify_cp(sid, resp_value, NULL, 0);
    }
    else
    {
        lin_uds_negative_response(lin_configured_NAD, (sid + 0x40), resp_value);
    }
}

/********************************************************
** \brief   session_control_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void session_control_handle(uint8_t *param, uint16_t length)
{
    uint8_t resp_type = POSITIVE;
    uint8_t sid = param[0];
    uint8_t resp_value = param[1];

    switch (param[1])
    {
        case 0x01:
            uds_request_info.sessionMode = DEFALUT_SESSION;
            break;

        case 0x02:
            uds_request_info.sessionMode = PROGRAM_SESSION;
            dfu_ctx.boot_state = BOOT_STATE_UPGRADE;
            led_indicate_init();
            break;

        case 0x03:
            uds_request_info.sessionMode = PROGRAM_SESSION;
            dfu_do_notify_cp(sid, param[1], (uint8_t *)boot_version, sizeof(boot_version));
            return;

        default:
            resp_type = NEGATIVE;
            resp_value = SUBFUNCTION_NOT_SUPPORTED;
            break;
    }

    dfu_do_notify_response(resp_type, sid, resp_value);
}

#if 1==CFG_SUPPORT_COMMUCATION
/********************************************************
** \brief   communication_control_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void communication_control_handle(uint8_t *param, uint16_t length)
{
    uint8_t resp_type = POSITIVE;
    uint8_t sid = param[0];
    uint8_t resp_value = param[1];

    /* sub function */
    switch (param[1])
    {
        case 0x03:
            break;

        default:
            resp_type = NEGATIVE;
            resp_value = SUBFUNCTION_NOT_SUPPORTED;
            break;
    }

    dfu_do_notify_response(resp_type, sid, resp_value);
}
#endif

/********************************************************
** \brief   link_control_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void link_control_handle(uint8_t *param, uint16_t length)
{
    uint8_t resp_type = POSITIVE;
    uint8_t sid = param[0];
    uint8_t resp_value = param[1];

    switch (param[1])
    {
        case 0x01:
            if (LIN_BRUAD_9600 == param[2])
            {
                dfu_ctx.lin_config.baudrate = 9600;
            }
            else if (LIN_BRUAD_19200 == param[2])
            {
                dfu_ctx.lin_config.baudrate = 19200;
            }
            else if (LIN_BRUAD_115200 == param[2])
            {
                dfu_ctx.lin_config.baudrate = 115200;
            }
            else
            {
                resp_type = NEGATIVE;
                resp_value = IMLOIF;
            }

            break;

        case 0x02:
            dfu_ctx.lin_config.update_flag = true;
            break;

        default:
            resp_type = NEGATIVE;
            resp_value = SUBFUNCTION_NOT_SUPPORTED;
            break;
    }

    dfu_do_notify_response(resp_type, sid, resp_value);
}

/********************************************************
** \brief   cpmpare_key
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC uint8_t cpmpare_key(uint8_t *seed, uint8_t *key, uint8_t length)
{
    /*compare key seed*/
    for (uint8_t i = 0; i < length; i++)
    {
        if (seed[i] != key[i])
        {
            return 0;
        }
    }

    return 1;
}

/********************************************************
** \brief   security_access_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void security_access_handle(uint8_t *param, uint16_t length)
{
    uint8_t resp_type = POSITIVE;
    uint8_t sid = param[0];
    uint8_t resp_value = param[1];

    switch (param[1])
    {
        case 0x01:
            dfu_do_notify_cp(param[0], param[1], (uint8_t *)security_code, sizeof(security_code));
            return;

        case 0x02:
        case 0x04:
            if (cpmpare_key((uint8_t *)security_code, &param[2], 4))
            {
                uds_request_info.securityLevel = SECURITY_LEVEL1;
            }
            else
            {
                uds_request_info.securityLevel = SECURITY_LEVEL0;
                resp_type = NEGATIVE;
                resp_value = INVALID_KEY;
            }

            break;

        default:
            resp_type = NEGATIVE;
            resp_value = SUBFUNCTION_NOT_SUPPORTED;
            break;
    }

    dfu_do_notify_response(resp_type, sid, resp_value);
}

/********************************************************
** \brief   firmware_info_sync_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void firmware_info_sync_handle(uint8_t *param, uint16_t length)
{
    dfu_ctx.op_code = DFU_CMD_SYNC_INFO;

    if (length < 14)
    {
        dfu_do_notify_response(NEGATIVE, param[0], IMLOIF);
        dfu_ctx.resp_value = DFU_MSG_SYNC_ERROR;
        return;
    }

    dfu_ctx.write_addr = (param[2] << 24) | (param[3] << 16) | (param[4] << 8) | param[5];
#if defined (__TCPL01X__)
    dfu_ctx.write_addr = FLASH_APP_ADDR;
#endif
    dfu_ctx.dfu_info.image_addr = dfu_ctx.write_addr;

    /* check image info */
    dfu_ctx.dfu_info.image_size = (param[6] << 24) | (param[7] << 16) | (param[8] << 8) | param[9];
    dfu_ctx.dfu_info.image_crc = (param[10] << 24) | (param[11] << 16) | (param[12] << 8) | param[13];
    dfu_ctx.write_length = 0;
    dfu_ctx.receive_length = 0;
    LOG_DFU("write_addr:0x%08x image_size:0x%08x image_crc:0x%08x\n", dfu_ctx.write_addr, dfu_ctx.dfu_info.image_size, dfu_ctx.dfu_info.image_crc);

    if ((dfu_ctx.dfu_info.image_size % FLASH_SECTOR_SIZE) || (dfu_ctx.dfu_info.image_addr % FLASH_SECTOR_SIZE) || ((dfu_ctx.dfu_info.image_addr + dfu_ctx.dfu_info.image_size) > FLASH_APP_END_ADDR) || (dfu_ctx.dfu_info.image_addr < FLASH_APP_ADDR))
    {
        dfu_do_notify_response(NEGATIVE, param[0], REQUEST_OUT_RANGE);
        dfu_ctx.resp_value = DFU_MSG_SYNC_ERROR;
        // LOG_DFU("dfu_ctx.write_length  no avail");
        return;
    }

    uint8_t resp_value[9];
    resp_value[0] = boot_version[0];
    resp_value[1] = boot_version[1];
    resp_value[2] = boot_version[2];
    resp_value[3] = boot_version[3];
    resp_value[4] = 0xFF & (DFU_PROGRAM_LENGTH >> 8);
    resp_value[5] = 0xFF & DFU_PROGRAM_LENGTH;
    resp_value[6] = 0xFF & (DFU_ERASE_WAITTIME >> 8);
    resp_value[7] = 0xFF & DFU_ERASE_WAITTIME;
    resp_value[8] = 0xFF & DFU_PROGRAM_WAITTIME;
    dfu_do_notify_cp(param[0], param[1], resp_value, 9);
    dfu_ctx.resp_value = DFU_MSG_SUCCESS;
    dfu_ctx.queue_list.tail = 0;
    dfu_ctx.queue_list.head = 0;
    dfu_ctx.recevice_index = 0x01;
    dfu_ctx.write_index = 0;
    dfu_ctx.receive_length = 0;
    dfu_ctx.write_length = 0;
    dfu_ctx.write_crc = 0xFFFFFFFF;
    LOG_DFU("firmware_info_sync_handle\r\n");
}

/********************************************************
** \brief   request_download_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void request_download_handle(uint8_t *param, uint16_t length)
{
    uint8_t resp_type = POSITIVE;
    uint8_t sid = param[0];
    uint8_t resp_value = param[1];

    if (DFU_CMD_FLASH_ERASE != dfu_ctx.op_code)
    {
        resp_type = NEGATIVE;
        resp_value = REQUEST_SEQUEENCE_ERROR;
        dfu_ctx.resp_value = DFU_MSG_TRANFER_REQUEST_ERROR;
    }
    else
    {
        switch (param[1])
        {
            case 0x01:
                if (DFU_MSG_ERASE_ERROR == dfu_ctx.resp_value)
                {
                    resp_type = NEGATIVE;
                    resp_value = GENERAL_PROGRAM_FAILURE;
                }
                else
                {
                    dfu_ctx.op_code = DFU_CMD_TRANFER_START;
                    dfu_ctx.resp_value = DFU_MSG_SUCCESS;
                }

                break;

            default:
                resp_type = NEGATIVE;
                resp_value = SUBFUNCTION_NOT_SUPPORTED;
                dfu_ctx.resp_value = DFU_MSG_TRANFER_REQUEST_ERROR;
                break;
        }
    }

    dfu_do_notify_response(resp_type, sid, resp_value);
}

/********************************************************
** \brief   transfer_data_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void transfer_data_handle(uint8_t *param, uint16_t length)
{
    if (DFU_CMD_TRANFER_START != dfu_ctx.op_code || DFU_MSG_PROGRA_ERROR == dfu_ctx.resp_value)
    {
        dfu_ctx.resp_value = REQUEST_SEQUEENCE_ERROR;
        dfu_ctx.queue_list.tail = 0;
        dfu_ctx.queue_list.head = 0;
    }
    else
    {
        dfu_ctx.resp_value = DFU_MSG_SUCCESS;

        if (length <= DFU_PACKET_HEAD_CRC_LENGTH)
        {
            dfu_ctx.resp_value = DFU_MSG_PACKET_LEN_ERROR;
        }

        if (dfu_ctx.recevice_index != param[1])
        {
            dfu_ctx.resp_value = DFU_MSG_INDEX_ERROR;
        }

        led_indicate_toggle();

        //LOG_DFU("recv_index=%d record_index=%d resp=%d\n", param[1], dfu_ctx.recevice_index,dfu_ctx.resp_value);
        if (DFU_MSG_SUCCESS == dfu_ctx.resp_value)
        {
            dfu_ctx.receive_length += (length - DFU_PACKET_HEAD_CRC_LENGTH);

            if ((dfu_ctx.receive_length >= DFU_PROGRAM_LENGTH) ||
                ((dfu_ctx.receive_length + dfu_ctx.write_length) == dfu_ctx.dfu_info.image_size))
            {
                if ((++(dfu_ctx.queue_list.tail)) >= QUEUE_LIN_LEN)
                {
                    dfu_ctx.queue_list.tail = 0;
                }
            }

            if (dfu_ctx.write_length == dfu_ctx.dfu_info.image_size)
            {
                dfu_ctx.queue_list.tail = 0;
                dfu_ctx.queue_list.head = 0;
            }
        }
        else
        {
            dfu_ctx.queue_list.tail = 0;
            dfu_ctx.queue_list.head = 0;
        }
    }
}

/********************************************************
** \brief   pal_lin_init
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void request_transfer_exit_handle(uint8_t *param, uint16_t length)
{
    uint8_t resp_type = POSITIVE;
    uint8_t sid = param[0];
    uint8_t resp_value = param[1];

    if (DFU_CMD_TRANFER_START != dfu_ctx.op_code)
    {
        resp_type = NEGATIVE;
        resp_value = REQUEST_SEQUEENCE_ERROR;
    }

    dfu_do_notify_response(resp_type, sid, resp_value);
    dfu_ctx.op_code = DFU_CMD_TRANFER_STOP;
}

/********************************************************
** \brief   routine_control_handle
**
** \param   uint8_t*        param
** \param   uint16_t        length
**
** \retval  None
*********************************************************/
STATIC void routine_control_handle(uint8_t *param, uint16_t length)
{
    uint8_t resp_type = POSITIVE;
    uint8_t sid = param[0];
    uint8_t resp_value = param[1];

    if (0x02 == param[1])
    {
        if (DFU_CMD_SYNC_INFO != dfu_ctx.op_code || DFU_MSG_SUCCESS != dfu_ctx.resp_value)
        {
            resp_type = NEGATIVE;
            resp_value = REQUEST_SEQUEENCE_ERROR;
        }
        else
        {
            dfu_ctx.op_code = DFU_CMD_FLASH_ERASE;
            dfu_do_notify_response(resp_type, sid, resp_value);
            delay_ms(50);
            dfu_ctx.resp_value = dfu_image_erase();
            return;
        }
    }
    else if (0x01 == param[1])
    {
        uint16_t routine_id = (param[2] << 8) | param[3];

        switch (routine_id)
        {
            case 0x0202:
                if (dfu_ctx.resp_value != DFU_MSG_SUCCESS || dfu_ctx.dfu_info.image_crc != dfu_ctx.write_crc)
                {
                    if (DFU_MSG_SUCCESS == dfu_ctx.resp_value)
                    {
                        dfu_ctx.resp_value = DFU_MSG_CRC_ERROR;
                        //LOG_DFU("image_crc=0x%08x   write_crc=0x%08x\n", dfu_ctx.dfu_info.image_crc, dfu_ctx.write_crc);
                    }

                    resp_type = NEGATIVE;
                    resp_value = GENERAL_PROGRAM_FAILURE;
                    //LOG_DFU("RoutineControl crc Failure:%d\n", dfu_ctx.resp_value);
                }

                dfu_process_exit(dfu_ctx.resp_value);
                break;
#if 0

            case 0x0201://packet check
                if ((dfu_ctx.resp_value != DFU_MSG_SUCCESS))   //The last package of programming result  and  write boot flag result
                {
                    resp_type = NEGATIVE;
                    resp_value = GENERAL_PROGRAM_FAILURE + dfu_ctx.resp_value;
                    LOG_DFU("RoutineControl Packet check Failure:%d %d\n", dfu_ctx.resp_value, dfu_ctx.recevice_index);
                }

                break;
#endif

            case 0xFF01:
                if (dfu_ctx.resp_value == DFU_MSG_SUCCESS && !queue_lin_empty())
                {
                    dfu_ctx.program_flag = 1;
#if !CFG_SUPPORT_DFU_V3
                    resp_value = dfu_ctx.recevice_index;
                    dfu_do_notify_response(resp_type, sid, resp_value);
#endif

                    if (DFU_MSG_SUCCESS ==
                        dfu_image_program(dfu_ctx.write_addr, &dfu_ctx.queue_list.packet[dfu_ctx.queue_list.head],
                                          DFU_PROGRAM_LENGTH))
                    {
                        if ((++(dfu_ctx.queue_list.head)) >= QUEUE_LIN_LEN)
                        {
                            dfu_ctx.queue_list.head = 0;
                        }

                        dfu_ctx.write_index = dfu_ctx.recevice_index;

                        dfu_ctx.write_addr += DFU_PROGRAM_LENGTH;
                        dfu_ctx.write_length += DFU_PROGRAM_LENGTH;
#if CFG_SUPPORT_DFU_V3
                        resp_value = dfu_ctx.recevice_index;
#endif
                        dfu_ctx.recevice_index++;

                    }
                    else
                    {
                        dfu_ctx.queue_list.tail = 0;
                        dfu_ctx.queue_list.head = 0;
                        resp_type = NEGATIVE;
                        resp_value = GENERAL_PROGRAM_FAILURE;
                        // LOG_DFU("write error\r\n");
                    }

#if !CFG_SUPPORT_DFU_V3
                    return;
#endif

                }
                else
                {
                    resp_type = NEGATIVE;
                    resp_value = GENERAL_PROGRAM_FAILURE;
                    // LOG_DFU("RoutineControl Packet check Failure:%d %d\r\n", dfu_ctx.resp_value, dfu_ctx.recevice_index);
                }

                break;

            case 0x7221:
                if (DFU_CMD_TRANFER_STOP != dfu_ctx.op_code || DFU_MSG_SUCCESS != dfu_ctx.resp_value)
                {
                    resp_type = NEGATIVE;
                    resp_value = REQUEST_SEQUEENCE_ERROR;
                }
                else
                {
                    NVIC_SystemReset();
                    dfu_ctx.boot_state = BOOT_STATE_USER_APP;
                }

                break;

            default:
                resp_type = NEGATIVE;
                resp_value = SUBFUNCTION_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        resp_type = NEGATIVE;
        resp_value = SUBFUNCTION_NOT_SUPPORTED;
    }

    dfu_do_notify_response(resp_type, sid, resp_value);
}

/********************************************************
** \brief   dfu_process_ctx dfu process
*********************************************************/
const dfu_process_context_t dfu_process_ctx[] =
{
    { SERVICE_ECU_RESET, NULL }, //0x11
    { SERVICE_SESSION_CONTROL, session_control_handle },//0x10
#if 1==CFG_SUPPORT_COMMUCATION
    { SERVICE_COMMUNICATION_CONTROL, communication_control_handle },//0x28
#endif
    { SERVICE_LINK_CONTROL, link_control_handle },//0x87
    { SERVICE_SECURITY_ACCESS, security_access_handle },//0x27
    { SERVICE_FIRMWARE_INFO_SYNC, firmware_info_sync_handle },//0x2E
    { SERVICE_ROUTINE_CONTROL, routine_control_handle },//0x31
    { SERVICE_REQUEST_DOWNLOAD, request_download_handle },//0x34
    { SERVICE_TRANSFER_DATA, transfer_data_handle },//0x36
    { SERVICE_REQUEST_TRANSFER_EXIT, request_transfer_exit_handle },//0x37
};

#define DFU_PROCESS_STEP_MAX    (sizeof(dfu_process_ctx) / sizeof(dfu_process_context_t))

/********************************************************
** \brief   lin_diag_service_handle
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_diag_service_handle(void)
{
    uint16_t length = 0;
    uds_request_info.requsetMode = PHYSICAL_ADDR;

    uint8_t *ptr = ((uint8_t *)&dfu_ctx.queue_list.packet[dfu_ctx.queue_list.tail] + 2);
    lin_uds_receive(lin_configured_NAD, ptr, &length);

    uint8_t sid = ptr[0];

    if (length)
    {
        for (uint8_t i = 0; i < SERVICE_UDS_NUM; i++)
        {
            if (sid == ServiceUDS[i].sid)
            {
                if (!(ServiceUDS[i].requsetMode & uds_request_info.requsetMode) ||
                    !(ServiceUDS[i].sessionMode & uds_request_info.sessionMode) ||
                    (ServiceUDS[i].securityLevel > uds_request_info.securityLevel))
                {
                    dfu_do_notify_response(NEGATIVE, ptr[0], SERVICE_NOT_SUPPORTED);
                    return;
                }

                if (NULL != dfu_process_ctx[i].func)
                {
                    dfu_process_ctx[i].func(ptr, length);
                }

                dfu_ctx.uds_timeout = 0;
                break;
            }
        }
    }
}

/********************************************************
** \brief   lin_sci_baudrate_update
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_sci_baudrate_update(void)
{
    if (!dfu_ctx.lin_config.update_flag)
    {
        return;
    }

    dfu_ctx.lin_config.update_flag = false;

    if (dfu_ctx.lin_config.baudrate == lin_baud_rate)
    {
        return;
    }

    lin_baud_rate = dfu_ctx.lin_config.baudrate;
    pal_lin_init(LIN_BUS_0, LIN_MODE_SLV, lin_baud_rate, lin_lld_isr_callback);
}

/********************************************************
** \brief   dfu_timeout_handle
**
** \param   None
**
** \retval  None
*********************************************************/
void dfu_timeout_handle(void)
{
    if (dfu_ctx.op_code)
    {
        dfu_ctx.uds_timeout++;

        if (dfu_ctx.uds_timeout > LIN_UDS_TIMEOUT)
        {
            if (LIN_BAUD_RATE != lin_baud_rate)
            {
                dfu_ctx.lin_config.baudrate = LIN_BAUD_RATE;
                dfu_ctx.lin_config.update_flag = true;
                lin_sci_baudrate_update();
            }

            dfu_process_exit(DFU_MSG_TIMEOUT);
        }
    }
    else
    {
        dfu_ctx.uds_timeout = 0;
    }
}

/********************************************************
** \brief   dfu_manager_init
**
** \param   None
**
** \retval  None
*********************************************************/
void dfu_manager_init(void)
{
    lin_configured_NAD = system_cfg_load();
#ifdef CFG_SUPPORT_DEBUG
    logging_init();
#endif
    memset(&dfu_ctx, 0, sizeof(dfu_ctx));
    pal_lin_init(LIN_BUS_0, LIN_MODE_SLV, LIN_BAUD_RATE, lin_lld_isr_callback);

    LOG_DFU("lin_configured_NAD = %02X\r\n", lin_configured_NAD);
}

/********************************************************
** \brief   main_loops
**
** \param   None
**
** \retval  None
*********************************************************/
void main_loops(void)
{
    STATIC uint32_t LoopCnt = 0;

    lin_diag_service_handle();

    if (dfu_ctx.boot_state == BOOT_STATE_IDLE)
    {
        delay_ms(1);

        /* about 42ms+5ms */
        if ((++LoopCnt) > 50)
        {
            LoopCnt = 0;

            if (DFU_MSG_SUCCESS == last_dfu_info_get(&dfu_ctx.dfu_info))
            {
                LOG_DFU("BOOT_STATE_USER_APP\r\n");
                dfu_ctx.boot_state = BOOT_STATE_USER_APP;
            }
            else
            {
                LOG_DFU("BOOT_STATE_UPGRADE\r\n");
                dfu_ctx.boot_state = BOOT_STATE_UPGRADE;
            }
        }
    }
    else if (dfu_ctx.boot_state == BOOT_STATE_USER_APP)
    {
        JumpToApp();/* jump user app*/
    }
}

/********************************************************
** \brief   os_task_update
**
** \param   None
**
** \retval  None
*********************************************************/
void os_task_update(void)
{
    dfu_timeout_handle();
}
