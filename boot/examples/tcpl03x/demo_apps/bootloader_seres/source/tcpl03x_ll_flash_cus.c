/**
 *****************************************************************************
 * @brief   Flash定制驱动源文件（TCPL03x低层Flash驱动）
 *
 * 提供TCPL03x系列芯片的Flash擦除、编程、读取及中断控制功能。
 * 支持两种操作模式：寄存器直接操作模式和RAM驱动调用模式。
 * RAM驱动程序以密文形式存储，运行时解密后执行，操作后销毁。
 *
 * @file    tcpl03x_ll_flash.c
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
 * <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.
 *
 *****************************************************************************
 */

#include "tcpl03x.h"
#include "system_tcpl03x.h"
#include "tcpl03x_ll_flash.h"

/** @brief Flash写入字节对齐值（8字节对齐） */
#define FLASH_WR_BYTE_ALIGN (8)
/** @brief Flash驱动程序密文大小（56字节） */
#define FLASH_DRV_CIPER_SIZE (56)

/**
 * @brief Flash操作数据类型（按对齐宽度选择）
 * @note 当 FLASH_WR_BYTE_ALIGN = 8 时使用 uint64_t，否则使用 uint32_t
 *      该类型决定了单次Flash读写的数据宽度
 */
#if 8 == FLASH_WR_BYTE_ALIGN
typedef uint64_t flash_size_t;
#else
typedef uint32_t flash_size_t;
#endif

/** @brief 外部Flash驱动程序缓冲区声明 */
extern uint8_t flash_driver[];
/** @brief Flash扇区擦除函数指针类型 */
typedef uint8_t (*tpfFLASH_DRV_EraseSector)(void);
/** @brief Flash编程函数指针类型 */
typedef uint8_t (*tpfFLASH_DRV_Program)(uint32_t addr, uint8_t *ptr);

/**
 * @brief Flash驱动程序密文缓冲区
 * @note 存储加密后的Flash驱动代码，使用时通过解密还原为可执行代码
 *      解密算法：flash_driver[i] = flash_driver_ciper[i] - 1
 * @warning 该缓冲区内容不可直接执行，需先解密
 */
uint8_t flash_driver_ciper[FLASH_DRV_CIPER_SIZE] =
    {
        0x02, 0x01, 0x01, 0x01, 0x1A, 0x01, 0x01, 0x01,
        0x05, 0x49, 0x02, 0x69, 0x02, 0x23, 0x0B, 0x44,
        0x03, 0x61, 0xC2, 0x6B, 0x8A, 0x08, 0xFD, 0xD5,
        0x71, 0x48, 0xC1, 0x47, 0x21, 0x01, 0xFF, 0x01,
        0x0B, 0x69, 0x4A, 0x69, 0x42, 0x61, 0x03, 0x61,
        0x03, 0x49, 0x02, 0x69, 0x4A, 0x08, 0xFD, 0xD5,
        0x71, 0x48, 0xC1, 0x47, 0x4D, 0x01, 0xFF, 0x01};

/**
 * @brief Flash扇区擦除函数指针
 * @note 指向位于0x10000009地址的Flash驱动擦除函数
 * @warning 该指针指向RAM中的解密后驱动程序
 */
tpfFLASH_DRV_EraseSector g_pfFLASH_DRV_EraseSector = (tpfFLASH_DRV_EraseSector)(0x10000009u);
/**
 * @brief Flash编程函数指针
 * @note 指向位于0x10000021地址的Flash驱动编程函数
 * @warning 该指针指向RAM中的解密后驱动程序
 */
tpfFLASH_DRV_Program g_tpfFLASH_DRV_Program = (tpfFLASH_DRV_Program)(0x10000021u);
/**
 * @brief  初始化Flash控制器时序配置
 *
 * 配置Flash控制器的各项时序参数，包括读取恢复时间、擦除/编程建立时间、
 * 编程时间、擦除时间和等待周期。所有时序配置在解锁状态下进行，配置完成后重新锁定。
 *
 * @note 时序参数需与芯片运行频率匹配，错误的时序配置可能导致Flash操作失败
 * @note 必须在调用擦除/编程操作前调用此函数
 *
 * @param  None
 * @retval None
 */
