#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "tc_log.h"
#include "tcae10_ll_def.h"
#include "system_tcae10.h"

//*******************************************************************************************

#if LOG_FAST_MODE
    #define LOG_WRITE_BUFFER_LEN    3                   /*!< 写缓冲区长度 */
#else
    #define LOG_WRITE_BUFFER_LEN    6                   /*!< 写缓冲区长度 */
#endif

#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
    uint8_t g_bUDSReadLogInfo = 0;
#endif

#define LOG_HEADER_LEN          2                       /*!< LOG命令包头长度 */

#if LOG_TO_RAMBUFFER
    static char TcLogRamBuffer[LOG_RAMBUFFER_SIZE];     /*!< 日志缓冲区大小 */
    static int TcLogRamBufferPos = 0;                   /*!< 日志缓冲区指针位置 */
#endif

static T_TcLogPrintPin printpin;          //打印引脚

#if LOG_SYMBOL_SWITCH
    /**
    * @brief       发送日志数据包，不分包发送
    * @param[in]   cmd 日志命令
    * @param[in]   address 数据地址
    * @param[in]   bytesNum 数据长度，单位字节
    * @retval
    */
    static void TcSendCmdPackNoSplit(uint8_t cmd, uint32_t address, uint32_t bytesNum);
#endif

/**
  * @brief      查表法计算CRC-16校验, CRC-16-IBM,生成多项式为x^16+x^15+x^2+1,poly:0xA001（逆序）
  * @param[in]  base 初始值
  * @param[in]  pBuff输入指针
  * @param[in]  nLen 输入长度
  * @return     一字CRC值
  */
static uint16_t CRC16_IBM_TAB(uint16_t base, uint8_t *pBuff, uint16_t nLen);

#if defined (__ARMCC_VERSION)
/**
  * @brief      发送字符
  * @param[in]  ch 字符
  * @param[in]  f 文件描述符
  * @return     发送的字符
  */
int  fputc(int ch, FILE *f)
{
#if LOG_LOCAL_LEVEL == TC_LOG_NONE
#else
#if LOG_INTERFACE_TYPE == LOG_INTERFACE_UART
    while (PRINT_UART->STATUS_F.TX_BUSY == 1);
    PRINT_UART->TX_DATA_F.TX_DATA = (uint8_t) ch;
#elif LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN_UART
//    while (LIN_SCI1->STATUS_F.TX_STATE);
    while (LIN_SCI1->STATUS_F.TX_FIFO_FULL);
    LIN_SCI1->TX_DATA_F.TX_DATA = (uint8_t)ch;
    while (LIN_SCI1->STATUS_F.TX_STATE);
#else
#endif
#endif
    return (ch);
}
#elif defined (__GNUC__)
/**
  * @brief      发送字符
  * @param[in]  ch 字符
  * @return     发送的字符
  */
int __io_putchar(int ch)
{
    ll_uart_sendbyte(ch);
    return (ch);
}
#endif

#if defined (__ICCARM__)//iar version
int putchar(int ch)
{
#if LOG_LOCAL_LEVEL == TC_LOG_NONE
#else
#if LOG_INTERFACE_TYPE == LOG_INTERFACE_UART
    while (PRINT_UART->STATUS_F.TX_BUSY == 1);
    PRINT_UART->TX_DATA_F.TX_DATA = (uint8_t) ch;
#elif LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN_UART
//    while (LIN_SCI1->STATUS_F.TX_STATE);
    while (LIN_SCI1->STATUS_F.TX_FIFO_FULL);
    LIN_SCI1->TX_DATA_F.TX_DATA = (uint8_t)ch;
    while (LIN_SCI1->STATUS_F.TX_STATE);
#else
#endif
#endif
    return (ch);
}
#endif

