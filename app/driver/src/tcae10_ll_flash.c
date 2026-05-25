/**
 *****************************************************************************
 * @brief   flash Source file.
 *
 * @file    tcae10_ll_flash.c
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

#include "tcae10.h"
#include "system_tcae10.h"
#include "tcae10_ll_flash.h"

#define FLASH_WR_BYTE_ALIGN    (8)

#if 8 == FLASH_WR_BYTE_ALIGN
    typedef  uint64_t  flash_size_t;
#else
    typedef  uint32_t  flash_size_t;
#endif

#define FLASH_DRV_CIPER_SIZE   (56)

typedef uint8_t (*tpfFLASH_DRV_EraseSector)(void);
typedef uint8_t (*tpfFLASH_DRV_Program)(uint32_t addr, uint8_t *ptr);       //写8字节

__attribute__((section(".ARM.__at_0x10000000"))) uint8_t flash_driver[66] = {0};
static const uint8_t flash_driver_ciper[FLASH_DRV_CIPER_SIZE] =
{
    0x02, 0x01, 0x01, 0x01, 0x1A, 0x01, 0x01, 0x01,
    0x05, 0x49, 0x02, 0x69, 0x02, 0x23, 0x0B, 0x44,
    0x03, 0x61, 0xC2, 0x6B, 0x8A, 0x08, 0xFD, 0xD5,
    0x71, 0x48, 0xC1, 0x47, 0x21, 0x01, 0xFF, 0x01,
    0x0B, 0x69, 0x4A, 0x69, 0x42, 0x61, 0x03, 0x61,
    0x03, 0x49, 0x02, 0x69, 0x4A, 0x08, 0xFD, 0xD5,
    0x71, 0x48, 0xC1, 0x47, 0x4D, 0x01, 0xFF, 0x01
};

tpfFLASH_DRV_EraseSector    g_pfFLASH_DRV_EraseSector = (tpfFLASH_DRV_EraseSector)(0x10000009u);
tpfFLASH_DRV_Program        g_tpfFLASH_DRV_Program = (tpfFLASH_DRV_Program)(0x10000021u);

/**
 * @brief   初始化Flash控制器时序配置
 * @param   None
 * @note    配置读时序、恢复时序、擦除/编程建立时序、编程时序、擦除时序和等待时序。
 *         时序参数基于系统时钟频率计算。
 * @retval  None
 */
void ll_flash_init(void)
{
    FLASH_UNLOCK_CONFIG();

    /* 读时序配置 */
    EFLASH->RD_TIME_CFG      = 0X00003033;
    /* 恢复时序配置 */
    EFLASH->RCV_TIME_CFG     = 0x00035CDC;
    /* 擦除或编程建立时序配置 */
    EFLASH->NVS_TIME_CFG     = 0x00016058;
    /* 编程时序配置 */
    EFLASH->PROG_TIME_CFG    = 0x00161ADC;
    /* 擦除时序配置 */
    EFLASH->ERASE_TIME_CFG   = 0x00000809;
    /* 等待时序配置 */
    EFLASH->LATENCY_TIME_CFG = 0x0000301A;

    FLASH_LOCK_CONFIG();
}

/**
 * @brief   获取Flash地址对应的扇区号
 * @param   type - Flash类型（NVM/NVR）
 * @param   addr - Flash地址
 * @note    扇区大小512字节，扇区号 = (addr - 基址) / 512
 * @retval  扇区索引号
 */
static uint16_t ll_flash_sector_get(flash_type_e type, uint32_t addr)
{
    return ((addr - NVM_FLASH_BASE_ADDR) >> 9);                  /* 每扇区512字节，右移9位 */
}

/**
 * @brief   检查Flash地址范围是否有效
 * @param   type   - Flash类型（NVM/NVR）
 * @param   addr   - 起始地址
 * @param   length - 数据长度
 * @retval  1 - 地址有效，0 - 地址超出范围
 */
static uint8_t ll_flash_addr_valid_check(flash_type_e type, uint32_t addr, uint32_t length)
{
    return (!((addr + length) > NVM_FLASH_SIZE));                /* 检查是否超出NVM最大地址 */
}

/**
 * @brief   扇区擦除（使用RAM中驱动代码）
 * @param   type   - Flash类型（NVM/NVR）
 * @param   addr   - 起始地址
 * @param   length - 擦除长度
 * @note    计算需要擦除的扇区范围，设置扇区索引后调用RAM驱动执行擦除。
 *         擦除前禁能只读保护。
 * @retval  None
 */