void ll_flash_init(void)
{
    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 读取时序配置：设置读取等待周期和采样点 */
    EFLASH->RD_TIME_CFG = 0X00003033;
    /* 恢复时序配置：设置两次连续访问之间的恢复时间 */
    EFLASH->RCV_TIME_CFG = 0x00035CDC;
    /* NVS时序配置：擦除或编程操作的建立时间 */
    EFLASH->NVS_TIME_CFG = 0x00016058;
    /* 编程时序配置：Flash编程脉冲宽度 */
    EFLASH->PROG_TIME_CFG = 0x00161ADC;
    /* 擦除时序配置：Flash擦除脉冲宽度 */
    EFLASH->ERASE_TIME_CFG = 0x00000809;
    /* 等待时序配置：设置Flash读取等待周期数 */
    EFLASH->LATENCY_TIME_CFG = 0x0000301A;

    /* 锁定Flash配置寄存器，防止意外修改 */
    FLASH_LOCK_CONFIG();
}

/**
 * @brief  获取Flash地址对应的扇区索引
 *
 * 根据Flash基地址计算指定地址所在的扇区编号。
 * 扇区大小为512字节（右移9位）。
 *
 * @param  type  Flash存储器类型
 * @param  addr  Flash绝对地址
 *
 * @retval uint16_t 扇区索引值（从0开始）
 */
static uint16_t ll_flash_sector_get(flash_type_e type, uint32_t addr)
{
    return ((addr - NVM_FLASH_BASE_ADDR) >> 9);
}

/**
 * @brief  检查Flash地址范围是否有效
 *
 * 验证指定的地址和长度是否在NVM Flash的有效地址范围内，
 * 防止越界访问。
 *
 * @param  type   Flash存储器类型
 * @param  addr   Flash起始地址
 * @param  length 数据长度（字节）
 *
 * @retval 1  地址范围有效
 * @retval 0  地址越界（addr + length > NVM_FLASH_SIZE）
 *
 * @note 该函数仅检查NVM Flash范围，不检查NVR类型
 */
static uint8_t ll_flash_addr_valid_check(flash_type_e type, uint32_t addr, uint32_t length)
{
    return (!((addr + length) > NVM_FLASH_SIZE));
}

/**
 * @brief  擦除Flash扇区（寄存器直接操作模式）
 *
 * 通过直接操作Flash控制器寄存器实现扇区擦除。
 * 遍历从起始到结束的所有扇区，逐个触发擦除操作并等待完成。
 *
 * @param  type   Flash存储器类型
 * @param  addr   待擦除的起始地址
 * @param  length 待擦除的区域长度（字节）
 *
 * @retval None
 *
 * @note 擦除前必须已调用 FLASH_UNLOCK_CONFIG() 解锁配置寄存器
 * @note 擦除过程中需等待 ERASE_BUSY_STATUS 清零，确保操作完成
 * @warning 擦除操作不可逆，擦除后数据将全部变为0xFF
 */
static void ll_flash_erase_reg(flash_type_e type, uint32_t addr, uint32_t length)
{
    uint16_t sector_start = ll_flash_sector_get(type, addr);
    uint16_t sector_end = ll_flash_sector_get(type, (addr + length - 1)) + 1;

    /* 关闭只读保护，允许擦除操作 */
    EFLASH->OP_CTRL_F.RDONLY_EN = false;

    /* 逐扇区擦除 */
    for (uint32_t index = sector_start; index < sector_end; index++)
    {
        /* 设置当前待擦除扇区索引 */
        EFLASH->ERASE_CFG_F.SECTOR_INDEX = index;
        /* 触发扇区擦除（注释掉的驱动函数方式作为备选） */
        // g_pfFLASH_DRV_EraseSector();
        EFLASH->ERASE_TRIG_F.SECTOR_ERASE_TRIG = 1;

        /* 等待擦除操作完成（忙等待） */
        while (EFLASH->STATUS_F.ERASE_BUSY_STATUS == 1)
        {
            ;
        }
    }

    /* 恢复只读保护 */
    EFLASH->OP_CTRL_F.RDONLY_EN = true;
}