#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN_UART
static uint32_t lin_uart_clock_get(void)
{
    uint32_t reg_val = 0;
    uint8_t status = 0;
    (void)(&status);

    reg_val = CRG->LIN_SCI1_CLKRST_CTRL;
    reg_val &= CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_MASK;
    reg_val >>= CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_SHIFT;

    return ((SystemGetHClkFreq()) / (reg_val + 1));
}
static void lin_uart_divided(unsigned int dlh, unsigned int dll, unsigned int frac)
{
    uint32_t reg_val = 0;
    uint8_t status = 0;
    (void)(&status);

    dlh &= 0x0f;
    dll &= 0xff;
    frac &= 0x0f;
    reg_val |= (frac << 12 | dlh << 8 | dll);
    LIN_SCI1->BAUD_CFG = reg_val;
}
void lin_uart_setbaudrate(uint32_t baudrate)
{
    uint32_t  clk;

    uint32_t div;
    uint8_t frac = 0;
    clk = lin_uart_clock_get();
    /* Fck/(16*Baud_Rate) */
    div = clk >> 4;
    frac = div % baudrate;
    frac = (frac << 4) / baudrate;
    div  = div / baudrate;
    lin_uart_divided(((div >> 8) & 0xff), ((div >> 0) & 0xff), frac);
}
#endif

/**
* @brief       打印初始化
* @param[in]   baud 波特率
* @retval
*/
void PrintInit(uint32_t baud)
{
#if LOG_INTERFACE_TYPE == LOG_INTERFACE_UART
    ll_uart_init(baud);
#elif LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN_UART

    //ll_lin_sci_uart_deinit();
    CRG_CONFIG_UNLOCK();
    CRG->LIN_SCI1_CLKRST_CTRL_F.RST_LIN_SCI1 = 1;
    __NOP();
    __NOP();
    CRG->LIN_SCI1_CLKRST_CTRL_F.RST_LIN_SCI1 = 0;
    __NOP();
    __NOP();
    CRG_CONFIG_LOCK();

    CRG_CONFIG_UNLOCK();
    //clk
    CRG->LIN_SCI1_CLKRST_CTRL_F.FCLK_DIV_LIN_SCI1 = 0;
    CRG->LIN_SCI1_CLKRST_CTRL_F.FCLK_EN_LIN_SCI1 = 0x1;
    CRG->LIN_SCI1_CLKRST_CTRL_F.PCLK_EN_LIN_SCI1 = 0x1;
    CRG_CONFIG_LOCK();

    lin_uart_setbaudrate(baud);
    LIN_SCI1->CTRL_UART_F.MODE = 0x1;
    LIN_SCI1->CTRL_UART_F.MP_TX_ADDR_DATA_SEL = 0;
    LIN_SCI1->CTRL_UART_F.MP_MODE_EN = 1; //multiprocessor mode
    LIN_SCI1->CTRL_F.LPBK_MODE = 0;
    LIN_SCI1->BRK_SYNC_CFG_F.BRK_NUM = 0x0d;
    LIN_SCI1->BRK_SYNC_CFG_F.DLT_NUM = 0;
    LIN_SCI1->TX_CFG_F.CHK_PT_SEL = 0;
    LIN_SCI1->CTRL_F.RX_FIFO_CLR = 1;
    LIN_SCI1->CTRL_F.TX_FIFO_CLR = 1;
    LIN_SCI1->CTRL_F.RX_ABORT = 0;
    LIN_SCI1->RX_CFG_F.MP_SLAVE_ADDR = 0x2A;

    LIN_SCI1->CTRL_F.GLB_EN = 1;
    LIN_SCI1->CTRL_F.TX_EN = 1;
    LIN_SCI1->CTRL_F.RX_EN = 0;
    LIN_SCI1->CTRL_F.AUTO_BAUD_EN = 0;
#else
#endif
}

void PrintSetPin(T_TcLogPrintPin pin)
{
    printpin = pin;

    switch (pin)
    {
        case PRINT_GPIO2:
            ll_uart_set_printpin(UART_PRINT_GPIO2);
            break;
        case PRINT_GPIO3:
            ll_uart_set_printpin(UART_PRINT_GPIO3);
            break;
        case PRINT_GPIO5:
            ll_uart_set_printpin(UART_PRINT_GPIO5);
            break;
        case PRINT_GPIO6:
            ll_uart_set_printpin(UART_PRINT_GPIO6);
            break;
    }
}

