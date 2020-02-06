#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef NULL
#   undef NULL
#endif
#define NULL                 ((void *)0)

#include "stdint.h"

typedef uint8_t              U8;
typedef int8_t               S8;
typedef uint16_t             U16;
typedef int16_t              S16;
typedef uint32_t             U32;
typedef int32_t              S32;
typedef uint64_t             U64;
typedef int64_t              S64;

typedef enum
{
    FW_FALSE     = 0,
    FW_TRUE      = 1,
} FW_BOOLEAN;

typedef enum
{
    FW_OK        = 0x0000,
    FW_SUCCESS   = 0x0000,
    FW_COMPLETE  = 0x0000,
    FW_ERROR     = 0xBAD0,
    FW_FAIL      = 0xFAC0,
} FW_RESULT;

#ifndef SIMULATOR
#  define STATIC             static
#else
#  define STATIC
#endif

#endif /* __TYPES_H__ */
