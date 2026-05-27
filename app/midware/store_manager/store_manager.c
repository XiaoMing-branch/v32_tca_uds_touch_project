/**
 *****************************************************************************
 * @brief   store manager source file.
 *
 * @file    store_manager.c
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

#include "store_manager.h"
#include "pal_store.h"

/**
 * @brief  全局系统配置实例，运行时保存在 RAM 中
 * @note   默认值：NAD=0x6F，触摸阈值使用典型值，原始 NAD=0x01
 */
sys_cfg_t g_sys_cfgs =
{
    .nad = 0x6F,
#if CFG_SUPPROT_LINSNPD_EXT_RES
    .cur_th_st12 = 0x68003C00,
    .cur_th_st34 = 0x50009D00,
#else
    .cur_th_st12 = 0x68003C00,
    .cur_th_st34 = 0x50009D00,
#endif
    .org_nad = 0x01,
};

//const uint32_t led_param_addr_map[] =
//{
//    LED_TEMP_PN_VOLT_OFFSET,
//    LED_RGB_OFFSET,
//    LED_WHITE_COLOR_OFFSET,
//    LED_RELATIVE_FACTOR_OFFSET,
//    LED_SERIES_NUM_OFFSET,
//};

/** @brief  系统参数 Flash 地址映射表，索引与 system_param_type_e 对应 */
const uint32_t sys_param_addr_map[] =
{
    SYSTEM_CFG_OFFSET,
    SYSTEM_ID_CFG_OFFSET,
};

/**
 * @brief  从快速存储区读取 LIN NAD 地址
 * @note   遍历 FAST_LIN_NAD_ADDR 区域，搜索第一个非 0xFF 的值作为 NAD，
 *         用于产线校准后快速获取 NAD 而不需要从完整系统配置中读取
 * @retval NAD 值（若未找到则返回 0xFF）
 */
static uint8_t fast_nad_read(void)
{
    uint8_t nad = 0xFF; /**< 返回的 NAD 值，默认 0xFF 表示未找到有效值 */
    uint8_t rdbuf[8]; /**< 8 字节读取缓冲区，用于逐块遍历 NAD 存储区 */

    /* 遍历快速 NAD 存储区，以 8 字节为步长搜索第一个非 0xFF 的有效 NAD */
    for (int i = 0; i < FLASH_SECTOR_SIZE; i += sizeof(rdbuf))
    {
        pal_store_read(STORE_TYPE_SEL, FAST_LIN_NAD_ADDR + i, rdbuf, sizeof(rdbuf));
        for (int j = 0; i < sizeof(rdbuf); ++j)
        {
            if (rdbuf[j] == 0xFF)
            {
                if (i + j != 0)
                {
                    pal_store_read(STORE_TYPE_SEL, FAST_LIN_NAD_ADDR + i + j - 1, &nad, sizeof(nad));
                }
                return nad;
            }
        }
    }
    return nad;
}

/**
 * @brief  系统数据初始化，从 Flash 加载配置参数到内存
 * @note   优先从快速 NAD 区读取 NAD，若有效则写入系统配置区；
 *         然后分别初始化系统配置参数和 LIN 配置参数的 Flash 存储区域
 * @retval 无
 */
__WEAK void store_system_data_init(void)
{
    uint32_t addr = 0; /**< 临时地址变量，用于计算 Flash 偏移和初始化 */

    /* 从快速 NAD 存储区读取产线校准写入的 NAD 值 */
    uint8_t nad = fast_nad_read();
    if (nad != 0xFF)
    {
        /* NAD 有效：更新系统配置，擦除快速 NAD 区，持久化到系统配置 Flash 中 */
        g_sys_cfgs.nad = nad;
        pal_store_erase(STORE_TYPE_SEL, FAST_LIN_NAD_ADDR, FLASH_SECTOR_SIZE);
        store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
    }

    /* 初始化系统配置参数：从 Flash 加载到 g_sys_cfgs，若 Flash 为空则写入默认值 */
    addr = SYSTEM_PARAM_BASE_ADDR + SYSTEM_CFG_OFFSET;
    pal_store_data_init(addr, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);

    extern uint8_t lin_configuration_RAM[];
    addr = SYSTEM_PARAM_BASE_ADDR + SYSTEM_ID_CFG_OFFSET;
    pal_store_data_init(addr, lin_configuration_RAM, SYSTEM_ID_CFG_SIZE);
}

