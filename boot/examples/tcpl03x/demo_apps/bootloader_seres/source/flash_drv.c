/**
 *****************************************************************************
 * @brief   flash driver source file.
 *
 * @file    flash_drv.c
 * @author  AE/FAE team
 * @date    2025.06.05
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2025 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */

#include "stdlib.h"
#include "stdint.h"
#include "tcpl03x_ll_flash.h"

/* #1 this offset should align with scatter file */
#define FLASH_DRV_OFFSET    (0x00000008)                        /**< Flash驱动偏移地址，需与分散加载文件对齐 */
#define CAL_OFFSET(funcPtr) ((uint32_t)(funcPtr) - FLASH_DRV_OFFSET) /**< 计算函数在Flash驱动段中的相对偏移地址 */


typedef uint8_t (*tpfFLASH_DRV_EraseSector) (void); /**< 扇区擦除操作函数指针类型，返回0表示成功 */
typedef uint8_t (*tpfFLASH_DRV_Program)     (void); /**< Flash编程操作函数指针类型，返回0表示成功 */

/**
 * @brief  Flash驱动程序API信息结构体
 *
 * 包含擦除和编程函数的函数指针，
 * 用于通过Flash驱动段进行间接跳转调用，
 * 确保在擦除/编程操作期间代码在RAM中执行。
 */
typedef struct
{
    tpfFLASH_DRV_EraseSector    pfFLASH_DRV_EraseSector; /**< 扇区擦除函数指针，指向RAM中的擦除代码 */
    tpfFLASH_DRV_Program        pfFLASH_DRV_Program;     /**< Flash编程写入函数指针，指向RAM中的编程代码 */
} tFlashDriverAPIInfo;




/**
 * @brief   Flash扇区擦除寄存器操作函数
 *
 * 触发EFLASH控制器的扇区擦除操作（SECTOR_ERASE_TRIG置位），
 * 并通过轮询ERASE_BUSY_STATUS状态寄存器等待擦除完成。
 *
 * @note    本函数位于 .Flash_Driver_Section 段（RAM中执行），
 *          以确保擦除操作期间代码不会从Flash读取（避免取指冲突）。
 *
 * @retval  无返回值
 */
__attribute__((section (".Flash_Driver_Section"))) void ll_flash_erase_reg()
{

    /* 置位扇区擦除触发位，启动EFLASH擦除操作 */
    EFLASH->ERASE_TRIG_F.SECTOR_ERASE_TRIG = 1;

    /* 轮询等待擦除忙状态清除，确保擦除操作完成 */
    while (EFLASH->STATUS_F.ERASE_BUSY_STATUS == 1)
    {
        ;
    }

}
/********************************************************
** @brief   ll_flash_write_reg — Flash编程写入寄存器操作函数
**
** @param[in]   addr    编程目标地址（需64位对齐）
** @param[in]   ptr     指向源数据的指针（64位数据）
**
** @retval  无返回值
** @note    本函数位于 .Flash_Driver_Section 段（RAM中执行），
**          确保编程期间代码不从Flash读取，避免总线冲突。
**          写入以64位（双字）为单位进行。
*********************************************************/
__attribute__((section (".Flash_Driver_Section"))) void ll_flash_write_reg(uint32_t addr, uint8_t *ptr)
{

    /* 将64位源数据写入目标Flash地址，触发编程操作 */
    *((volatile uint64_t *)addr) = *((volatile uint64_t *)ptr);

    /* 轮询等待编程忙状态清除，确保写入操作完成 */
    while (EFLASH->STATUS_F.PROG_BUSY_STATUS == 1)
    {
        ;
    }

}

/**
 * @brief   Flash驱动API信息表，存放于 .Flash_Driver_Section_Offset 段
 *
 * 使用 CAL_OFFSET 宏计算擦除和编程函数在Flash驱动段中的偏移地址，
 * 引导加载程序通过此表间接调用Flash操作函数。
 * 该表位于固定偏移地址（FLASH_DRV_OFFSET），
 * 使得引导加载程序不依赖具体链接地址即可定位Flash操作入口。
 *
 * @note    __attribute__((used)) 防止链接器优化移除该表。
 */
__attribute__((used)) __attribute__((section (".Flash_Driver_Section_Offset"))) static const tFlashDriverAPIInfo g_FlashDriverAPI = {
    (tpfFLASH_DRV_EraseSector) CAL_OFFSET(ll_flash_erase_reg), /**< 擦除函数在驱动段中的偏移地址 */
    (tpfFLASH_DRV_Program) CAL_OFFSET(ll_flash_write_reg),     /**< 编程函数在驱动段中的偏移地址 */
};