/**
 * @brief  擦除Flash扇区（驱动程序调用模式）
 *
 * 通过调用RAM中解密的Flash驱动程序实现扇区擦除。
 * 擦除过程中关闭全局中断，防止驱动程序执行被中断打断。
 *
 * @param  type   Flash存储器类型
 * @param  addr   待擦除的起始地址
 * @param  length 待擦除的区域长度（字节）
 *
 * @retval None
 *
 * @note 使用 g_pfFLASH_DRV_EraseSector 函数指针调用RAM驱动
 * @note 擦除前后需 __disable_irq() / __enable_irq() 保护
 * @warning 中断关闭期间不得执行耗时操作，否则影响系统实时性
 */
static void ll_flash_erase_reg_drv(flash_type_e type, uint32_t addr, uint32_t length)
{
    uint16_t sector_start = ll_flash_sector_get(type, addr);
    uint16_t sector_end = ll_flash_sector_get(type, (addr + length - 1)) + 1;

    /* 关闭只读保护，允许擦除操作 */
    EFLASH->OP_CTRL_F.RDONLY_EN = false;

    /* 逐扇区擦除 */
    for (uint32_t index = sector_start; index < sector_end; index++)
    {
        /* 设置当前待擦除扇区索引 */
        EFLASH->ERASE_CFG_F.SECTOR_INDEX = index;
        /* 关中断，调用RAM驱动擦除函数 */
        __disable_irq();
        g_pfFLASH_DRV_EraseSector();
        /* 恢复中断 */
        __enable_irq();
    }

    /* 恢复只读保护 */
    EFLASH->OP_CTRL_F.RDONLY_EN = true;
}

/**
 * @brief  写入数据到Flash（寄存器直接操作模式）
 *
 * 通过直接操作Flash控制器寄存器实现数据编程。
 * 按FLASH_WR_BYTE_ALIGN对齐宽度分块写入，先写入对齐的主数据块，
 * 再处理末尾未对齐的剩余数据。每次写入后等待编程忙状态清除。
 *
 * @param  addr   目标Flash地址
 * @param  buffer 待写入数据的源缓冲区指针
 * @param  length 待写入数据长度（字节）
 *
 * @retval None
 *
 * @note 写入前目标扇区必须先执行擦除操作
 * @note 写入地址需在有效Flash地址范围内
 * @note 地址和数据长度无对齐要求，函数内部自动处理
 * @warning 编程过程中等待 PROG_BUSY_STATUS 清零，不可中断
 */