void PrintEnterSleep(void)
{
    switch (printpin)
    {
        case PRINT_GPIO2:
            ll_gpio_afio_config(GPIO_PIN_2, (gpio_afio_mux_e)GPIO2_SOFTWARE_INPUT_FUNCTION_GPIO);
            break;
        case PRINT_GPIO3:
            ll_gpio_afio_config(GPIO_PIN_3, (gpio_afio_mux_e)GPIO3_SOFTWARE_INPUT_FUNCTION_GPIO);
            break;
        case PRINT_GPIO5:
            ll_gpio_afio_config(GPIO_PIN_5, (gpio_afio_mux_e)GPIO5_SOFTWARE_INPUT_FUNCTION_GPIO);
            break;
        case PRINT_GPIO6:
            break;
    }
}

void PrintWakeup(void)
{
    PrintSetPin(printpin);
}

#if LOG_USE_STD_WRITE
/**
  * @brief      自定义fwrite实现
  * @param[in]  ptr 数据地址
  * @param[in]  blksize 块尺寸
  * @param[in]  blknum 块个数
  * @param[in]  fp 文件描述符
  * @return     发送数据块数
  */
#define TcFWrite(ptr,blksize,blknum,fp)     fwrite((ptr),(blksize),(blknum),(fp))
#else
#if LOG_TO_RAMBUFFER
/**
  * @brief      自定义fwrite实现
  * @param[in]  ptr 数据地址
  * @param[in]  blksize 块尺寸
  * @param[in]  blknum 块个数
  * @param[in]  fp 文件描述符
  * @return     发送数据块数
  */
static size_t TcFWrite(const void *ptr, size_t blksize, size_t blknum, FILE *fp)
{
    const char *cp = ptr;

    for (size_t i = 0; i < blknum; ++i)
    {
        for (size_t j = 0; j < blksize; ++j)
        {
            if (TcLogRamBufferPos < LOG_RAMBUFFER_SIZE)
            {
                TcLogRamBuffer[TcLogRamBufferPos++] = *cp;
            }
            else
            {
                return i;
            }
            ++cp;
        }
    }

    return blknum;
}
#else
/**
  * @brief      自定义fwrite实现
  * @param[in]  ptr 数据地址
  * @param[in]  blksize 块尺寸
  * @param[in]  blknum 块个数
  * @param[in]  fp 文件描述符
  * @return     发送数据块数
  */
static size_t TcFWrite(const void *ptr, size_t blksize, size_t blknum, FILE *fp)
{
    const char *cp = ptr;

    for (size_t i = 0; i < blknum; ++i)
    {
        for (size_t j = 0; j < blksize; ++j)
        {
#if defined (__ICCARM__)//iar version
            putchar(*cp);
#else
            fputc(*cp, fp);
#endif
            ++cp;
        }
    }

    return blksize;
}
#endif
#endif

#if LOG_CUSTOM_OUTPUT       //用户自定义输出
/**
  * @brief      自定义写日志数据
  * @param[in]  format 日志格式符
  * @return
  */
void TcLogWrite(const char *format, ...)
{
    static char buffer[LOG_PRINT_BUFFER_SIZE];
    va_list vargs;

    va_start(vargs, format);
    vsnprintf(buffer, LOG_PRINT_BUFFER_SIZE - 1, format, vargs);
    va_end(vargs);

    TcFWrite(buffer, strlen(buffer), 1, stdout);
}
#endif

#if !LOG_FAST_MODE
/**
* @brief       打印符号表
* @param[in]   p map文件中定义的data符号的变量的地址
* @param[in]   len 数据长度，单位byte
* @retval
*/
int TC_LOG_SYMBOL(const void *p, int len)
{
#if LOG_SYMBOL_SWITCH
    int i;

    for (i = 0; i < LOG_SEND_REPEAT; i++)
    {
        TcSendCmdPackNoSplit(TC_LOG_CMD_SYMBOL, (uint32_t)p, len);
    }
#endif

    return 1;
}
#endif

