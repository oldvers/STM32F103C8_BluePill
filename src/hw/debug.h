#ifndef __H_DUART_H__
#define __H_DUART_H__

#include <stdio.h>

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#  define LOG(...)      printf(__VA_ARGS__)
#else
#  define LOG(...)
#endif

void Debug_Init(void);

#endif //__H_DUART_H__
