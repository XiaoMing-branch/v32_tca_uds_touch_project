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
#include "store_manager.h"

/**
 * @brief  写入数据到Flash存储（带CRC16校验保护）
 * @param  addr   - 目标地址
 * @param  data   - 待写入数据指针
 * @param  length - 数据长度（字节）
 * @retval None
 */
void pal_store_data_set(uint32_t addr, uint8_t *data, uint16_t length)
{
//    uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
//    uint32_t offset = addr % STORE_SECTOR_SIZE;
//    uint32_t crc __attribute__((unused));

//    ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);

//    if (memcmp((uint8_t *)&nvrdata[offset], data, length))
//    {
//        memcpy((uint8_t *)&nvrdata[offset], data, length);
//        crc = crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length);
//        memcpy(&nvrdata[offset + length], &crc, sizeof(uint32_t));
//        ll_flash_smart_write(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
//    }

    uint32_t crc = crc16_calculate_func(0xFFFF, data, length);

    store_slow_write(addr, data, length);
    store_slow_write(addr + length, (uint8_t *)&crc, sizeof(crc));
}

/**
 * @brief  从Flash读取数据（带CRC16校验验证）
 * @param  addr   - 源地址
 * @param  data   - 读取数据缓冲区指针
 * @param  length - 数据长度（字节）
 * @retval true  - 读取成功（CRC校验通过）
 * @retval false - 读取失败（CRC校验失败）
 */
bool pal_store_data_get(uint32_t addr, uint8_t *data, uint16_t length)
{
//    uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
//    uint32_t offset = addr % STORE_SECTOR_SIZE;
//    uint32_t crc;

//    ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);

//    memcpy((uint8_t *)&crc, (uint8_t *)&nvrdata[offset + length], sizeof(uint32_t));

//    if (crc == crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length))
//    {
//        memcpy((uint8_t *)data, (uint8_t *)&nvrdata[offset], length);
//        return true;
//    }

//    return false;

    uint32_t crc;
    store_slow_smart_read(addr + length, (uint8_t *)&crc, sizeof(crc));

    uint8_t b;
    uint16_t initcrc = 0x0;
    for (uint16_t i = 0; i < length; i += sizeof(b))
    {
        store_slow_smart_read(addr + i, &b, sizeof(b));
        initcrc = crc16_calculate_func(~initcrc, &b, sizeof(b));
    }

    if (crc == initcrc)
    {
        store_slow_smart_read(addr, data, length);
        return true;
    }

    return false;
}

/**
 * @brief  Flash存储数据初始化（若CRC校验失败则写入默认值）
 * @param  addr   - 目标地址
 * @param  data   - 默认数据指针（CRC失败时写入）
 * @param  length - 数据长度（字节）
 * @retval true  - 初始化成功（已有有效数据）
 * @retval false - 初始化为默认值（原数据无效）
 */
bool pal_store_data_init(uint32_t addr, uint8_t *data, uint16_t length)
{
//    uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
//    uint32_t offset = addr % STORE_SECTOR_SIZE;
//    uint32_t crc;

//    ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);

//    memcpy((uint8_t *)&crc, (uint8_t *)&nvrdata[offset + length], sizeof(uint32_t));

//    if (crc != crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length))
//    {
//        memcpy((uint8_t *)&nvrdata[offset], (uint8_t *)data, length);
//        crc = crc16_calculate_func(0xFFFF, (uint8_t *)&nvrdata[offset], length);
//        memcpy(&nvrdata[offset + length], (uint8_t *)&crc, sizeof(uint32_t));
//        ll_flash_smart_write(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
//        return false;
//    }

//    memcpy(data, &nvrdata[offset], length);
//    return true;

    uint32_t crc;
    store_slow_smart_read(addr + length, (uint8_t *)&crc, sizeof(crc));

    uint8_t b;
    uint16_t initcrc = 0x0;
    for (uint16_t i = 0; i < length; i += sizeof(b))
    {
        store_slow_smart_read(addr + i, &b, sizeof(b));
        initcrc = crc16_calculate_func(~initcrc, &b, sizeof(b));
    }

    if (initcrc != crc)
    {
        crc = crc16_calculate_func(0xFFFF, data, length);
        store_slow_write(addr, data, length);
        store_slow_write(addr + length, (uint8_t *)&crc, sizeof(crc));
        return false;
    }

    store_slow_smart_read(addr, data, length);
    return true;
}

/**
 * @brief  擦除Flash存储区域（仅支持整扇区擦除）
 * @param  addr   - 起始地址（需扇区对齐）
 * @param  length - 擦除长度（需扇区对齐）
 * @retval true  - 擦除成功
 * @retval false - 地址或长度未扇区对齐
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
//        uint8_t nvrdata[STORE_SECTOR_SIZE] __attribute((aligned(4)));
//        ll_flash_read(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
//        memset((uint8_t *)&nvrdata[offset], 0xFF, length);
//        ll_flash_smart_write(STORE_TYPE_SEL, addr - offset, (uint8_t *)nvrdata, STORE_SECTOR_SIZE);
        return false;
    }

    return true;
}

/**
 * @brief  擦除Flash指定区域（底层接口）
 * @param  type   - Flash类型
 * @param  addr   - 起始地址
 * @param  length - 擦除长度
 * @retval true  - 擦除成功
 * @retval false - 擦除失败
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
 * @brief  写入数据到Flash指定区域（底层接口）
 * @param  type   - Flash类型
 * @param  addr   - 目标地址
 * @param  value  - 待写入数据指针
 * @param  length - 数据长度
 * @retval true  - 写入成功
 * @retval false - 写入失败
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
 * @brief  从Flash指定区域读取数据（底层接口）
 * @param  type   - Flash类型
 * @param  addr   - 源地址
 * @param  value  - 读取缓冲区指针
 * @param  length - 数据长度
 * @retval true  - 读取成功
 * @retval false - 读取失败
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
 * @brief  获取芯片唯一标识符（96位UID）
 * @param  uid - 输出UID缓冲区（3个uint32_t）
 * @retval None
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
 * @param  boot_ver - 输出Boot版本号
 * @retval None
 */
void pal_store_boot_ver_get(uint32_t *boot_ver)
{
    uint32_t ver;
    ll_flash_read(FLASH_TYPE_NVM, BOOT_VERSION_ADDR, (uint8_t *)&ver, sizeof(uint32_t));
    ver = (ver & 0xFF) * 10000 + ((ver & 0xFF00) >> 8)  * 100 + ((ver & 0xFF0000) >> 16) ;
    memcpy((uint8_t *)boot_ver, (uint8_t *)&ver, sizeof(ver));
}

/**
 * @brief  获取芯片版本号和ID
 * @param  chip_ver - 输出芯片版本
 * @param  chip_id  - 输出芯片ID
 * @retval None
 */
void pal_store_chip_ver_id_get(uint8_t *chip_ver, uint16_t *chip_id)
{
    ll_syscfg_info_get(chip_ver, chip_id);
}

/**
 * @brief  Flash寄存器读写操作
 * @param  is_write - true:写入 false:读取
 * @param  addr     - 寄存器地址
 * @param  value    - 写入/读取的数据指针
 * @retval None
 */
void pal_store_reg_rw(bool is_write, uint32_t addr, uint32_t *value)
{
    ll_flash_reg_wr(is_write, addr, value);
}