static void ll_flash_erase_reg_drv(flash_type_e type, uint32_t addr, uint32_t length)
{
    uint16_t sector_start = ll_flash_sector_get(type, addr);
    uint16_t sector_end = ll_flash_sector_get(type, (addr + length - 1)) + 1;

    EFLASH->OP_CTRL_F.RDONLY_EN = false;                         /* 关闭只读保护，允许写入 */

    for (uint32_t index = sector_start; index < sector_end; index++)
    {
        EFLASH->ERASE_CFG_F.SECTOR_INDEX = index;                /* 设置扇区索引 */
        g_pfFLASH_DRV_EraseSector();                             /* 调用RAM驱动执行扇区擦除 */
    }

    EFLASH->OP_CTRL_F.RDONLY_EN = true;                          /* 恢复只读保护 */
}

/**
 * @brief   页编程写入（使用RAM中驱动代码）
 * @param   addr   - 目标地址
 * @param   buffer - 数据缓冲区
 * @param   length - 数据长度
 * @note    处理地址不对齐情况，按8字节对齐写入。
 *         先读回原始数据补齐未对齐部分，调用RAM驱动执行编程。
 * @retval  None
 */
static void ll_flash_write_reg_drv(uint32_t addr, uint8_t *buffer, uint32_t length)
{
    uint32_t remain_length, reseverd_length;
    flash_size_t temp_data;
    uint32_t offset = addr % FLASH_BYTE_ALIGN;
    uint8_t *ptr = (uint8_t *)&temp_data;
    uint32_t write_len __attribute__((unused));

    EFLASH->OP_CTRL_F.RDONLY_EN = false;                         /* 关闭只读保护 */

    /* 处理起始地址不对齐 */
    if (offset)
    {
#if 8 == FLASH_WR_BYTE_ALIGN
        temp_data = 0xFFFFFFFFFFFFFFFF;
#else
        temp_data = 0xFFFFFFFF;
#endif
        *((volatile flash_size_t *)ptr) = *((volatile flash_size_t *)(addr - offset)); /* 读回原始数据 */
        write_len = (length > (FLASH_WR_BYTE_ALIGN - offset)) ? FLASH_WR_BYTE_ALIGN - offset : length;
        memcpy((uint8_t *)(ptr + offset), (uint8_t *)buffer, write_len); /* 补齐对齐 */
#if 8 == FLASH_WR_BYTE_ALIGN
        g_tpfFLASH_DRV_Program(addr - offset, ptr);              /* 调用RAM驱动编程8字节 */
#else
        g_tpfFLASH_DRV_Program(addr - offset, ptr);
#endif
        offset = write_len;
    }

    remain_length = length - offset;
    reseverd_length = remain_length % FLASH_WR_BYTE_ALIGN;

    /* 按对齐长度循环编程 */
    if (remain_length > reseverd_length)
    {
        for (uint32_t i = 0; i < remain_length - reseverd_length; i += FLASH_WR_BYTE_ALIGN)
        {
            memcpy((uint8_t *)ptr, (uint8_t *)(buffer + offset), sizeof(flash_size_t));
#if 8 == FLASH_WR_BYTE_ALIGN
            g_tpfFLASH_DRV_Program(addr + offset, ptr);
#else
            g_tpfFLASH_DRV_Program(addr + offset, ptr);
#endif
            offset += FLASH_WR_BYTE_ALIGN;
        }
    }

    /* 处理末尾多余数据 */
    if (reseverd_length)
    {
#if 8 == FLASH_WR_BYTE_ALIGN
        temp_data = 0xFFFFFFFFFFFFFFFF;
#else
        temp_data = 0xFFFFFFFF;
#endif
        memcpy((uint8_t *)ptr, (uint8_t *)&buffer[length - reseverd_length], reseverd_length);
#if 8 == FLASH_WR_BYTE_ALIGN
        g_tpfFLASH_DRV_Program(addr + offset, ptr);
#else
        g_tpfFLASH_DRV_Program(addr + offset, ptr);
#endif
    }

    EFLASH->OP_CTRL_F.RDONLY_EN = true;                          /* 恢复只读保护 */
}

/**
 * @brief   从Flash指定地址读取数据
 * @param   addr   - 源地址
 * @param   buffer - 目标缓冲区
 * @param   length - 读取长度
 * @note    处理地址不对齐情况，按对齐长度循环读取
 * @retval  None
 */