static void ll_flash_write_reg(uint32_t addr, uint8_t *buffer, uint32_t length)
{
    uint32_t remain_length, reseverd_length;
    flash_size_t temp_data;
    uint32_t offset = 0;
    uint8_t *ptr = (uint8_t *)&temp_data;

#if 8 == FLASH_WR_BYTE_ALIGN
    /* 对齐宽度为8字节，模板值初始化为全1 */
    temp_data = 0xFFFFFFFFFFFFFFFF;
#else
    /* 对齐宽度为4字节，模板值初始化为全1 */
    temp_data = 0xFFFFFFFF;
#endif

    /* 关闭只读保护，允许编程操作 */
    EFLASH->OP_CTRL_F.RDONLY_EN = false;

    remain_length = length;
    /* 计算末尾不对齐的剩余字节数 */
    reseverd_length = remain_length % FLASH_WR_BYTE_ALIGN;

    /* 处理对齐的主数据部分 */
    if (remain_length > reseverd_length)
    {
        for (uint32_t i = 0; i < remain_length - reseverd_length; i += FLASH_WR_BYTE_ALIGN)
        {
            /* 将源数据拷贝到临时对齐缓冲区 */
            memcpy((uint8_t *)ptr, (uint8_t *)(buffer + offset), sizeof(flash_size_t));
            /* 直接写Flash寄存器（注释掉的驱动函数方式作为备选） */
            // g_tpfFLASH_DRV_Program(addr + offset, ptr);
            *((volatile flash_size_t *)(addr + offset)) = *((volatile flash_size_t *)ptr);
            /* 等待编程操作完成 */
            while (EFLASH->STATUS_F.PROG_BUSY_STATUS == 1)
            {
                ;
            }
            offset += FLASH_WR_BYTE_ALIGN;
        }
    }

    /* 处理末尾未对齐的剩余数据 */
    if (reseverd_length)
    {
#if 8 == FLASH_WR_BYTE_ALIGN
        /* 重新初始化为全1（8字节） */
        temp_data = 0xFFFFFFFFFFFFFFFF;
#else
        /* 重新初始化为全1（4字节） */
        temp_data = 0xFFFFFFFF;
#endif
        /* 拷贝剩余部分到对齐缓冲区的高字节 */
        memcpy((uint8_t *)ptr, (uint8_t *)&buffer[length - reseverd_length], reseverd_length);
        /* 写入最后一块（含未修改的高字节保持0xFF） */
        *((volatile flash_size_t *)(addr + offset)) = *((volatile flash_size_t *)ptr);
        /* 等待最后一次编程完成 */
        while (EFLASH->STATUS_F.PROG_BUSY_STATUS == 1)
        {
            ;
        }
    }

    /* 恢复只读保护 */
    EFLASH->OP_CTRL_F.RDONLY_EN = true;
}

/**
 * @brief  写入数据到Flash（驱动程序调用模式）
 *
 * 通过调用RAM中解密的Flash驱动程序实现数据编程。
 * 按FLASH_WR_BYTE_ALIGN对齐宽度分块写入，每次编程前后关/开中断保护。
 *
 * @param  addr   目标Flash地址
 * @param  buffer 待写入数据的源缓冲区指针
 * @param  length 待写入数据长度（字节）
 *
 * @retval None
 *
 * @note 使用 g_tpfFLASH_DRV_Program 函数指针调用RAM驱动
 * @note 每次编程调用前后需关/开中断，防止驱动执行被中断
 * @warning 写入前目标扇区必须先执行擦除操作
 */
static void ll_flash_write_reg_drv(uint32_t addr, uint8_t *buffer, uint32_t length)
{
    uint32_t remain_length, reseverd_length;
    flash_size_t temp_data;
    uint32_t offset = 0;
    uint8_t *ptr = (uint8_t *)&temp_data;

#if 8 == FLASH_WR_BYTE_ALIGN
    /* 对齐宽度为8字节，模板值初始化为全1 */
    temp_data = 0xFFFFFFFFFFFFFFFF;
#else
    /* 对齐宽度为4字节，模板值初始化为全1 */
    temp_data = 0xFFFFFFFF;
#endif

    /* 关闭只读保护，允许编程操作 */
    EFLASH->OP_CTRL_F.RDONLY_EN = false;

    remain_length = length;
    /* 计算末尾不对齐的剩余字节数 */
    reseverd_length = remain_length % FLASH_WR_BYTE_ALIGN;

    /* 处理对齐的主数据部分 */
    if (remain_length > reseverd_length)
    {
        for (uint32_t i = 0; i < remain_length - reseverd_length; i += FLASH_WR_BYTE_ALIGN)
        {
            /* 将源数据拷贝到临时对齐缓冲区 */
            memcpy((uint8_t *)ptr, (uint8_t *)(buffer + offset), sizeof(flash_size_t));
            /* 关中断，调用RAM驱动编程函数 */
            __disable_irq();
            g_tpfFLASH_DRV_Program(addr + offset, ptr);
            /* 恢复中断 */
            __enable_irq();
            offset += FLASH_WR_BYTE_ALIGN;
        }
    }

    /* 处理末尾未对齐的剩余数据 */
    if (reseverd_length)
    {
#if 8 == FLASH_WR_BYTE_ALIGN
        /* 重新初始化为全1（8字节） */
        temp_data = 0xFFFFFFFFFFFFFFFF;
#else
        /* 重新初始化为全1（4字节） */
        temp_data = 0xFFFFFFFF;
#endif
        /* 拷贝剩余部分到对齐缓冲区 */
        memcpy((uint8_t *)ptr, (uint8_t *)&buffer[length - reseverd_length], reseverd_length);
        /* 关中断，调用RAM驱动编程函数写入最后一块 */
        __disable_irq();
        g_tpfFLASH_DRV_Program(addr + offset, ptr);
        /* 恢复中断 */
        __enable_irq();
    }

    /* 恢复只读保护 */
    EFLASH->OP_CTRL_F.RDONLY_EN = true;
}

