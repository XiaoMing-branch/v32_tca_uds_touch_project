#include "fff_aes_cmac.h"

DEFINE_FAKE_VOID_FUNC(aes_cmac, unsigned char *, unsigned char *, s32, unsigned char *);
DEFINE_FAKE_VOID_FUNC(Gen_CMACkey, unsigned char *);
DEFINE_FAKE_VOID_FUNC(sha256,  unsigned char *, uint32_t *, uint8_t*);