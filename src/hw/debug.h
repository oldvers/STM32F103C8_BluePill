#ifndef __H_DEBUG_H__
#define __H_DEBUG_H__

#include <stdio.h>

#define DBG_ENABLE

#ifdef DBG_ENABLE
#  define DBG(...)    printf(__VA_ARGS__)
#else
#  define DBG(...)
#endif

void DBG_Init(void);
void DBG_SetDefaultColors(void);
void DBG_ClearScreen(void);
void DBG_SetTextColorRed(void);
void DBG_SetTextColorGreen(void);
void DBG_SetTextColorYellow(void);
void DBG_SetTextColorBlue(void);

#endif //__H_DEBUG_H__