/**
 * @brief  从Flash读取数据
 *
 * 从指定Flash地址读取数据到用户缓冲区。
 * 自动处理地址未对齐的情况，按FLASH_WR_BYTE_ALIGN对齐宽度读取。
 * 先处理首地址未对齐部分，再读取对齐的主数据块，最后处理末尾剩余部分。
 *
 * @param  addr   源Flash地址
 * @param  buffer 目标数据缓冲区指针
 * @param  length 待读取数据长度（字节）
 *
 * @retval None
 *
 * @note Flash读取不需要解锁配置寄存器，可直接读取
 * @note 地址无对齐要求，函数内部自动处理边界对齐
 */
static void ll_flash_read_reg(uint32_t addr, uint8_t *buffer, uint32_t length)
{
    uint32_t remain_length, reseverd_length;
    flash_size_t temp_data;

    uint32_t offset = addr % FLASH_BYTE_ALIGN;
    uint8_t *ptr = (uint8_t *)&temp_data;
    uint32_t read_len __attribute__((unused));

    /* 处理首地址未对齐：从对齐地址读取，再偏移拷贝 */
    if (offset)
    {
        /* 从对齐的Flash地址读取整块数据 */
        *((volatile flash_size_t *)ptr) = *((volatile flash_size_t *)(addr - offset));
        /* 计算本次实际可读长度 */
        read_len = (length > (FLASH_WR_BYTE_ALIGN - offset)) ? FLASH_WR_BYTE_ALIGN - offset : length;
        /* 从偏移位置拷贝所需字节到用户缓冲区 */
        memcpy((uint8_t *)(buffer), (uint8_t *)(ptr + offset), read_len);
        offset = read_len;
    }

    /* 剩余数据长度 */
    remain_length = length - offset;
    /* 计算末尾不对齐的剩余字节数 */
    reseverd_length = remain_length % FLASH_WR_BYTE_ALIGN;

    /* 按对齐宽度循环读取主数据块 */
    if (remain_length > reseverd_length)
    {
        for (uint32_t i = 0; i < remain_length - reseverd_length; i += FLASH_WR_BYTE_ALIGN)
        {
            /* 从Flash读取对齐数据到临时缓冲区 */
            *((volatile flash_size_t *)ptr) = *((volatile flash_size_t *)(addr + offset));
            /* 拷贝到用户缓冲区 */
            memcpy((uint8_t *)(buffer + offset), (uint8_t *)ptr, sizeof(flash_size_t));
            offset += FLASH_WR_BYTE_ALIGN;
        }
    }

    /* 读取末尾未对齐的剩余数据 */
    if (reseverd_length)
    {
        /* 从Flash读取最后一块对齐数据 */
        *((volatile flash_size_t *)ptr) = *((volatile flash_size_t *)(addr + offset));
        /* 拷贝所需字节数 */
        memcpy((uint8_t *)(buffer + offset), (uint8_t *)ptr, reseverd_length);
    }
}