/**
* @brief       带名称的符号表打印，内部使用
* @param[in]   cmd 打印符号命令
* @param[in]   name 名称
* @param[in]   p 数据地址
* @param[in]   len 数据长度，单位byte
* @retval
*/
int TC_LOG_SYMBOL_NAMED(uint8_t cmd, const char *name, const void *p, int len)
{
#if LOG_SYMBOL_SWITCH
    uint8_t wbuf[LOG_WRITE_BUFFER_LEN];
    uint8_t notedesp[1];
    uint16_t crc16;
    uint16_t nameLen;
    int i;

#if LOG_FAST_MODE
    uint8_t pklen;
#else
    uint32_t pklen;
#endif

    nameLen = strlen(name);
    if (nameLen > 255)                      //超过最大备注长度
    {
        return -1;
    }
    notedesp[0] = nameLen;

    for (i = 0; i < LOG_SEND_REPEAT; i++)
    {
        wbuf[0] = 0xA5;
        wbuf[1] = cmd;
        pklen = len + nameLen + 1;
        memcpy(&wbuf[2], &pklen, sizeof(pklen));

        crc16 = 0xffff;
        crc16 = CRC16_IBM_TAB(crc16, wbuf, sizeof(wbuf));
        crc16 = CRC16_IBM_TAB(crc16, notedesp, sizeof(notedesp));
        crc16 = CRC16_IBM_TAB(crc16, (uint8_t *)name, nameLen);
        crc16 = CRC16_IBM_TAB(crc16, (uint8_t *)p, len);

        //发送命令包
        TcFWrite(wbuf, sizeof(wbuf), 1, stdout);
        TcFWrite(notedesp, sizeof(notedesp), 1, stdout);
        TcFWrite(name, nameLen, 1, stdout);
        TcFWrite((void *)p, len, 1, stdout);
        TcFWrite(&crc16, sizeof(crc16), 1, stdout);
    }
#endif

    return 1;
}

/**
* @brief       打印系统状态
* @param[in]   status 系统状态
* @retval
*/
int TC_LOG_SYSTEM_STATUS(T_TcLogSystemStatus status)
{
#if LOG_SYSTEM_STATUS
    uint8_t wbuf[LOG_WRITE_BUFFER_LEN];
    uint8_t v = status;
    uint16_t crc16;

#if LOG_FAST_MODE
    uint8_t pklen;
#else
    uint32_t pklen;
#endif

    wbuf[0] = 0xA5;
    wbuf[1] = TC_LOG_CMD_SYSTEM_STATUS;
    pklen = 1;
    memcpy(&wbuf[2], &pklen, sizeof(pklen));

    crc16 = 0xffff;
    crc16 = CRC16_IBM_TAB(crc16, wbuf, sizeof(wbuf));
    crc16 = CRC16_IBM_TAB(crc16, (uint8_t *)&v, sizeof(v));

    for (int i = 0; i < LOG_SEND_REPEAT; i++)
    {
        //发送命令包
        TcFWrite(wbuf, sizeof(wbuf), 1, stdout);
        TcFWrite(&v, sizeof(v), 1, stdout);
        TcFWrite(&crc16, sizeof(crc16), 1, stdout);
    }
#endif
    return 1;
}