/**
 * @brief  存储管理器初始化入口
 * @note   调用 store_system_data_init 从 Flash 加载系统配置到 g_sys_cfgs
 * @retval 无
 */
void store_manager_init(void)
{
    store_system_data_init();
}

/**
 * @brief  清除所有存储的系统参数，擦除系统参数区 Flash
 * @retval 无
 */
void store_manager_clear(void)
{
    pal_store_data_clear(SYSTEM_PARAM_BASE_ADDR, STORE_SECTOR_SIZE);
}

/**
 * @brief  将系统参数写入 Flash 存储
 * @param[in]  type  - 参数类型（SYSTEM_CFG_PARAM / SYSTEM_ID_CFG_PARAM）
 * @param[in]  param - 参数数据指针
 * @param[in]  len   - 数据长度（字节）
 * @note   根据 type 从 sys_param_addr_map 查找对应的 Flash 偏移地址，
 *         调用 pal_store_data_set 写入
 * @retval 无
 */
void store_system_data_set(system_param_type_e type, uint8_t *param, uint16_t len)
{
    uint32_t addr = SYSTEM_PARAM_BASE_ADDR + sys_param_addr_map[type]; /**< 参数在 Flash 中的目标写入地址 */
    pal_store_data_set(addr, param, len);
}

/**
 * @brief  从 Flash 读取系统参数
 * @param[in]  type   - 参数类型（SYSTEM_CFG_PARAM / SYSTEM_ID_CFG_PARAM）
 * @param[out] param  - 存放读取数据的缓冲区指针
 * @param[in]  len    - 期望读取的数据长度
 * @note   根据 type 从 sys_param_addr_map 查找对应的 Flash 偏移地址，
 *         调用 pal_store_data_get 读取
 * @retval true   - 读取成功
 * @retval false  - 读取失败
 */
bool store_system_data_get(system_param_type_e type, uint8_t *param, uint16_t len)
{
    uint32_t addr = SYSTEM_PARAM_BASE_ADDR + sys_param_addr_map[type]; /**< 参数在 Flash 中的源读取地址 */
    return (pal_store_data_get(addr, param, len));
}


/**
 * @brief  读取芯片硬件信息
 * @param[in]  type   - 芯片信息类型
 *         - CHIP_INFO_VER_ID: 版本号和 ID
 *         - CHIP_INFO_UUID:   唯一标识 UUID
 *         - CHIP_INFO_BOOT_VER: Bootloader 版本
 * @param[out] param  - 存放读取数据的缓冲区指针
 * @param[in]  len    - 缓冲区长度（当前未使用）
 * @retval true   - 读取成功
 */
bool store_chip_info_get(chip_info_type_e type, uint8_t *param, uint16_t len)
{
    chip_ver_id_t *chip_ver_id = (chip_ver_id_t *)param; /**< 芯片版本/ID 结构体指针，复用输出缓冲区 */

    /* 根据芯片信息类型分发到对应的 PAL 层读取接口 */
    switch (type)
    {
    case CHIP_INFO_VER_ID:
        pal_store_chip_ver_id_get(&chip_ver_id->ver, &chip_ver_id->id);
        break;

    case CHIP_INFO_UUID:
        pal_store_uid_get((uint32_t *)param);
        break;

    case CHIP_INFO_BOOT_VER:
        pal_store_boot_ver_get((uint32_t *)param);
        break;
    }

    return true;
}

/**
 * @brief  写入客户自定义参数到 Flash 客户区（CUSTOMER_PARAM_BASE_ADDR + offset）
 * @param[in]  addr_offset - 客户参数区内的偏移地址
 * @param[in]  param       - 参数数据指针
 * @param[in]  len         - 数据长度（字节）
 * @note   检查 addr_offset + len 是否超出 FLASH_SOCK_SIZE 范围
 * @retval true        - 写入成功
 * @retval false       - 地址越界
 */
bool store_customer_data_set(uint32_t addr_offset, uint8_t *param, uint16_t len)
{
    /* 校验地址偏移和长度是否超出客户参数区的扇区边界 */
    if (addr_offset >= FLASH_SECTOR_SIZE || (addr_offset + len) > FLASH_SECTOR_SIZE)
    {
        return false;
    }

    uint32_t addr = CUSTOMER_PARAM_BASE_ADDR + addr_offset; /**< 客户参数区的实际 Flash 写入地址 */

    pal_store_data_set(addr, param, len);
    return true;
}