/**
 * @brief  全片擦除Flash（芯片擦除）
 *
 * 对整个Flash芯片执行擦除操作。擦除期间关闭全局中断（保留NMI和HardFault），
 * 解锁配置寄存器，触发芯片擦除并等待操作完成。
 *
 * @param  None
 *
 * @retval 0  擦除成功
 *
 * @note 全片擦除操作不可逆，所有用户数据将丢失
 * @note 擦除过程中必须保持中断关闭，防止操作被打断
 * @warning 芯片擦除耗时较长，需确保系统电源稳定
 */
int ll_flash_erase_chip(void)
{
    /* 关闭全局中断（保留NMI和HardFault），防止擦除过程被中断干扰 */
    __disable_irq();

    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 选择NVM扇区（非NVR） */
    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;
    /* 触发全片擦除 */
    EFLASH->ERASE_TRIG_F.CHIP_ERASE_TRIG = 1;

    /* 等待芯片擦除完成 */
    while (EFLASH->STATUS_F.ERASE_BUSY_STATUS == 1)
    {
        ;
    }

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();

    /* 恢复全局中断 */
    __enable_irq();

    return 0;
}

/**
 * @brief  擦除Flash指定区域（寄存器直接操作模式）
 *
 * 擦除指定地址和大小的Flash区域。通过寄存器直接操作实现扇区擦除。
 * 参数校验通过后，关闭中断、解锁配置寄存器，调用底层擦除函数。
 *
 * @param  type   Flash存储器类型（仅支持FLASH_TYPE_NVM）
 * @param  addr   待擦除的起始地址
 * @param  length 待擦除的区域长度（字节）
 *
 * @retval 0  擦除成功
 * @retval -1 参数错误（类型不支持或地址越界）
 *
 * @note 仅支持NVM类型Flash擦除
 * @note 擦除过程中关闭全局中断，函数返回后恢复
 */
int ll_flash_erase(flash_type_e type, uint32_t addr, uint32_t length)
{
    int res = 0;

    /* 参数校验：仅支持NVM类型，且地址范围有效 */
    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }

    /* 关闭全局中断（保留NMI和HardFault） */
    __disable_irq();

    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 选择NVM扇区 */
    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;

    /* 执行寄存器模式扇区擦除 */
    ll_flash_erase_reg(type, addr, length);

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();

    /* 恢复全局中断 */
    __enable_irq();

    return res;
}

/**
 * @brief  擦除Flash指定区域（驱动程序调用模式）
 *
 * 通过RAM中解密的Flash驱动程序擦除指定区域。
 * 解锁配置后，调用驱动方式的扇区擦除函数。
 *
 * @param  type   Flash存储器类型（仅支持FLASH_TYPE_NVM）
 * @param  addr   待擦除的起始地址
 * @param  length 待擦除的区域长度（字节）
 *
 * @retval 0  擦除成功
 * @retval -1 参数错误（类型不支持或地址越界）
 *
 * @note 区别于 ll_flash_erase，此函数不关闭全局中断
 *       （中断保护在驱动调用层 ll_flash_erase_reg_drv 内部处理）
 * @warning 调用前需确保Flash驱动程序已在RAM中解密就绪
 */
int ll_flash_erase_drv(flash_type_e type, uint32_t addr, uint32_t length)
{
    int res = 0;

    /* 参数校验：仅支持NVM类型，且地址范围有效 */
    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }

    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 选择NVM扇区 */
    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;

    /* 执行驱动模式扇区擦除 */
    ll_flash_erase_reg_drv(type, addr, length);

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();

    return res;
}

/**
 * @brief  从Flash读取数据到缓冲区
 *
 * 从指定Flash地址读取指定长度的数据到用户提供的缓冲区。
 * 调用底层读取函数，不涉及中断控制或配置寄存器解锁。
 *
 * @param  type   Flash存储器类型
 * @param  addr   源Flash地址
 * @param  buffer 目标数据缓冲区指针
 * @param  length 待读取数据长度（字节）
 *
 * @retval 0  读取成功
 * @retval -1 地址越界
 *
 * @note Flash读取操作无需解锁配置或关闭中断
 */