static void ll_flash_read_reg(uint32_t addr, uint8_t *buffer, uint32_t length)
{
    uint32_t remain_length, reseverd_length;
    flash_size_t temp_data;

    uint32_t offset = addr % FLASH_BYTE_ALIGN;
    uint8_t *ptr = (uint8_t *)&temp_data;
    uint32_t read_len __attribute__((unused));

    if (offset)
    {
        *((volatile flash_size_t *)ptr) = *((volatile flash_size_t *)(addr - offset)); /* 读回对齐数据 */
        read_len = (length > (FLASH_WR_BYTE_ALIGN - offset)) ? FLASH_WR_BYTE_ALIGN - offset : length;
        memcpy((uint8_t *)(buffer), (uint8_t *)(ptr + offset), read_len); /* 拷贝有效字节 */
        offset = read_len;
    }

    remain_length = length - offset;
    reseverd_length = remain_length % FLASH_WR_BYTE_ALIGN;

    if (remain_length > reseverd_length)
    {
        for (uint32_t i = 0; i < remain_length - reseverd_length; i += FLASH_WR_BYTE_ALIGN)
        {
            *((volatile flash_size_t *)ptr) = *((volatile flash_size_t *)(addr + offset)); /* 读取对齐数据 */
            memcpy((uint8_t *)(buffer + offset), (uint8_t *)ptr, sizeof(flash_size_t));
            offset += FLASH_WR_BYTE_ALIGN;
        }
    }

    if (reseverd_length)
    {
        *((volatile flash_size_t *)ptr) = *((volatile flash_size_t *)(addr + offset)); /* 读取剩余数据 */
        memcpy((uint8_t *)(buffer + offset), (uint8_t *)ptr, reseverd_length);
    }
}

/**
 * @brief   全片擦除Flash
 * @note    屏蔽所有中断，解锁配置，触发全片擦除，等待完成
 * @retval  0 - 擦除成功
 */
int ll_flash_erase_chip(void)
{
    /* 屏蔽所有中断（除NMI和HardFault） */
    __disable_irq();

    FLASH_UNLOCK_CONFIG();

    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;                      /* 选择NVM扇区 */
    EFLASH->ERASE_TRIG_F.CHIP_ERASE_TRIG = 1;                    /* 触发全片擦除 */

    while (EFLASH->STATUS_F.ERASE_BUSY_STATUS == 1)              /* 等待擦除完成 */
    {
        ;
    }

    FLASH_LOCK_CONFIG();

    /* 恢复中断 */
    __enable_irq();

    return 0;
}

/**
 * @brief   Flash扇区擦除
 * @param   type   - Flash类型（仅支持FLASH_TYPE_NVM）
 * @param   addr   - 起始地址
 * @param   length - 擦除长度
 * @note    解码Flash驱动到RAM中执行，屏蔽中断，擦除后销毁驱动代码。
 *         使用RAM驱动擦除是出于安全考虑，防止驱动代码被读取。
 * @retval  0 - 擦除成功，-1 - 参数错误
 */
int ll_flash_erase(flash_type_e type, uint32_t addr, uint32_t length)
{
    int res = 0;

    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }
    /* 解码Flash驱动到RAM */
    for (int i = 0; i < FLASH_DRV_CIPER_SIZE; i++)
    {
        flash_driver[i] = flash_driver_ciper[i] - 1;
    }
    flash_driver[30] = 0xFF;
    flash_driver[54] = 0xFF;
    /* 屏蔽所有中断 */
    __disable_irq();

    FLASH_UNLOCK_CONFIG();

    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;                      /* 选择NVM扇区 */

    ll_flash_erase_reg_drv(type, addr, length);                  /* 执行擦除 */

    FLASH_LOCK_CONFIG();

    /* 恢复中断 */
    __enable_irq();
    /* 销毁Flash驱动RAM代码 */
    memset(flash_driver, 0, FLASH_DRV_CIPER_SIZE);
    return res;
}

/**
 * @brief   从Flash读取数据到缓冲区
 * @param   type   - Flash类型
 * @param   addr   - 源地址
 * @param   buffer - 目标缓冲区指针
 * @param   length - 读取字节数
 * @retval  0 - 读取成功，-1 - 地址无效
 */
int ll_flash_read(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if (false == ll_flash_addr_valid_check(type, addr, length))
    {
        return -1;
    }

    ll_flash_read_reg(addr, buffer, length);                     /* 执行读取 */

    return 0;
}

