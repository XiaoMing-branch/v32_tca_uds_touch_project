/**
 *****************************************************************************
 * @brief   pal store source file.
 *
 * @file    pal_store.c
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

#include "pal_store.h"
#include "utilities.h"

/**
 * @brief  存储数据到Flash(带CRC校验保护)
 * @param  addr - Flash起始地址
 * @param  data - 待写入数据缓冲区
 * @param  length - 数据长度
 * @note   读取扇区内容，比较数据差异后写入，附加CRC16校验
 * @retval 无
 */
void pal_store_data_set(uint32_t addr, uint8_t *data, uint16_t length)
{
    uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
    uint32_t offset = addr % STORE_SECTOR_SIZE;
    uint32_t crc __attribute__((unused));

    ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);

    if (memcmp((uint8_t *)&nvrdata[offset], data, length))
    {
        memcpy((uint8_t *)&nvrdata[offset], data, length);
        crc = crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length);
        memcpy(&nvrdata[offset + length], &crc, sizeof(uint32_t));
        ll_flash_smart_write(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
    }
}

/**
 * @brief  从Flash读取数据(带CRC校验验证)
 * @param  addr - Flash起始地址
 * @param  data - 输出数据缓冲区
 * @param  length - 数据长度
 * @note   读取扇区数据，通过CRC16校验确保数据完整性
 * @retval true - 读取成功(CRC校验通过), false - 校验失败
 */
bool pal_store_data_get(uint32_t addr, uint8_t *data, uint16_t length)
{
    uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
    uint32_t offset = addr % STORE_SECTOR_SIZE;
    uint32_t crc;

    ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);

    memcpy((uint8_t *)&crc, (uint8_t *)&nvrdata[offset + length], sizeof(uint32_t));

    if (crc == crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length))
    {
        memcpy((uint8_t *)data, (uint8_t *)&nvrdata[offset], length);
        return true;
    }

    return false;
}

/**
 * @brief  初始化存储数据(CRC校验失败时写入默认值)
 * @param  addr - Flash起始地址
 * @param  data - 默认数据(校验失败时写入)
 * @param  length - 数据长度
 * @note   若CRC校验失败，说明数据未初始化或损坏，写入默认值
 * @retval true - 数据有效(CRC校验通过), false - 已重新初始化
 */
bool pal_store_data_init(uint32_t addr, uint8_t *data, uint16_t length)
{
    uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
    uint32_t offset = addr % STORE_SECTOR_SIZE;
    uint32_t crc;

    ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);

    memcpy((uint8_t *)&crc, (uint8_t *)&nvrdata[offset + length], sizeof(uint32_t));

    if (crc != crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length))
    {
        memcpy((uint8_t *)&nvrdata[offset], (uint8_t *)data, length);
        crc = crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length);
        memcpy(&nvrdata[offset + length], (uint8_t *)&crc, sizeof(uint32_t));
        ll_flash_smart_write(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
        return false;
    }

    memcpy(data, &nvrdata[offset], length);

    return true;
}

/**
 * @brief  清除存储数据(擦除或置0xFF)
 * @param  addr - Flash起始地址
 * @param  length - 数据长度
 * @note   地址对齐扇区时直接擦除，否则按扇区读-改-写
 * @retval true - 清除成功
 */
bool pal_store_data_clear(uint32_t addr, uint16_t length)
{
    uint32_t offset = addr % STORE_SECTOR_SIZE;

    if (!offset && !(length % STORE_SECTOR_SIZE))
    {
        ll_flash_erase(STORE_TYPE_SEL, addr, length);
    }
    else
    {
        uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
        ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
        memset((uint8_t *)&nvrdata[offset], 0xFF, length);
        ll_flash_smart_write(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
    }

    return true;
}

/**
 * @brief  擦除Flash扇区
 * @param  type - Flash类型(NVR/NVM)
 * @param  addr - 起始地址
 * @param  length - 擦除长度
 * @retval true - 擦除成功, false - 擦除失败
 */
bool pal_store_erase(flash_type_e type, uint32_t addr, uint16_t length)
{
    if (0 != ll_flash_erase(type, addr, length))
    {
        return false;
    }

    return true;
}

/**
 * @brief  写Flash数据
 * @param  type - Flash类型(NVR/NVM)
 * @param  addr - 写入起始地址
 * @param  value - 待写入数据
 * @param  length - 数据长度
 * @retval true - 写入成功, false - 写入失败
 */
bool pal_store_write(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length)
{
    if (0 != ll_flash_write(type, addr, value, length))
    {
        return false;
    }

    return true;
}

/**
 * @brief  读Flash数据
 * @param  type - Flash类型(NVR/NVM)
 * @param  addr - 读取起始地址
 * @param  value - 输出数据缓冲区
 * @param  length - 读取长度
 * @retval true - 读取成功, false - 读取失败
 */
bool pal_store_read(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length)
{
    if (0 != ll_flash_read(type, addr, value, length))
    {
        return false;
    }

    return true;
}

/**
 * @brief  获取芯片唯一ID(96位)
 * @param  uid - 输出UID缓冲区(3个uint32_t)
 * @note   TCPL01X:从UID_BASE_ADDR读取，若全0xFF则读备份地址
 * @retval 无
 */
void pal_store_uid_get(uint32_t *uid)
{
#if defined (__TCPL01X__)
    uint32_t buffer[5];

    ll_flash_read(STORE_TYPE_SEL, UID_BASE_ADDR, (uint8_t *)&buffer[0], 3 * sizeof(uint32_t));

    if (buffer[0] == 0xFFFFFFFF && buffer[1] == 0xFFFFFFFF && buffer[2] == 0xFFFFFFFF)
    {
        ll_flash_read(STORE_TYPE_SEL, UID_BASE_ADDR_BAK, (uint8_t *)&buffer[0], 5 * sizeof(uint32_t));

        buffer[2] = ((buffer[2] & 0xFF) << 24) | ((buffer[3] & 0xFF) << 16) | ((buffer[4] & 0xFF)  << 8) | 0xFF;
    }

    memcpy((uint8_t *)uid, (uint8_t *)&buffer[0], 3 * sizeof(uint32_t));
#endif
}

/**
 * @brief  获取Bootloader版本号
 * @param  boot_ver - 输出版本号(编码格式:主版本*10000+次版本*100+修订)
 * @retval 无
 */
void pal_store_boot_ver_get(uint32_t *boot_ver)
{
    uint32_t ver;
    ll_flash_read(FLASH_TYPE_NVM, BOOT_VERSION_ADDR, (uint8_t *)&ver, sizeof(uint32_t));
    ver = (ver & 0xFF) * 10000 + ((ver & 0xFF00) >> 8)  * 100 + ((ver & 0xFF0000) >> 16) ;
    memcpy((uint8_t *)boot_ver, (uint8_t *)&ver, sizeof(ver));
}

/**
 * @brief  获取芯片版本和ID
 * @param  chip_ver - 输出版本号
 * @param  chip_id - 输出芯片ID
 * @retval 无
 */
void pal_store_chip_ver_id_get(uint8_t *chip_ver, uint16_t *chip_id)
{
    ll_syscfg_info_get(chip_ver, chip_id);
}

/**
 * @brief  Flash寄存器读写操作
 * @param  is_write - true:写入, false:读取
 * @param  addr - 寄存器地址
 * @param  value - 写入/读取的值
 * @retval 无
 */
void pal_store_reg_rw(bool is_write, uint32_t addr, uint32_t *value)
{
    ll_flash_reg_wr(is_write, addr, value);
}
