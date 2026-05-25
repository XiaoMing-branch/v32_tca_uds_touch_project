#include "fff_tcpl03x_ll_flash.h"

DEFINE_FAKE_VOID_FUNC(ll_flash_init);
DEFINE_FAKE_VALUE_FUNC(int,ll_flash_erase,flash_type_e,uint32_t,uint32_t);
DEFINE_FAKE_VALUE_FUNC(int,ll_flash_erase_drv,flash_type_e,uint32_t,uint32_t);
DEFINE_FAKE_VALUE_FUNC(int,ll_flash_read,flash_type_e,uint32_t,uint8_t *,uint32_t);
DEFINE_FAKE_VALUE_FUNC(int,ll_flash_write,flash_type_e,uint32_t,uint8_t *,uint32_t);
DEFINE_FAKE_VALUE_FUNC(int,ll_flash_write_drv,flash_type_e,uint32_t,uint8_t *,uint32_t);
DEFINE_FAKE_VALUE_FUNC(int,ll_flash_smart_write,flash_type_e,uint32_t,uint8_t *,uint32_t);
DEFINE_FAKE_VALUE_FUNC(int,ll_flash_reg_wr,bool,uint32_t,uint32_t *);