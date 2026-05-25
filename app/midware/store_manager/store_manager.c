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

const uint32_t sys_param_addr_map[] =
{
    SYSTEM_CFG_OFFSET,
    SYSTEM_ID_CFG_OFFSET,
};

static uint8_t fast_nad_read(void)
{
    uint8_t nad = 0xFF;
    uint8_t rdbuf[8];

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

/********************************************************
** \brief   store_system_data_init
**
** \param   None
**
** \retval  None
*********************************************************/
__WEAK void store_system_data_init(void)
{
    uint32_t addr = 0;

    uint8_t nad = fast_nad_read();
    if (nad != 0xFF)
    {
        g_sys_cfgs.nad = nad;
        pal_store_erase(STORE_TYPE_SEL, FAST_LIN_NAD_ADDR, FLASH_SECTOR_SIZE);
        store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
    }

    /*system cfg param init*/
    addr = SYSTEM_PARAM_BASE_ADDR + SYSTEM_CFG_OFFSET;
    pal_store_data_init(addr, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);

    extern uint8_t lin_configuration_RAM[];
    addr = SYSTEM_PARAM_BASE_ADDR + SYSTEM_ID_CFG_OFFSET;
    pal_store_data_init(addr, lin_configuration_RAM, SYSTEM_ID_CFG_SIZE);
}

/********************************************************
** \brief   store_manager_init
**
** \param   None
**
** \retval  None
*********************************************************/
void store_manager_init(void)
{
    store_system_data_init();
}

/********************************************************
** \brief   store_manager_clear
**
** \param   None
**
** \retval  None
*********************************************************/
void store_manager_clear(void)
{
    pal_store_data_clear(SYSTEM_PARAM_BASE_ADDR, STORE_SECTOR_SIZE);
}

/********************************************************
** \brief   store_system_data_set
**
** \param   system_param_type_e type
** \param   uint8_t*            param
** \param   uint16_t            len
**
** \retval  bool
*********************************************************/
void store_system_data_set(system_param_type_e type, uint8_t *param, uint16_t len)
{
    uint32_t addr = SYSTEM_PARAM_BASE_ADDR + sys_param_addr_map[type];
    pal_store_data_set(addr, param, len);
}

/********************************************************
** \brief   store_system_data_get
**
** \param   system_param_type_e type
** \param   uint8_t*            param
** \param   uint16_t            len
**
** \retval  None
*********************************************************/
bool store_system_data_get(system_param_type_e type, uint8_t *param, uint16_t len)
{
    uint32_t addr = SYSTEM_PARAM_BASE_ADDR + sys_param_addr_map[type];
    return (pal_store_data_get(addr, param, len));
}


/********************************************************
** \brief   store_chip_info_get
**
** \param   chip_info_type_e    type
** \param   uint8_t*            param
** \param   uint16_t            len
**
** \retval  bool
*********************************************************/
bool store_chip_info_get(chip_info_type_e type, uint8_t *param, uint16_t len)
{
    chip_ver_id_t *chip_ver_id = (chip_ver_id_t *)param;

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

/********************************************************
** \brief   store_customer_data_set
**
** \param   uint32_t            addr_offset
** \param   uint8_t*            param
** \param   uint16_t            len
**
** \retval  bool
*********************************************************/
bool store_customer_data_set(uint32_t addr_offset, uint8_t *param, uint16_t len)
{
    if (addr_offset >= FLASH_SECTOR_SIZE || (addr_offset + len) > FLASH_SECTOR_SIZE)
    {
        return false;
    }

    uint32_t addr = CUSTOMER_PARAM_BASE_ADDR + addr_offset;

    pal_store_data_set(addr, param, len);
    return true;
}

/********************************************************
** \brief   store_customer_data_get
**
** \param   uint32_t            addr_offset
** \param   uint8_t*            param
** \param   uint16_t            len
**
** \retval  bool
*********************************************************/
bool store_customer_data_get(uint32_t addr_offset, uint8_t *param, uint16_t len)
{
    if (addr_offset >= FLASH_SECTOR_SIZE || (addr_offset + len) > FLASH_SECTOR_SIZE)
    {
        return false;
    }

    uint32_t addr = CUSTOMER_PARAM_BASE_ADDR + addr_offset;
    return (pal_store_data_get(addr, param, len));
}

bool store_slow_read(uint32_t addr, uint8_t *value, uint16_t length)
{
    return pal_store_read(STORE_TYPE_SEL, addr, value, length);
}

bool store_slow_smart_read(uint32_t addr, uint8_t *value, uint16_t length)
{
    uint32_t rdbuf[2];
    uint8_t *ptr = (uint8_t *)rdbuf;
    uint32_t offset = addr % sizeof(rdbuf);
    uint32_t align_addr;
    uint16_t left_length;
    uint16_t value_begin;

    if (offset) //读开头部分
    {
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
        align_addr = addr;
        left_length = length;
        value_begin = 0;
    }

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

bool store_slow_write(uint32_t addr, uint8_t *value, uint16_t length)
{
    uint32_t sector_addr = ((addr / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE);
    uint32_t swap_buf[4];
    uint8_t *ptr = (uint8_t *)swap_buf;
    uint32_t begin_addr;
    uint32_t end_addr;
    uint32_t prog_end_addr = addr + length;

    if (!pal_store_erase(STORE_TYPE_SEL, FLASH_SWAP_BASE_ADDR, FLASH_SECTOR_SIZE))
    {
        return false;
    }
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

    if (!pal_store_erase(STORE_TYPE_SEL, sector_addr, FLASH_SECTOR_SIZE))
    {
        return false;
    }
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += sizeof(swap_buf))
    {
        if (!pal_store_read(STORE_TYPE_SEL, FLASH_SWAP_BASE_ADDR + i, (uint8_t *)swap_buf, sizeof(swap_buf)))
        {
            return false;
        }

        begin_addr = sector_addr + i;
        end_addr = begin_addr + sizeof(swap_buf);

        if (end_addr <= addr || begin_addr >= prog_end_addr)    //无交集
        {
        }
        else
        {
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

        if (!pal_store_write(STORE_TYPE_SEL, begin_addr, (uint8_t *)swap_buf, sizeof(swap_buf)))
        {
            return false;
        }
    }

    return true;
}
