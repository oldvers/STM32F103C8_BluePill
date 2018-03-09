#ifndef __H_DUART_H__
#define __H_DUART_H__

#include <stdio.h>
#include "types.h"
#include "stm32f1xx.h"

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#  define LOG(...)    printf(__VA_ARGS__)
#else
#  define LOG(...)
#endif

#endif //__H_DUART_H__
