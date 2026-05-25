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
#define SYSTEM_PARAM_BASE_ADDR      (0x00800400UL)  /**< @brief TCPL01X系统参数存储基地址(NVM区) */
/** @brief 安全访问密钥(种子=密钥，简化实现)，TCPL01X版本 = {0x41,0x31,0x12,0x01} */
const uint8_t security_code[4] = { 0x041, 0x31, 0x12, 0x01 };
/** @brief Bootloader版本号，TCPL01X = v2.1.0.0 */
const uint8_t boot_version[4] = { 2, 1, 0, 0 };
#elif defined (__TCPL03X__)
#define SYSTEM_PARAM_BASE_ADDR      (0x0000FA00UL)  /**< @brief TCPL03X系统参数存储地址(Flash末尾，应用参数区起始) */
#if defined (TCAE10X)
const uint8_t security_code[4] = { 0x040, 0x31, 0x12, 0x02 };
#else
const uint8_t security_code[4] = { 0x041, 0x31, 0x12, 0x02 };
#endif
const uint8_t boot_version[4] = { 3, 0, 3, 0 };
#endif

extern void lin_lld_isr_callback(uint32_t isr); /**< @brief LIN底层中断回调函数声明(在pal_lin_comm中实现) */

/**
 * @brief DFU管理器全局上下文实例，零初始化
 * @note 包含OTA升级全流程的运行状态：目标地址、已写入长度、CRC、队列、LIN配置、DFU记录等
 */
STATIC dfu_manager_context_t dfu_ctx = { 0 };

/**
 * @brief 当前UDS会话信息，记录活动的会话模式、寻址方式和安全等级
 * @note 每次UDS请求时以此与ServiceUDS权限矩阵比对，决定服务是否可执行
 */
STATIC ServiceUDS_TypeDef uds_request_info =
{
    .sessionMode = DEFALUT_SESSION,     /**< @brief 初始为默认会话 */
    .requsetMode = REQUEST_ID_ERROR,    /**< @brief 初始为无效寻址 */
    .securityLevel = SECURITY_LEVEL0    /**< @brief 初始安全等级0(未解锁) */
};

/**
 * @brief UDS服务权限矩阵表，定义每个诊断服务的访问控制策略
 * @note 用于lin_diag_service_handle中的三层权限校验:
 *       1. 寻址模式检查(requsetMode)
 *       2. 会话模式检查(sessionMode)
 *       3. 安全等级检查(securityLevel)
 *       三项全部通过才允许执行对应handler
 */
