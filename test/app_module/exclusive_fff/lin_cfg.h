#ifndef __LIN_CFG_H__
#define __LIN_CFG_H__

#include <stdint.h>
#include <stdbool.h>

typedef unsigned char l_bool;
typedef unsigned char l_u8;
typedef unsigned short l_u16;
typedef unsigned long l_u32;
typedef signed long l_s32;

#define MAX_QUEUE_SIZE      8
#define LIN_SIZE_OF_CFG     9

/* LIN protocol version (app-side values) */
#define PROTOCOL_21          0
#define PROTOCOL_J2602       1
#define PROTOCOL_20          2
#define LIN_PROTOCOL         PROTOCOL_21

#define _DIAG_NUMBER_OF_SERVICES_  27
#define _TL_FRAME_SUPPORT_         1  /* _TL_MULTI_FRAME_ */
#define _TL_MULTI_FRAME_           1
#define _TL_SINGLE_FRAME_          0

typedef enum {
    LI0 = 0,
    LI1,
    LI2,
    LI3,
    LI4,
    LI5,
    LI6,
    LI7,
    LI8,
    LI9,
    LI10,
    LI11,
    LI12,
    LI13,
    LI14,
    LI15,
} l_ifc_handle;

#endif /* __LIN_CFG_H__ */