int ll_flash_read(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if (false == ll_flash_addr_valid_check(type, addr, length))
    {
        return -1;
    }

    ll_flash_read_reg(addr, buffer, length);

    return 0;
}

/**
 * @brief  写入数据到Flash（寄存器直接操作模式）
 *
 * 将数据写入指定Flash地址。校验参数后，关闭中断、解锁配置寄存器，
 * 调用寄存器直接操作方式的写入函数。
 *
 * @param  type   Flash存储器类型（仅支持FLASH_TYPE_NVM）
 * @param  addr   目标Flash地址
 * @param  buffer 待写入数据的源缓冲区指针
 * @param  length 待写入数据长度（字节）
 *
 * @retval 0  写入成功
 * @retval -1 参数错误（类型不支持或地址越界）
 *
 * @note 写入前目标扇区必须先执行擦除操作
 * @note 写入过程关中断保护，防止编程时序被中断破坏
 * @note 数据长度无对齐要求，内部自动处理
 */
int ll_flash_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }

    /* 关闭全局中断（保留NMI和HardFault） */
    __disable_irq();

    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 选择NVM扇区 */
    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;

    /* 执行寄存器模式写入 */
    ll_flash_write_reg(addr, buffer, length);

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();

    /* 恢复全局中断 */
    __enable_irq();

    return 0;
}

/**
 * @brief  写入数据到Flash（驱动程序调用模式）
 *
 * 通过RAM中解密的Flash驱动程序将数据写入指定Flash地址。
 * 校验参数后，关中断、解锁配置，调用驱动方式的写入函数。
 *
 * @param  type   Flash存储器类型（仅支持FLASH_TYPE_NVM）
 * @param  addr   目标Flash地址
 * @param  buffer 待写入数据的源缓冲区指针
 * @param  length 待写入数据长度（字节）
 *
 * @retval 0  写入成功
 * @retval -1 参数错误（类型不支持或地址越界）
 *
 * @note 区别于 ll_flash_write，此函数通过RAM驱动编程函数写入
 * @note 中断保护在驱动调用层 ll_flash_write_reg_drv 内部逐次处理
 * @warning 调用前需确保Flash驱动程序已在RAM中解密就绪
 */
int ll_flash_write_drv(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }

    /* 关闭全局中断（保留NMI和HardFault） */
    __disable_irq();

    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 选择NVM扇区 */
    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;

    /* 执行驱动模式写入 */
    ll_flash_write_reg_drv(addr, buffer, length);

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();

    /* 恢复全局中断 */
    __enable_irq();

    return 0;
}

/**
 * @brief  智能写入Flash（含驱动解密、擦除、编程、销毁）
 *
 * 集成了Flash驱动解密、扇区擦除、数据编程和驱动销毁的完整写入流程。
 * 运行时从密文缓冲区解密Flash驱动程序到RAM，执行擦除和编程操作，
 * 完成后立即销毁RAM中的驱动程序以防止泄露。
 *
 * @param  type   Flash存储器类型（仅支持FLASH_TYPE_NVM）
 * @param  addr   目标Flash地址
 * @param  buffer 待写入数据的源缓冲区指针
 * @param  length 待写入数据长度（字节）
 *
 * @retval 0  操作成功
 * @retval -1 参数错误（类型不支持或地址越界）
 *
 * @note 解密算法：flash_driver[i] = flash_driver_ciper[i] - 1
 * @note flash_driver[30]和flash_driver[54]强制设为0xFF（特殊修正字节）
 * @note 操作完成后立即用memset清零驱动缓冲区，防止固件逆向
 * @warning 该函数包含完整的擦除+编程序列，目标扇区数据将被覆盖
 */