/**
* @brief       打印原始数据，内部使用
* @param[in]   cmd 打印原始数据命令
* @param[in]   type 数据类型
* @param[in]   status 数据状态
* @param[in]   p 数据地址
* @param[in]   len 数据长度，单位byte
* @retval
*/
int TC_LOG_RAWDATA(uint8_t cmd, T_TcLogRawdataType type, uint8_t status, const void *p, int len)
{
#if LOG_RAWDATA
    uint8_t wbuf[LOG_WRITE_BUFFER_LEN];
    uint8_t rawheader[LOG_HEADER_LEN];
    uint16_t crc16;
    int i;

#if LOG_FAST_MODE
    uint8_t pklen;
#else
    uint32_t pklen;
#endif

    rawheader[0] = type;
    rawheader[1] = status;

    for (i = 0; i < LOG_SEND_REPEAT; i++)
    {
        wbuf[0] = 0xA5;
        wbuf[1] = cmd;
        pklen = len + 2;
        memcpy(&wbuf[2], &pklen, sizeof(pklen));

        crc16 = 0xffff;
        crc16 = CRC16_IBM_TAB(crc16, wbuf, sizeof(wbuf));
        crc16 = CRC16_IBM_TAB(crc16, rawheader, sizeof(rawheader));
        crc16 = CRC16_IBM_TAB(crc16, (uint8_t *)p, len);

        //发送命令包
        TcFWrite(wbuf, sizeof(wbuf), 1, stdout);
        TcFWrite(rawheader, sizeof(rawheader), 1, stdout);
        TcFWrite((void *)p, len, 1, stdout);
        TcFWrite(&crc16, sizeof(crc16), 1, stdout);
    }
#endif
    return 1;
}

#if LOG_SYMBOL_SWITCH
/**
* @brief       发送日志数据包，不分包发送
* @param[in]   cmd 日志命令
* @param[in]   address 数据地址
* @param[in]   bytesNum 数据长度，单位字节
* @retval
*/
static void TcSendCmdPackNoSplit(uint8_t cmd, uint32_t address, uint32_t bytesNum)
{
    uint8_t wbuf[LOG_WRITE_BUFFER_LEN];
    uint16_t crc16;

#if LOG_FAST_MODE
    uint8_t pklen;
#else
    uint32_t pklen;
#endif

    wbuf[0] = 0xA5;
    wbuf[1] = cmd;
    pklen = bytesNum + 4;
    memcpy(&wbuf[2], &pklen, sizeof(pklen));

    crc16 = 0xffff;
    crc16 = CRC16_IBM_TAB(crc16, wbuf, sizeof(wbuf));
    crc16 = CRC16_IBM_TAB(crc16, (uint8_t *)&address, sizeof(address));
    crc16 = CRC16_IBM_TAB(crc16, (uint8_t *)address, bytesNum);

    //发送命令包
    TcFWrite(wbuf, sizeof(wbuf), 1, stdout);
    TcFWrite(&address, sizeof(address), 1, stdout);
    TcFWrite((void *)address, bytesNum, 1, stdout);
    TcFWrite(&crc16, sizeof(crc16), 1, stdout);
}

#endif

#if LOG_TO_RAMBUFFER
/**
* @brief       获取Ram缓冲区数据
* @param[out]   bp 缓冲区指针，为NULL表示仅读取缓冲区数据长度
* @retval       缓冲区数据长度
*/
int TC_LOG_GetRamBuffer(const char **bp)
{
    if (bp)
    {
        *bp = TcLogRamBuffer;
    }
    return TcLogRamBufferPos;
}

/**
* @brief       清空缓冲区
* @retval
*/
void TC_LOG_ClrRamBuffer(void)
{
    TcLogRamBufferPos = 0;
}
#endif

/**
* @brief       CRC16表，CRC-16-IBM,生成多项式为x^16+x^15+x^2+1,poly:0xA001（逆序）
*/
static const uint16_t CRC16_IBM_Tab[] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
};

/**
  * @brief      查表法计算CRC-16校验, CRC-16-IBM,生成多项式为x^16+x^15+x^2+1,poly:0xA001（逆序）
  * @param[in]  base 初始值
  * @param[in]  pBuff输入指针
  * @param[in]  nLen 输入长度
  * @return     一字CRC值
  */
static uint16_t CRC16_IBM_TAB(uint16_t base, uint8_t *pBuff, uint16_t nLen)
{
    uint16_t i = 0;
    uint16_t wResult = base;
    uint16_t wTableNo = 0;

    for (i = 0; i < nLen; i++)
    {
        wTableNo = ((wResult & 0xff) ^ (pBuff[i] & 0xff));
        wResult = ((wResult >> 8) & 0xff) ^ CRC16_IBM_Tab[wTableNo];
    }

    return (wResult);
}