/**
 * @brief  从 Flash 客户区读取客户自定义参数
 * @param[in]  addr_offset - 客户参数区内的偏移地址
 * @param[out] param       - 存放读取数据的缓冲区指针
 * @param[in]  len         - 读取长度
 * @note   检查 addr_offset + len 是否超出 FLASH_SECTOR_SIZE 范围
 * @retval true        - 读取成功
 * @retval false       - 地址越界
 */
bool store_customer_data_get(uint32_t addr_offset, uint8_t *param, uint16_t len)
{
    /* 校验地址偏移和长度是否超出客户参数区的扇区边界 */
    if (addr_offset >= FLASH_SECTOR_SIZE || (addr_offset + len) > FLASH_SECTOR_SIZE)
    {
        return false;
    }

    uint32_t addr = CUSTOMER_PARAM_BASE_ADDR + addr_offset; /**< 客户参数区的实际 Flash 读取地址 */
    return (pal_store_data_get(addr, param, len));
}

/**
 * @brief  慢速读取 Flash（地址需 4 字节对齐）
 * @param[in]  addr   - 读取地址（需 4 字节对齐）
 * @param[out] value  - 存放读取数据的缓冲区指针
 * @param[in]  length - 读取长度（字节）
 * @retval true   - 读取成功
 * @retval false  - 读取失败
 */
bool store_slow_read(uint32_t addr, uint8_t *value, uint16_t length)
{
    return pal_store_read(STORE_TYPE_SEL, addr, value, length);
}

/**
 * @brief  智能慢速读取 Flash（无地址对齐限制）
 * @param[in]  addr   - 读取地址（任意对齐）
 * @param[out] value  - 存放读取数据的缓冲区指针
 * @param[in]  length - 读取长度（字节）
 * @note   自动处理非对齐地址，分三段读取：
 *         - 开头：读取对齐边界前的不完整部分
 *         - 中间：以 8 字节对齐批量读取
 *         - 结尾：末尾不足 8 字节的部分
 * @retval true   - 读取成功
 * @retval false  - 读取失败
 */
bool store_slow_smart_read(uint32_t addr, uint8_t *value, uint16_t length)
{
    uint32_t rdbuf[2]; /**< 8 字节对齐读取缓冲区（2×uint32_t），用于分段对齐读取 */
    uint8_t *ptr = (uint8_t *)rdbuf; /**< rdbuf 的字节指针，便于按字节偏移访问 */
    uint32_t offset = addr % sizeof(rdbuf); /**< 地址相对于 8 字节对齐边界的偏移量 */
    uint32_t align_addr; /**< 对齐后的起始读取地址（向下对齐到 8 字节） */
    uint16_t left_length; /**< 对齐处理后剩余的待读取字节数 */
    uint16_t value_begin; /**< 输出缓冲区 value 中已填充数据的起始偏移 */

    /* 地址未对齐时：先读取包含目标起始位置的完整对齐块，再截取需要的部分 */
    if (offset) //读开头部分
    {
        /* 从对齐地址读取 8 字节到临时缓冲区 */
        pal_store_read(STORE_TYPE_SEL, addr - offset, ptr, sizeof(rdbuf));

        uint16_t dlen = sizeof(rdbuf) - offset;
        if (dlen >= length)
        {
            dlen = length;
            memcpy(value, &ptr[offset], dlen);
            return true;
        }
        else
        {
            memcpy(value, &ptr[offset], dlen);
            align_addr = addr - offset + sizeof(rdbuf);
            left_length = length - dlen;
            value_begin = dlen;
        }
    }
    else
    {
        /* 地址已对齐，直接使用原始地址进行批量读取 */
        align_addr = addr;
        left_length = length;
        value_begin = 0;
    }

    /* 以 8 字节为单位循环读取剩余数据，保证每次读取都在对齐地址上 */
    for (uint16_t i = 0; i < left_length; i += sizeof(rdbuf))
    {
        pal_store_read(STORE_TYPE_SEL, align_addr + i, ptr, sizeof(rdbuf));
        if (i + sizeof(rdbuf) >= left_length)       //读结尾部分
        {
            memcpy(&value[value_begin + i], ptr, left_length - i);
        }
        else        //读中间部分
        {
            memcpy(&value[value_begin + i], ptr, sizeof(rdbuf));
        }
    }

    return true;
}

