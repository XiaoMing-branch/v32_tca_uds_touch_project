#ifndef __FFF_TCPL03X_LL_FLASH_H__
#define __FFF_TCPL03X_LL_FLASH_H__


#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x.h"
#else
    #include "tcpl03x.h"
#endif

#include "fff.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define  FLASH_BYTE_ALIGN                   (4)

/** @defgroup FLASH_LOCK_Definitions
  * @{
  */
#define FLASH_LOCK_CONFIG()             (void *)0
#define FLASH_UNLOCK_CONFIG()           (void *)0

#define FLASH_LOCK_NVR_ACCESS()         (void *)0
#define FLASH_UNLOCK_NVR_ACCESS()       (void *)0

/** @defgroup FLASH_NVM_Definitions
  * @{
  */
#define  FLASH_SECTOR_SIZE              (512)

/* NVM: 64k */
#define  NVM_FLASH_BASE_ADDR            (0x00000000UL)
#define  NVM_FLASH_SIZE                 (0x00010000UL)
#define  NVM_FLASH_END                  (void *)0
#define  NVM_FLASH_SECTOR_SIZE          (void *)0

typedef enum
{
    FLASH_TYPE_NVM,
    FLASH_TYPE_MAX,
} flash_type_e;


DECLARE_FAKE_VOID_FUNC(ll_flash_init);
DECLARE_FAKE_VALUE_FUNC(int,ll_flash_erase,flash_type_e,uint32_t,uint32_t);
DECLARE_FAKE_VALUE_FUNC(int,ll_flash_erase_drv,flash_type_e,uint32_t,uint32_t);
DECLARE_FAKE_VALUE_FUNC(int,ll_flash_read,flash_type_e,uint32_t,uint8_t *,uint32_t);
DECLARE_FAKE_VALUE_FUNC(int,ll_flash_write,flash_type_e,uint32_t,uint8_t *,uint32_t);
DECLARE_FAKE_VALUE_FUNC(int,ll_flash_write_drv,flash_type_e,uint32_t,uint8_t *,uint32_t);
DECLARE_FAKE_VALUE_FUNC(int,ll_flash_smart_write,flash_type_e,uint32_t,uint8_t *,uint32_t);
DECLARE_FAKE_VALUE_FUNC(int,ll_flash_reg_wr,bool,uint32_t,uint32_t *);


#ifdef __cplusplus
}
#endif
#endif /* __TCPL03X_LL_FLASH_H__ */