STATIC const ServiceUDS_TypeDef ServiceUDS[] =
{
    { 0x11u, DEFALUT_SESSION | PROGRAM_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, },   /* $11 ECU复位 */
    { 0x10u, DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, }, /* $10 会话控制 */
#if 1==CFG_SUPPORT_COMMUCATION
    { 0x28u, DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, }, /* $28 通信控制(未启用) */
#endif
    { 0x87u, DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION, PHYSICAL_ADDR | FUNCTION_ADDR, SECURITY_LEVEL0, }, /* $87 链路控制 */
    { 0x27u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL0, },                                   /* $27 安全访问 */
    { 0x2Eu, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                   /* $2E 固件信息同步 */
    { 0x31u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                   /* $31 例程控制 */
    { 0x34u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                   /* $34 请求下载 */
    { 0x36u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                   /* $36 传输数据 */
    { 0x37u, PROGRAM_SESSION, PHYSICAL_ADDR, SECURITY_LEVEL1, },                                   /* $37 请求传输退出 */
};
/** @brief 服务权限表条目数 */
#define SERVICE_UDS_NUM    (sizeof(ServiceUDS) / sizeof(ServiceUDS_TypeDef))

#define LIN_BAUD_RATE    19200UL       /**< @brief LIN默认波特率19200bps(从机模式) */

STATIC uint32_t lin_baud_rate = LIN_BAUD_RATE;     /**< @brief 当前运行的LIN波特率(初始值19200，可被$87服务动态修改) */
STATIC uint8_t  lin_configured_NAD = 0x01;          /**< @brief 本地LIN节点地址(NAD)，从Flash加载，用于LIN帧过滤 */

typedef void (*FUNC_PTR)(void);

/**
 * @brief  LED指示灯初始化，在进入编程会话($10 0x02)时调用
 * @note   OTA升级状态指示：
 *         - TCPL01X: 初始化UART日志(115200)用于调试输出
 *         - TCPL03X: 配置PWM模块驱动LED灯带(5V LDO使能)，上电初始化RGB LED通道
 *         直接操作寄存器以减少代码体积
 * @param  None
 * @retval None
 */
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

/**
 * @brief  LED指示灯去初始化，在DFU退出时调用
 * @note   关闭LED指示：
 *         - TCPL01X: 关闭UART日志
 *         - TCPL03X: 关闭ADC参考缓冲、复位PWM模块
 *         直接操作寄存器以减少代码体积
 * @param  None
 * @retval None
 */
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

/**
 * @brief  LED指示灯翻转(闪烁)，在每接收到一包数据时调用
 * @note   用于指示OTA数据传输中：
 *         - TCPL01X: UART发送0x55作为调试指示
 *         - TCPL03X: PWM亮度翻转(0↔1000)，实现LED呼吸闪烁效果
 * @param  None
 * @retval None
 */
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

/**
 * @brief  从Bootloader跳转到应用程序固件
 * @note   跳转流程(分3步):
 *         1. 关闭所有中断(NVIC Disable)：TIMER、LINSCI/AFE、SysTick
 *         2. 去初始化LIN外设(pal_lin_deinit)，释放总线
 *         3. 设置MSP(主栈指针)为应用程序入口向量表的首字
 *         4. 跳转到应用程序复位向量(向量表第二字)
 *         Bootloader与APP共享中断向量表，跳转前必须关闭所有中断避免冲突
 * @param  None
 * @retval None (函数不返回)
 */
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

/**
 * @brief  检查LIN接收队列是否为空
 * @note   通过比较队列的head和tail指针判断：
 *         head == tail 表示队列空(无待处理数据包)
 *         head != tail 表示队列非空(有待编程数据)
 * @param  None
 * @retval 1 - 队列空；0 - 队列非空
 */
STATIC uint8_t queue_lin_empty(void)
{
    return ((dfu_ctx.queue_list.head == dfu_ctx.queue_list.tail) ? 1 : 0);
}

/**
 * @brief  从Flash加载系统配置，获取LIN节点地址(NAD)
 * @note   从SYSTEM_PARAM_BASE_ADDR读取4字节配置参数，取低字节作为NAD。
 *         若读取值为0xFF或0x00(未配置或无效)，则返回默认NAD=0x01。
 *         该NAD用于LIN UDS通信帧的地址过滤。
 * @param  None
 * @retval 配置的NAD值(1~254)，无效配置返回默认值0x01
 */
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

/**
 * @brief  擦除应用程序区的Flash扇区，为固件写入做准备
 * @note   $31 0xFF00例程触发：擦除范围从FLASH_DFU_INFO_ADDR到FLASH_APP_IMAGE_SIZE，
 *         即清除DFU信息区和整个应用程序区，确保后续编程写入干净的Flash。
 *         擦除结果记录在dfu_ctx.resp_value中供后续流程判断。
 * @param  None
 * @retval DFU_MSG_SUCCESS - 擦除成功；DFU_MSG_ERASE_ERROR - 擦除失败
 */
STATIC uint8_t dfu_image_erase(void)
{
    if (pal_store_erase(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, FLASH_DFU_INFO_SIZE + FLASH_APP_IMAGE_SIZE))
    {
        return DFU_MSG_SUCCESS;
    }

    return DFU_MSG_ERASE_ERROR;
}

/**
 * @brief  编程一包固件数据到Flash，并进行CRC回读校验
 * @note   编程流程:
 *         1. 检查目标地址是否在合法范围(FLASH_APP_ADDR ~ FLASH_APP_END_ADDR)
 *         2. 将packet->data写入Flash(pal_store_write)
 *         3. 从Flash逐字回读，累计计算CRC32(dfu_ctx.write_crc)
 *         4. 比对回读CRC与数据包自带CRC(packet->crc32)
 *         5. CRC不匹配则擦除该扇区并返回编程错误
 * @param  addr    目标Flash地址(必须≥FLASH_APP_ADDR且<FLASH_APP_END_ADDR)
 * @param  packet  数据包指针(含数据和头部CRC)
 * @param  length  写入数据长度(字节)
 * @retval DFU_MSG_SUCCESS     - 编程并校验成功
 * @retval DFU_MSG_PROGRA_ERROR - 地址越界或CRC校验失败
 */
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

/**
 * @brief  从Flash DFU信息区读取上次升级记录
 * @note   从FLASH_DFU_INFO_ADDR读取last_dfu_info_t结构体，校验magic(0xDEADBEEF)和
 *         reason(DFU_MSG_SUCCESS)是否都有效。两者同时满足才认为上次升级成功完成，
 *         Bootloader将跳转到应用程序执行。
 * @param  info  输出参数，存放读取的DFU记录
 * @retval DFU_MSG_SUCCESS - DFU记录有效(可跳转APP)；DFU_MSG_ERROR - 记录无效(进入升级模式)
 */
STATIC uint8_t last_dfu_info_get(last_dfu_info_t *info)
{
    pal_store_read(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)info, sizeof(last_dfu_info_t));

    if (DFU_INFO_MAGIC != info->magic || DFU_MSG_SUCCESS != info->reason)
    {
        return DFU_MSG_ERROR;
    }

    return DFU_MSG_SUCCESS;
}

/**
 * @brief  将DFU升级完成记录写入Flash DFU信息区
 * @note   DFU_SUCCESS时调用，将magic=0xDEADBEEF、reason=SUCCESS、版本号和时间戳
 *         写入FLASH_DFU_INFO_ADDR。下次上电Bootloader读取到此有效记录后直接跳转APP。
 * @param  info  待写入的DFU完成记录(含magic/reason/version/time等)
 * @retval DFU_MSG_SUCCESS - 写入成功
 */
STATIC uint8_t last_dfu_info_update(last_dfu_info_t *info)
{
    pal_store_write(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)info, sizeof(last_dfu_info_t));
    return (DFU_MSG_SUCCESS);
}

/**
 * @brief  退出DFU升级流程，根据结果码决定后续行为
 * @note   两种退出路径:
 *         - DFU_MSG_SUCCESS: 升级成功，更新DFU记录到Flash(magic+reason+version+time)，
 *           下次上电Bootloader读取到有效记录跳转APP
 *         - DFU_MSG_TIMEOUT: 诊断超时，重置dfu_ctx为零(恢复初始状态)，
 *           设置boot_state=UPGRADE保持升级模式等待新请求
 *         最后统一关闭LED指示
 * @param  reason  退出原因码(DFU_MSG_SUCCESS表示成功，DFU_MSG_TIMEOUT表示超时)
 * @retval None
 */
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

/**
 * @brief  发送UDS正响应(带子功能码和数据载荷)
 * @note   构建响应帧格式: [SID+0x40] [SubFunc] [Data0..DataN]
 *         通过lin_uds_send发送到LIN总线
 * @param  sid       请求SID(响应时自动+0x40转为正响应)
 * @param  sub_func  子功能码(原样回显)
 * @param  data      响应数据载荷指针(可为NULL)
 * @param  length    数据载荷长度(字节)
 * @retval None
 */
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

/**
 * @brief  发送UDS响应(正响应或负响应)
 * @note   根据resp_type选择发送方式:
 *         - POSITIVE: 调用dfu_do_notify_cp发送正响应帧(SID+0x40)
 *         - NEGATIVE: 调用lin_uds_negative_response发送负响应帧(含NRC错误码)
 * @param  resp_type  响应类型: POSITIVE(正响应) / NEGATIVE(负响应)
 * @param  sid        请求SID
 * @param  resp_value 子功能码(POSITIVE时)或NRC错误码(NEGATIVE时)
 * @retval None
 */
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

/**
 * @brief  UDS $10 诊断会话控制服务处理函数
 * @note   子功能:
 *         - 0x01: 切换至默认会话(DEFALUT_SESSION)，复位安全等级
 *         - 0x02: 切换至编程会话(PROGRAM_SESSION)，设置boot_state=UPGRADE，初始化LED
 *         - 0x03: 切换至扩展会话(实际为PROGRAM_SESSION)，同时返回Bootloader版本号
 *         其他子功能返回NRC=SUBFUNCTION_NOT_SUPPORTED
 * @param  param    UDS请求数据([0]=SID, [1]=子功能码)
 * @param  length   请求数据长度
 * @retval None
 */
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
/**
 * @brief  UDS $28 通信控制服务处理函数(当前未启用，CFG_SUPPORT_COMMUCATION=0)
 * @note   目前仅支持子功能0x03(启用通信)，其余返回NRC=SUBFUNCTION_NOT_SUPPORTED
 * @param  param    UDS请求数据
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief  UDS $87 链路控制服务处理函数，动态切换LIN波特率
 * @note   子功能:
 *         - 0x01: 设置目标波特率(param[2]指定: 1=9600, 2=19200, 5=115200)
 *         - 0x02: 执行切换(置位update_flag，由主循环中的lin_sci_baudrate_update完成实际切换)
 *         波特率切换用于OTA升级时加速数据传输(默认19200→115200)，完成后切回
 * @param  param    UDS请求数据([0]=SID, [1]=子功能, [2]=波特率标识)
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief  安全密钥比对函数，比较种子和密钥是否完全一致
 * @note   当前实现简化了UDS安全访问机制：种子(seed)=密钥(key)，无需复杂算法。
 *         逐字节比较两个缓冲区，完全匹配才返回成功。
 * @param  seed    种子缓冲区(在简化实现中直接使用security_code)
 * @param  key     密钥缓冲区(来自Tester的$27 0x02/0x04请求数据)
 * @param  length  比较长度(字节)
 * @retval 1 - 匹配成功；0 - 不匹配
 */
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

/**
 * @brief  UDS $27 安全访问服务处理函数
 * @note   UDS安全访问流程(简化的种子密钥模式):
 *         - 子功能0x01(请求种子): 返回security_code作为种子
 *         - 子功能0x02/0x04(发送密钥): 比较Tester发来的密钥与security_code，
 *           匹配则升级securityLevel=SECURITY_LEVEL1(允许固件操作)，
 *           不匹配则securityLevel=SECURITY_LEVEL0并返回NRC=INVALID_KEY
 *         当前实现简化了标准UDS安全访问(seed≠key)，直接使用固定密钥比对。
 * @param  param    UDS请求数据([0]=SID, [1]=子功能, [2..5]=密钥数据)
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief  UDS $2E 固件信息同步服务处理函数，接收Tester下发的固件参数
 * @note   功能:
 *         1. 设置op_code=DFU_CMD_SYNC_INFO，标记进入固件同步阶段
 *         2. 解析协议数据(最少14字节): [SID][SubFunc][Addr(4B)][Size(4B)][CRC(4B)]
 *         3. 验证参数合法:
 *            - image_size必须是FLASH_SECTOR_SIZE(512)的整数倍
 *            - image_addr必须在FLASH_APP_ADDR ~ FLASH_APP_END_ADDR范围
 *            - image_addr+image_size不能超过FLASH_APP_END_ADDR
 *         4. 回复Bootloader版本号、DFU_PROGRAM_LENGTH、擦除/编程等待时间等能力信息
 *         5. 初始化接收计数器(索引=1, 队列清空, CRC初始值=0xFFFFFFFF)
 * @param  param    UDS请求数据(最小14字节: 头部+地址+大小+CRC)
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief  UDS $34 请求下载服务处理函数，确认擦除完成后进入数据传输阶段
 * @note   必须在上一步擦除($31 0xFF00)完成后才能调用，否则返回NRC=REQUEST_SEQUEENCE_ERROR。
 *         子功能0x01: 检查擦除结果，若成功则op_code切换至DFU_CMD_TRANFER_START。
 *         其他子功能返回NRC=SUBFUNCTION_NOT_SUPPORTED。
 * @param  param    UDS请求数据([0]=SID, [1]=子功能码)
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief  UDS $36 传输数据服务处理函数，接收Tester下发的固件数据包
 * @note   数据包格式: [SID][块索引][Data(0~512B)][CRC32(4B)]
 *         接收逻辑:
 *         1. 检查当前状态必须为TRANFER_START且无编程错误
 *         2. 检查包长度>DFU_PACKET_HEAD_CRC_LENGTH(6字节)，否则PACKET_LEN_ERROR
 *         3. 检查块索引与期望的recevice_index一致，否则INDEX_ERROR
 *         4. 成功时累计接收字节数，当接收量≥DFU_PROGRAM_LENGTH或接收完毕时，
 *            后移tail指针触发队列调度(由$31 0xFF01例程消费)
 *         5. 每次成功接收翻转LED指示
 *         双缓冲机制: tail指向待填充缓冲区，填满后tail++，主循环检测队列非空时写入Flash
 * @param  param    UDS请求数据
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief  UDS $37 请求传输退出服务处理函数，结束数据传输阶段
 * @note   检查当前状态必须为TRANFER_START，否则NRC=REQUEST_SEQUEENCE_ERROR。
 *         成功后置op_code=DFU_CMD_TRANFER_STOP，表示数据接收完毕，
 *         后续$31 0x0202例程进行完整固件CRC校验和升级退出处理。
 * @param  param    UDS请求数据([0]=SID, [1]=子功能码)
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief  UDS $31 例程控制服务处理函数，承载OTA最核心的操作
 * @note   子功能0x02(启动例程) — 例程ID 0xFF00:
 *         擦除Flash(dfu_image_erase)，必须在SYNC_INFO完成后才能执行
 *         子功能0x01(开始例程) — 例程ID对照:
 *         - 0x0202: CRC完整性校验，比对image_crc与write_crc，然后调用dfu_process_exit
 *         - 0xFF01: Flash编程，从队列取数据包写入Flash(dfu_image_program)，
 *           每包写入后更新write_addr/write_length，发送正响应含当前块索引
 *         - 0x7221: 系统复位(NVIC_SystemReset)，升级完成跳转APP
 *         其他例程ID或子功能返回SUBFUNCTION_NOT_SUPPORTED
 * @param  param    UDS请求数据([0]=SID, [1]=子功能, [2-3]=例程ID)
 * @param  length   请求数据长度
 * @retval None
 */
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

/**
 * @brief UDS服务调度表，建立SID到处理函数的映射
 * @note  lin_diag_service_handle在ServiceUDS权限检查通过后，
 *        通过此表索引到对应的handler并执行。ECU复位($11)函数为NULL暂不处理。
 * @see   dfu_process_context_t
 */
const dfu_process_context_t dfu_process_ctx[] =
{
    { SERVICE_ECU_RESET, NULL },                           /* $11 - ECU复位(暂未实现具体handler) */
    { SERVICE_SESSION_CONTROL, session_control_handle },   /* $10 - 会话控制 */
#if 1==CFG_SUPPORT_COMMUCATION
    { SERVICE_COMMUNICATION_CONTROL, communication_control_handle }, /* $28 - 通信控制(未启用) */
#endif
    { SERVICE_LINK_CONTROL, link_control_handle },         /* $87 - 链路控制 */
    { SERVICE_SECURITY_ACCESS, security_access_handle },   /* $27 - 安全访问 */
    { SERVICE_FIRMWARE_INFO_SYNC, firmware_info_sync_handle }, /* $2E - 固件信息同步 */
    { SERVICE_ROUTINE_CONTROL, routine_control_handle },   /* $31 - 例程控制(擦除/编程/CRC/复位) */
    { SERVICE_REQUEST_DOWNLOAD, request_download_handle }, /* $34 - 请求下载 */
    { SERVICE_TRANSFER_DATA, transfer_data_handle },       /* $36 - 传输数据 */
    { SERVICE_REQUEST_TRANSFER_EXIT, request_transfer_exit_handle }, /* $37 - 请求传输退出 */
};

#define DFU_PROCESS_STEP_MAX    (sizeof(dfu_process_ctx) / sizeof(dfu_process_context_t))

/**
 * @brief  UDS诊断服务主分发入口，接收LIN UDS请求并执行权限检查和调度
 * @note   处理流程:
 *         1. 调用lin_uds_receive从LIN总线接收UDS请求(数据存入队列当前tail缓冲区)
 *         2. 提取请求SID，遍历ServiceUDS[]权限矩阵进行三层校验:
 *            a) 寻址模式检查: 请求的寻址模式是否在服务允许范围内
 *            b) 会话模式检查: 当前会话是否允许该服务
 *            c) 安全等级检查: 当前安全等级是否满足服务要求
 *         3. 校验通过后，通过dfu_process_ctx[]调度表调用对应的handler
 *         4. 成功处理后重置UDS超时计数器(uds_timeout=0)
 *         权限不通过则回复NRC=SERVICE_NOT_SUPPORTED
 * @param  None
 * @retval None
 */
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

/**
 * @brief  LIN SCI波特率动态更新函数，在主循环中检测并执行切换
 * @note   $87链路控制服务设置update_flag和baudrate后，主循环调用此函数:
 *         1. 检查update_flag是否置位，未置位直接返回
 *         2. 清除update_flag
 *         3. 检查目标波特率与当前是否相同，相同则跳过
 *         4. 更新全局lin_baud_rate变量
 *         5. 重新初始化LIN从机(pal_lin_init)使新波特率生效
 * @param  None
 * @retval None
 */
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

/**
 * @brief  UDS诊断超时监控函数，3秒无有效UDS请求则退出DFU模式
 * @note   在os_task_update中周期性调用(通常每个OS tick一次):
 *         1. 如果op_code!=0(DFU正在进行中)，递增超时计数器uds_timeout
 *         2. 当uds_timeout > LIN_UDS_TIMEOUT(3000ms)时触发超时处理:
 *            a) 若当前波特率非默认值(19200)，先恢复默认波特率
 *            b) 调用dfu_process_exit(DFU_MSG_TIMEOUT)退出升级
 *         3. 如果op_code==0(无活动)，清零超时计数器
 *         正常UDS请求到达时，lin_diag_service_handle会清零uds_timeout
 * @param  None
 * @retval None
 */
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

/**
 * @brief  DFU管理器初始化，Bootloader启动时的首个调用
 * @note   初始化步骤:
 *         1. 从Flash加载NAD配置(system_cfg_load)
 *         2. 初始化调试日志(如果CFG_SUPPORT_DEBUG启用)
 *         3. 清零dfu_ctx全局上下文
 *         4. 初始化LIN从机(默认波特率19200)，注册中断回调
 *         Bootloader在main函数或启动文件中首先调用此函数完成基础初始化
 * @param  None
 * @retval None
 */
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

/**
 * @brief  Bootloader主循环函数，负责UDS请求处理和跳转决策
 * @note   执行逻辑:
 *         1. 每次调用先执行lin_diag_service_handle处理UDS请求
 *         2. IDLE状态下:
 *            - 延时1ms
 *            - 每42次(约42ms+5ms=47ms)检查一次DFU记录:
 *              a) DFU记录有效(last_dfu_info_get成功) → 状态切至USER_APP
 *              b) DFU记录无效 → 状态切至UPGRADE(等待UDS升级请求)
 *            - 42ms等待窗口期间若有UDS $10 0x02编程会话请求，
 *              则提前进入UPGRADE模式
 *         3. USER_APP状态: 调用JumpToApp跳转至应用程序(不再返回)
 *         4. UPGRADE状态: 由其他UDS服务驱动，等待Tester下发固件
 * @param  None
 * @retval None (跳转到APP后不返回)
 */
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

/**
 * @brief  OS周期任务更新函数，由系统滴答定时器或RTOS tick周期性调用
 * @note   当前功能仅调用dfu_timeout_handle实现UDS超时监控。
 *         在无RTOS的裸机系统中，此函数在SysTick中断或主循环中周期性调用。
 * @param  None
 * @retval None
 */
void os_task_update(void)
{
    dfu_timeout_handle();
}