/**
 * @brief  慢速写入 Flash（扇区擦除-交换-合并-回写流程）
 * @param[in]  addr   - 写入地址
 * @param[in]  value  - 待写入数据指针
 * @param[in]  length - 写入长度（字节）
 * @note   写入流程：
 *         1. 将目标扇区数据备份到交换区（FLASH_SWAP_BASE_ADDR）
 *         2. 擦除目标扇区
 *         3. 从交换区读取回原始数据，将新数据合并到对应位置
 *         4. 将合并后的数据写回目标扇区
 *         此方式保证在写入过程中即使掉电也不会丢失原始数据
 * @retval true   - 写入成功
 * @retval false  - 写入失败（擦除/读取/写入任一环节失败）
 */
bool store_slow_write(uint32_t addr, uint8_t *value, uint16_t length)
{
    uint32_t sector_addr = ((addr / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE); /**< 目标扇区基地址（向下对齐到扇区边界） */
    uint32_t swap_buf[4]; /**< 16 字节交换缓冲区（4×uint32_t），用于扇区数据的备份传输 */
    uint8_t *ptr = (uint8_t *)swap_buf; /**< swap_buf 的字节指针，便于按字节偏移合并 */
    uint32_t begin_addr; /**< 当前处理块在目标扇区中的起始地址 */
    uint32_t end_addr; /**< 当前处理块在目标扇区中的结束地址 */
    uint32_t prog_end_addr = addr + length; /**< 待写入数据在扇区中的结束地址（addr + length） */

    /* 步骤1：擦除交换区（FLASH_SWAP_BASE_ADDR），为扇区备份准备空间 */
    if (!pal_store_erase(STORE_TYPE_SEL, FLASH_SWAP_BASE_ADDR, FLASH_SECTOR_SIZE))
    {
        /* 交换区擦除失败，直接返回，不破坏目标扇区原始数据 */
        return false;
    }
    /* 步骤2：将目标扇区逐块复制到交换区（遇到读取或写入失败立即中止返回） */
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += sizeof(swap_buf))
    {
        if (!pal_store_read(STORE_TYPE_SEL, sector_addr + i, (uint8_t *)swap_buf, sizeof(swap_buf)))
        {
            return false;
        }
        if (!pal_store_write(STORE_TYPE_SEL, FLASH_SWAP_BASE_ADDR + i, (uint8_t *)swap_buf, sizeof(swap_buf)))
        {
            return false;
        }
    }

    /* 步骤3：备份完成，安全擦除目标扇区（即使此时掉电，原始数据仍在交换区） */
    if (!pal_store_erase(STORE_TYPE_SEL, sector_addr, FLASH_SECTOR_SIZE))
    {
        /* 目标扇区擦除失败，交换区仍保留完整备份，系统可恢复 */
        return false;
    }
    /* 步骤4：从交换区回读数据，将新内容合并到对应位置，重新写入目标扇区 */
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += sizeof(swap_buf))
    {
        if (!pal_store_read(STORE_TYPE_SEL, FLASH_SWAP_BASE_ADDR + i, (uint8_t *)swap_buf, sizeof(swap_buf)))
        {
            return false;
        }

        begin_addr = sector_addr + i;
        end_addr = begin_addr + sizeof(swap_buf);

        /* 判断当前 16 字节块与待写入区间 [addr, prog_end_addr) 是否有交集 */
        if (end_addr <= addr || begin_addr >= prog_end_addr)    //无交集：当前块完全在写入区间之外，无需修改
        {
        }
        else
        {
            /* 有交集：根据四种重叠情况，将新数据合并到 swap_buf 的对应位置 */
            if (begin_addr <= addr && end_addr > addr && end_addr < prog_end_addr) //左交叉
            {
                memcpy(&ptr[addr - begin_addr], value, end_addr - addr);
            }
            else if (begin_addr < prog_end_addr && begin_addr >= addr && end_addr > prog_end_addr) //右交叉
            {
                memcpy(ptr, &value[begin_addr - addr], prog_end_addr - begin_addr);
            }
            else if (begin_addr >= addr && end_addr <= prog_end_addr) //prog buf包含 swap buf
            {
                memcpy(ptr, &value[begin_addr - addr], end_addr - begin_addr);
            }
            else    //swap buf包含prog buf
            {
                memcpy(&ptr[addr - begin_addr], value, prog_end_addr - addr);
            }
        }

        /* 将合并后的 16 字节数据写回目标扇区当前块地址 */
        if (!pal_store_write(STORE_TYPE_SEL, begin_addr, (uint8_t *)swap_buf, sizeof(swap_buf)))
        {
            /* 写入失败返回 false，交换区仍保留完整扇区备份 */
            return false;
        }
    }

    return true;
}
