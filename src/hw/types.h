#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef NULL
#   undef NULL
#endif
#define NULL             ((void *)0)

typedef unsigned char        U8;
typedef char                 S8;
typedef unsigned char const  U8C;
typedef const unsigned char  CU8;
typedef unsigned short       U16;
typedef short                S16;
typedef unsigned short const U16C;
typedef const unsigned short CU16;
typedef int                  S32;
typedef unsigned int         U32;
typedef unsigned int const   U32C;
typedef const unsigned int   CU32;
typedef unsigned char        BOOLEAN;

#ifdef SUCCESS
#   undef SUCCESS
#endif
typedef enum RESULT_E
{
    FALSE = 0,
    TRUE = 1,
    OK = 0,
} RESULT;

#ifndef SIMULATOR
#  define STATIC             static
#else
#  define STATIC
#endif

#endif /* __TYPES_H__ */