/**
 * @brief   写数据到Flash（页编程）
 * @param   type   - Flash类型（仅支持FLASH_TYPE_NVM）
 * @param   addr   - 目标地址
 * @param   buffer - 数据缓冲区指针
 * @param   length - 数据长度
 * @note    解码Flash驱动到RAM中执行编程，屏蔽中断，编程后销毁驱动代码。
 *         使用RAM驱动编程是出于安全考虑。
 * @retval  0 - 写入成功，-1 - 参数错误
 */
int ll_flash_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }
    /* 解码Flash驱动到RAM */
    for (int i = 0; i < FLASH_DRV_CIPER_SIZE; i++)
    {
        flash_driver[i] = flash_driver_ciper[i] - 1;
    }
    flash_driver[30] = 0xFF;
    flash_driver[54] = 0xFF;
    /* 屏蔽所有中断 */
    __disable_irq();

    FLASH_UNLOCK_CONFIG();

    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;

    ll_flash_write_reg_drv(addr, buffer, length);                /* 执行编程 */

    FLASH_LOCK_CONFIG();

    /* 恢复中断 */
    __enable_irq();
    /* 销毁Flash驱动RAM代码 */
    memset(flash_driver, 0, FLASH_DRV_CIPER_SIZE);
    return 0;
}

/**
 * @brief   智能写Flash（先擦除后编程）
 * @param   type   - Flash类型（仅支持FLASH_TYPE_NVM）
 * @param   addr   - 目标地址
 * @param   buffer - 数据缓冲区指针
 * @param   length - 数据长度
 * @note    组合调用擦除和编程操作，先擦除目标扇区再写入数据。
 *         适用于需要更新已编程区域的场景。
 * @retval  0 - 操作成功，-1 - 参数错误
 */
int ll_flash_smart_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }

    /* 解码Flash驱动到RAM */
    for (int i = 0; i < FLASH_DRV_CIPER_SIZE; i++)
    {
        flash_driver[i] = flash_driver_ciper[i] - 1;
    }
    flash_driver[30] = 0xFF;
    flash_driver[54] = 0xFF;
    /* 屏蔽所有中断 */
    __disable_irq();

    FLASH_UNLOCK_CONFIG();

    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;

    ll_flash_erase_reg_drv(type, addr, length);                  /* 先擦除 */
    ll_flash_write_reg_drv(addr, buffer, length);                /* 再编程 */

    FLASH_LOCK_CONFIG();

    /* 恢复中断 */
    __enable_irq();
    /* 销毁Flash驱动RAM代码 */
    memset(flash_driver, 0, FLASH_DRV_CIPER_SIZE);

    return 0;
}

/**
 * @brief   Flash寄存器读写操作
 * @param   is_write  - true写操作，false读操作
 * @param   addr      - 寄存器地址
 * @param   reg_value - 指向数据的指针：写时为写入值，读时为读取结果
 * @note    APB总线地址范围（>APB_BASE_ADDRESS）执行读写，否则仅执行读操作
 * @retval  0 - 操作成功
 */
int ll_flash_reg_wr(bool is_write, uint32_t addr, uint32_t *reg_value)
{
    /* APB总线地址 */
    if (addr > APB_BASE_ADDRESS)
    {
        if (is_write)
        {
            REG_WRITE32(addr, *reg_value);                       /* 写寄存器 */
        }
        else
        {
            *reg_value = REG_READ32(addr);                       /* 读寄存器 */
        }
    }

    {
        *reg_value = REG_READ32(addr);                           /* 返回寄存器值 */
    }

    return 0;
}

/**
 * @brief   使能或禁能Flash中断
 * @param   isr_flag - 中断标志位
 * @param   enable   - true使能中断，false禁能
 * @retval  0 - 操作成功
 */
int ll_flash_isr_enable(uint32_t isr_flag, bool enable)
{
    FLASH_UNLOCK_CONFIG();

    if (enable)
    {
        EFLASH->IMR &= ~(isr_flag);                              /* 清除掩码位，使能中断 */
    }
    else
    {
        EFLASH->IMR |= isr_flag;                                 /* 设置掩码位，禁能中断 */
    }

    FLASH_LOCK_CONFIG();

    return 0;
}

/**
 * @brief   Flash中断处理函数
 * @param   None
 * @note    读取中断状态寄存器，清除中断标志
 * @retval  None
 */
void FLASH_IRQHandler(void)
{
    uint32_t isr = EFLASH->ISR;                                  /* 读取中断状态 */

    FLASH_UNLOCK_CONFIG();

    EFLASH->ICR = isr;                                           /* 清除中断标志 */

    FLASH_LOCK_CONFIG();

    /* 回调处理 */
}
