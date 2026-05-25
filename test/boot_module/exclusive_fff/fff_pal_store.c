#include "fff_pal_store.h"

DEFINE_FAKE_VOID_FUNC(pal_store_data_set,uint32_t,uint8_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool,pal_store_data_get,uint32_t,uint8_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool,pal_store_data_init,uint32_t,uint8_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool,pal_store_data_clear,uint32_t,uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool,pal_store_erase,flash_type_e,uint32_t,uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool,pal_store_write,flash_type_e,uint32_t,uint8_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool,pal_store_read,flash_type_e,uint32_t,uint8_t *,uint16_t);
DEFINE_FAKE_VOID_FUNC(pal_store_uid_get,uint32_t *);
DEFINE_FAKE_VOID_FUNC(pal_store_boot_ver_get,uint32_t *);
DEFINE_FAKE_VOID_FUNC(pal_store_chip_ver_id_get,uint8_t *,uint16_t *);
DEFINE_FAKE_VOID_FUNC(pal_store_reg_rw,bool,uint32_t,uint32_t *);