int ll_flash_smart_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if ((FLASH_TYPE_NVM != type) || (false == ll_flash_addr_valid_check(type, addr, length)))
    {
        return -1;
    }
    /* 将密文Flash驱动程序解密到RAM缓冲区 */
    for (int i = 0; i < FLASH_DRV_CIPER_SIZE; i++)
    {
        flash_driver[i] = flash_driver_ciper[i] - 1;
    }
    /* 特殊修正字节：强制写入0xFF */
    flash_driver[30] = 0xFF;
    flash_driver[54] = 0xFF;

    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 选择NVM扇区 */
    EFLASH->ERASE_CFG_F.NVR_SECTOR_SEL = 0;

    /* 执行驱动模式擦除和写入 */
    ll_flash_erase_reg_drv(type, addr, length);
    ll_flash_write_reg_drv(addr, buffer, length);

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();

    /* 销毁RAM中的Flash驱动程序，防止泄露 */
    memset(flash_driver, 0, FLASH_DRV_CIPER_SIZE);
    return 0;
}

/**
 * @brief  Flash寄存器读写操作
 *
 * 对Flash控制器的寄存器执行读写操作。
 * 若地址大于APB总线基地址则判定为APB从设备寄存器，
 * 根据is_write标志执行读或写操作。无论是否写入，均执行一次回读。
 *
 * @param  is_write  true=写入寄存器，false=读取寄存器
 * @param  addr      寄存器地址
 * @param  reg_value 写入时指向待写入值；读取时用于返回寄存器值
 *
 * @retval 0  操作成功
 *
 * @note 回读操作用于验证写入结果或获取最新寄存器值
 */
int ll_flash_reg_wr(bool is_write, uint32_t addr, uint32_t *reg_value)
{
    /* 判断是否为APB从设备地址范围 */
    if (addr > APB_BASE_ADDRESS)
    {
        if (is_write)
        {
            /* 写入寄存器 */
            REG_WRITE32(addr, *reg_value);
        }
        else
        {
            /* 读取寄存器值 */
            *reg_value = REG_READ32(addr);
        }
    }

    /* 执行寄存器回读 */
    {
        *reg_value = REG_READ32(addr);
    }

    return 0;
}

/**
 * @brief  使能或禁用Flash中断
 *
 * 控制Flash控制器的中断屏蔽寄存器（IMR）。
 * enable为true时清除对应位（使能中断），为false时设置对应位（屏蔽中断）。
 *
 * @param  isr_flag 中断标志位掩码，表示要配置的中断源
 * @param  enable    true=使能中断，false=禁用（屏蔽）中断
 *
 * @retval 0  配置成功
 *
 * @note 修改IMR寄存器前需解锁配置寄存器，修改后重新锁定
 * @note IMR寄存器中对应位为0时使能中断，为1时屏蔽中断
 */
int ll_flash_isr_enable(uint32_t isr_flag, bool enable)
{
    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    if (enable)
    {
        /* 清除中断屏蔽位，使能指定中断源 */
        EFLASH->IMR &= ~(isr_flag);
    }
    else
    {
        /* 设置中断屏蔽位，禁用指定中断源 */
        EFLASH->IMR |= isr_flag;
    }

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();

    return 0;
}

/**
 * @brief  Flash中断服务函数
 *
 * Flash控制器中断处理入口。读取中断状态寄存器（ISR）获取中断源，
 * 将状态值写入中断清除寄存器（ICR）清除中断标志。
 *
 * @param  None
 *
 * @retval None
 *
 * @note 读取ISR后需将值写入ICR以清除中断标志
 * @note 如需处理具体中断事件，可在函数中扩展中断源判断逻辑
 * @warning 操作ICR寄存器前需要解锁配置寄存器
 */
void FLASH_IRQHandler(void)
{
    /* 读取中断状态寄存器 */
    uint32_t isr = EFLASH->ISR;

    /* 解锁Flash配置寄存器 */
    FLASH_UNLOCK_CONFIG();

    /* 将中断状态写入中断清除寄存器，清除中断标志 */
    EFLASH->ICR = isr;

    /* 锁定Flash配置寄存器 */
    FLASH_LOCK_CONFIG();
}
